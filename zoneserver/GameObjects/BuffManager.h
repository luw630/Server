#pragma once
#include "networkmodule/playertypedef.h"

class CFightObject;
class BuffModify;
class SBuffBaseData;
//class TSaveBuffData;

#include <set>

// AI�������ϵ�Buff������
class BuffManager
{
public:
	typedef std::map<DWORD, BuffModify *> BuffList;		// <Buff ID, Buff>
	typedef std::map<WORD, DWORD>		  BuffGroup;	// <��ID, Buff ID>
	typedef std::set<BuffModify *>		  BuffToOther;	// �ͷų�ȥ��Buff

public:
	BuffManager() : m_curObj(0) {};
	~BuffManager();

	void Init(CFightObject *object);
	void Free();
	bool AddBuff(DWORD id, CFightObject *pSender);
	bool BuildStub(BuffModify *e);						// Buff��ʼǰ
	void ClearStub(BuffModify *e, bool IsInterrupt);	// Buff����ǰ
	bool CanAddBuff(const SBuffBaseData *pData, CFightObject *pSender);
	void CheckDelBuffSpot(DWORD delFlag);
	void CheckDelBuffType(DWORD delType);
	void DeleteBuff(DWORD id);
	void PusherNotAvaliable();

	// ��ȡ�ͻָ��������Լ����ϵ�BUFF�¼�
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