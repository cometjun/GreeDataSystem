/*
 * main.cpp
 *
 *  Created on: 2015年10月23日
 *      Author: yjunlei
 */
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include "config.h"
#include "uploadService.h"
#include "HdfsConnectionPool.h"

#include "LOG.h"
#define ZLOGPATH "conf/zlog.conf"
int main() {
	// 读取 hadoop IP
	std::map<std::string, std::string> m = ReadConfig("./src/ip.cfg");
	std::cout << "hadoop IP" << m["ip"] << std::endl;
	::HdfsConnectionPool::ip = m["ip"];
	std::string path = "./src/ip.cfg";
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
	server::uploadService service;
	service.start();
}

