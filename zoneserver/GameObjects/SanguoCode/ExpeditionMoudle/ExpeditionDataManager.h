// ----- CExpeditionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的过关斩将的相关数据的管理
// --------------------------------------------------------------------
//   --  通过管理存储在数据库中的过关斩将相关数据，来为过关斩将的相关的逻辑服务，实现业务层与数据层的分离
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>

struct SExpeditionData;
struct SHeroData;
struct CGlobalConfig;
struct ExpeditionInstanceInfor;
class TimerEvent_SG;

class CExpeditionDataManager :
	public CExtendedDataManager
{
public:
	CExpeditionDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CExpeditionDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();
	
	///@brief 检测玩家是否全部阵亡
	bool CheckCompletelyAnnihilated();

	///@brief 设置远征关卡是否通关的状态
	void SetConquredState(bool state);
	///@brief 设置是否奖励了武将的状态
	void SetHeroRewardState(bool state);
	///@brief 设置奖励是否被领取的状态
	void SetRewardsUnclaimedState(bool state);
	///@brief 设置远征激活的状态
	void SetExpeditionActiveState(bool state);
	///@brief 设置当前远征关卡的ID
	void SetCurExpeditionInstanceID(DWORD ID);
	///@brief 设置当前关卡的难度系数
	void SetCurLevelFactor(float factor);
	///@brief 花掉一张门票
	void CostATicket();
	///@brief 设置当前奖励的物品的ID
	void SetCurItemObtained(DWORD itemID);
	///@brief 重置远征敌人的数据信息
	void ResetEnemyInfor();
	///@brief 判断武将ID是否在已经记录的地方武将列表中
	bool IsEnemyExist(DWORD heroID);
	///@brief 将匹配好的敌人的ID压入到容器中
	void CacheMarchedEnmey();
	///@brief 将玩家选择的武将记录进来
	///@param selectedHero为选中的英雄的数组，个数为5
	void CacheSelectedHero(const DWORD* selectedHero);
	///@brief 设置敌人的血量比例值、能量比例值信息
	void SetEnemyInfor(DWORD heroID, float healthScale, float manaScale);
	///@brief 设置是否准许了进副本关卡
	void SetPermissionGotState(bool state);
	///@brief 设置敌人是否在匹配的状态
	void SetEnemyMarchingState(bool state);

	///获取许可证状态
	bool GetPermissionState();
	///@brief 获取敌人匹配的状态（是否在匹配中）
	bool GetEnemyMarchingState();
	///@brief 获取远征关卡通关的情况
	bool GetConquredState();
	///@brief 获取远征关卡是否奖励的英雄的状态
	bool GetHeroRewardState();
	///@brief 获取奖励是否被领取的状态
	bool GetRewardsUnclaimedState();
	///@brief 获取远征是否激活的状态
	bool GetExpeditionActiveState();
	///@brief 获取远征关卡的当前ID
	///@param data 要获取的数据信息
	///@return 是否成功取得所要的值
	bool GetCurExpeditionInstanceID(OUT DWORD& data);
	///@brief 获取当前远征关卡的票数
	///@param data 要获取的数据信息
	///@return 是否成功取得所要的值
	bool GetCurTicketsNum(OUT DWORD& data);
	///@brief 获取当前的获得的物品奖励的ID
	///@param data 要获取的数据信息
	///@return 是否成功取得所要的值
	bool GetCurItemObtained(OUT DWORD& data);
	///@brief 获取当前敌对玩家的武将的属性
	///@param arraySize 数组的大小
	SHeroData* GetCurHostileCharacterInfor();
	///@brief 检测某一个英雄ID是否是玩家刚刚选择了的
	bool IsHeroSelected(DWORD heroID);
	///@brief 获取当前的VIP等级对应的效益提升比例
	void CacheCurProceedsRiseState();
	///@brief 获得当前的会获得的金钱收益
	///@return 成功返回对应的值，失败放回-1
	int GetCurExpectedMoneyProceeds();
	///@brief 获得当前的会获得金钱收益
	///@return 成功返回对应的值，失败放回-1
	int GetCurExpectedExploitProceeds();

	///@brief 用来刷新远征关卡的票数
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	void RefreshTicket(int sendMsg = 0);

private:
	bool _checkExpeditionDataAvaliable();

	bool m_bPermissionGot;
	bool m_bEnemyMarching;
	int m_iProceedsRiseState; ///<最终获得收益提成的比例
	int m_dwCurMaxAvaliableTickets; ///<当前最多能重置多少次远征
	DWORD m_dwAvaliableHostileCharacterNum;
	SExpeditionData* m_ptrData;
	const CGlobalConfig& globalConfig;
	unordered_set<int> m_selectedHeroID;
	unordered_map<DWORD, SHeroData*> m_hostileCharacterList;
};

