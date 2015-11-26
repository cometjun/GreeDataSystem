/*
 * tsdb_session.h
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */

#ifndef SERVER_TSDB_SESSION1_H1_
#define SERVER_TSDB_SESSION1_H1_

#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <queue>
#include <list>
#include "hdfs.h"
#include "tsdb_server.h"
#include "ListWrapper.h"

#include "data_frame.h"
/*
#include <boost/lexical_cast.hpp>
 #include <protocol/TBinaryProtocol.h>//使用thrift库文件
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include "ThriftHadoopFileSystem.h"
*/
using boost::asio::ip::tcp;
///#define MAXQUEUE 100
namespace server {

class tsdb_server;
class tsdb_session :
		public boost::enable_shared_from_this<tsdb_session> {
public:
	tsdb_session(boost::asio::io_service& io_service,server::tsdb_server & tsdb_server_);
	virtual ~tsdb_session();
	void SetTransmit(bool isTransmit) {
		this->m_isTransmit = isTransmit;
	}
	boost::shared_ptr<data_frame> Get90Frame() {
		return this->m_90Frame;
	}
	boost::shared_ptr<data_frame> Get91Frame() {
		this->m_91FrameMutex.lock();
		boost::shared_ptr<data_frame> _91Frame = this->m_91Frame;
		this->m_91FrameMutex.unlock();
		return _91Frame;
	}
	// 增加心跳，空闲 60s 开始，每 60s 一次，共 3 次
	void keepalive() ;
	// 返回客户端 socket
	tcp::socket& socket();
	// 将局部缓存放入全局缓存
	void putToServer();
	// 开始读 gprs 数据
	void StartRead();
	// 读到报文头的回调函数
	void handle_read_header(const boost::system::error_code& error);
	// 读取报文数据部分后的回调函数
	void handle_read_body(const boost::system::error_code& error);
	void StartWrite(boost::shared_ptr<data_frame> data);
	void HandleWrite(const boost::system::error_code& error, std::size_t bytes_transfered);
	// 将 gprs 从已连接队列中删除
	void RemoveSession();
	// 将报文放入缓存队列中
	void PushToQueue();
	// 关闭 gprs 连接
	void CloseSession();
	void CloseSession(const boost::system::error_code & error);
	// 设置局部缓存需要加入到的全局缓存队列
	void SetGlobalQueue( boost::shared_ptr<server::ListWrapper> globalQueue) {
		this->m_globalQueue = globalQueue;
	}
private:
  boost::shared_ptr<tcp::socket> socket_;
  // 接收报文的缓存
  boost::shared_ptr<data_frame> read_frame_;
  server::tsdb_server& m_tsdb_server;
  // gprs 信息
  std::string m_srcAddress; // ip:port
  std::string m_imei;
  std::string m_sysIden;
  // 局部缓存队列
  std::list<boost::shared_ptr<data_frame> >  m_readQueue;
  // 全局缓存队列
  boost::shared_ptr<server::ListWrapper> m_globalQueue;
  // 发送到 gprs 的数据队列
  std::list<boost::shared_ptr<data_frame> > m_writeQueue;
  boost::mutex m_writeQueueMutex;

  bool m_isLogin; 	// 是否开启写线程
  bool m_isTransmit; // 是否转发数据

	std::size_t m_total; // 已发送的帧的个数
  boost::shared_ptr<data_frame> m_90Frame;
  boost::shared_ptr<data_frame> m_91Frame;
  boost::mutex m_91FrameMutex;
public:
  static std::size_t MaxQueueSize;
};

typedef boost::shared_ptr<tsdb_session> tsdb_session_ptr;
} /* namespace server */

#endif /* SERVER_TSDB_SESSION1_H1_ */
