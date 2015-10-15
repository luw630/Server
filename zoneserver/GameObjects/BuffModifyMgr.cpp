#include "Stdafx.h"

/*
BuffModifyMgr::BuffModifyMgr()
{
	m_curObj	  = 0;
	m_curBuffList = 0;
}

BuffModifyMgr::~BuffModifyMgr()
{
	m_curObj	  = 0;
	m_curBuffList = 0;
}

void BuffModifyMgr::Init(CFightObject *object, BuffList *buffList)
{
	assert(object && buffList);

	m_curObj		= object;
	m_curBuffList	= buffList;
}

bool BuffModifyMgr::IsOwnerBuff(WORD id)
{
	BuffEntryList::iterator it = m_BuffEntity.find(id);

	return it != m_BuffEntity.end() ? true : false;
}

void BuffModifyMgr::DropBuff(WORD id)
{
	BuffEntryList::iterator it = m_BuffEntity.find(id);
	
	if (it != m_BuffEntity.end() && it->second->m_ID == id)
	{
		it->second->Interrupt();
		return;
	}
}
//*/

/*
bool BuffModifyMgr::AddBuff(WORD id, bool isForever)
{
	if (IsOwnerBuff(id))
		DropBuff(id);

	BuffModify *curBuff = new BuffModify(this);
	
	if (!curBuff->InitBuffData(id, isForever))
	{
		delete curBuff;
		return false;
	}

	if (curBuff->BeginBuff(m_curBuffList))
	{
		m_BuffEntity[id] = curBuff;
		return true;
	}

	return false;
}//*/