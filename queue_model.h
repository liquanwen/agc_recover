#ifndef QUEUE_MODEL_H
#define QUEUE_MODEL_H
#include <deque>
#include "pub_std.h"
//=============================================================
//队列模板
//=============================================================

template<typename T>
class CpacketQueue
{
public:
	CpacketQueue(){};
	~CpacketQueue(){};

public:
	//添加队列
	void AddTail(const T& t)
	{
		CIIAutoMutex mutex(&m_cs);
		m_que.push_back(t);
	}
    //得到队列头
	bool GetHead(T &t)
	{
		CIIAutoMutex mutex(&m_cs);
		if (!m_que.empty()) {
			t = m_que.front();
			m_que.pop_front();
			return true;
		}
		return false;
	}
	//队列是否为空
	bool IsEmpty(void) const
	{
		CIIAutoMutex mutex(&m_cs);
		return m_que.empty();
	}
	//清空队列
	void Clear(void)
	{
		CIIAutoMutex mutex(&m_cs);
		m_que.clear();
	}
private:
	mutable CIIMutex	  m_cs;
	std::deque<T>		  m_que;
};

#endif  //QUEUE_MODEL_H
