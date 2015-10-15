#pragma once

#include "BaseObject.h"
#include "networkmodule\itemtypedef.h"
#include <vector>
const __int32 IID_ITEM = 0x1168eb14;

#define MakeLifeTime(m) ((m) * 60000)

#include <list>
#include <set>
using namespace std;

class CRegion;

class CItem : public IObject
{
public:
	struct SParameter
    {
		SParameter()
		{
			xTile			= 0;
			yTile			= 0;
			ItemsInPack		= 0;
			PackageModel	= IPM_ITEM;
			MoneyType		= 0;
			Money			= 0;
			ProtectedGID	= 0;
			ProtectTeamID	= 0;
			dwLife			= 0xffffffff;
			memset(pItem, 0, sizeof(pItem));
		}

		WORD	xTile;			// 坐标X
		WORD	yTile;			// 坐标Y
		WORD	ItemsInPack;	// 包裹道具个数
		WORD	PackageModel;	// 掉物模型
		WORD	MoneyType;		// 货币类型
		DWORD	Money;			// 金钱
        DWORD	ProtectedGID;	// 玩家保护		
        DWORD	ProtectTeamID;	// 队伍保护
        DWORD	dwLife;			// 生存时间，以毫秒计算，超时后该道具消失，为0xffffffff时说明该道具永远存在
		SItemBase *pItem[g_PackageMaxItem];	// 道具数据
    };

public:
	static BOOL GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CItem::SParameter &args, std::vector<SRawItemBuffer> &item, const LogInfo &log);
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
    CItem(void);
    ~CItem(void);

public:
	WORD					m_xTile;						// 坐标X
	WORD					m_yTile;						// 坐标Y
	WORD					m_ItemsInPack;					// 包裹中道具个数
	WORD					m_PackageModel;					// 包裹的模型（布袋，檀木箱，银口箱，黄金箱）
	WORD					m_Remain;						// 包裹中剩余的道具数（包括金钱）
    DWORD					m_dwGID;						// 包裹玩家保护
    DWORD					m_dwTeamID;						// 包裹队伍保护
	DWORD					m_dwSegTime;					// 道具产生时间
	DWORD					m_dwLifeTime;					// 生存时间（为0xffffffff时道具永远存在）
	DWORD					m_Money;						// 包裹中的货币量
	WORD					m_MoneyType;					// 掉落的货币类型
	SRawItemBuffer			m_GroundItem[g_PackageMaxItem];	// 道具数据
	BYTE					m_PickStatus[g_PackageMaxItem];	// 道具拾取规则
	BYTE					m_ExPosition[g_PackageMaxItem];	// 原始的位置
	class CArea				*m_pParentArea;					// 所在区域
	
	std::set<CPlayer *>		m_Checker;						// 当前的所有查看者
	std::set<DWORD>			m_ChanceItems;					// 包裹中摇号的数量

	bool					m_IsDispatched;					// 是否已分发

	int						m_IndexMask[g_PackageMaxItem];	// 一个快速索引

	std::vector<std::string> m_strItemlog;
};

inline void CItem::AddChecker(CPlayer *pChecker)
{
	m_Checker.insert(pChecker);
}

inline void CItem::RemoveChecker(CPlayer *pChecker)
{
	m_Checker.erase(pChecker);
}
