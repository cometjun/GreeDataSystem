/*
 * BufferedFileName.cpp
 *
 *  Created on: Oct 15, 2015
 *      Author: huynh
 */

// #include "hdfs.h"
#include "UTIL.h"
#include "Debug.h"
#include <fstream>
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include "BufferedFileName.h"
#include "Debug.h"
using namespace std;
namespace server {

BufferedFileName::BufferedFileName() : m_fileName(COUNT_FILE_NAME){

}
BufferedFileName::BufferedFileName(std::string fileName) :
		m_fileName(fileName) {
}
BufferedFileName::~BufferedFileName() {
}

void BufferedFileName::setFileName(std::string fileName) {
	this->m_fileName = fileName;
}

void BufferedFileName::RestoreFileName() {
	std::string path = FILEPATH + this->m_fileName;
	std::ifstream istream(path.c_str());
	if (!istream) {
		return ;
	}
	std::vector<std::string> vec;
	std::string line;
	// 将文件内容读入内存
	while (getline(istream,line,'\n')) {
		boost::split(vec,line,boost::is_any_of(" ") );
		if (vec.size() >= 2) {
			this->m_sysCountMapMutex.lock();
			this->m_sysCountMap[vec[0]] = server::StringToInt(vec[1]);
			this->m_sysCountMapMutex.unlock();
		}
	}

}

void BufferedFileName::writeFileNameCount() {
	std::ofstream outstream;
	outstream.open((FILEPATH + this->m_fileName).c_str(), std::ios::out); //打开文件用于写，若文件不存在就创建它

	this->m_sysCountMapMutex.lock();
	std::map<std::string, std::size_t>::iterator iter;
	std::string bufferWriten("");
	for (iter = this->m_sysCountMap.begin(); iter != this->m_sysCountMap.end();
			iter++) {
		bufferWriten.append(iter->first + " ");
		bufferWriten.append(server::IntToString(iter->second) + "\n");
	}
	this->m_sysCountMapMutex.unlock();
	outstream.write(bufferWriten.c_str(), bufferWriten.size());
	outstream.close();
}

std::string BufferedFileName::getNextCountString(std::string sysIden) {
	if (sysIden.empty()) {
		return "";
	}
	std::string countStr = "";
	this->m_sysCountMapMutex.lock();
	if (this->m_sysCountMap.find(sysIden) == this->m_sysCountMap.end()) {
		this->m_sysCountMap[sysIden] = 0;
		countStr =  server::IntToString(1);
	} else {
	countStr = server::IntToString(this->m_sysCountMap[sysIden] +1);
	}
	this->m_sysCountMapMutex.unlock();
	return countStr;
}

void BufferedFileName::countPlus(std::string sysIden) {

	this->m_sysCountMapMutex.lock();
	if (this->m_sysCountMap.find(sysIden) == this->m_sysCountMap.end() ) {

		this->m_sysCountMapMutex.unlock();
		return;
	}
	this->m_sysCountMap[sysIden] ++;
	this->m_sysCountMapMutex.unlock();
	this->writeFileNameCount();
}

boost::shared_ptr<std::ofstream> BufferedFileName::GetWriteFile(std::string sysIden,std::string& fileName) {
	this->m_sysCountMapMutex.lock();

	std::string fileNameCount = this->getNextCountString(sysIden);
	std::string filePath( HADOOP_PATH + sysIden + "/" + fileNameCount);
	std::string sysDir(HADOOP_PATH + sysIden + "/");
	if (access(sysDir.c_str(),R_OK) != 0) {
		if (mkdir(sysDir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
			this->m_sysCountMapMutex.unlock();
			DEBUG(std::string("mkdir (" + sysDir + ") failed : " + strerror(errno) ).c_str() );
			return boost::shared_ptr<std::ofstream>();
		}
	}
	// 打开新的文件
	boost::shared_ptr<std::ofstream> outstream = boost::shared_ptr<std::ofstream>(new std::ofstream());
	if (outstream == NULL) {
		this->m_sysCountMapMutex.unlock();
		return outstream;
	}
	outstream->open(filePath.c_str(), std::ios::out); //打开文件用于写，若文件不存在就创建它,不能追加，防止文件末尾的数据不完整
	if (outstream->is_open()) {
		 this->countPlus(sysIden);
	} 
	this->m_sysCountMapMutex.unlock();
	fileName = filePath;
	// 文件名改变时输出新的路径名
		DEBUG(std::string("change to file : " + filePath).c_str());
	return outstream;

}

} /* namespace server */

