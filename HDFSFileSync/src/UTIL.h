/*
 * UTIL.h
 *
 *  Created on: 2015年10月10日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_UTIL_H_
#define SRC_SERVER_UTIL_H_
#include <iostream>
#include <string>
namespace UTIL {
/**
 * 字节转换为 16 进制字符串
 */
std::string ByteToHex(const char * head,int len) ;
std::string IntToString (long l);
std::string Int4ToHex (int i);
int StringToInt (std::string str);

} // end of namespace server



#endif /* SRC_SERVER_UTIL_H_ */
