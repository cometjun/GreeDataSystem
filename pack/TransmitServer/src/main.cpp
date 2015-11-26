//下面是一个异步模式的简单的Tcp echo服务器
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <map>
#include <sys/socket.h>
#include <sys/un.h>
#include "cfg/config.h"
#include "ClientServer.h"
#include "GetSysMac.h"
#include "UTIL.h"
#include "Debug.h"
#include <zlog.h>

#define ZLOGPATH "conf/zlog.conf"
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

int main(int argc, char* argv[]) {
	// 获取端口
	map<string, string> m = ReadConfig("src/ip.cfg");
	unsigned short clientport = 2000;
	if (m.find("clientport") != m.end()) {
		clientport = atoi(m["clientport"].c_str());
	}
	std::cout <<"clinet port : " << clientport << std::endl;
	// 获取本机 mac 地址
	char buffer[7];
	server::GetSysMac getMac;
	getMac.mac(buffer, 7);
	server::data_frame::mac = "";
	server::data_frame::mac.append(buffer, 7);
	std::cout << "mac : "
			<< UTIL::ByteToHex(server::data_frame::mac.c_str(),
					server::data_frame::mac.length()) << std::endl;

	boost::asio::io_service io_service;
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
	// client 服务器
	tcp::endpoint endpoint(tcp::v4(), clientport);
	boost::shared_ptr<server::ClientServer> clientServer(
			new server::ClientServer(io_service, endpoint));
	clientServer->start();
	io_service.run();
}
