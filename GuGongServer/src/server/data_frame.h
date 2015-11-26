/*
 * data_frame.h
 *
 *  Created on: 2015年5月26日
 *      Author: wb
 */

#ifndef SERVER_DATA_FRAME_H_
#define SERVER_DATA_FRAME_H_

#include <cstddef>
#include <string>
#define HLEN 18

namespace server {
class data_frame {
public:
	static std::string mac;
public:
	enum{ HEADER_LENGTH=1500};
	 enum CLIENT_TYPE{ NONE,GPRS, CLIENT, READER};
	data_frame();
	~data_frame();
	const char *header() const;
	char *header();
	std::string sys_identity();
	// 构造对应功能码的帧
	void Construct90Response(data_frame &data_);
	void Construct91Response(data_frame& data_);
	void ConstructF3Response(data_frame& data_);
	void Construct93Response(std::string srcAddress, uint8_t result);
	void ConstructEEResponse(std::string srcAddress, uint8_t result);
	void Construct88Frame(std::string srcAddress);
	void Construct88Frame(std::string srcAddress,std::string dstAddress);

	uint8_t flag93() {
		return this->header_[19];
	}
	uint8_t pattern() {
		return this->header_[19];
	}
	/**
	 * 源地址
	 */
	std::string  src_address();
	/**
	 * 目的地址
	 */
	std::string  dst_address();
	/**
	 * 设备标识
	 */
	std::string imei();
	/**
	 * 功能码
	 */
	unsigned char cmd();

	uint8_t exception() ;
	/**
	 * 引导码
	 */
	uint16_t lead_code();
	/**
	 * 报文总长度
	 */
	uint16_t length();
	/**
	 * 数据长度
	 */
	uint16_t data_length();
	/**
	 * 数据长度 +1
	 */
	uint16_t tail_length();
private:
	// 计算校验码
	uint16_t GetCheckCode(const char * mac);
	uint8_t crc(uint8_t *pkt, int size);
private:
	char header_[HEADER_LENGTH];
};

} /* namespace server */

#endif /* SERVER_DATA_FRAME_H_ */
