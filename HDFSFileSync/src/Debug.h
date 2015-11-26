/*
 * Debug.h
 *
 *  Created on: 2015年10月15日
 *      Author: yjunlei
 */

#ifndef DEBUG_H_
#define DEBUG_H_
#include "LOG.h"
#include <boost/date_time.hpp>
#define DEBUG(str) 	{ \
    zlog_info(server::LOG::category,str); \
}

#endif /* DEBUG_H_ */
