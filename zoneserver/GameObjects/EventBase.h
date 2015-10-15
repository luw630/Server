#ifndef EVENTBASE_H
#define EVENTBASE_H

#include <Windows.h>

// �¼�����
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

private:								// ����ӿ�
	virtual void SelfDestory() {}		// �����ٷ���
	virtual void OnCancel()		= 0;	// �¼����ж�
	virtual void OnActive()		= 0;	// �¼�������

private:
	DWORD m_actTime;					// �¼�����ʱ��

private:
	EventBase(const EventBase&);
	EventBase& operator=(const EventBase&);
};

#endif // EVENTBASE_H