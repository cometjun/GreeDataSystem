/*
 * ClientServer.cpp
 *
 *  Created on: 2015年10月10日
 *      Author: yjunlei
 */

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "ClientServer.h"
#include "Debug.h"
#include "UTIL.h"

namespace server {

ClientServer::ClientServer(boost::asio::io_service& io_service,
		const boost::asio::ip::tcp::endpoint& endpoint) :
		m_io_service(io_service), m_acceptor(io_service, endpoint), m_socketfd(
				-1), m_isConnValid(false) {
}
ClientServer::~ClientServer() {
}

void ClientServer::start() {
	// 处理与服务器的连接与写数据线程
	boost::function0<void> fun = boost::bind(
			&ClientServer::HandleConnectToServer, this);
	boost::thread thrd(fun);
	thrd.detach();
	// 启动心跳线程
	boost::function0<void> heartBeatFun = boost::bind(&ClientServer::HeartBeat,
			this);
	boost::thread hbeatT(heartBeatFun);
//	hbeatT.detach();

	this->StartAccept();
}
void ClientServer::StartAccept() {
	ClientSessionPtr pClientSession(new ClientSession(m_io_service, *this));
	m_acceptor.async_accept(pClientSession->socket(),
			boost::bind(&ClientServer::handle_accept, this, pClientSession,
					boost::asio::placeholders::error));
}

void ClientServer::handle_accept(ClientSessionPtr pClientSessionPtr,
		const boost::system::error_code& error) {

	if (!error) {
		// 由 client session 处理数据
		pClientSessionPtr->keepalive();
		pClientSessionPtr->StartRead();
	} else {
		DEBUG(error.message().c_str());
	}
	this->StartAccept();	// 重新开始侦听
}

bool ClientServer::AddClientConnection(std::string address,
		boost::shared_ptr<server::ClientSession> clientSessionPtr) {
	int size = 0;
	this->m_clientSessionMutex.lock();
	size = this->m_clientSession.size();
	if (size < CLINET_NUMBER) { // 限制连接数

		if (this->m_clientSession.find(address)
				!= this->m_clientSession.end()) {
			this->m_clientSessionMutex.unlock();
			return false;
		}
	this->m_clientSession[address] = clientSessionPtr;
	size = this->m_clientSession.size();
	this->m_clientSessionMutex.unlock();
	DEBUG(UTIL::IntToString(size).c_str() );
	// 发送所有 91 帧到新客户端
	this->m_gprsMapMutex.lock();
	std::map<std::string,boost::shared_ptr<server::GprsClientList> >::iterator iter;
	for (iter = this->m_gprsMap.begin(); iter != this->m_gprsMap.end(); iter ++) {
		if (iter->second->frame91 != NULL) {
			clientSessionPtr->StartWrite(iter->second->frame91);
		}
	}
	this->m_gprsMapMutex.unlock();
	}
	return false;
}

void ClientServer::broadcast(boost::shared_ptr<data_frame> data) {
	std::map<std::string, boost::shared_ptr<server::ClientSession> >::iterator iter;
	this->m_clientSessionMutex.lock();
	DEBUG(UTIL::IntToString(this->m_clientSession.size() ).c_str() );
	for (iter = this->m_clientSession.begin();
			iter != this->m_clientSession.end(); iter++) {
		iter->second->StartWrite(data);
	}
	this->m_clientSessionMutex.unlock();
}

void ClientServer::StartRealMonitoring(
		boost::shared_ptr<server::ClientSession> clientSession,
		boost::shared_ptr<data_frame> monitorFrame) {
	if (monitorFrame->cmd() != 0x93) {
		return;
	}
	// 如果没有正在实时监控,则将数据发送到服务器
	this->m_gprsMapMutex.lock();
	boost::shared_ptr<server::GprsClientList> clientList;
	std::string gprsAddress = monitorFrame->dst_address();
	if (this->m_gprsMap.find(gprsAddress) == this->m_gprsMap.end()) { // 首个监听客户端
		clientList = boost::shared_ptr<server::GprsClientList>(
				new server::GprsClientList());
		this->m_gprsMap[gprsAddress] = clientList;
		this->WriteToServerQueue(monitorFrame);
		this->AddToGprsCliens(clientList, clientSession);
		this->m_gprsMapMutex.unlock();
		return;
	}
	// 如果请求的 gprs 已有客户端
	clientList = this->m_gprsMap[gprsAddress];


	if (clientList->frame90 == NULL) { // 已有客户端正在等待，但还没有收到回复
		this->WriteToServerQueue(monitorFrame);
		this->AddToGprsCliens(clientList, clientSession);
		this->m_gprsMapMutex.unlock();
		return;
	}

	// 响应 93 帧
	boost::shared_ptr<server::data_frame> response = boost::shared_ptr<
			data_frame>(new data_frame());
	response->Construct93Response(monitorFrame->dst_address(), 0); // 已经在监控
	clientSession->StartWrite(response);

	if (clientList->frame90 != NULL) {
		clientSession->StartWrite(clientList->frame90);
	}
	if (clientList->frame91 != NULL) {
		clientSession->StartWrite(clientList->frame91);
	}
	this->AddToGprsCliens(clientList, clientSession);
	this->m_gprsMapMutex.unlock();
}

void ClientServer::AddToGprsCliens(
		boost::shared_ptr<server::GprsClientList> clientList,
		boost::shared_ptr<server::ClientSession> clinetSession) {
	//clientList->clientsMutex.lock();
	clientList->clients.insert(clinetSession);
	//clientList->clientsMutex.unlock();
}
void ClientServer::AddToGprsCliens(std::string srcAddress,
		boost::shared_ptr<server::ClientSession> clinetSession) {
	this->m_gprsMapMutex.lock();
	if (this->m_gprsMap.find(srcAddress) == this->m_gprsMap.end()) {
		this->m_gprsMap[srcAddress] = boost::shared_ptr<server::GprsClientList>(
				new server::GprsClientList());
	}
	boost::shared_ptr<server::GprsClientList> clientList =
			this->m_gprsMap[srcAddress];
	clientList->clientsMutex.lock();
//	std::set<boost::shared_ptr<server::ClientSession> >::iterator iter =
//			find(clientList->clients.begin(),clientList->clients.end(),clinetSession);
	clientList->clients.insert(clinetSession);
	clientList->clientsMutex.unlock();
}

void ClientServer::StopRealMonitoring(
		boost::shared_ptr<server::ClientSession> clinetSession,
		boost::shared_ptr<data_frame> monitorFrame) {
	if (monitorFrame->cmd() != 0x92) {
		return;
	}
	boost::shared_ptr<server::GprsClientList> clist;
	std::string gprsAddress = monitorFrame->dst_address();
	this->m_gprsMapMutex.lock();
	// 判断 gprs 是否正在实时监控状态
	if (this->m_gprsMap.find(gprsAddress) == this->m_gprsMap.end()) {
		this->m_gprsMapMutex.unlock();
		boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(
						new data_frame());
				response->Construct92Frame("",gprsAddress, 0);
		clinetSession->StartWrite(response);
		return;
	}

	if (monitorFrame->StopMode() == 0x01) { // 强制停止
//		clist->clients.clear();
//		clist->frame90.reset();
//		clist->frame91.reset();
		this->m_gprsMap.erase(gprsAddress);
		this->m_gprsMapMutex.unlock();
		this->WriteToServerQueue(monitorFrame);
		return;
	}
	clist = this->m_gprsMap[gprsAddress];
	if (clist == NULL) {
		this->m_gprsMapMutex.unlock();
		return;
	}
	if (clist->clients.empty()) {
		this->m_gprsMap.erase(monitorFrame->dst_address());
		this->m_gprsMapMutex.unlock();
		return;
	}
	std::set<boost::shared_ptr<server::ClientSession> >::iterator iter =
			clist->clients.find(clinetSession);
	if (clist->clients.size() == 1 && iter != clist->clients.end()) { // 要停止的 gprs 只有一个在监控
		boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(
				new data_frame());
		response->Construct92Frame(monitorFrame->dst_address(),monitorFrame->src_address(), 0);
		iter->get()->StartWrite(response);
		clist->clients.erase(iter);
		this->m_gprsMap.erase(monitorFrame->dst_address());
		this->WriteToServerQueue(monitorFrame);
	} else if (iter != clist->clients.end()) { // 有多个客户端, 非强制停止
		boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(
				new data_frame());
		response->Construct92Frame(monitorFrame->dst_address(),monitorFrame->src_address(), 1);
		iter->get()->StartWrite(response);
	}
	this->m_gprsMapMutex.unlock();
}

void ClientServer::HandleReceiveFromServer(int sockfd) {
	while (true) {
		if (this->m_isConnValid == false) {
			DEBUG("read thread : connection is invalie");
			return;
		}
		boost::shared_ptr<data_frame> rdata = boost::shared_ptr<data_frame>(
				new data_frame());
		int totalRead = 0;
		int bytesRead = 0;
		while (totalRead < HLEN) { // 读报文头
			bytesRead = read(sockfd, rdata->header() + totalRead,
			HLEN - totalRead);
			if (bytesRead == -1) {
				if (errno == EINTR) {
					continue;
				}
				DEBUG(strerror(errno));
				return;
			} else if (bytesRead == 0) {
				return;
			}
			totalRead += bytesRead;
		} // end while

		if (rdata->lead_code() != 0x7e7e) {
			DEBUG("received data foramt is wrong");
			this->m_isConnValid = false;
			close(sockfd);
			return;
		}
		int tailLen = rdata->tail_length();
		totalRead = 0;
		while (totalRead < tailLen) { // 读报文数据部分
			bytesRead = read(sockfd, rdata->header() + HLEN + totalRead,
					tailLen - totalRead);
			if (bytesRead == -1) {
				if (errno == EINTR) {
					continue;
				}
				DEBUG(strerror(errno));
				close(sockfd);
				return;
			} else if (bytesRead == 0) { // 文件结束
				return;
			}
			totalRead += bytesRead;
		} // end while
		  // 十六进制打印收到的报文
		DEBUG(UTIL::ByteToHex(rdata->header(), HLEN + 8).c_str() );
		this->HandleReceivedData(rdata);
	}
}

void ClientServer::HandleConnectToServer() {
	struct sockaddr_un address;
	int sockfd;
	unsigned int addrLength = 0;
	connect: while (true) {
		// 连接服务器
		this->m_isConnValid = false;
		if ((sockfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
			DEBUG(strerror(errno));
			boost::this_thread::sleep(boost::posix_time::seconds(10));
			goto connect;
		}
		address.sun_family = AF_LOCAL;
		strcpy(address.sun_path, UNIX_PATH);
		addrLength = sizeof address;
		if (connect(sockfd, (struct sockaddr*) &address, addrLength) == -1) {
			DEBUG(strerror(errno));
			boost::this_thread::sleep(boost::posix_time::seconds(10));
			goto connect;
		}

		this->m_isConnValid = true;
		DEBUG("socket connected");
		// 开启读取服务器数据线程
		boost::function0<void> fun1 = boost::bind(
				&ClientServer::HandleReceiveFromServer, this, sockfd);
		boost::thread thrd1(fun1);
		thrd1.detach();

		while (true) { // 写入数据
			if (this->m_isConnValid == false) {
				DEBUG("write thread : connection is invalid");
				close(sockfd);
				goto connect;
			}
			std::list<boost::shared_ptr<data_frame> > list;
			this->m_writeToServerQueueMutex.lock();
			list.splice(list.end(), this->m_writeToServerQueue);
			this->m_writeToServerQueueMutex.unlock();
			if (list.empty()) {
				boost::this_thread::sleep(boost::posix_time::seconds(1));
				continue;
			}
			while (!list.empty()) {
				boost::shared_ptr<data_frame> data = list.front();
				std::size_t dataLen = data->length();
				std::size_t totalWriten = 0;
				while (totalWriten < dataLen) {
					int bytesWriten = write(sockfd,
							data->header() + totalWriten,
							dataLen - totalWriten);
					if (bytesWriten == -1) {
						if (errno == EINTR) {
							DEBUG("write interrupted");
							continue;
						} else if (errno == EPIPE) {
							DEBUG("connection closed");
							close(sockfd);
							goto connect;
						}
						DEBUG(std::string(strerror(errno)).c_str());
						close(sockfd);
						goto connect;
					}
					totalWriten += bytesWriten;
				} // end of while
				list.pop_front();
			} // end of while list
		} // end of while
	} // end of while
}

void ClientServer::HandleReceivedData(boost::shared_ptr<data_frame> data) {
	if (data->lead_code() != 0x7e7e) {
		DEBUG("data format error");
		return;
	}
	// 不存在监控的 gprs
	std::string gprsAddress = data->src_address();
	this->m_gprsMapMutex.lock();
	// 没有客户端监控且还是 91 帧
	if (this->m_gprsMap.find(gprsAddress) == this->m_gprsMap.end() && data->cmd() != 0x91) {
		this->m_gprsMapMutex.unlock();
		boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(
				new data_frame());
		response->Construct92Frame("",gprsAddress, 1);
		this->WriteToServerQueue(response);
		return;
	}
	// 存在监控的 gprs
	boost::shared_ptr<server::GprsClientList> clist;
	if (data->cmd() == 0x90) {
		this->m_gprsMap[gprsAddress]->frame90 = data;
		clist = this->m_gprsMap[gprsAddress];
		this->broadcast(clist, data);
		this->m_gprsMapMutex.unlock();
		return;
	} else if (data->cmd() == 0x91) { // 转发所有客户端
		if (this->m_gprsMap.find(gprsAddress)
				== this->m_gprsMap.end()) { // 如果没有发过 91 帧，则转发所有客户端
			this->m_gprsMap[gprsAddress] = boost::shared_ptr<
					server::GprsClientList>(new server::GprsClientList());
		}
		// 一个 gprs 可能发送多个 91 帧
		this->m_gprsMap[gprsAddress]->frame91 = data;
		this->broadcast(data);
	} else if (data->cmd() == 0x88) { // gprs 断开连接
		this->m_gprsMap.erase(gprsAddress);
	} else if (data->cmd() == 0xEE) { // 处理异常帧
		if (data->exception() == 0x07) { // 0xEE07 异常转发所有客户端
			this->broadcast(data);
		} else if (data->exception() == 0x08 ){ // 其它数据异常帧转发监听客户端， gprs 三分钟没有数据,转发所有监控客户端
			//clist = this->m_gprsMap[gprsAddress];
			this->m_gprsMap.erase(gprsAddress);
			this->broadcast( data);
		} else if (data->exception() == 0x09) {	// 发送短信后两分钟 gprs 仍然没有连接
			clist = this->m_gprsMap[gprsAddress];
			this->broadcast(clist, data);
		} else {
			clist = this->m_gprsMap[gprsAddress];
			this->broadcast(clist, data);
		}
	} else {
		clist = this->m_gprsMap[gprsAddress];
		this->broadcast(clist, data);
	}
	// 如果没有客户端在监听，则关闭 gprs
	if (clist != NULL && clist->clients.empty()) {
		this->m_gprsMap.erase(gprsAddress);
		boost::shared_ptr<data_frame> response = boost::shared_ptr<data_frame>(
				new data_frame());
		response->Construct92Response(gprsAddress, 1);
		this->WriteToServerQueue(response);
	}
	int gprsMapSize = this->m_gprsMap.size();
	this->m_gprsMapMutex.unlock();
	DEBUG(UTIL::IntToString(gprsMapSize).c_str());
}

void ClientServer::broadcast(boost::shared_ptr<server::GprsClientList> clist,
		boost::shared_ptr<data_frame> data) {
	if (clist == NULL || data == NULL) {
		return;
	}
//clist->clientsMutex.lock();
	std::set<boost::shared_ptr<server::ClientSession> >::iterator iter;
	for (iter = clist->clients.begin(); iter != clist->clients.end(); iter++) {
		if (iter->get()->IsExit()) {
			clist->clients.erase(iter);
			continue;
		}
		iter->get()->StartWrite(data);
	}
//clist->clientsMutex.unlock();
}

void ClientServer::WriteToServerQueue(boost::shared_ptr<data_frame> data) {
	DEBUG(UTIL::ByteToHex(data->header(),data->length()).c_str() );
	if (data->cmd() == 0x92) {
	DEBUG(UTIL::ByteToHex(data->header(),data->length()).c_str() );
	}
	this->m_writeToServerQueueMutex.lock();
	this->m_writeToServerQueue.push_back(data);
	this->m_writeToServerQueueMutex.unlock();
}

void ClientServer::HeartBeat() {
	boost::asio::deadline_timer heartbeatTimer(this->m_io_service);
	while (true) {
		// 定时一分钟
		heartbeatTimer.expires_at(
				boost::posix_time::second_clock::universal_time()
						+ boost::posix_time::minutes(1));
		heartbeatTimer.wait();
		this->m_clientSessionMutex.lock();
		std::map<std::string, boost::shared_ptr<ClientSession> >::iterator iter;
		for (iter = this->m_clientSession.begin();
				iter != this->m_clientSession.end(); iter++) {
			int hbeatcnt = iter->second->hearbeat();
			if (hbeatcnt > 3) { // 将没有收到心跳的客户端移除
				boost::shared_ptr<ClientSession> client = iter->second;
				iter->second.reset();
				client->Exit();
				//client->CloseSession();
				continue;
			}
			// 发送心跳 -- 不发送心跳
//			boost::shared_ptr<data_frame> hbeatF3 =
//					boost::shared_ptr<data_frame>(new data_frame());
//			hbeatF3->ConstructHeartBeat(iter->first);
//			iter->second->StartWrite(hbeatF3);
		}
		// 将超时客户端移除
		for (iter = this->m_clientSession.begin();
				iter != this->m_clientSession.end(); iter++) {
			if (iter->second == NULL) {
				this->m_clientSession.erase(iter);
			}
		}
		this->m_clientSessionMutex.unlock();
	} // end while
}

bool ClientServer::RemoveClientConnection(std::string address) {
	int size = 0;
	this->m_clientSessionMutex.lock();
	this->m_clientSession.erase(address);
	size = this->m_clientSession.size();
	this->m_clientSessionMutex.unlock();
	DEBUG(UTIL::IntToString(size).c_str() );
	return true;
}

} /* namespace server */
