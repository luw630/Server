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

	// 一般只在创建时设置
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
	DWORD	m_playerID;				// 放入此动态场景的玩家的GID
	
public:
	std::set<DWORD> m_Monster;		// 这里存放的是死一次就消失的怪物列表
									// 从设计上讲，如果这些死一次就消失的
									// 怪物都消失了，也是退出动态场景之时
public:
	DWORD	m_prevRegID;			// 玩家之前的场景ID
	WORD	m_prevX;				// 坐标
	WORD	m_prevY;				
	WORD	m_DynamicIndex;			// 配置表中的索引
	bool	m_CanMoveOut;			// 是否可以传回去
	DWORD	m_MoveOutTimeSeg;		// 传送回去的计时时间点
	DWORD	m_MoveOutOT;			// 送回延时
	DWORD	m_MoveOutTimeleft;			// 送回延时
	BYTE			m_MoveOutTime;			// 送回延时
	int     m_DyRegionEnd;
};
