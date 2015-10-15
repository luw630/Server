#pragma once
#include "../ExtendedDataManager.h"
#include <memory>
#include <functional>
//#include <vector>

class CBaseDataManager;
struct SChaseThiefData;
struct SThiefActivate;

typedef std::function<void()> EmptyFunction;

class CChaseThiefDataManager : public CExtendedDataManager
{
public:
	CChaseThiefDataManager(CBaseDataManager& baseDataMgr);
	~CChaseThiefDataManager();
	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();
	/**@brief 盗贼生效处理*/
	void TakeEffect();
	/**@brief 盗贼失效处理*/
	void LoseEffectiveness();
	/**@brief 检查有效性*/
	bool CheckEffectiveness();
	/**@brief 设置追击盗贼的武将*/
	void SetChasingHeroes(int heroArr[], int num);
	/**@brief 追击盗贼参战英雄*/
	const std::vector<int>& GetChasingHeroes() const;
	EmptyFunction& GetActivateFunc() { return m_funcActivate; }
	/**@brief 设置追击小偷标志*/
	void SetChasing(bool flag) { m_bChaseStarted = flag; }
	/**@brief 获取追击小偷标志*/
	bool GetChaseFlag() { return m_bChaseStarted; }
	/**@brief 随机战斗奖励*/
	void RandomReward();
	/**@brief 填充奖励数据
	@param [in] win 战斗结果
	@param [out] itemID 奖励物品ID
	@param [out] itemNum 奖励物品数量
	*/
	void FillReward(bool win, OUT int& itemID, OUT int& itemNum);
private:
	bool _ChaseThiefDataIsValid() const;
private:
	static DWORD m_dwChaseThiefDurationTime;	///<追击盗贼持续时间
	static DWORD m_dwChaseThiefOpenLevel;	///<追击盗贼开放等级
	static float m_fChaseThiefActivateRate;	///<追击盗贼触发几率
	SChaseThiefData *m_pChaseThiefData;
	__time64_t m_tEffectiveTime;	///<追击盗贼生效时间
	shared_ptr<SThiefActivate> m_pActivateMsg;	///<激活消息
	EmptyFunction m_funcActivate;
	bool m_bChaseStarted;	///<追击开始标志
	std::vector<int> m_vecChasingHeroes;	///<追击盗贼的武将
	int m_iRewardID;	///<奖励ID
	int m_iRewardNum;  ///<奖励数量
};

