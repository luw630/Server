#pragma once
#include "region.h"

const __int32 IID_DYNAMICREGION = 0x19eaaa2f; 

class CDynamicRegion : public CRegion
{
public:
    IObject *VDC(const __int32 IID) { return (IID == IID_DYNAMICREGION) ? this : CRegion::VDC(IID); }

protected:
	int OnCreate(_W64 long pParameter);
    void OnClose();

public:
	virtual int AddObject(LPIObject pChild);
	virtual int DelObject(LPIObject pChild);
	virtual int DelMonster(DWORD dwGID);

protected:
	void OnRun(void);

public:
	CDynamicRegion(void);
    ~CDynamicRegion(void);

	// һ��ֻ�ڴ���ʱ����
	void SetCopySceneGID(DWORD GID) { m_CopySceneGID = GID; }
	DWORD GetCopySceneGID() { return m_CopySceneGID; }

	void SetPlayerRInfo(DWORD prevRID, WORD x, WORD y)
	{
		m_prevRegID = prevRID;
		m_prevX		= x;
		m_prevY		= y;
	}

	void SetPlayerID(DWORD gID) { m_playerID = gID; }
	DWORD GetPlayerID() { return m_playerID; }

public:
    static DWORD s_DynamicRegionNumber;

protected:
	DWORD	m_CopySceneGID;
	DWORD	m_playerID;				// ����˶�̬��������ҵ�GID
	
public:
	std::set<DWORD> m_Monster;		// �����ŵ�����һ�ξ���ʧ�Ĺ����б�
									// ������Ͻ��������Щ��һ�ξ���ʧ��
									// ���ﶼ��ʧ�ˣ�Ҳ���˳���̬����֮ʱ
public:
	DWORD	m_prevRegID;			// ���֮ǰ�ĳ���ID
	WORD	m_prevX;				// ����
	WORD	m_prevY;				
	WORD	m_DynamicIndex;			// ���ñ��е�����
	bool	m_CanMoveOut;			// �Ƿ���Դ���ȥ
	DWORD	m_MoveOutTimeSeg;		// ���ͻ�ȥ�ļ�ʱʱ���
	DWORD	m_MoveOutOT;			// �ͻ���ʱ
	DWORD	m_MoveOutTimeleft;			// �ͻ���ʱ
	BYTE			m_MoveOutTime;			// �ͻ���ʱ
	int     m_DyRegionEnd;
};
