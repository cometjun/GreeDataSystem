/*
 * UTIL.cpp
 *
 *  Created on: 2015年10月11日
 *      Author: yjunlei
 */
#include "UTIL.h"
#include "Debug.h"
#include <sstream>
namespace server {
std::string ByteToHex(const char* head,int len) {
	char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
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

std::string HexStringToByteString(std::string hexString) {
	std::string result  = "";
	for (std::size_t i = 0;i < hexString.size(); i += 2) {
		unsigned char ch = hexString[i];
		unsigned char ch1 = hexString[i+1];
		// 第一个字节
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') ) {
			if (ch < 'A') {
				ch -= '0';
			} else {
				ch -= 'A' - 10;
			}
		}else {
			return std::string();
		}
		// 第二个字节
		if ((ch1 >= '0' && ch1 <= '9') || (ch1 >= 'A' && ch1 <= 'F') ) {
			if (ch1 < 'A') {
				ch1 -= '0';
			} else {
				ch1 -= 'A' - 10;
			}
		}else {
			return std::string();
		}
		unsigned char uchar = (ch <<4) | ch1;
		result.append((char*)&uchar,1);
	}
	return result;
}
std::string IntToString(unsigned long l) {
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
	return server::ByteToHex(buf,4);
}

unsigned int hash(std::string str) {
	  unsigned int hash = 0;
	   unsigned int x    = 0;
	   unsigned int i    = 0;
	   for(i = 0; i < str.length(); i++)
	   {
	      hash = (hash << 4) + (str[i]);
	      if((x = hash & 0xF0000000L) != 0)
	      {
	         hash ^= (x >> 24);
	      }
	      hash &= ~x;
	   }
	   return hash;
}
} // end of namespace server

