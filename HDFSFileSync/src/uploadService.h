/*
 * uploadService.h
 *
 *  Created on: 2015年10月24日
 *      Author: yjunlei
 */

#ifndef SRC_UPLOADSERVICE_H_
#define SRC_UPLOADSERVICE_H_
#include "BufferedFileName.h"
#include "hdfs.h"
#include <boost/thread/mutex.hpp>
#define BUFFER__SIZE 1024*3
namespace server {

class uploadService {
public:
	uploadService();
	virtual ~uploadService();
	// 搜索数据保存到本地的目录，
	void StartSearchSysIden();
	void StartUpload();
	// 读取文件上传到 hdfs
	void Upload();
	void start();
private:
	BufferedFileName m_bufferedFileName;
	bool breakThread;
	std::queue<std::string> m_sysIdenQueue;
	boost::mutex m_sysIdenQueueMutex;
	hdfsFS m_fs;
};

} /* namespace server */

#endif /* SRC_UPLOADSERVICE_H_ */
