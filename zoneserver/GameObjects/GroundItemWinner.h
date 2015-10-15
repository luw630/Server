#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
#include <vector>

class CItem;
class CPlayer;
class CSingleItem;

#define CI_MAXPLAYER (5)

// 每一个摇色子条目
struct ChangeItem
{
	ChangeItem() { memset(this, 0, sizeof(*this)); }

	CItem	*m_pItem;					// 包裹
	CSingleItem	*m_pSingleItem;					// 包裹
	CPlayer	*m_Player[CI_MAXPLAYER];	// 参与的玩家
	WORD	m_MaxPoint[CI_MAXPLAYER];	// 参与玩家的点数
	BYTE	m_Index;					// 包裹中的索引
	BYTE	m_InitPeople;				// 最初参加的人数
	DWORD	m_StartTime;				// 开始时间
};

// 包裹摇号系统
class CGroundItemWinner
{
private:
	CGroundItemWinner() {}

public:
	static CGroundItemWinner& GetInstance()
	{
		static CGroundItemWinner instance;
		return instance;
	}

	BOOL AddChanceItem(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index);		// 加入一个摇号
	BOOL AddChanceItem(std::vector<CPlayer *> &playerList, CSingleItem *pItem);		// 加入一个摇号

	BOOL AddDummyChance(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index);		// 发送一个废号
	BOOL AddDummyChance(std::vector<CPlayer *> &playerList, CSingleItem *pItem);		// 发送一个废号

	BOOL StartTakeChance(CPlayer *player, DWORD index, WORD playerIndex, bool IsGiveup);	// 玩家操作

	BOOL PlayerLogOut(CPlayer *player, DWORD index, WORD playerIndex);	// 玩家下线了
	BOOL GroundItemGone(CItem *pItem);									// 包裹消失了
	BOOL GroundItemGone(CSingleItem *pItem);									// 包裹消失了

	BOOL ChoiceTimeOut(DWORD index, ChangeItem *pChance);				// 摇号超时了
	BOOL FinishTakeChance(ChangeItem *pChance, DWORD index);			// 摇号结束

	BOOL Run();

private:
	typedef std::hash_map<DWORD, ChangeItem *> ChanceItems;

	ChanceItems m_ChanceItem;

private:
	static DWORD counter;
};

#pragma warning(pop)