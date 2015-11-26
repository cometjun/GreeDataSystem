/*
 * ClientSession.cpp
 *
 *  Created on: 2015年10月10日
 *      Author: yjunlei
 */

#include "ClientSession.h"
#include "Debug.h"
#include "UTIL.h"
#include <boost/bind.hpp>

namespace server {


ClientSession::ClientSession(boost::asio::io_service& io_service,ClientServer & clientServer):
		m_socket(new boost::asio::ip::tcp::socket(io_service)),m_clientServer(clientServer),m_isExit(false),m_heartbeat(0){
}

ClientSession::~ClientSession() {
}

boost::asio::ip::tcp::socket& ClientSession::socket() {
	return *m_socket;
}

void ClientSession::StartRead() {
	this->m_dataFramePtr = boost::shared_ptr<data_frame>(new data_frame());
	boost::asio::async_read(*m_socket,
			boost::asio::buffer(this->m_dataFramePtr->header(), HLEN),
			boost::bind(
					&ClientSession::handle_read_header,shared_from_this(),
					boost::asio::placeholders::error));
}

void ClientSession::handle_read_header(const boost::system::error_code& error) {

	// DEBUG(UTIL::ByteToHex(this->m_dataFramePtr->header(),HLEN+1));
	if (!error ) {
		if (this->m_dataFramePtr->lead_code() != 0x7E7E){
			DEBUG("报文格式错误");
			this->CloseSession();
			return;
		}
		// 读取报文数据内容
	    boost::asio::async_read(*m_socket,boost::asio::buffer(this->m_dataFramePtr->header()+HLEN, this->m_dataFramePtr->tail_length()),
	    		boost::bind(&ClientSession::handle_read_body, this->shared_from_this(),
				boost::asio::placeholders::error));
	}else {
		this->CloseSession(error);
	}
}

void ClientSession::handle_read_body(const boost::system::error_code& error) {
	DEBUG(UTIL::ByteToHex(this->m_dataFramePtr->header(),this->m_dataFramePtr->length()).c_str());
	    if (!error) {
	    	// 这是处理收到的数据
		// 保存客户端 mac 地址
		if (this->m_srcAddress.empty()) {
			this->m_srcAddress = this->m_dataFramePtr->src_address();
			this->m_clientServer.AddClientConnection(this->m_srcAddress,boost::shared_ptr<server::ClientSession>(this->shared_from_this()));
		}
	    	switch (this->m_dataFramePtr->cmd()) {
	    	case 0x90:
	    		this->Handle90Package();
	    		break;
	    	case 0x91:
	    		this->Handle91Package();
	    		break;
	    	case 0x92:
	    		this->Handle92Package();
	    		break;
	    	case 0x93:
	    		this->Handle93Package();
	    		break;
	    	case 0x96:
	    		this->Handle96Package();
	    		break;
	    	case 0xEE:
	    		this->HandleEEPackage();
	    		break;
	    	case 0xF3:
	    		this->HandleF3Package();
	    		break;
	    	default:
	    		break;
	    	}
	    	// 重新读取报文头
	    	this->StartRead();
	    } else {
	    	this->CloseSession(error);
	    }
}

void ClientSession::StartWrite(boost::shared_ptr<data_frame> data) {
	// DEBUG(UTIL::ByteToHex(data->header(),data->length()) );
	if (data == NULL) {
		return;
	}
	this->m_writeQueueMutex.lock();
		this->m_writeQueue.push_back(data);

	if (this->m_writeQueue.size() == 1) {
			boost::asio::async_write(*this->m_socket,
					boost::asio::buffer(data->header(),data->length()),
					boost::bind(&ClientSession::handle_write,this->shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
	}

	this->m_writeQueueMutex.unlock();
}

void ClientSession::handle_write(const boost::system::error_code& error,
		std::size_t bytes_transfered) {
		if (!error){ // 发送成功

			this->m_writeQueueMutex.lock();
			this->m_writeQueue.pop_front();
			// 如果队列不为空，重新取数据发送
			if (!this->m_writeQueue.empty()) {
				boost::shared_ptr<data_frame> data = this->m_writeQueue.front();
				boost::asio::async_write(*this->m_socket,
							boost::asio::buffer(data->header(),data->length()),
							boost::bind(&ClientSession::handle_write,this->shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

			}

			this->m_writeQueueMutex.unlock();
		} else {
			this->CloseSession(error);
		}
}

void ClientSession::RemoveSession() {
	m_clientServer.RemoveClientConnection(m_srcAddress); 	// 移除客户端
}

void ClientSession::CloseSession() {
	if (!this->m_srcAddress.empty() ) {
		this->m_isExit = true;
		this->m_socket->close();
		this->RemoveSession();
	}
}

void ClientSession::CloseSession(const boost::system::error_code& error) {
	if (!this->m_srcAddress.empty()) {
		DEBUG(error.message().c_str() );
		this->CloseSession();
	}
}

void ClientSession::Handle90Package() {
	this->m_srcAddress = this->m_dataFramePtr->src_address();
	boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(new data_frame());
	response->Construct90Response(*this->m_dataFramePtr);
	this->StartWrite(response);
	this->m_clientServer.AddClientConnection(this->m_srcAddress,boost::shared_ptr<server::ClientSession>(this->shared_from_this()));
}

void ClientSession::Handle92Package() {
	this->m_clientServer.StopRealMonitoring(boost::shared_ptr<server::ClientSession>(this->shared_from_this()),this->m_dataFramePtr);
}

void ClientSession::Handle93Package() {
	this->m_clientServer.StartRealMonitoring(boost::shared_ptr<server::ClientSession>(this->shared_from_this() ),this->m_dataFramePtr);
}

void ClientSession::Handle96Package() {
	return;
}

void ClientSession::HandleEEPackage() {
	return;
}

void ClientSession::keepalive() {
	int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)
		int keepIdle = 60;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
		int keepInterval = 60;   // 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
		int keepCount = 3;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
		setsockopt(this->socket().native(), SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
		setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
		setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
		setsockopt(this->socket().native(), SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
}

void ClientSession::Handle91Package() {
	return;
}

void ClientSession::HandleF3Package() {
	boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(new data_frame());
	response->ConstructF3Response(*this->m_dataFramePtr);
	this->m_heartbeat = 0;
	this->StartWrite(response);
}

} /* namespace server */
