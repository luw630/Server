#pragma once
#include "DynamicRegion.h"

const __int32 IID_DYNAMICDAILYAREA = 0x0f94be80;

class CDynamicDailyArea : public CDynamicRegion
{
public:
	CDynamicDailyArea();
	~CDynamicDailyArea();
public:
	IObject *VDC(const __int32 IID) { return (IID == IID_DYNAMICDAILYAREA) ? this : CDynamicRegion::VDC(IID); }

protected:
	int OnCreate(_W64 long pParameter);
	void OnClose();

public:
	virtual int AddObject(LPIObject pChild);
	virtual int DelObject(LPIObject pChild);
	virtual int DelMonster(DWORD dwGID);
	// һ��ֻ�ڴ���ʱ����
	void SetCopySceneGID(DWORD GID) { m_CopySceneGID = GID; }
	DWORD GetCopySceneGID() { return m_CopySceneGID; }

public:
	void OnRun(void);

	void SetPlayerRInfo(DWORD prevRID, WORD x, WORD y)
	{
		m_prevRegID = prevRID;
		m_prevX = x;
		m_prevY = y;
	}

	void SetPlayerID(DWORD gID) { m_playerID = gID; }
	DWORD GetPlayerID() { return m_playerID; }

protected:
	DWORD	m_CopySceneGID;
	DWORD	m_playerID;				// ����˶�̬��������ҵ�GID

public:
	DWORD	m_prevRegID;			// ���֮ǰ�ĳ���ID
	WORD	m_prevX;				// ����
	WORD	m_prevY;
public:
	DWORD	m_DynamicIndex;			// ���ñ��е�����
};

