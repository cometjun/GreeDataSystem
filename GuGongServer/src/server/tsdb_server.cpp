/*
 * tsdbs_erver.cpp
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <stdio.h>
#include "tsdb_server.h"
#include "Debug.h"
#include "UTIL.h"
#include "HDFSReadWrite.h"
namespace server {
std::size_t tsdb_server::MaxFileSize = 120000000;
tsdb_server::tsdb_server(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint) :
		io_service_(io_service), acceptor_(io_service, endpoint), breakThread(
				false), m_AcceptSocketfd(-1), m_isClientValid(false),m_messageTimer(io_service) {
	this->m_fileCount.RestoreFileName();
}

tsdb_server::~tsdb_server() {
}

void tsdb_server::start() {
	// 开始监听客户端的连接
//	boost::function0<void> fun = boost::bind(
//			&tsdb_server::HandleClientConnection, this);
//	boost::thread thrd(fun);
//	thrd.detach();
	this->startAccept();
}

void tsdb_server::HandleClientConnection() {
	accept: while (true) { // 接收新的连接
		this->m_isClientValid = false;
		// 重置队列
		this->m_writeToClientQueueMutex.lock();
		this->m_writeToClientQueue.clear();
		this->m_writeToClientQueueMutex.unlock();

		struct sockaddr_un clientAddr = {};
		int clientSock = -1;
		unsigned int len = sizeof(clientAddr);
		DEBUG("start accept");
		// 接收客户端连接
		clientSock = accept(this->m_AcceptSocketfd,
				(struct sockaddr*) &clientAddr, &len);
		if (clientSock == -1) {
			DEBUG(strerror(errno));
			goto accept;
		}
		DEBUG(std::string("客户端已连接" + std::string(clientAddr.sun_path ) ).c_str() );
		// 重置发送到客户端的队列
		this->m_writeToClientQueueMutex.lock();
		this->m_writeToClientQueue.clear();
		this->m_writeToClientQueueMutex.unlock();
		// 将转发服务器置为已连接
		this->m_isClientValid = true;
		// 开启写数据到客户端的线程
		boost::function0<void> fun = boost::bind(&tsdb_server::WriteToClientT,
				this, clientSock);
		boost::thread thrd(fun);
		thrd.detach();

		// 开始接收客户端发来的数据
		while (true) {
			boost::shared_ptr<data_frame> rdata = boost::shared_ptr<data_frame>(
					new data_frame());
			// 读报文头
			int totalRead = 0;
			int bytesRead = 0;
			while (totalRead < HLEN) {
				bytesRead = read(clientSock, rdata->header() + totalRead,
				HLEN - totalRead);
				if (bytesRead == -1) {
					if (errno == EINTR) { // 如果读数据时被中断，则重新读取数据
						continue;
					}
					// 其它错误，断开客户端连接，重新接收新的连接
					DEBUG(strerror(errno));
					close(clientSock);
					goto accept;
				} else if (bytesRead == 0) { // 读到文件尾
					goto accept;
				}
				totalRead += bytesRead;
			} // end while

			//检查报文格式
			if (rdata->lead_code() != 0x7e7e) {
				DEBUG(std::string("received data foramt is wrong : " + server::ByteToHex(rdata->header(),HLEN+1) ).c_str());
				close(clientSock); // 关闭客户端连接，重新等待连接
				goto accept;
			}
			// 读报文数据部分
			int tailLen = rdata->tail_length();
			totalRead = 0;
			while (totalRead < tailLen) {
				bytesRead = read(clientSock, rdata->header() + HLEN + totalRead,
						tailLen - totalRead);
				if (bytesRead == -1) {
					if (errno == EINTR) {
						continue;
					}
					DEBUG(strerror(errno));
					close(clientSock);
					goto accept;
				} else if (bytesRead == 0) { // 文件结束
					goto accept;
				}
				totalRead += bytesRead;
			} // end while
			this->HandleReceivedClientData(rdata);
		}
		//关闭客户端连接
		close(clientSock);
	} // end while
}

void tsdb_server::WriteToClientT(int clientfd) {
	if (clientfd < 0) {
		return;
	}
	// 客户端重新连接时重新发送所有已连接客户端的 91 帧
	this->m_gprsSessionMutex.lock();
	 std::map<std::string,boost::shared_ptr<tsdb_session> >::iterator iter;
	 for (iter = this->m_gprsSession.begin(); iter != this->m_gprsSession.end(); iter ++) {
		 this->WriteToClientQueue(iter->second->Get91Frame()	);
	 }
	this->m_gprsSessionMutex.unlock();

	while (true) { // 写会出现 EPIP ,读会返回 0
		std::list<boost::shared_ptr<data_frame> > list;

		if (this->m_isClientValid == false) {
			DEBUG("客户端连接无效");
			return;
		}
		// 从发送队列中取出数据，发送到客户端
		this->m_writeToClientQueueMutex.lock();
		list.splice(list.end(), this->m_writeToClientQueue);
		this->m_writeToClientQueueMutex.unlock();

		if (list.empty()) {
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			continue;
		}
		// 将数据发送到客户端
		while (!list.empty()) {
			boost::shared_ptr<data_frame> data = list.front();
			int ret = write(clientfd, data->header(), data->length());
			if (ret == -1) {
				if (errno == EPIPE) { // 连接已断开
					DEBUG("客户端连接断开");
					return;
				}
				continue;
			}
			list.pop_front();
		}
	}
}

void tsdb_server::startAccept() {
	tsdb_session_ptr new_session(new tsdb_session(io_service_, *this));	//new tsdb_session
	acceptor_.async_accept(new_session->socket(),
			boost::bind(&tsdb_server::handle_accept, this, new_session,
					boost::asio::placeholders::error));
}

void tsdb_server::handle_accept(tsdb_session_ptr session,
		const boost::system::error_code& error) {
	if (!error) {
		// 开始读客户端数据
		session->keepalive();
		session->StartRead();
	}
	this->startAccept();
}

bool tsdb_server::WriteToClientQueue(boost::shared_ptr<data_frame> data) {
	if (this->m_isClientValid == false) {
		return false;
	}
	this->m_writeToClientQueueMutex.lock();
	this->m_writeToClientQueue.push_back(data);
	this->m_writeToClientQueueMutex.unlock();
	return true;
}

bool tsdb_server::AddGprsConnection(std::string sysIden, std::string address,
		boost::shared_ptr<tsdb_session> tsdb_session_ptr) {
	//DEBUG(tsdb_session_ptr->socket().remote_endpoint().address());
	// 将连接装入队列
	bool flag = true;
	std::size_t len = 0;	// 已连接数

	this->m_gprsSessionMutex.lock();
	if (this->m_gprsSession.size() > CONNECTION_SIZE) {
		flag = false;
	} else {
		this->m_gprsSession[address] = tsdb_session_ptr;
	}
	len = this->m_gprsSession.size();
	this->m_gprsSessionMutex.unlock();
	if (flag == false) {
		return false;
	}
	boost::shared_ptr<server::ListWrapper> gQueue = this->GetGlobalQueue(
			sysIden, address);
	if (gQueue == NULL) {
		return false;
	}
	//
	tsdb_session_ptr->SetGlobalQueue(gQueue);
	DEBUG(std::string("连接数 : " + server::IntToString(len) +" "+ address).c_str()); // 打印连接数与连接到的客户端

	// 查看短信等待队列中是否有相应的 gprs 模块
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
	this->m_messageQueueMutex.lock();
	std::list<server::MessageInfo>::iterator iter;
	// 从等待短信的队列中找到对应的客户端，将其从等待队列中移除
	for (iter = this->m_messageQueue.begin(); iter != this->m_messageQueue.end(); iter ++) {
		if (iter->arrTime < now) {
			if (iter->gprsAddress == address) {
				tsdb_session_ptr->SetTransmit(true);
				this->WriteToClientQueue(tsdb_session_ptr->Get90Frame());
				this->m_messageQueue.erase(iter);
				if (this->m_messageQueue.empty()) {
					this->m_messageTimer.cancel(); // 取消定时器
				}
				break;
			}
			continue;
		}
		break;
	}
	this->m_messageQueueMutex.unlock();
	return flag;
}

bool tsdb_server::RemoveGprsConnection(std::string address) {
	this->m_gprsSessionMutex.lock();
	int ret = this->m_gprsSession.erase(address);
	this->m_gprsSessionMutex.unlock();

	boost::shared_ptr<data_frame> end = boost::shared_ptr<data_frame>(new data_frame());
	end->ConstructEEResponse(address,0x08); // EE08
	this->WriteToClientQueue(end);

//	end = boost::shared_ptr<data_frame>(new data_frame());
//	end->Construct88Frame(address);
//	this->WriteToClientQueue(end);
	return ret == 1;
}

void tsdb_server::WriteToHdfs(std::string sysIden,
		boost::shared_ptr<server::ListWrapper> listWrapper) {
	while (!breakThread) {
		std::list<boost::shared_ptr<data_frame> > list;
		boost::shared_ptr<server::ListWrapper> listWrapper;
		int index;
		// 从全局队列中取数据
		this->m_gprsQueueMapMutex.lock();
		server::VectorInfo & vInfo = this->m_gprsQueueMap[sysIden];
		index = vInfo.index;
		listWrapper = vInfo.vec[index];
		vInfo.index++;
		vInfo.index = vInfo.index % LIST_NUM;
		this->m_gprsQueueMapMutex.unlock();
		if (listWrapper == NULL) {
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			continue;
		}
		listWrapper->AppendOut(list);
		if (list.empty()) {
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			continue;
		}
		this->WriteToLocal(sysIden, list);
	} 	// end while
}

void tsdb_server::StartWriteToHdfs(std::string sysIden,
		boost::shared_ptr<server::ListWrapper> listWrapper) {
	boost::function0<void> fun = boost::bind(&tsdb_server::WriteToHdfs, this,
			sysIden, listWrapper);
	boost::thread thrd(fun);
	thrd.detach();
}
// 返回将当前队列写入 hdfs 的文件描述符
boost::shared_ptr<std::ofstream> tsdb_server::GetWriteFile(
		std::string sysIden) {
	//std::string newFileName = server::HDFSReadWrite::GetHdfsFileName();
	// 如果在同一个时间段，直接返回
	struct HdfsFileInfo& fileInfo = this->m_writeFileMap[sysIden];
	if (fileInfo.writeFile != NULL && fileInfo.bytesWriten < server::tsdb_server::MaxFileSize) {
		return fileInfo.writeFile;
	}
	if (fileInfo.writeFile != NULL) {
		fileInfo.writeFile->close();
	}
	fileInfo.writeFile = this->m_fileCount.GetWriteFile(sysIden,
			fileInfo.fileName);
	fileInfo.bytesWriten = 0;

	return fileInfo.writeFile;
}

void tsdb_server::WriteToLocal(std::string sysIden,
		std::list<boost::shared_ptr<data_frame> >& list) {
	std::string byteBuffer;
	boost::shared_ptr<std::ofstream> writeStream = this->GetWriteFile(sysIden);
	while (!list.empty()) { // 遍历 list

		boost::shared_ptr<data_frame> data = list.front();
		byteBuffer.append(data->header(), data->length());
		list.pop_front();
		if (byteBuffer.size() > MaxBufferSize || data->cmd() == 0x88) {
			// 将数据写入文件
			int bytes = server::HDFSReadWrite::HDFSWrite(writeStream,
					byteBuffer.c_str(), byteBuffer.length());
			if (bytes > 0) {
				this->updateBytesWriten(sysIden, bytes);
			}
			byteBuffer.clear();
		}
	} // end while
	if (!byteBuffer.empty()) {
		int bytes = server::HDFSReadWrite::HDFSWrite(writeStream,
				byteBuffer.c_str(), byteBuffer.length());
		if (bytes > 0) {
			this->updateBytesWriten(sysIden, bytes);
		}
		byteBuffer.clear();
	}
}

void tsdb_server::updateBytesWriten(std::string sysIden, std::size_t bytes) {
	struct HdfsFileInfo & fileInfo = this->m_writeFileMap[sysIden];
	fileInfo.bytesWriten += bytes;
}

boost::shared_ptr<server::ListWrapper> tsdb_server::GetGlobalQueue(
		std::string sysIden, std::string address) {
	bool startThread = false;
	unsigned int ind = server::hash(address) % LIST_NUM;
	this->m_gprsQueueMapMutex.lock();
	if (this->m_gprsQueueMap.find(sysIden) == this->m_gprsQueueMap.end()) {
		startThread = true;
		server::VectorInfo vInfo;
		// 初始化队列信息
		vInfo.index = 0;
		for (int i = 0; i < LIST_NUM; i++) {
		}
		this->m_gprsQueueMap[sysIden] = vInfo;
	}
	// 随机选择一个队列
	if (this->m_gprsQueueMap[sysIden].vec[ind] == NULL) {
		this->m_gprsQueueMap[sysIden].vec[ind] = boost::shared_ptr<
				server::ListWrapper>(new server::ListWrapper());
	}
	boost::shared_ptr<server::ListWrapper> gList =
			this->m_gprsQueueMap[sysIden].vec[ind];
	this->m_gprsQueueMapMutex.unlock();
	// 如果新的系统开始传输数据，则开启一个新线程处理
	if (startThread == true) {
		this->StartWriteToHdfs(sysIden,
				boost::shared_ptr<server::ListWrapper>());
	}
	return gList;
}

void tsdb_server::HandleReceivedClientData(boost::shared_ptr<data_frame> data) {
	if (data->lead_code() != 0x7e7e) {
		DEBUG(std::string("报文格式错误" + server::ByteToHex(data->header(),HLEN+1) ).c_str() );
		return;
	}
	std::string gprsAddress = data->dst_address();
	if (data->cmd() == 0x92) { // 如果要关闭系统
		this->m_gprsSessionMutex.lock();
		if (this->m_gprsSession.find(gprsAddress)
				== this->m_gprsSession.end()) { // 要关闭的 gprs 模块没有正在传输数据
			this->m_gprsSessionMutex.unlock();
			DEBUG(std::string("要关闭的 gprs 没有登陆 : "+ data->dst_address() ).c_str() );
			return;
		}
		// 停止客户端发送数据
		boost::shared_ptr<server::tsdb_session> session =
				this->m_gprsSession[gprsAddress];
		this->m_gprsSessionMutex.unlock();
		session->SetTransmit(false);
		session->StartWrite(data);
		return;
	} else if (data->cmd() == 0x93) {
		this->m_gprsSessionMutex.lock();
		if (this->m_gprsSession.find(gprsAddress)
				== this->m_gprsSession.end()) { // 如果 gprs 模块没有登陆，则发短信通知 gprs
			this->m_gprsSessionMutex.unlock();
			// gprs 没有登陆,发送短信
			if (this->m_messageClient != NULL)
				this->m_messageClient->SendMessage(data);

			this->PutToMessageQueue(gprsAddress);
			return;
		}
		boost::shared_ptr<server::tsdb_session> session =
				this->m_gprsSession[gprsAddress];
		this->m_gprsSessionMutex.unlock();
		// 获取 gprs 当前传输模式
		boost::shared_ptr<data_frame> frame91 = session->Get91Frame();
		uint8_t pat = frame91->pattern();
		// 根据 gprs 模式选择回复方式
		if (pat == 0x03) { // 实时上报
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct93Response(gprsAddress, 0); // 0x9300
			this->WriteToClientQueue(response);
			session->SetTransmit(true);
			this->WriteToClientQueue(session->Get90Frame());
			this->WriteToClientQueue(session->Get91Frame());
			return;
		} else if (pat == 0x00 || pat == 0x05) { // 内外机故障上报
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct93Response(gprsAddress, 1); // 0x9301
			this->WriteToClientQueue(response);
		} else if (pat == 0x01) { // 调试上报
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct93Response(gprsAddress, 2); // 0x9302
			this->WriteToClientQueue(response);
		} else if (pat == 0x02) { // 按键触发上报
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct93Response(gprsAddress, 3); // 0x9303
			this->WriteToClientQueue(response);
		} else if (pat == 0x04) { // 亚健康上报
			boost::shared_ptr<data_frame> response = boost::shared_ptr<
					data_frame>(new data_frame());
			response->Construct93Response(gprsAddress, 4); // 0x9304
			this->WriteToClientQueue(response);
		}
		if (data->flag93() == 0x01) { // 强制监控
			session->SetTransmit(true);
			session->StartWrite(data);
		}
		return;
	} // cmd 93
}

void tsdb_server::PutToMessageQueue(std::string gprsAddress) {
	this->m_messageQueueMutex.lock();
	server::MessageInfo messageInfo = {gprsAddress,boost::posix_time::microsec_clock::universal_time()};
	this->m_messageQueue.push_back(messageInfo);
	if (this->m_messageQueue.size() == 1) {
		this->m_messageTimer.expires_from_now(boost::posix_time::minutes(2)); // 定时 2 minutes
		this->m_messageTimer.async_wait(boost::bind(&tsdb_server::HandleMessageTimeOut,this,boost::asio::placeholders::error ) );
	}
	this->m_messageQueueMutex.unlock();
}

void tsdb_server::HandleMessageTimeOut(const boost::system::error_code& error) {
	if (error.value() == boost::system::errc::operation_canceled) { // 定时被取消
		return;
	}
	if (error) { // 其它错误
		DEBUG(error.message().c_str());
		return;
	}
	this->m_messageQueueMutex.lock();
	// 对于超时等待的客户端，发送 EE09
while (!this->m_messageQueue.empty()) {
	server::MessageInfo & mInfo = this->m_messageQueue.front();
	if (mInfo.arrTime + boost::posix_time::minutes(2) <= this->m_messageTimer.expires_at()) {
		boost::shared_ptr<data_frame> response = boost::shared_ptr<
				data_frame>(new data_frame());
		response->ConstructEEResponse(mInfo.gprsAddress, 0x09);
		this->WriteToClientQueue(response);
		this->m_messageQueue.pop_front();
		continue;
	}
	break;
}
if (!this->m_messageQueue.empty()) {
	server::MessageInfo & mInfo = this->m_messageQueue.front();
	this->m_messageTimer.expires_at(mInfo.arrTime + boost::posix_time::minutes(2) );
	this->m_messageTimer.async_wait(boost::bind(&tsdb_server::HandleMessageTimeOut,this,boost::asio::placeholders::error) );
}
this->m_messageQueueMutex.unlock();

}

} /* namespace server */
