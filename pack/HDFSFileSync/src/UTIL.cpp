/*
 * UTIL.cpp
 *
 *  Created on: 2015年10月11日
 *      Author: yjunlei
 */
#include "UTIL.h"
#include "Debug.h"
#include <sstream>
namespace UTIL {
std::string ByteToHex(const char* head,int len) {
	char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char * temp = new char[2*len+1];
	for (int i = 0;i < len;i ++) {
		int j = 0;
		j = head[i]>>4&0x0F;
		temp[2*i] = hex[j];
		j = head[i]&0x0F;
		temp[2*i+1] = hex[j];
	}
	temp[2*len] = '\0';
	std::string strHex(temp);
	delete[] temp;
	return strHex;
}

std::string IntToString(long l) {
	std::stringstream sstream;
	sstream<<l;
	std::string s = "";
	sstream>>s;
	return s;
}

int StringToInt(std::string str) {
	std::stringstream sstream;
	sstream<<str;
	int i = 0;
	sstream>>i;
	return i;
}

std::string Int4ToHex(int i) {
	char buf[4];
	int j = 0 ;
	buf[j++] = i >>24 & 0x000000FF;
	buf[j++] = i >>16 & 0x000000FF;
	buf[j++] = i >>8 & 0x000000FF;
	buf[j++] = i  & 0x000000FF;
	return UTIL::ByteToHex(buf,4);
}
} // end of namespace server

