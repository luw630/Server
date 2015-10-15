// ----- CHeroTraining.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   萌斗三国的武将训练玩法的逻辑管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;

class CBaseDataManager;
struct SHeroExtendsGamplayMsg;
struct HeroTrainingMsg;
struct GameFeatureActivationConfig;

class CHeroTraining
{
public:
	explicit CHeroTraining(CBaseDataManager& baseDataManager);
	~CHeroTraining();

	void DispachMsg(const SHeroExtendsGamplayMsg* pMsg);

private:
	///@brief 客户端申请进行某一个英雄的训练
	void RequestStartTraining(const HeroTrainingMsg* pMsg);
	///@brief 客户端申请进行训练的收益结算
	void RequestLiquidation(const HeroTrainingMsg* pMsg);

	CBaseDataManager& m_baseDataMgr;
	const GameFeatureActivationConfig& m_gameFeatureConfig;
	unique_ptr<HeroTrainingMsg> m_ptrAllowStartTrainingMsg;
	unique_ptr<HeroTrainingMsg> m_ptrEarningsLiquidationMsg;

	bool bTrainedFlag; ///是否有武将训练过
};

