/*
 * ClientSession.h
 *
 *  Created on: 2015年10月10日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_CLIENTSESSION_H_
#define SRC_SERVER_CLIENTSESSION_H_
#include <boost/asio.hpp>
#include "data_frame.h"
#include "ClientServer.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <queue>
#include <list>
namespace server {
class ClientServer;
class ClientSession:
		public boost::enable_shared_from_this<ClientSession>{
public:

	ClientSession(boost::asio::io_service & io_service,ClientServer& clientServerPtr);
	virtual ~ClientSession();
	boost::asio::ip::tcp::socket& socket();
	// 读客户端数据
	void StartRead();
	// 读到数据后的回调函数
	void handle_read_header(const boost::system::error_code& error);
	void handle_read_body(const boost::system::error_code& error);
	// 数据放入写队列中
	void StartWrite(boost::shared_ptr<data_frame> data);
	// 异步写回调函数
	void handle_write(const boost::system::error_code& error, std::size_t bytes_transfered);
	void keepalive();
	// 客户端是否已退出
	bool IsExit() {
		return this->m_isExit;
	}
	// 标记客户端为退出
	void Exit() {
		try {
		this->m_socket->close();
		} catch (boost::system::system_error & error){
		}
		 this->m_isExit = true;
	}
	// 增加心跳计数
	int hearbeat() {
		this->m_heartbeat ++ ;
		return this->m_heartbeat;
	}
	// 关闭连接
	void CloseSession();
	void CloseSession(const boost::system::error_code& error);
	void RemoveSession();
	// 处理对应的报文
	void Handle90Package();
	void Handle91Package();
	void Handle92Package();
	void Handle93Package();
	void Handle96Package();
	void HandleEEPackage();
	void HandleF3Package();
private:
	 // 与客户端对应的套接字连接
	  boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
		// 侦听程序的引用
		 ClientServer& m_clientServer;
	  // 存放收到的数据
	  boost::shared_ptr<data_frame> m_dataFramePtr;
	  // 写到客户端的数据队列
	  std::list<boost::shared_ptr<data_frame> > m_writeQueue;
	  boost::mutex m_writeQueueMutex;
	  // 客户端是否已无效
	  bool m_isExit;
	  // 心跳计数
	  int m_heartbeat;
	  // 客户端标识
	  std::string m_srcAddress;
	  std::string m_imei;
};
typedef boost::shared_ptr<ClientSession> ClientSessionPtr;
} /* namespace server */

#endif /* SRC_SERVER_CLIENTSESSION_H_ */
