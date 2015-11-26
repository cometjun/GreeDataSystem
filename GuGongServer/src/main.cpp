//下面是一个异步模式的简单的Tcp echo服务器
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <zlog.h>
#include "server/tsdb_server.h"
#include <map>
#include "cfg/config.h"
#include "server/GetSysMac.h"
#include "server/UTIL.h"
#include "server/LOG.h"
#define ZLOGPATH "conf/zlog.conf"
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

// 生成一个侦听套接字
int GetAcceptSocket() {
	  int acc_sock;
	  // 使用 unix 域套接字
	    acc_sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	    if(acc_sock == -1) {
	        perror("create accept socket failed ");
	        return -1;
	    }
	    struct sockaddr_un serv_addr;
	    serv_addr.sun_family = AF_LOCAL;
	    strcpy(serv_addr.sun_path, UNIX_PATH);
	    unlink(UNIX_PATH);
	   if ( bind(acc_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		   std::cout<<"bind error : "<<std::string(strerror(errno));
		   exit(-1);
	   }
	   if ( listen(acc_sock, 10) < 0) {
		   std::cout<<"listen error : "<<std::string(strerror(errno));
		   exit(-1);
	   }
	    return acc_sock;
}
int main(int argc, char* argv[])
{
	// 获取程序配置信息
	map<string, string> m1 = ReadConfig("conf/param.conf");
	// 缓存的单个文件最大大小
	if (m1.find("maxfilesize") != m1.end() && !m1["maxfilesize"].empty()) {
		server::tsdb_server::MaxFileSize = atol(m1["maxfilesize"].c_str());
	}
	// 局部队列缓存长度
	if (m1.find("localqueuesize") != m1.end() && !m1["localqueuesize"].empty()) {
		server::tsdb_session::MaxQueueSize = atol(m1["localqueuesize"].c_str());
	}
	std::cout<<"文件大小 : "<<server::tsdb_server::MaxFileSize<< "\n局部队列大小 : "<<server::tsdb_session::MaxQueueSize;
	// 获取 ip 与端口配置信息
	map<string, string> m = ReadConfig("src/ip.cfg");
	unsigned short gprsport = 2001;
	unsigned short messageport = 2000;
	std::string  messageip = "127.0.0.1";
	if (m.find("gprsport") != m.end()) {
		gprsport = atoi(m["gprsport"].c_str());
	}
	if (m.find("messageip") != m.end()) {
		messageip = m["messageip"];
	}
	if (m.find("messageport") != m.end()) {
		messageport = atoi(m["messageport"].c_str());
	}
	std::cout<<"gprs port : "<<gprsport<<"\nmessage server ip: "<<messageip<<"\nmessage server port "<<messageport<<std::endl;
	// 获取监听客户端的 socket
	int acceptfd = ::GetAcceptSocket();
	boost::asio::io_service io_service;
	// 获取本机 mac 地址
		char buffer[7];
		server::GetSysMac getMac;
		getMac.mac(buffer,7);
		server::data_frame::mac = "";
		server::data_frame::mac.append(buffer,7);
		std::cout<<"mac : "<<server::ByteToHex(server::data_frame::mac.c_str(),server::data_frame::mac.length())<<std::endl;
		// 初始化日志系统
	    int rc;
	     rc = zlog_init(ZLOGPATH);
	    if (rc) {
	        printf("init zlog failed\n");
	        return -1;
	    }
	    server::LOG::category = zlog_get_category("runtime");
	    if (!server::LOG::category) {
	    			printf("get category fail\n");
	        zlog_fini();
	        return -2;
	    }
		// GRPS 服务器
		tcp::endpoint gprsEndpoint(tcp::v4(), gprsport);
		server::tsdb_server_ptr gprsServer(new server::tsdb_server(io_service, gprsEndpoint));
		// message client
		tcp::endpoint messageEndpoint(boost::asio::ip::address::from_string(messageip), messageport);
		boost::shared_ptr<server::MessageClient> messageClient = boost::shared_ptr<server::MessageClient>(
				new server::MessageClient(io_service,messageEndpoint));
		gprsServer->SetMessageClient(messageClient);
		gprsServer->SetAcceptSockfd(acceptfd);
		//messageClient->start();
		gprsServer->start();
		io_service.run();
}
