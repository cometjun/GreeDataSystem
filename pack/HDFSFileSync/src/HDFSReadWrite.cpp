/*
 * HDFSReadWrite.cpp
 *
 *  Created on: 2015年10月12日
 *      Author: yjunlei
 */

#include "HDFSReadWrite.h"
#include <boost/date_time.hpp>
#include <hdfs.h>
#include "Debug.h"
#include "HdfsConnectionPool.h"
#include "UTIL.h"

namespace server {
HDFSReadWrite::HDFSReadWrite() {
}

HDFSReadWrite::~HDFSReadWrite() {
}

tSize HDFSReadWrite::HDFSWrite(std::string fileName, const char* buffer,
		tSize len) {
	fileName = "/" + fileName;
	int flag = O_WRONLY | O_CREAT;
	if (hdfsExists(::HdfsConnectionPool::hdfs(), fileName.c_str()) == 0) {
		flag = O_WRONLY | O_APPEND;
	}
	hdfsFile writeFile = hdfsOpenFile(::HdfsConnectionPool::hdfs(),
			fileName.c_str(), flag, 0, 0, 0);

	if (writeFile != NULL) {

		tSize bytesWriten = hdfsWrite(::HdfsConnectionPool::hdfs(), writeFile,
				buffer, len);
		;

		if (hdfsFlush(::HdfsConnectionPool::hdfs(), writeFile) == -1) {
			return -1;
		}

		hdfsCloseFile(::HdfsConnectionPool::hdfs(), writeFile);
		return bytesWriten;
	} else {
		DEBUG("hdfs open file failed!");
	}
	return -1;
}

tSize HDFSReadWrite::HDFSWrite(hdfsFS fs, hdfsFile writeFile, std::string buf) {
	if (writeFile != NULL) {
		tSize bytesWriten = hdfsWrite(fs, writeFile, buf.c_str(), buf.length());
//		if (hdfsFlush(::HdfsConnectionPool::hdfs(), writeFile) == -1) {
//			return -1;
//		}
		return bytesWriten;
	} else {
		DEBUG("hdfs open file failed!");
	}
	return -1;
}

std::string HDFSReadWrite::GetHdfsFileName(std::string iden) {
	boost::posix_time::ptime now =
			boost::posix_time::second_clock::local_time();
	std::string fileName;
	std::string fileTime(boost::posix_time::to_iso_string(now).substr(0, 11));
	fileName.append(fileTime + iden);

	return fileName;
}

std::string HDFSReadWrite::GetHdfsFileName() {
	return server::HDFSReadWrite::GetHdfsFileName("");
}

bool HDFSReadWrite::AddFileName(std::string time_, std::string fileName) {
	server::HDFSReadWrite::HDFSWrite(time_, fileName.c_str(), fileName.size());
///	server::HDFSReadWrite::m_fileNames[time_].push_back(fileName);
	return true;
}

tSize HDFSReadWrite::HDFSWrite(hdfsFS fs, hdfsFile writeFile, const char* buf,
		std::size_t len) {
	if (writeFile != NULL) {
		std::size_t bytesWriten = 0;
		while (bytesWriten < len) {
			int num = hdfsWrite(fs, writeFile, buf, len);
			if (num == -1) {
				return -1;
			}
			bytesWriten += num;
		}
		return bytesWriten;
	} else {
		DEBUG("write handler is null");
	}
	return -1;
}

} /** end of namespace server */
