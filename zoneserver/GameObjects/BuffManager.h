#pragma once
#include "networkmodule/playertypedef.h"

class CFightObject;
class BuffModify;
class SBuffBaseData;
//class TSaveBuffData;

#include <set>

// AI生物身上的Buff管理器
class BuffManager
{
public:
	typedef std::map<DWORD, BuffModify *> BuffList;		// <Buff ID, Buff>
	typedef std::map<WORD, DWORD>		  BuffGroup;	// <组ID, Buff ID>
	typedef std::set<BuffModify *>		  BuffToOther;	// 释放出去的Buff

public:
	BuffManager() : m_curObj(0) {};
	~BuffManager();

	void Init(CFightObject *object);
	void Free();
	bool AddBuff(DWORD id, CFightObject *pSender);
	bool BuildStub(BuffModify *e);						// Buff开始前
	void ClearStub(BuffModify *e, bool IsInterrupt);	// Buff结束前
	bool CanAddBuff(const SBuffBaseData *pData, CFightObject *pSender);
	void CheckDelBuffSpot(DWORD delFlag);
	void CheckDelBuffType(DWORD delType);
	void DeleteBuff(DWORD id);
	void PusherNotAvaliable();

	// 读取和恢复作用于自己身上的BUFF事件
	BYTE GetSaveBuffList(TSaveBuffData*& aSaveData, BYTE byMaxCount) const;
	BYTE RecoverSaveBuffList(const TSaveBuffData*& aSaveData, BYTE byMaxCount);
private:
	void RemoveBuff(BuffModify *buff);

public:
	BuffList		m_buffList;
	BuffGroup		m_buffGroup;
	BuffToOther		m_buffToOther;
	CFightObject	*m_curObj;
};