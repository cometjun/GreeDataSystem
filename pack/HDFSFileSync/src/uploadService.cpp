/*
 * uploadService.cpp
 *
 *  Created on: 2015年10月24日
 *      Author: yjunlei
 */

#include "uploadService.h"
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "HdfsConnectionPool.h"
#include "HDFSReadWrite.h"
#include "hdfs.h"
#include "Debug.h"
#include "UTIL.h"
namespace server {

uploadService::uploadService() :breakThread(false){
	this->m_fs = ::HdfsConnectionPool::hdfs();
	if (this->m_fs == NULL) {
		exit(-1);
	}
}

uploadService::~uploadService() {
	// TODO Auto-generated destructor stub
}

void uploadService::StartSearchSysIden() {
	while (! this->breakThread) {
		this->m_bufferedFileName.SearchSysIden();
		boost::shared_ptr<std::queue<std::string> > tempQueuePtr = this->m_bufferedFileName.GetSysIden();
		this->m_sysIdenQueueMutex.lock();
		this->m_sysIdenQueue = *tempQueuePtr;
		this->m_sysIdenQueueMutex.unlock();
		boost::this_thread::sleep(boost::posix_time::minutes(2));	// 每隔一段时间扫描一次
	}
}

void uploadService::StartUpload() {
}

void uploadService::Upload() {

	while (!breakThread) {
		this->m_sysIdenQueueMutex.lock();
		if (this->m_sysIdenQueue.empty()) {
			this->m_sysIdenQueueMutex.unlock();
			boost::this_thread::sleep(boost::posix_time::seconds(10));	// 每隔一段时间扫描一次
			continue;
		}
		std::string sysIden = this->m_sysIdenQueue.front();
		this->m_sysIdenQueue.pop();
		this->m_sysIdenQueue.push(sysIden);
		this->m_sysIdenQueueMutex.unlock();

		std::string fileCount = "";	// 打开的文件路径,与 hadoop 路径一致
		boost::shared_ptr<std::ifstream> instream = this->m_bufferedFileName.GetReadFileStream(sysIden,fileCount);
		if (instream == NULL ) {
			boost::this_thread::sleep(boost::posix_time::seconds(10));	// 每隔一段时间扫描一次
			continue;
		}

		std::string hadoopPath = "/" + sysIden + "/" + fileCount;
		std::string localPath = HADOOP_FILE_PATH + sysIden + "/" + fileCount;
		DEBUG(std::string("start upload file : " + localPath).c_str());
		hdfsFile writeFile = hdfsOpenFile(this->m_fs, hadoopPath.c_str(),
				O_WRONLY | O_EXCL, 0, 0, 0);
		if (writeFile == NULL) {
			instream->close();
			DEBUG(std::string("opend hdfs file failed : " + hadoopPath).c_str() );
			boost::this_thread::sleep(boost::posix_time::seconds(10));	// 每隔一段时间扫描一次
			continue;
		}
		char buffer[BUFFER__SIZE];
		std::size_t totalSize = 0;
		 if (instream->peek()  != EOF) { // 文件不为空
			 std::size_t bytesRead = 0;
			while (!instream->eof()) {
				 instream->read(buffer,BUFFER__SIZE);
				 bytesRead = instream->gcount();
				if (bytesRead > 0) {
					server::HDFSReadWrite::HDFSWrite(this->m_fs,writeFile,buffer,bytesRead);
					totalSize += bytesRead;

				} else {
					break;
				}
				if (totalSize % 10000000 < 3*1024) {
					DEBUG(std::string(UTIL::IntToString(totalSize) + " bytes transfered" ).c_str());
				}
			}
		 }

		instream->close();
		if ( remove(localPath.c_str()) == 0) {  // 删除文件
			DEBUG(std::string("delete file success : " + localPath ).c_str());
		} else {
			DEBUG(std::string("delete file failed : " + std::string(strerror(errno)) ).c_str() );
		}
		hdfsCloseFile(this->m_fs,writeFile);
	}
}

void uploadService::start() {
	boost::function0<void> fun = boost::bind(&uploadService::StartSearchSysIden, this);
		boost::thread thrd(fun);
		thrd.detach();
	this->Upload();
}

} /* namespace server */
