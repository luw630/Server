#ifndef EVENTUTILITY_H
#define EVENTUTILITY_H

#include "EventBase.h"

// �¼�����Ƚ���
class EventCompare : public std::binary_function<DWORD, DWORD, bool>
{
public:
	bool operator() (DWORD lhs, DWORD rhs) const
	{
		return lhs < rhs;
	}
};

#endif // EVENTUTILITY_H