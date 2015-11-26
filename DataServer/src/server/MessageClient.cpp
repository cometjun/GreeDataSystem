/*
 * MessageClient.cpp
 *
 *  Created on: 2015年11月5日
 *      Author: yjunlei
 */

#include <boost/thread.hpp>
#include "MessageClient.h"
#include "Debug.h"
namespace server {

MessageClient::MessageClient(boost::asio::io_service& io_service,
		 boost::asio::ip::tcp::endpoint& endpoint) :
		m_ioService(io_service),  m_endpoint(endpoint) {
}

MessageClient::~MessageClient() {
}

void MessageClient::ConnectToMessageServer() {
	if (this->m_socket != NULL) {
			this->m_socket->close();
			this->m_socket.reset();
		}
		this->m_socket = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(this->m_ioService));
	while (true) {
		try {
			this->m_socket->connect(this->m_endpoint);
			return;
		} catch ( boost::system::system_error &error) {
			DEBUG(std::string(error.what()).c_str());
		}
		boost::this_thread::sleep(boost::posix_time::seconds(20));
	} // end of while

}

void MessageClient::start() {
	// 开启与短信服务器交互接口
	boost::function0<void> fun = boost::bind(
			&MessageClient::WriteMessageT, this);
	boost::thread thrd(fun);
	thrd.detach();
}

void MessageClient::WriteMessageT() {
	std::list<boost::shared_ptr<data_frame> > list;
	while (true) {
		// 连接到服务器

		connect: this->ConnectToMessageServer();
		// 清空历史短信
		this->m_writeQueueMutex.lock();
		this->m_writeQueue.clear()	;
		this->m_writeQueueMutex.unlock();
		while (true) {
			// 从写队列中取出数据
		this->m_writeQueueMutex.lock();
		std::set<boost::shared_ptr<data_frame> >::iterator iter;
		for (iter = this->m_writeQueue.begin(); iter != this->m_writeQueue.end(); iter ++) {
			list.push_back(*iter);
		}
		// 清除队列中的内容
		this->m_writeQueue.clear();
		//list.splice(list.end(),this->m_writeQueue);
		this->m_writeQueueMutex.unlock();
//		boost::shared_ptr<data_frame> test = boost::shared_ptr<data_frame>(new data_frame());
//		test->Construct88Frame("1234567");
//		list.push_back(test);
		if (list.empty()) {
			boost::this_thread::sleep(boost::posix_time::seconds(10));
			continue;
		}
		while (!list.empty()) {
			std::string byteBuffer;
			boost::shared_ptr<data_frame> data = list.front();
			byteBuffer.append(data->header(),data->length());
			try {
				this->m_socket->write_some(boost::asio::buffer(byteBuffer.c_str(),byteBuffer.size() ) );
			//	boost::asio::write(*this->m_socket,boost::asio::buffer(byteBuffer.c_str(),byteBuffer.size() ) );
			} catch (boost::system::system_error & error) { // 写数据到短信服务器出错
				DEBUG(std::string(error.what()).c_str());
				goto connect;
			}
			list.pop_front();
			//this->m_socket.write_some(boost::asio::buffer(byteBuffer.c_str(),byteBuffer.size() ) );
		} // end of while
		} // end while
	} // end while
}

void MessageClient::SendMessage(boost::shared_ptr<data_frame> message) {
	this->m_writeQueueMutex.lock();
	this->m_writeQueue.insert(message);
	this->m_writeQueueMutex.unlock();
}

} /* namespace server */
