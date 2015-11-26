/*
 * BufferedMessage.h
 *
 *  Created on: Oct 15, 2015
 *      Author: root
 */

#ifndef SRC_SERVER_BUFFEREDFILENAME_H_
#define SRC_SERVER_BUFFEREDFILENAME_H_
#include <string>
#include <queue>
#include <boost/thread/recursive_mutex.hpp>
#define COUNT_FILE_PATH "./src/"
#define HADOOP_FILE_PATH "/root/hadoop/"
namespace server {
class BufferedFileName {
public:
	BufferedFileName();
	BufferedFileName(std::string fileName);
	virtual ~BufferedFileName();
	void setFileName(std::string path);
	// 将计数内容读入内存
	void RestoreFileName();
	// 将计数存储到文件
	void writeFileNameCount();
	// 获取下一个计数
	std::string getCountString(std::string sysIden);
	std::string getNextCountString(std::string sysIden);
	// 增加文件名计数
	void countPlus(std::string sysIden);
	// 获取对应系统要写入的文件名
	boost::shared_ptr<std::ifstream> GetReadFileStream(const std::string& sysIden,std::string& fileName);
	// 搜索 hadoop 目录，查找新添加系统标识, 以文件夹为标识
	void SearchSysIden();
	// 获取系统标识的集合
	boost::shared_ptr<std::queue<std::string> > GetSysIden();
private:
	std::map<std::string,std::size_t> m_sysCountMap; 	// 系统标识与计数
	boost::recursive_mutex m_sysCountMapMutex;
	std::string m_fileName;	// 存放文件名计数的文件名
};

} /* namespace server */

#endif /* SRC_SERVER_BUFFEREDFILENAME_H_ */
