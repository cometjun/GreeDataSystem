/*
 * tsdb_server.h
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */

#ifndef SERVER_TSDB_SERVER_H_
#define SERVER_TSDB_SERVER_H_

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <fstream>

#include "tsdb_session.h"
#include "BufferedFileName.h"
#include "data_frame.h"
#include "ListWrapper.h"
#include "MessageClient.h"
#define CONNECTION_SIZE 10000000
//#define MAXFILESIZE 120000000 // 100M 0x20000000 // 500M
#define THREADNUM 1
#define LIST_NUM 1 	// 对就系统的请求平均分配给 LIST_NUM 个队列
using boost::asio::ip::tcp;

#define UNIX_PATH "/root/unixsocket"
#define MaxBufferSize 1024*3
namespace server {
class tsdb_session;
class ClientServer;
struct MessageInfo {
	std::string gprsAddress;
	boost::posix_time::ptime arrTime; 	// 到达时间
};
struct SessionInfo {
	std::string address;
	std::string fileName;
};
struct HdfsFileInfo {
	std::string fileName; 	// 文件路径 文件名
	boost::shared_ptr<std::ofstream> writeFile; // 文件描述符
	std::size_t bytesWriten;
};

struct GprsInfo {
	std::string address; 	// gprs 地址
	std::string sysIden; // 系统标识
};
struct VectorInfo {
	std::size_t index;
	boost::shared_ptr<server::ListWrapper> vec[LIST_NUM];
};
class tsdb_server {
public:
	tsdb_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint);
	// 接收 gprs 连接
	void startAccept();
	// 连接建立或出现错误时的回调函数
	void handle_accept(boost::shared_ptr<tsdb_session> session, const boost::system::error_code& error);

	// 接收与客户端的连接，并读数据，如果连接断开则重新接收连接
	void HandleClientConnection();
	// 从写队列中取数据写到客户端
	void WriteToClientT(int clientfd);
	// 将数据写到客户端队列中
	bool WriteToClientQueue(boost::shared_ptr<data_frame> data);

	// 处理客户端收到的报文
	void HandleReceivedClientData(boost::shared_ptr<data_frame> data);
	// 启动侦听客户端的连接与接收 gprs 的连接
	void start();
	virtual ~tsdb_server();
	// 从全局队列中取出数据写入本地系统
	void WriteToHdfs(std::string sysIden , boost::shared_ptr<server::ListWrapper > listWrapper);
	void WriteToLocal(std::string sysIden , std::list<boost::shared_ptr<data_frame> > &list);
	void SetAcceptSockfd(int fd) {
		this->m_AcceptSocketfd = fd;
	}
	void SetMessageClient(boost::shared_ptr<server::MessageClient> messageClient) {
		this->m_messageClient = messageClient;
	}
	// 将需要通过短信激活的 gprs 模块加入等待队列中
	void PutToMessageQueue(std::string gprsAddress);
	// 处理短信超时事件
	void HandleMessageTimeOut(const boost::system::error_code & error);

	// 添加与移除 gprs 连接
	bool  AddGprsConnection(std::string sysIden,std::string address,boost::shared_ptr<tsdb_session>  tsdb_session_);
	bool RemoveGprsConnection(std::string address);

	// 开启写数据到本地文件的线程，第个系统对应一个线程
	void StartWriteToHdfs(std::string sysIden , boost::shared_ptr<server::ListWrapper > listWrapper);
	// 获取准备要写的文件名
	boost::shared_ptr<std::ofstream> GetWriteFile(std::string sysIden);
	// 更新已写入文件字节数
	void updateBytesWriten(std::string sysIden,std::size_t bytes);
	// 给每个客户端随机选择一个全局队列
	boost::shared_ptr<server::ListWrapper> GetGlobalQueue(std::string sysIden, std::string address);
private:
	  boost::asio::io_service& io_service_;
	  tcp::acceptor acceptor_;
	  std::map<std::string,boost::shared_ptr<tsdb_session> > m_gprsSession; // 保存所有 gprs 客户端连接
	  boost::mutex m_gprsSessionMutex;
	  std::map<std::string, VectorInfo > m_gprsQueueMap;
	  boost::mutex m_gprsQueueMapMutex;
	  // 写到 client 的数据队列
	  std::list<boost::shared_ptr<data_frame> > m_writeToClientQueue;
	  boost::mutex m_writeToClientQueueMutex;
	  // 是否中断写线程，始终为 false
	  bool breakThread;
	  // 监听套接字
	  int m_AcceptSocketfd;
	  // 客户端是否已连接
	  bool m_isClientValid;
	  // 发送短信消息的客户端
	  boost::shared_ptr<server::MessageClient> m_messageClient;

	  server::BufferedFileName m_fileCount;
	  // 等待短信消息的客户端队列
	  std::list<server::MessageInfo> m_messageQueue;
	  boost::mutex m_messageQueueMutex;
	  // 定时器
	  boost::asio::deadline_timer m_messageTimer;

	  std::map<std::string, struct HdfsFileInfo> m_writeFileMap;	//	保存各个系统当前打开的文件指针,当前已写入字节数,key 为系统标识
public:
	  static std::size_t MaxFileSize; // 存入本地缓存的单个文件大小
};

typedef boost::shared_ptr<tsdb_server> tsdb_server_ptr;

} /* namespace server */


#endif /* SERVER_TSDB_SERVER_H_ */
