/*
 * BufferedFileName.cpp
 *
 *  Created on: Oct 15, 2015
 *      Author: huynh
 */

#include "UTIL.h"
#include "Debug.h"
#include <fstream>
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <dirent.h>
#include "BufferedFileName.h"
using namespace std;
namespace server {
BufferedFileName::BufferedFileName():  m_fileName("syscount.txt"){
	this->RestoreFileName();
}
BufferedFileName::BufferedFileName(std::string fileName) :
		m_fileName(fileName) {
	BufferedFileName();
}
BufferedFileName::~BufferedFileName() {
}

void BufferedFileName::setFileName(std::string fileName) {
	this->m_fileName = fileName;
}

void BufferedFileName::RestoreFileName() {

	std::string path = COUNT_FILE_PATH + this->m_fileName;
	std::ifstream istream(path.c_str());
	if (!istream) {
		return;
	}
	std::vector<std::string> vec;
	std::string line;
	// 将文件内容读入内存
	while (getline(istream, line, '\n')) {
		boost::split(vec, line, boost::is_any_of(" "));

		if (vec.size() >= 2) {
			this->m_sysCountMapMutex.lock();
			this->m_sysCountMap[vec[0]] = UTIL::StringToInt(vec[1]);
			this->m_sysCountMapMutex.unlock();
		}
	}

}

void BufferedFileName::writeFileNameCount() {
	std::ofstream outstream;
	outstream.open((COUNT_FILE_PATH + this->m_fileName).c_str(), std::ios::out); //打开文件用于写，若文件不存在就创建它
	this->m_sysCountMapMutex.lock();
	std::map<std::string, std::size_t>::iterator iter;
	std::string bufferWriten("");
	for (iter = this->m_sysCountMap.begin(); iter != this->m_sysCountMap.end();
			iter++) {
		bufferWriten.append(iter->first + " ");
		bufferWriten.append(UTIL::IntToString(iter->second) + "\n");
	}
	this->m_sysCountMapMutex.unlock();
	outstream.write(bufferWriten.c_str(), bufferWriten.size());
	outstream.close();
}

std::string BufferedFileName::getCountString(std::string sysIden) {
	if (sysIden.empty()) {
		return "";
	}
	std::string countStr = "";
	this->m_sysCountMapMutex.lock();
	if (this->m_sysCountMap.find(sysIden) == this->m_sysCountMap.end()) {
		this->m_sysCountMap[sysIden] = 1;
		countStr = UTIL::IntToString(1);
	} else {
		countStr = UTIL::IntToString(this->m_sysCountMap[sysIden]);
	}
	this->m_sysCountMapMutex.unlock();

	return countStr;
}

std::string BufferedFileName::getNextCountString(std::string sysIden) {
	if (sysIden.empty()) {
			return "";
		}
		std::string countStr = "";
		this->m_sysCountMapMutex.lock();
		if (this->m_sysCountMap.find(sysIden) == this->m_sysCountMap.end()) {
			this->m_sysCountMap[sysIden] = 1;
			countStr = UTIL::IntToString(2);
		} else {
			countStr = UTIL::IntToString(this->m_sysCountMap[sysIden]+1);
		}
		this->m_sysCountMapMutex.unlock();

		return countStr;
}

void BufferedFileName::countPlus(std::string sysIden) {

	this->m_sysCountMapMutex.lock();
	if (this->m_sysCountMap.find(sysIden) == this->m_sysCountMap.end()) {
		this->m_sysCountMapMutex.unlock();
		return;
	}
	this->m_sysCountMap[sysIden]++;
	this->m_sysCountMapMutex.unlock();
	this->writeFileNameCount();
}

boost::shared_ptr<std::ifstream> BufferedFileName::GetReadFileStream(
		const std::string& sysIden,std::string & fileName ) {

	this->m_sysCountMapMutex.lock();
	std::string fileNameCount = this->getNextCountString(sysIden);
	std::string filePath(HADOOP_FILE_PATH + sysIden + "/" + fileNameCount);

	if (access(filePath.c_str(),R_OK) != 0) {
		this->m_sysCountMapMutex.unlock();
			DEBUG(std::string(filePath+" can not read").c_str()) ;
			return boost::shared_ptr<std::ifstream>();
	}
	fileNameCount = this->getCountString(sysIden);
	filePath = HADOOP_FILE_PATH + sysIden + "/" + fileNameCount;
	boost::shared_ptr<std::ifstream> instream = boost::shared_ptr<std::ifstream>(new std::ifstream(filePath.c_str(),ios::in|ios::binary) );
	if (!*instream) {
		this->m_sysCountMapMutex.unlock();
		DEBUG(std::string("open file failed : " + filePath).c_str());
		return boost::shared_ptr<std::ifstream>();
	}
	this->countPlus(sysIden);
	fileName = fileNameCount;
	this->m_sysCountMapMutex.unlock();
	DEBUG(std::string("change to file : " + fileName).c_str());

	return instream;
}

void BufferedFileName::SearchSysIden() {

	DIR* dir;
	struct dirent * ptr;
	dir = opendir(HADOOP_FILE_PATH);
	if (dir == NULL) {
		std::string err = std::string("open dir ") + HADOOP_FILE_PATH + " failed";
		perror(err.c_str());
		return;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) { ///current dir OR parrent dir
			continue;
		} else if (ptr->d_type == 8) {	// file
			continue;
		} else if (ptr->d_type == 4) { // dir
			std::string path(ptr->d_name);
			std::cout<<"path " <<path<<std::endl;
			m_sysCountMapMutex.lock();
			if (this->m_sysCountMap.find(path) == this->m_sysCountMap.end()) {
				this->m_sysCountMap[path] = 2;
			}
			m_sysCountMapMutex.unlock();
		}
	}
	closedir(dir);
}

boost::shared_ptr<std::queue<std::string> > BufferedFileName::GetSysIden() {

	this->m_sysCountMapMutex.lock();
	std::map<std::string,std::size_t>::iterator iter;
	boost::shared_ptr<std::queue<std::string> > tempQueuePtr =
			boost::shared_ptr<std::queue<std::string> >(
					new std::queue<std::string>());
	if (tempQueuePtr != NULL) {
		for (iter = this->m_sysCountMap.begin();
				iter != this->m_sysCountMap.end(); iter++) {
				tempQueuePtr->push(iter->first);
		}
	}
	this->m_sysCountMapMutex.unlock();
	return tempQueuePtr;
}

} /* namespace server */

