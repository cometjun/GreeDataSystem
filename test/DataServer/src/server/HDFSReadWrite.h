/*
 * HDFSReadWrite.h
 *
 *  Created on: 2015年10月12日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_HDFSREADWRITE_H_
#define SRC_SERVER_HDFSREADWRITE_H_
#include <map>
#include <fstream>
#include <vector>
#include <hdfs.h>
#include <boost/asio.hpp>
#include <string>
namespace server {
class HDFSReadWrite {
public:
	HDFSReadWrite();
	virtual ~HDFSReadWrite();
	// 将数据写到本地文件
	static tSize HDFSWrite(boost::shared_ptr<std::ofstream> writeFile,const char * buf,std::size_t len);
	/** 获取文件名 */
	static std::string GetHdfsFileName(std::string iden) ;
	static std::string GetHdfsFileName() ;
private:
	static std::string preTime;
	static std::map<std::string, std::vector<std::string> > m_fileNames;
};

} 	/* end of namespace server */
#endif /* SRC_SERVER_HDFSREADWRITE_H_ */
