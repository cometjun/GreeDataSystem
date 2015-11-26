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
#include "UTIL.h"

namespace server {
HDFSReadWrite::HDFSReadWrite() {
}

HDFSReadWrite::~HDFSReadWrite() {
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

tSize HDFSReadWrite::HDFSWrite(boost::shared_ptr<std::ofstream> writeFile, const char* buf,
		std::size_t len) {
	if (writeFile != NULL && writeFile->is_open()) {
			 writeFile->write(buf, len);
		return len;

	} else {
		DEBUG("write handler is null");
	}
	return -1;
}

} /** end of namespace server */
