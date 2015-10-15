// ----- CTollgateDrop.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的关卡的掉落计算
// --------------------------------------------------------------------
//   --  管理某一个关卡的掉落计算
//---------------------------------------------------------------------   
#pragma once
#include <unordered_map>
#include <vector>

struct BattleDropConfig;
struct BossItemMissedCounter;

class CTollgateDrop
{
public:
	CTollgateDrop();
	~CTollgateDrop();

	void Init(const BattleDropConfig* dropConfig);
	///@brief 计算掉落的物品
	///@param countData 玩家错过BOSS物品掉落的次数信息
	///@param singleDrop 是否每个物品只会掉落一次
	///@param bMopUpOperation 是否为扫荡操作
	void CalculateDropItem(BossItemMissedCounter* countData, bool singleDrop = false, bool bMopUpOperation = false);
	///@brief 暂时专用于第一次关卡掉落中往链表添加东西
	void AddBossDropedItem(int itemID, int itemNum);
	///@brief 重置上一次的物品掉落信息
	void Reset();
	///@brief 获取普通物品掉落列表
	const unordered_map<int, int>& GetCommonDropItemList();
	///@brief 获取BOSS掉落的物品列表
	const unordered_map<int, int>& GetBossDropItemList();

protected:
	///@brief 进行物品的随机掉落计算
	///@param dropNum 要掉落几个
	///@param itemNum itemList的size
	///@param itemList 从该容器中随机一些物品作为掉落物品
	///@param stepIndex 随机中用到的记录上一次在容器中随机出的物品的位置，往后随机步进几位后得到新的随机的物品的位置并返回出来，为的是尽可能的避免singleDrop为false的时候同一个物品掉落多次的情况
	///@param dropedItemNum 已经掉落了多少个物品，随机完了后会被修改到对应的个数并返回出来
	///@param singleDrop 是否允许同一个物品掉落dropNum要求的个数
	void RandomItem(int dropNum, int itemNum, const vector<int>* itemList, OUT int& stepIndex, OUT int& dropedItemNum, bool singleDrop = false);

private:
	int m_iMinNumber;       ///最小掉落数
	int m_iMaxNumber;       ///最大掉落数
	int m_iWhiteItemWeight;     ///白色物品掉落权重
	int m_iGreenItemWeight;     ///绿色物品掉落权重
	int m_iBlueItemWeight;      ///蓝色物品掉落权重
	int m_iPurpleItemWeight;    ///紫色物品掉落权重
	int m_iBossItem1;
	int m_iBossItemNumber1;
	int m_iBossItemDropPercent1;
	int m_iBossItem2;
	int m_iBossItemNumber2;
	int m_iBossItemDropPercent2;
	int m_iBossItem3;
	int m_iBossItemNumber3;
	int m_iBossItemDropPercent3;
	const vector<int>* m_whiteItemList;
	const vector<int>* m_greenItemList;
	const vector<int>* m_blueItemList;
	const vector<int>* m_purpleItemList;
	const vector<int>* m_propItem;		///<扫荡会掉落的物品ID列表
	const vector<int>* m_propItemNum;	///<扫荡会掉落的物品对应的个数
	unordered_map<int, int> m_commonDropItemIdList; ///普通最终掉落物品ID列表,key为物品ID，value为物品数量
	unordered_map<int, int> m_bossDropItemIdList;   ///boss最终掉落物品ID列表,key为物品ID，value为物品数量
};

