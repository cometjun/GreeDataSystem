/*
 * ListWrapper.cpp
 *
 *  Created on: 2015年10月25日
 *      Author: yjunlei
 */

#include "ListWrapper.h"

namespace server {

ListWrapper::ListWrapper() {
	// TODO Auto-generated constructor stub

}

ListWrapper::~ListWrapper() {
	// TODO Auto-generated destructor stub
}

void ListWrapper::append(std::list<boost::shared_ptr<data_frame> >& list) {
	this->m_rmutex.lock();
	this->m_frameList.splice(this->m_frameList.end(),list);
	this->m_rmutex.unlock();
}

void ListWrapper::AppendOut(std::list<boost::shared_ptr<data_frame> >& list) {
	this->m_rmutex.lock();
	list.splice(list.end(),this->m_frameList);
	this->m_rmutex.unlock();
}

void ListWrapper::clear() {
	this->m_rmutex.lock();
	this->m_frameList.clear();
	this->m_rmutex.unlock();
}
void ListWrapper::push_back(boost::shared_ptr<data_frame> data) {
	this->m_rmutex.lock();
	this->m_frameList.push_back(data);
	this->m_rmutex.unlock();
}
} /* namespace server */

