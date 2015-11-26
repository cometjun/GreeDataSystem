/*
 * ClientServer.h
 *
 *  Created on: 2015年10月10日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_CLIENTSERVER_H_
#define SRC_SERVER_CLIENTSERVER_H_
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <set>
#include "ClientSession.h"
#include "ListWrapper.h"
#define CLINET_NUMBER 200
#define UNIX_PATH "/root/unixsocket"
namespace server {

class ClientSession;
struct GprsClientList {
	std::set<boost::shared_ptr<ClientSession> > clients;
	boost::shared_ptr<data_frame> frame90;
	boost::shared_ptr<data_frame> frame91;
	boost::recursive_mutex clientsMutex;
	server::ListWrapper list;
};

//class tsdb_server;
class ClientServer {
public:
	ClientServer(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint);
	virtual ~ClientServer();
	// 开启与服务器建立连接的线程
	void start();
	// 接收连接
	void StartAccept();
	// 处理连接到的客户端
	void handle_accept(boost::shared_ptr<ClientSession> pClientSessionPtr, const boost::system::error_code& error);
//	void SetGprsServer(boost::shared_ptr<tsdb_server> tsdb_server_ptr_);
	// 向所有客户端广播
	void broadcast(boost::shared_ptr<data_frame> data);
	// 转发到所有监控客户端
	void broadcast(boost::shared_ptr<server::GprsClientList> clients,boost::shared_ptr<data_frame> data);

	void SetSocketfd(int sockfd) {
		this->m_socketfd = sockfd;
	}
	// 开始与停止监听
	void StartRealMonitoring(boost::shared_ptr<server::ClientSession> clinetSession,boost::shared_ptr<data_frame> monitorFrame);
	void StopRealMonitoring(boost::shared_ptr<server::ClientSession> clinetSession,boost::shared_ptr<data_frame> monitorFrame);
	// 添加连接到监听 gprs 的队列中
	void AddToGprsCliens(std::string srcAddress,boost::shared_ptr<server::ClientSession> clinetSession);
	void AddToGprsCliens(boost::shared_ptr<server::GprsClientList> clientList,boost::shared_ptr<server::ClientSession> clinetSession);
	// 将数据放到写到服务器的队列中
	void WriteToServerQueue(boost::shared_ptr<data_frame> data);
	// 处理接入服务器发送的数据
	void HandleReceiveFromServer(int sockfd);
	void HandleReceivedData(boost::shared_ptr<data_frame> data);
	// 心跳线程，每分钟一次，三次未收到心跳即断开连接
	void HeartBeat();
	// 处理与服务器的连接
	void HandleConnectToServer();
public:
	// 添加与删除已连接客户端
	bool AddClientConnection(std::string address,boost::shared_ptr<ClientSession>  clientSessionPtr);
	bool RemoveClientConnection(std::string address);
private:
	boost::asio::io_service& m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::map<std::string,boost::shared_ptr<ClientSession> > m_clientSession;	// 保存已连接的所有客户端
	boost::recursive_mutex m_clientSessionMutex;
	std::map<std::string,boost::shared_ptr<server::GprsClientList> > m_gprsMap;	// 实时监控 gprs 的客户端与 gprs 发来的数据队列
	boost::recursive_mutex m_gprsMapMutex;
	std::list<boost::shared_ptr<data_frame> > m_writeToServerQueue; 	// 写到服务器的队列
	boost::mutex m_writeToServerQueueMutex;
	int m_socketfd; // 与服务器通信的套接字
	bool m_isConnValid;

//	boost::shared_ptr<tsdb_server> m_tsdb_server_ptr;
};
typedef boost::shared_ptr<ClientServer> ClientServerPtr;

} /* namespace server */
#endif /* SRC_SERVER_CLIENTSERVER_H_ */
