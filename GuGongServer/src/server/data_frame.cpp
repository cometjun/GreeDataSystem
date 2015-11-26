/*
 * data_frame.cpp
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */

#include <arpa/inet.h>

#include "data_frame.h"
#include <iostream>
#include <cstdlib>
#include <memory.h>
#include <boost/date_time.hpp>
#include "UTIL.h"
#include "Debug.h"
#include "GetSysMac.h"
namespace server {
std::string data_frame::mac = "'";
data_frame::data_frame() {

	memset(header_,0,HEADER_LENGTH);
}

data_frame::~data_frame() {
}

const char * data_frame::header() const {
	return header_;
}

char * data_frame::header() {
	return header_;
}

/**
 * 返回地址的十六进制表示
 */
std::string data_frame::src_address() {
	return server::ByteToHex(&header_[9],7);
}

unsigned char data_frame::cmd() {
	return header_[18];
}
uint8_t data_frame::exception() {
	return header_[19];
}
uint16_t data_frame::lead_code() {
	return *(uint16_t*)header_;
}


uint16_t data_frame::length() {
	return data_length() + HLEN + 1;
}

uint16_t data_frame::data_length() {
	uint16_t len = 0;
	len = header_[16] & 0x00FF;
	len <<= 8;
	len |= header_[17] & 0xFF;
	return len;
}

uint16_t data_frame::tail_length() {
	return data_length() + 1;
}

std::string data_frame::dst_address() {
	return server::ByteToHex(&header_[2],7);
}

std::string data_frame::imei() {
	if (this->cmd() == 0x90) {
		return std::string(&header_[23]);
	} else if (this->cmd() == 0x89) {
		return std::string(&header_[25]);
	}
	return "other";
}

void data_frame::Construct90Response(data_frame& data_) {
	memset(header_,0,HEADER_LENGTH);
	if (data_.cmd() != 0x90 && data_.cmd() != 0x89) {
		DEBUG(server::ByteToHex(data_.header_, data_.length()).c_str());
		return;
	}
	memcpy(this->header_,data_.header_,2); 	// copy 0x7e7e
	memcpy(&this->header_[2], &data_.header_ [9],7); 	// copy src address to dst address
	memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

	this->header_[16] = 0x00;
	this->header_[17] = 0x03;	// data length
	unsigned short checkCode = this->GetCheckCode(&this->header_[9]);

	this->header_[18] = 0x90;
	this->header_[19] = (checkCode>>8) & 0xFF;
	this->header_[20] = checkCode & 0xFF;
	this->header_[21] = this->crc((uint8_t*)&this->header_[2],21-2);
	//DEBUG(server::ByteToHex(this->header_,this->length()));
}

void data_frame::Construct91Response(data_frame& data_) {

	memset(header_,0,HEADER_LENGTH);
		if (data_.cmd() != 0x91) {
			DEBUG(server::ByteToHex(data_.header_, data_.length()).c_str());
			return;
		}
		memcpy(this->header_,data_.header_,2); 	// copy 0x7e7e
		memcpy(&this->header_[2], &data_.header_ [9],7); 	// copy src address to dst address

		memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

		this->header_[16] = 0x00;
		this->header_[17] = 0x02;	// data length
		this->header_[18] = 0x91;
		this->header_[19] = 0x00;
		this->header_[20] = this->crc((uint8_t*)&this->header_[2],20-2);
		//DEBUG(server::ByteToHex(this->header_,this->length()));
}

void data_frame::Construct93Response(std::string srcAddress, uint8_t result) {
	memset(header_,0,HEADER_LENGTH);
	this->header_[0] = 0x7e;
	this->header_[1] = 0x7e;

	srcAddress = server::HexStringToByteString(srcAddress);

	if (!srcAddress.empty())
			memcpy(&this->header_[9], srcAddress.c_str(),7); 	// copy src address to dst address

		//	memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

			this->header_[16] = 0x00;
			this->header_[17] = 0x02;	// data length
			this->header_[18] = 0x93;	// cmd
			this->header_[19] = result;
			this->header_[20] = this->crc((uint8_t*)&this->header_[2],20-2);
}

void data_frame::ConstructEEResponse(std::string srcAddress, uint8_t result) {
	memset(header_,0,HEADER_LENGTH);
	this->header_[0] = 0x7e;
	this->header_[1] = 0x7e;
	srcAddress = server::HexStringToByteString(srcAddress);
	if (!srcAddress.empty())
			memcpy(&this->header_[9], srcAddress.c_str(),7); 	// copy src address to

			//memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

			this->header_[16] = 0x00;
			this->header_[17] = 0x02;	// data length
			this->header_[18] = 0xEE;	// cmd
			this->header_[19] = result;
			this->header_[20] = this->crc((uint8_t*)&this->header_[2],20-2);
}

void data_frame::Construct88Frame(std::string srcAddress) {

	memset(header_,0,HEADER_LENGTH);
	this->header_[0] = 0x7e;
	this->header_[1] = 0x7e;
	srcAddress = server::HexStringToByteString(srcAddress);
	if (!srcAddress.empty())
			memcpy(&this->header_[9], srcAddress.c_str(),7); 	// copy src address

			//memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

			this->header_[16] = 0x00;
			this->header_[17] = 0x01;	// data length
			this->header_[18] = 0x88;	// cmd
			this->header_[19] = this->crc((uint8_t*)&this->header_[2],19-2);
}

void data_frame::Construct88Frame(std::string srcAddress,
		std::string dstAddress) {

	memset(header_,0,HEADER_LENGTH);
	this->header_[0] = 0x7e;
	this->header_[1] = 0x7e;
	dstAddress = server::HexStringToByteString(dstAddress);
	if (!dstAddress.empty())
			memcpy(&this->header_[2], dstAddress.c_str(),7); 	// copy src address

	srcAddress = server::HexStringToByteString(srcAddress);
	if (!srcAddress.empty())
			memcpy(&this->header_[9],srcAddress.c_str(),7); 	// src mac address

			this->header_[16] = 0x00;
			this->header_[17] = 0x01;	// data length
			this->header_[18] = 0x88;	// cmd
			this->header_[19] = this->crc((uint8_t*)&this->header_[2],19-2);
}
void data_frame::ConstructF3Response(data_frame& data_) {
	memset(header_,0,HEADER_LENGTH);
		if (data_.cmd() != 0xF3) {
			DEBUG(server::ByteToHex(data_.header_, data_.length()).c_str());
			return;
		}
		memcpy(this->header_,data_.header_,2); 	// copy 0x7e7e
		memcpy(&this->header_[2], &data_.header_ [9],7); 	// copy src address to dst address

		memcpy(&this->header_[9],server::data_frame::mac.c_str(),7); 	// src mac address

		this->header_[16] = 0x00;
		this->header_[17] = 0x07;	// data length
		this->header_[18] = 0xF3;	// cmd

		boost::posix_time::ptime now =
				boost::posix_time::second_clock::local_time();
		this->header_[19] = (now.date().year() -2014)& 0xFF;
		this->header_[20] = (now.date().month()) & 0xFF;
		this->header_[21] = now.date().day() & 0xFF;
		this->header_[22] = now.time_of_day().hours() & 0xFF;
		this->header_[23] = now.time_of_day().minutes()& 0xFF;
		this->header_[24] = now.time_of_day().seconds() & 0xFF;
		this->header_[25] = this->crc((uint8_t*)&this->header_[2],25-2);
		//DEBUG(server::ByteToHex(this->header_,this->length()));
}

std::string data_frame::sys_identity() {
	if (this->cmd() == 0x90) {
		return "900001";
	}
	return "unknown";
}

uint16_t data_frame::GetCheckCode(const char * mac) {
	return mac[0]*3 + mac[1]*6 +mac[2] *0 + mac[3]*9 + mac[4] * 7 + mac[5] *4 + mac[6]*10;
}

uint8_t data_frame::crc(uint8_t *pkt, int size) {
	const uint8_t gc_DowCrcTable[256] =
	{
	// Maxim 1-Wire CRC of all bytes passed to it.The result am_Testumulates in the global variable CRC.
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
	};
	int i  = 0;
	uint8_t m_CheckSum = 0;
	//uint8_t buf [] = {0x7E,0x7E,0x3D,0x02,0x28,0x16,0x26,0x51,0x4b,0x9c,0xb6,0x54,0x95,0x27,0xd4,0x00,0x00,0x03,0x90,0xf9,0xB6};
//	DEBUG(server::ByteToHex((char*)pkt,size) );

	for (i = 0; i < size; i++){
	m_CheckSum = gc_DowCrcTable[m_CheckSum ^pkt[i]];
	}

	return m_CheckSum;
}

} /* namespace server */

