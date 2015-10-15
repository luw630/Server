#pragma once
/********************************************************************
	created:	2012/02/13
	created:	13:2:2012   16:46
	filename: 	e:\GameServer\区域服务器\GameObjects\SingleItem.h
	file path:	e:\GameServer\区域服务器\GameObjects
	file base:	SingleItem
	file ext:	h
	author:		luwei
	
	purpose:	单个物品的掉落，现在掉落物品直接掉落，不在掉落包裹
*********************************************************************/
#include "BaseObject.h"
#include "networkmodule\itemtypedef.h"
#include <vector>
const __int32 IID_SINGLEITEM = 0x117d82c3; 

#define MakeLifeTime(m) ((m) * 60000)

#include <list>
#include <set>
using namespace std;

class CRegion;

class CSingleItem : public IObject
{
public:
	struct SParameter
    {
		SParameter()
		{
			xTile			= 0;
			yTile			= 0;
			ItemsNum		= 0;
			PackageModel	= IPM_ITEM;
			ProtectedGID	= 0;
			ProtectTeamID	= 0;
			dwLife			= 0xffffffff;
			ItemsID = 0 ;
			pItem = 0 ;
			MoneyType = 0;
		}

		WORD	xTile;			// 坐标X
		WORD	yTile;			// 坐标Y
		WORD	ItemsNum;	// 道具个数
		WORD	PackageModel;	// 掉物模型
 		WORD	MoneyType;		// 货币类型
// 		DWORD	Money;			// 金钱
        DWORD	ProtectedGID;	// 玩家保护		
        DWORD	ProtectTeamID;	// 队伍保护
        DWORD	dwLife;			// 生存时间，以毫秒计算，超时后该道具消失，为0xffffffff时说明该道具永远存在
		SItemBase *pItem;	// 道具数据
		DWORD	ItemsID;	//道具ID
    };

public:
	static BOOL GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CSingleItem::SParameter &args, SRawItemBuffer &item, const LogInfo &log);
	static void SaveAllLog();
public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
	void OnClose();

	void RemoveMe();
    struct SAAddGroundItemMsg *GetStateMsg();
	
	void AddChecker(CPlayer *pChecker);
	void RemoveChecker(CPlayer *pChecker);
	void UpdateItemPicked(WORD index);

	// 包裹分配
	BOOL StartDispatch(CPlayer *pChecker);

	

protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter);

public:
    CSingleItem(void);
    ~CSingleItem(void);

public:
	WORD					m_xTile;						// 坐标X
	WORD					m_yTile;						// 坐标Y
	//WORD					m_ItemsInPack;					// 包裹中道具个数
	WORD					m_ItemsNum;					// 道具个数

	WORD					m_PackageModel;					// 包裹的模型（布袋，檀木箱，银口箱，黄金箱）
	//WORD					m_Remain;						// 包裹中剩余的道具数（包括金钱）
    DWORD					m_dwGID;						// 包裹玩家保护
    DWORD					m_dwTeamID;						// 包裹队伍保护
	DWORD					m_dwSegTime;					// 道具产生时间
	DWORD					m_dwLifeTime;					// 生存时间（为0xffffffff时道具永远存在）
	DWORD					m_ItemsID;						//道具在道具表中的ID。
	//DWORD					m_Money;						// 包裹中的货币量
	WORD					m_MoneyType;					// 掉落的货币类型
	SRawItemBuffer			m_GroundItem;	// 道具数据
	BYTE					m_PickStatus;	// 道具拾取规则
	BYTE					m_ExPosition;	// 原始的位置
	class CArea				*m_pParentArea;					// 所在区域
	
	std::set<CPlayer *>		m_Checker;						// 当前的所有查看者
	std::set<DWORD>			m_ChanceItems;					// 包裹中摇号的数量

	bool					m_IsDispatched;					// 是否已分发

	int						m_IndexMask;	// 一个快速索引

	std::vector<std::string> m_strItemlog;
};

inline void CSingleItem::AddChecker(CPlayer *pChecker)
{
	std::set<CPlayer *>::iterator iter = m_Checker.find(pChecker);
	if (iter == m_Checker.end()){
		m_Checker.insert(pChecker);
	}
}

inline void CSingleItem::RemoveChecker(CPlayer *pChecker)
{
	std::set<CPlayer *>::iterator iter = m_Checker.find(pChecker);
	if (iter != m_Checker.end()){
		m_Checker.erase(iter);
	}
	//m_Checker.erase(pChecker);
}
