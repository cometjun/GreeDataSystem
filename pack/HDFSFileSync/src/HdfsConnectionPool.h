/*
 * HdfsConnectionPool.h
 *
 *  Created on: 2015年9月13日
 *      Author: yjunlei
 */

#ifndef SERVER_HDFSCONNECTIONPOOL_H_
#define SERVER_HDFSCONNECTIONPOOL_H_
#include "hdfs.h"
#include "map"
#include "string"
#include "config.h"
class HdfsConnectionPool {
public:
	HdfsConnectionPool();
	virtual ~HdfsConnectionPool();

	static hdfsFS hdfs() {
		if (fs == NULL)
			fs = hdfsConnect(ip.c_str(), 9000);
		return fs;
	}
public:
	static string ip;
private:
	static hdfsFS fs ;

};


#endif /* SERVER_HDFSCONNECTIONPOOL_H_ */
