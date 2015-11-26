/*
 * MessageClient.h
 *
 *  Created on: 2015年11月5日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_MESSAGECLIENT_H_
#define SRC_SERVER_MESSAGECLIENT_H_

#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <list>
#include <set>
#include "data_frame.h"
namespace server {

class MessageClient {
public:
	MessageClient(boost::asio::io_service & io_service, boost::asio::ip::tcp::endpoint  & endpoint);
	virtual ~MessageClient();
	// 启动写发送数据线程
	void start();
	// 连接短信服务器与发送数据线程
	void WriteMessageT();
	// 连接短信服务器
	void ConnectToMessageServer();
	// 发送 93 帧到短信队列中
	void SendMessage(boost::shared_ptr<data_frame> message);
private:
	boost::asio::io_service& m_ioService;
	boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
	boost::asio::ip::tcp::endpoint & m_endpoint;
	// 写队列
	std::set<boost::shared_ptr<data_frame> > m_writeQueue;
	boost::mutex m_writeQueueMutex;

};

} /* namespace server */

#endif /* SRC_SERVER_MESSAGECLIENT_H_ */
