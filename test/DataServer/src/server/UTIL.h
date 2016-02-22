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
namespace server {
/**
 * 字节转换为 16 进制字符串
 */
std::string ByteToHex(const char * head,int len) ;
// 将十六进制的字符串转换为对应的字节字符
std::string HexStringToByteString(std::string hexString) ;
// 整数转换为字符串
std::string IntToString (unsigned long l);
// 整数转换为 16 进制字符串
std::string Int4ToHex (int i);
// 字符串转换为整数
int StringToInt (std::string str);
// 计算 hash 值
unsigned int hash(std::string str);


} // end of namespace server



#endif /* SRC_SERVER_UTIL_H_ */
