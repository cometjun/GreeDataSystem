/*
 * BufferedMessage.h
 *
 *  Created on: Oct 15, 2015
 *      Author: root
 */

#ifndef SRC_SERVER_BUFFEREDFILENAME_H_
#define SRC_SERVER_BUFFEREDFILENAME_H_
#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <boost/thread/recursive_mutex.hpp>
#define COUNT_FILE_NAME "syscount.txt"
#define FILEPATH "/root/hadoop/"
#define HADOOP_PATH "/root/hadoop/"
namespace server {
struct location{
	std::string fileName;
	std::size_t index;
};
class BufferedFileName {
public:
	BufferedFileName();
	BufferedFileName(std::string fileName);
	virtual ~BufferedFileName();
	void setFileName(std::string path);
	// 将计数内容读入内存,一次调用
	void RestoreFileName();
	// 将计数存储到文件
	void writeFileNameCount();
	// 获取下一个计数
	std::string getNextCountString(std::string sysIden);
	// 将计数值 +1
	void countPlus(std::string sysIden);
	// 获取准备写入的文件名
	boost::shared_ptr<std::ofstream> GetWriteFile(std::string sysIden,std::string& fileName);
private:
	std::map<std::string,std::size_t> m_sysCountMap; 	// 系统标识与计数
	boost::recursive_mutex m_sysCountMapMutex;
	std::string m_fileName;	// 存放文件名计数的文件名
	//location mLocation;
};

} /* namespace server */

#endif /* SRC_SERVER_BUFFEREDFILENAME_H_ */
