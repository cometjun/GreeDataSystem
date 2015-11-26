/*
 * GetSysMac.h
 *
 *  Created on: 2015年10月17日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_GETSYSMAC_H_
#define SRC_SERVER_GETSYSMAC_H_
#include <iostream>
namespace server {

class GetSysMac {
public:
	static char MAC[7];
public:
	GetSysMac();
	virtual ~GetSysMac();
	long mac(char  *addr,std::size_t len);
private:
	void add_interface_name(const char * name);
	char * get_name(char *name, char *p);
	int get_procnet_list();
private:
	char ifname_buf[2048];
	char *ifnames;
	int count;
};

} /* namespace server */

#endif /* SRC_SERVER_GETSYSMAC_H_ */
