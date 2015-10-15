#pragma once

#include "Networkmodule\ItemTypedef.h"
#include <vector>
// 怪物掉落配置信息
class SMonsterDropTable
{
public:
	SMonsterDropTable() { memset(this, 0, sizeof(SMonsterDropTable)); }

public:
	WORD	m_MonsterID;				// 怪物ID
	WORD	m_MoneyType;				// 掉落货币类型
	DWORD	m_MoneyMount;				// 掉落货币量
	WORD	m_MoneyRate;				// 掉落货币几率
	DWORD	m_DropTable[10][4];			// 掉落物品数组（最多10个物品）
};


// 特殊掉落配置信息
class SMonsterDropSpecial
{
public:
	SMonsterDropSpecial() { memset(this, 0, sizeof(SMonsterDropSpecial)); }

public:
	DWORD				m_DropTime;					// 掉落时间间隔
	std::vector<WORD>	m_vGoodsID;					// 物品ID
	std::map<WORD,WORD>	m_vGoods;					// 物品ID与掉落几率
	std::vector<WORD>	m_vMonsterID;				// 怪物ID
	DWORD				g_DropCount;				// 全部掉落数量
	BYTE				m_DropType;					// 掉落类型保留
};

///玩家掉落装备几率表
struct PlayerDropTable
{
	enum
	{
		STATE_DROPITEM_BODY,//掉落身上装备
		STATE_DROPITEM_BAG,//掉落背包装备
		STATE_DROPITEM_MONEY,//掉落金钱
	};
	WORD m_wDropItem_body;			//身上装备掉落机率
	WORD m_wDropItem_bag;			//背包装备掉落机率

};