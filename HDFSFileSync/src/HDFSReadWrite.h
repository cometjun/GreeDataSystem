/*
 * HDFSReadWrite.h
 *
 *  Created on: 2015年10月12日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_HDFSREADWRITE_H_
#define SRC_SERVER_HDFSREADWRITE_H_
#include <map>
#include <vector>
#include <hdfs.h>
#include <string>
namespace server {
class HDFSReadWrite {
public:
	HDFSReadWrite();
	virtual ~HDFSReadWrite();
	// 写数据到 hdfs
	static tSize HDFSWrite(std::string iden,const char * buffer,tSize len);
	static tSize HDFSWrite(hdfsFS fs,hdfsFile writeFile,std::string buf);
	static tSize HDFSWrite(hdfsFS fs,hdfsFile writeFile,const char * buf,std::size_t len);
	/** 获取文件名 */
	static std::string GetHdfsFileName(std::string iden) ;
	static std::string GetHdfsFileName() ;
	static bool AddFileName(std::string time_, std::string fileName);
private:
	static std::string preTime;
	static std::map<std::string, std::vector<std::string> > m_fileNames;
};

} 	/* end of namespace server */
#endif /* SRC_SERVER_HDFSREADWRITE_H_ */
