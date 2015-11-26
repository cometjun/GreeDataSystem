/*
 * HdfsConnectionPool.cpp
 *
 *  Created on: 2015年9月13日
 *      Author: yjunlei
 */

#include "HdfsConnectionPool.h"


hdfsFS HdfsConnectionPool::fs = NULL;
string HdfsConnectionPool::ip = "127.0.0.1";
HdfsConnectionPool::HdfsConnectionPool() {
}

HdfsConnectionPool::~HdfsConnectionPool() {
	// TODO Auto-generated destructor stub
}
