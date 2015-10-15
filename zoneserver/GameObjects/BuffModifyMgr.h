#ifndef BUFFMODIFYMGR_H
#define BUFFMODIFYMGR_H

#include "EventMgr.h"
#include "BuffPropertyChange.h"

class BuffModifyMgr
{
public:
	typedef std::map<WORD, BuffModify*> BuffEntryList;

public:
	BuffModifyMgr();
	~BuffModifyMgr();

	void Init(CFightObject *object, BuffList *buffList);
	bool AddBuff(WORD id, bool isForever = false);
	bool IsOwnerBuff(WORD id);			// ÊÇ·ñ´æÔÚbuff
	void DropBuff(WORD id);

private:
	BuffEntryList	m_BuffEntity;
	CFightObject	*m_curObj;
	BuffList		*m_curBuffList;
};

#endif // BUFFMODIFYMGR_H