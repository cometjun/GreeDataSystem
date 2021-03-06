/*
 * GetSysMac.cpp
 *
 *  Created on: 2015年10月17日
 *      Author: yjunlei
 */

#include "GetSysMac.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include "Debug.h"
namespace server {
char GetSysMac::MAC[] = {};
GetSysMac::GetSysMac() :count(0){
	ifnames = ifname_buf;
}

GetSysMac::~GetSysMac() {
	// TODO Auto-generated destructor stub
}

void GetSysMac::add_interface_name(const char * name) {
	int i;
	for (i = 0; i < count; i++) {
		if (!strcmp(ifnames + i * IFNAMSIZ, name))
			return;
	}
	strncpy(ifnames + (count++) * IFNAMSIZ, name, IFNAMSIZ - 1);
}

char *  GetSysMac::get_name(char *name, char *p) {
	while (isspace(*p))
		p++;
	while (*p) {
		if (isspace(*p))
			break;
		if (*p == ':') { /* could be an alias */
			char *dot = p, *dotname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') { /* it wasn't, backup */
				p = dot;
				name = dotname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name++ = '\0';
	return p;
}

// get /proc/net/dev interface name list into buffer
// return 0 if success
int GetSysMac::get_procnet_list() {
	FILE *fh;
	char buf[512];
	fh = fopen("/proc/net/dev", "r");
	if (!fh)
		return -1;

	fgets(buf, sizeof buf, fh); /* eat title lines */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh)) {
		char name[IFNAMSIZ];
		get_name(name, buf);
		add_interface_name(name);
	}
	fclose(fh);
	return 0;
}

long GetSysMac::mac(char* addr, std::size_t len) {
	/* implementation for Linux */
		struct ifreq ifr;
		struct ifreq *IFR;
		struct ifconf ifc;
		char buf[1024];
		int s, i;
		int ok = 0;

		// clear buffer
		memset(ifname_buf, 0, sizeof(ifname_buf));

		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s == -1) {
			return -1;
		}

		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		ioctl(s, SIOCGIFCONF, &ifc);

		IFR = ifc.ifc_req;
		// put the ioctl interface names in the list
		for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++) {
			add_interface_name(IFR->ifr_name);
		}
		// put the /proc/net/dev interface names in the list
		if (get_procnet_list())
			return -1;

		// get the first mac address of eth* device hardware address
		for (i = 0; i < count; i++) {
			strcpy(ifr.ifr_name, ifnames + i * IFNAMSIZ);
			if (!strncmp(ifr.ifr_name, "eth", 3))
				if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
					if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
						if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
							char *p = (char *) ifr.ifr_hwaddr.sa_data;
							if (!*((int *) p) && !*((int *) (p + 2)))
								continue;
							// if not 00:00:00:00:00:00, yes, we get the real mac addr
							ok = 1;
							break;
						}
					}
				}
		}

		close(s);
		if (ok) {
			bcopy(ifr.ifr_hwaddr.sa_data, addr, 6);
		} else {
			return -1;
		}
		return 0;
}

} /* namespace server */
