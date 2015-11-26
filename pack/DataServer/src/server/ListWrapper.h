/*
 * ListWrapper.h
 *
 *  Created on: 2015年10月25日
 *      Author: yjunlei
 */

#ifndef SRC_SERVER_LISTWRAPPER_H_
#define SRC_SERVER_LISTWRAPPER_H_
#include <list>
#include <boost/asio.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "data_frame.h"
namespace server {
// 对 list 加锁
class ListWrapper {
public:
	ListWrapper();
	virtual ~ListWrapper();
	void append(std::list<boost::shared_ptr<data_frame> > & list);
	std::size_t size() {
		this->m_rmutex.lock();
		std::size_t size =  this->m_frameList.size();
		this->m_rmutex.unlock();
		return size;
	}
	void AppendOut(std::list<boost::shared_ptr<data_frame> > & list);
	void push_back(boost::shared_ptr<data_frame> data) ;
	void clear();
private:
	std::list<boost::shared_ptr<data_frame> > m_frameList;
	boost::recursive_mutex m_rmutex;
};

} /* namespace server */

#endif /* SRC_SERVER_LISTWRAPPER_H_ */
