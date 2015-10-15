#ifndef EVENTBASE_H
#define EVENTBASE_H

#include <Windows.h>

// 事件基类
class EventBase
{
	friend class EventManager;
	friend class EventCompare;
	friend class CDManager;

public:
	EventBase() : m_actTime(0) {}
	virtual ~EventBase(){}				// Do Nothing

public:
	bool Interrupt();

private:								// 虚拟接口
	virtual void SelfDestory() {}		// 自销毁方法
	virtual void OnCancel()		= 0;	// 事件被中断
	virtual void OnActive()		= 0;	// 事件被激活

private:
	DWORD m_actTime;					// 事件激活时间

private:
	EventBase(const EventBase&);
	EventBase& operator=(const EventBase&);
};

#endif // EVENTBASE_H