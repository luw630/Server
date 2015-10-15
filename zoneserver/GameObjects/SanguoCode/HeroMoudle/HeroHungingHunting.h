// ----- CHeroHungingHunting.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   萌斗三国的武将挂机寻宝的玩法的逻辑管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;

class CBaseDataManager;
struct SHeroExtendsGamplayMsg;
struct HeroHungingHuntingMsg;
struct AnswerHeroHuntingApplyMsg;
struct AnswerSynSurprisePresentsMsg;
struct GameFeatureActivationConfig;
struct m_ptrSynSurprisePresentMsg;
struct HeroHungingHuntingCfg;

class CHeroHungingHunting
{
public:
	explicit CHeroHungingHunting(CBaseDataManager& baseDataManager);
	~CHeroHungingHunting();

	void DispachMsg(const SHeroExtendsGamplayMsg* pMsg);

private: 
	///@brief 客户端申请开始挂机寻宝
	void RequestHunging(const HeroHungingHuntingMsg* pMsg);
	///@brief 客户端申请掉宝
	void RequestSurprisePresents(const HeroHungingHuntingMsg* pMsg);
	///@brief 客户端申请挂机寻宝的收益结算
	void RequestLiquidation(const HeroHungingHuntingMsg* pMsg);
	///@brief 开始随机掉落宝物
	void RandomSurprise(int heroID, const HeroHungingHuntingCfg* config);

	CBaseDataManager& m_baseDataMgr;
	const GameFeatureActivationConfig& m_gameFeatureConfig;
	unique_ptr<AnswerHeroHuntingApplyMsg> m_ptrAllowToHungingHuntingMsg;///同意客户端某一个英雄开始挂机寻宝的消息
	unique_ptr<AnswerHeroHuntingApplyMsg> m_ptrEarningsLiquidationSucceedMsg;///通知客户端寻宝收益结算成功
	unique_ptr<AnswerSynSurprisePresentsMsg> m_ptrSynSurprisePresentMsg;///通知客户端本次寻宝掉落的物品
};

