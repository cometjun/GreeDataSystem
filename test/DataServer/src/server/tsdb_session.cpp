/*
 * tsdb_session.cpp
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */
#include "tsdb_session.h"

#include <boost/bind.hpp>
#include <vector>
#include <arpa/inet.h>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "HDFSReadWrite.h"
#include "Debug.h"
#include "UTIL.h"

#include<iostream>

namespace server {
std::size_t tsdb_session::MaxQueueSize = 100;
tsdb_session::tsdb_session(boost::asio::io_service& io_service,
		server::tsdb_server & tsdb_server_) :
		socket_(new tcp::socket(io_service)), m_tsdb_server(tsdb_server_), m_isLogin(
				false), m_isTransmit(false),m_total(0) {
}

tsdb_session::~tsdb_session() {
}

tcp::socket& tsdb_session::socket() {
	return *socket_;
}

void tsdb_session::StartRead() {
	this->read_frame_ = boost::shared_ptr<data_frame>(new data_frame());
	// 读报文头
	boost::asio::async_read(*socket_,
			boost::asio::buffer(read_frame_->header(), HLEN),
			boost::bind(&tsdb_session::handle_read_header, shared_from_this(),
					boost::asio::placeholders::error));
}

void tsdb_session::handle_read_header(const boost::system::error_code& error) {
	// DEBUG(server::ByteToHex(this->read_frame_->header(),HLEN));
	if (!error) {
		// 如果格式错误则关闭连接
		if (read_frame_->lead_code() != 0x7E7E) {
			this->CloseSession();
			return;
		}
		if (this->read_frame_->length() > data_frame::HEADER_LENGTH) {
			//	this->StartRead();
			this->CloseSession(error);
			return;
		}
		// 读取数据部分
		boost::asio::async_read(*socket_,
				boost::asio::buffer(read_frame_->header() + HLEN,
						read_frame_->tail_length()),
				boost::bind(&tsdb_session::handle_read_body, shared_from_this(),
						boost::asio::placeholders::error));
	} else {
		this->CloseSession(error);
	}
}

void tsdb_session::handle_read_body(const boost::system::error_code& error) {
	// 输出收到的数据
	//DEBUG(server::ByteToHex(this->read_frame_->header(),HLEN+5));

	if (!error) {
		// 获取登陆信息
		if (this->read_frame_->cmd() == 0x90) { 	// 旧系统
			this->m_srcAddress = this->read_frame_->src_address();
			this->m_imei = this->read_frame_->imei();
			this->m_sysIden = this->read_frame_->sys_identity();
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct90Response(*this->read_frame_);
			this->StartWrite(response);
			this->m_90Frame = this->read_frame_; // 保存 90 帧
		} else if (this->read_frame_->cmd() == 0x89) { 	// 新系统
			this->m_srcAddress = this->read_frame_->src_address();

			this->m_imei = this->read_frame_->imei();
			this->m_sysIden = this->read_frame_->sys_identity();
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct90Response(*this->read_frame_);
			this->StartWrite(response);

			this->m_90Frame = this->read_frame_; // 保存 90 帧

		} else if (this->read_frame_->cmd() == 0x91) { 	//
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct91Response(*this->read_frame_);
			this->StartWrite(response);
			this->m_91FrameMutex.lock();
			this->m_91Frame = this->read_frame_; // 保存最新 91 帧
			this->m_91FrameMutex.unlock();
			// 将连接加入已连接队列
			if (!this->m_isLogin) {
				if (!this->m_tsdb_server.AddGprsConnection(this->m_sysIden,
						this->m_srcAddress,
						boost::shared_ptr<tsdb_session>(
								this->shared_from_this()))) {
					this->m_readQueue.clear();
					return;
				}
				this->m_isLogin = true;

			}
		} else if (this->read_frame_->cmd() == 0xF3) { 	// 心跳检测,心跳需要保存
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->ConstructF3Response(*this->read_frame_);
			this->StartWrite(response);
		} else if (this->read_frame_->cmd() == 0xF4) {
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->ConstructF4Response(*this->read_frame_,0x00); // 成功
			this->StartWrite(response);
		} else if (this->read_frame_->cmd() == 0x96) {
			if (this->m_90Frame != NULL && this->m_90Frame->cmd() == 0x89 && this->read_frame_->TransMode() == 0x01) { // 去重模式
				boost::shared_ptr<data_frame> response = boost::shared_ptr<
						data_frame>(new data_frame());
				response->Construct96Response(*this->read_frame_,0x00); // 成功
				this->StartWrite(response);
			}
		}
		// 将报文放入 session 队列
		this->PushToQueue();
		// 重新读取报文头
		this->StartRead();
	} else {
		this->CloseSession(error);
	}
}
void tsdb_session::putToServer() {
	if (this->m_globalQueue != NULL) {
		this->m_globalQueue->append(this->m_readQueue);
		this->m_total ++;
	}
}

void tsdb_session::RemoveSession() {
	// 将连接从 server 队列中移除
	DEBUG(this->m_srcAddress.c_str());
	this->m_tsdb_server.RemoveGprsConnection(this->m_srcAddress);
}

void tsdb_session::StartWrite(boost::shared_ptr<data_frame> data) {
	//DEBUG(this->m_writeQueue.size());
	this->m_writeQueueMutex.lock();
	this->m_writeQueue.push_back(data);
	// 如果加入队列时队列为空，则连接没有在写数据
	if (this->m_writeQueue.size() == 1) {
		boost::asio::async_write(*this->socket_,
				boost::asio::buffer(data->header(), data->length()),
				boost::bind(&tsdb_session::HandleWrite,
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	this->m_writeQueueMutex.unlock();
}

void tsdb_session::HandleWrite(const boost::system::error_code& error,
		std::size_t bytes_transfered) {
	//DEBUG(bytes_transfered);
	if (!error) {
		// 如果发送成功，则将数据从队列中删除
		this->m_writeQueueMutex.lock();

		this->m_writeQueue.pop_front();
		// 如果队列不为空，则继续写数据
		if (!this->m_writeQueue.empty()) {
			boost::shared_ptr<data_frame> data = this->m_writeQueue.front();
			boost::asio::async_write(*this->socket_,
					boost::asio::buffer(data->header(), data->length()),
					boost::bind(&tsdb_session::HandleWrite,
							this->shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
		}
		this->m_writeQueueMutex.unlock();
	} else {
		this->CloseSession(error);
	}
}

void tsdb_session::CloseSession() {
	DEBUG(std::string("总共发送的帧数为 : " + server::IntToString(this->m_total) ).c_str() );
	if (this->m_isLogin)
		this->RemoveSession();
}

void tsdb_session::PushToQueue() {
	if (this->m_isTransmit == true) {
		this->m_isTransmit = this->m_tsdb_server.WriteToClientQueue(
				this->read_frame_);
	}
	if (this->read_frame_->cmd() == 0x91) { // 转发所有 91 帧
		this->m_tsdb_server.WriteToClientQueue(this->read_frame_);
	} else if (this->read_frame_->cmd() == 0xEE
			&& this->read_frame_->exception() == 0x07) { // 转发所有 EE07 帧
		this->m_tsdb_server.WriteToClientQueue(this->read_frame_);
	}
	this->m_readQueue.push_back(this->read_frame_);
	// 如果队列长度大于一定值，则将其放到全局队列中
	if (this->m_readQueue.size() > server::tsdb_session::MaxQueueSize)
		this->putToServer();
}

void tsdb_session::CloseSession(const boost::system::error_code& error) {
	if (error != 0) {
		if (this->m_isLogin)
			DEBUG(error.message().c_str());
	}
	if (this->m_isLogin) {
		if (!this->m_readQueue.empty()) {
			this->putToServer();
		}
	}
	this->CloseSession();
}

void tsdb_session::keepalive() {
	int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)
	int keepIdle = 60;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
	int keepInterval = 60;   // 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
	int keepCount = 3;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
	setsockopt(this->socket().native(), SOL_SOCKET, SO_KEEPALIVE,
			(void*) &keepAlive, sizeof(keepAlive));
	setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPIDLE,
			(void*) &keepIdle, sizeof(keepIdle));
	setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPINTVL,
			(void*) &keepInterval, sizeof(keepInterval));
	setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPCNT,
			(void*) &keepCount, sizeof(keepCount));
}

} 		// end of namespace server
