// ----- CDuplicateDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的副本及其所包含的关卡的数据管理
// --------------------------------------------------------------------
//   --  由于将关卡相关的数据下放到每个tollgate管理模块比较繁杂，故暂时统一都放到该类来管理
//--------------------------------------------------------------------- 
#pragma once
#include <memory>
#include <list>
#include <functional>
#include <unordered_map>
#include "..\ExtendedDataManager.h"
using namespace std;
typedef function<void(int)> FunctionPtr;

enum InstanceType;
///@brief 暂时用来刷新副本的挑战次数
struct SFixData;
struct SSanguoTollgate;
struct SSanguoStoryEliteTollgate;
struct SSanguoStoryTollgate;
class TimerEvent_SG;

///@brief 用于普通副本、精英副本中的关卡掉落的伪随机算法
struct BossItemMissedCounter
{
public:
	BossItemMissedCounter()
	{
		m_iRef = 5;
		m_iBossItem1RefCount = 0;
		m_iBossItem2RefCount = 0;
		m_iBossItem3RefCount = 0;
	}

	bool AddItem1RefenceCount()
	{
		++m_iBossItem1RefCount;
		if (m_iBossItem1RefCount >= m_iRef)
		{
			m_iBossItem1RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem1Counter()
	{
		m_iBossItem1RefCount = 0;
	}

	bool AddItem2RefenceCount()
	{
		++m_iBossItem2RefCount;
		if (m_iBossItem2RefCount >= m_iRef)
		{
			m_iBossItem2RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem2Counter()
	{
		m_iBossItem2RefCount = 0;
	}

	bool AddItem3RefenceCount()
	{
		++m_iBossItem3RefCount;
		if (m_iBossItem3RefCount >= m_iRef)
		{
			m_iBossItem3RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem3Counter()
	{
		m_iBossItem3RefCount = 0;
	}

protected:
	int m_iRef; ///<判断多少次错过BOSS掉落的物品后，就必掉对应的BOSS物品
private:
	int m_iBossItem1RefCount; ///<关卡掉落中的BOSS掉落物品的伪随机算法要用到的计数器
	int m_iBossItem2RefCount; ///<关卡掉落中的BOSS掉落物品的伪随机算法要用到的计数器
	int m_iBossItem3RefCount; ///<关卡掉落中的BOSS掉落物品的伪随机算法要用到的计数器
};

///@brief 错过普通故事副本BOSS掉落的物品的计数器
struct NormalBossItemMissedCounter : public BossItemMissedCounter
{
public:
	NormalBossItemMissedCounter()
		:BossItemMissedCounter()
	{
		m_iRef = 5;
	}
};

///@brief 错过精英副本BOSS掉落的物品的计数器
struct EliteBossItemMissedCounter : public BossItemMissedCounter
{
public:
	EliteBossItemMissedCounter()
		:BossItemMissedCounter()
	{
		m_iRef = 3;
	}
};

class CDuplicateDataManager :
	public CExtendedDataManager
{
public:
	CDuplicateDataManager(CBaseDataManager& baseDataManager);
	virtual ~CDuplicateDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 获取对应副本的挑战过的次数
	///@return 成功返回对应的值，失败暂时返回0
	///@note 现在暂时只有将神传说副本、秘境寻宝副本才会有副本挑战次数的现在，普通副本（又名故事副本）、精英副本都没有副本挑战次数的限制，只有关卡的挑战次数的限制
	int GetDuplicateChallengedTimes(InstanceType type, int duplicateID);
	///@brief 获取对应关卡的挑战分数、挑战次数信息
	///@param tollgateID 要查找的关卡的ID
	///@param starLevel[out] 读取当前的关卡挑战分数（没挑战过的话就为0）
	///@param challengeTime[out] 读取当前关卡的已挑战的次数
	void GetTollgateData(InstanceType type, int tollgateID, OUT int& starLevel, OUT int& challengeTime);
	///@brief 获取普通副本（又名故事副本）关卡的挑战分数、挑战次数、是否为第一次挑战的信息
	///@param tollgateID 要查找的故事副本关卡的ID
	///@param bFirstTime[out] 当前要查找的关卡是否是第一次挑战，用于第一次关卡掉落的判断
	///@param starLevel[out] 读取当前的关卡挑战分数（没挑战过的话就为0）
	///@param challengeTime[out] 读取当前关卡的已挑战的次数
	void GetStroyTollgateData(int tollgateID, OUT bool& bFirstTime, OUT int& starLevel, OUT int& challengeTime);
	///@brief 获取当前参战武将的的经验值的提升
	int GetCurHeroExpIncreasement();
	///@brief 更新对应关卡类型的挑战分数、挑战次数的信息
	///@param type 要跟新的关卡的类型，主要为普通副本（又名故事副本）关卡、精英副本关卡、将神传说副本关卡、秘境寻宝副本关卡这四个关卡
	///@param duplicateID 副本ID
	///@param tollgateID 要跟新的关卡的ID
	///@param starLevel 新的挑战的分数
	///@param challengeTime 新的挑战的次数
	void UpdateTollgateData(InstanceType type, int duplicateID, int tollgateID, int starLevel, int challengeTime);
	///@brief 用来更新玩家选择的英雄链表
	///@param heroNum 玩家选中的英雄的数量
	///@param heroArray 玩家选中的英雄的ID数组
	void UpdateSelectedHero(int heroNum, const int heroArray[]);
	///@brief 更新最新的普通副本（又名故事副本）关卡ID
	void UpdateLatestNormalTollgateID(int tollgateID);
	///@brief 更新最新的精英副本关卡ID
	void UpdateLatestEliteTollgateID(int tollgateID);
	///@brief 重置指定关卡的挑战次数信息,并记录已经重置的次数
	bool ResetSpecifyTollgateChallegedTimes(InstanceType instanceType, int tollgateID);
	///@brief 获取指定关卡今天已经重置过的次数
	///@return 成功返回对应的值，失败返回-1
	int GetSpecifyTollgateResetedTimes(InstanceType instanceType, int tollgateID);
	///@brief 获取最新的普通副本（又名故事副本）关卡ID
	int GetLatestNormalTollgateID();
	///@brief 获取最新的精英副本关卡ID
	int GetLatestEliteTollgateID();
	///@brief 获取选择的玩家的列表
	const list<int>& GetSelectedHeroList();
	///@brief 设置当前参战武将的的经验值的提升
	void SetHeroExpIncreasement(int value);
	///@brief 是否重置副本的相关数据的判断
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	void RefreshData(int sendMsg = 0);
	///@brief 获取玩家的错过普通副本关卡BOSS掉落物品的次数
	NormalBossItemMissedCounter* GetMissedBossItemDataInStoryDup() { return &m_sBossItemMissedCount; }
	///@brief 获取玩家的错过精英副本关卡BOSS掉落物品的次数
	EliteBossItemMissedCounter* GetMissedBossItemDataInEliteDup() { return &m_eBossItemMissedCount; }
	///@brief 获取上阵武将
	const list<int>& GetFightHeroes() const { return m_listSelectedHero; }
private:
	typedef unordered_map<int, int*>::iterator duplicateChallengedTimesItor;
	typedef unordered_map<int, SSanguoTollgate*>::iterator commonTollgateDataItor;
	typedef unordered_map<int, SSanguoStoryTollgate*>::iterator storyTollgateDataItor;
	typedef unordered_map<int, SSanguoStoryEliteTollgate*>::iterator storyEliteTollgateDataItor;

	int m_iLatestNormalTollgateID;	///<最新的普通副本（又名故事副本）关卡ID
	int m_iLatestEliteTollgateID;	///<最新的精英副本关卡的ID
	int m_iCurHeroExpGain;			///<当前的参战武将会获得的经验值
	NormalBossItemMissedCounter m_sBossItemMissedCount;///<记录了普通故事副本错过BOSS掉落的物品的次数
	EliteBossItemMissedCounter m_eBossItemMissedCount;///<记录了精英副本错过BOSS掉落物品的次数
	list<int> m_listSelectedHero;///<玩家选择的英雄的ID的链表
	SFixData* m_playerData;///<暂时用这个作为数据成员
	unordered_map<int, int*> m_leagueOfLegendDuplicateChallengedTiems; ///< 将神传说副本挑战次数信息
	unordered_map<int, int*> m_treasureHuntingDuplicateChallengedTimes; ///< 秘境寻宝副本挑战次数信息
	unordered_map<int, SSanguoStoryEliteTollgate*> m_eliteTollgateDatas; ///< 精英关卡数据
	unordered_map<int, SSanguoTollgate*> m_leagueOfLegendTollgateDatas; ///< 将神传说关卡数据
	unordered_map<int, SSanguoTollgate*> m_treasureHuntingTollgateDatas; ///< 秘境寻宝关卡数据
	unordered_map<int, SSanguoStoryTollgate*> m_normalTollgateDatas; ///<普通关卡数据
};