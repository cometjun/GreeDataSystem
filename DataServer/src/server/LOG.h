/*
 * LOG.h
 *
 *  Created on: 2015年11月7日
 *      Author: yjunlei
 */

#ifndef LOG_H_
#define LOG_H_
#include <zlog.h>
namespace server {
// 日志类
class LOG {
public:
	LOG();
	virtual ~LOG();
public:
	static  zlog_category_t * category;
};

} /* namespace server */

#endif /* LOG_H_ */
