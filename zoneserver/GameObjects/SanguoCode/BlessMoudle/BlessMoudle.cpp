#include"stdafx.h"
#include"..\common\TabReader.h"
#include "..\common\ConfigManager.h"
#include "..\BaseDataManager.h"
#include "..\AchievementMoudle\RandomAchieveUpdate.h"
#include "..\MissionMoudle\CMissionUpdate.h"
#include "BlessMoudle.h"
#include"BlessDataMgr.h"
#include "BlessRandomPrize.h"
#include "Random.h"
#include "../GuideMoudle/GuideManager.h"
#include"ScriptManager.h"
//#include "DMainApp.h"
using namespace std;
BlessMoudle::BlessMoudle()
	:globalConfig(CConfigManager::getSingleton()->globalConfig)
{
	m_upRandomObject.reset(new CBlessRandomPrize(globalConfig.GetHeroForCashParam1, globalConfig.GetHeroForCashParam2));
}

BlessMoudle::~BlessMoudle()
{
	Release();
}

bool BlessMoudle::Release()
{
	return true;
}

void BlessMoudle::DispatchBlessMsg(CBlessDataMgr *pBlessDataMgr, SBlessMsg *pMsg)
{
	if (pBlessDataMgr == nullptr)
		return;

	SQRequstBless * pBlessMsg = (SQRequstBless*)pMsg;
	switch (pMsg->_protocol)
	{
	case SBlessMsg::MONEY_FREE_BLESS: //免费祈福
		_ProcessFreeMoneyBless(pBlessDataMgr); //免费铜钱祈福
		break;
	case SBlessMsg::MONEY_SINGLE_BLESS:
		_ProcessSingleMoneyBless(pBlessDataMgr); //单次铜钱祈福
		break;
	case SBlessMsg::MONEY_MUTIPLE_BLESS:
		_ProcessMutipleMoneyBless(pBlessDataMgr); //多次铜钱祈福
		break;
	case SBlessMsg::DIAMOND_FREE_BLESS:
		_ProcessFreeDiamondBless(pBlessDataMgr); //免费宝石祈福
		break;
	case SBlessMsg::DIAMOND_SINGLE_BLESS:
		_ProcessSingleDiamondBless(pBlessDataMgr); //单次宝石祈福
		break;
	case SBlessMsg::DIAMOND_MUTIPLE_BLESS:
		_ProcessMutipleDiamondBless(pBlessDataMgr); //多次宝石祈福
		break;
	default:
		break;
	}
}

int32_t BlessMoudle::_FirstMoneyBless()
{
	return 0;
}

int32_t BlessMoudle::_FirstDiamondBless()
{
	return 0;
}

void BlessMoudle::_ProcessFreeMoneyBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	if (pBlessDataMgr->IsFirstMonyBless()) //第一次祈福
	{
		pBlessDataMgr->DiaFirstMoneyBless();
		SASingleRequstBless singleMsg(SASingleRequstBless::MONEY_FREE_BLESS);
		singleMsg.m_dwProperty = baseDataMgr.GetMoney();
		singleMsg.m_dwPrizeID = globalConfig.FristBlessHero;
		pBlessDataMgr->SetMoneyFreeBlessCount(pBlessDataMgr->GetMoneyFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeMoneyDate(time(nullptr));
		g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
		baseDataMgr.AddGoods_SG(GoodsType::hero, singleMsg.m_dwPrizeID, 1, GoodsWay::bless, false);
		/// 首次金钱祈福 引导管理类通知客户端前往下一步指引
		baseDataMgr.ProcessOperationOfGuide(FunctionMoudleType::Function_Bless);
		return;
	}

	if (pBlessDataMgr->GetMoneyFreeBlessCount() > 0 //剩余的祈福次数
		&& time(nullptr) - pBlessDataMgr->GetLastFreeMoneyBlessDate() >= globalConfig.BlessFreeTimeForMoney) //如果上次免费祈福时间大于5MIN
	{
		pBlessDataMgr->SetMoneyFreeBlessCount(pBlessDataMgr->GetMoneyFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeMoneyDate(time(nullptr));
	}
	else //单词祈福校验失败 直接返回
	{
		return;
	}
	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //调用祈福随机获取物品接口
	pBlessDataMgr->AddUpMoneyBlessCount(); //祈福次数累加
	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetMoneyBlessCount(); //重置祈福次数
	if (objectType == BlessObjectType::Hero)
		baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
	else
		baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);
	SASingleRequstBless singleMsg(SASingleRequstBless::MONEY_FREE_BLESS);
	singleMsg.m_dwProperty = baseDataMgr.GetMoney();
	singleMsg.m_dwPrizeID = prizeID;
	g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}


void BlessMoudle::_ProcessSingleMoneyBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	if (baseDataMgr.GetMoney() < globalConfig.LotteryOnceMoney)// ( 付费祈福有足够金钱)
		return;

	//baseDataMgr.PlusMoney(-globalConfig.LotteryOnceMoney); //扣钱
	baseDataMgr.DecGoods_SG(GoodsType::money, 0, globalConfig.LotteryOnceMoney, GoodsWay::bless);

	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //调用祈福随机获取物品接口
	pBlessDataMgr->AddUpMoneyBlessCount(); //祈福次数累加
	
	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetMoneyBlessCount(); //重置祈福次数
	
	if (objectType == BlessObjectType::Hero)
		baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
	else
		baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);

	SASingleRequstBless singleMsg(SASingleRequstBless::MONEY_SINGLE_BLESS);
	singleMsg.m_dwProperty = baseDataMgr.GetMoney();
	singleMsg.m_dwPrizeID = prizeID;
	g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}

void BlessMoudle::_ProcessMutipleMoneyBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	if (baseDataMgr.GetMoney() < globalConfig.LotteryTenTimesMoney)//多次祈福需要的金钱是否满足
		return;
	//baseDataMgr.PlusMoney(-globalConfig.LotteryTenTimesMoney);
	baseDataMgr.DecGoods_SG(GoodsType::money, 0, globalConfig.LotteryTenTimesMoney, GoodsWay::bless);
	//循环祈福逻辑
	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	SARequstMutipleBless msg(SARequstMutipleBless::MONEY_MUTIPLE_BLESS); //返回客户端消息


	for (int count = 0; count < MUTIPL_BLEES; count++)
	{
		prizeID = -1;
		objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //调用祈福随机获取物品接口
		pBlessDataMgr->AddUpMoneyBlessCount(); //累加
		//获取物品为蓝色以上品质，则重置祈福次数至0
		if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
			pBlessDataMgr->ResetMoneyBlessCount();
		
		if (objectType == BlessObjectType::Hero) //添加新的祈福物品
			baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
		else
			baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);

		msg.m_dwPrizeIDs[count] = prizeID;
	}
	msg.m_dwProperty = baseDataMgr.GetMoney();
	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SARequstMutipleBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve(MUTIPL_BLEES);
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(MUTIPL_BLEES);
}


void BlessMoudle::_ProcessFreeDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	if (pBlessDataMgr->IsFirstDiamondBless())
	{
		SASingleRequstBless singleMsg(SASingleRequstBless::DIAMOND_FREE_BLESS);
		singleMsg.m_dwProperty = baseDataMgr.GetDiamond();
		singleMsg.m_dwPrizeID = globalConfig.SecondBlessHero;
		pBlessDataMgr->SetDiamondFreeBlessCount(pBlessDataMgr->GetDiamondFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeDiamondDate(time(nullptr));
		pBlessDataMgr->DidFirstDiamondBless();
		baseDataMgr.AddGoods_SG(GoodsType::hero, singleMsg.m_dwPrizeID, 1, GoodsWay::bless, false);
		g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
		/// 首次元宝祈福 引导管理类通知客户端前往下一步指引
		baseDataMgr.ProcessOperationOfGuide(FunctionMoudleType::Function_Bless);
		return;
	}

	if (time(nullptr) - pBlessDataMgr->GetLastFreeDiamondBlessDate() >= globalConfig.BlessFreeTime) //-免费时间是否满足
	{
		//pBlessDataMgr->SetDiamondFreeBlessCount(pBlessDataMgr->GetDiamondFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeDiamondDate(time(nullptr));
	}
	else  //不满足免费条件直接返回
	{
		return;
	}

	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID); //调用祈福随机获取物品接口
	pBlessDataMgr->AddUpDiamondBlessCount(); //祈福次数累加

	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetDiamondBlessCount(); //重置祈福次数

	if (objectType == BlessObjectType::Hero)
		baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
	else
		baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);

	SASingleRequstBless singleMsg(SASingleRequstBless::DIAMOND_FREE_BLESS);
	singleMsg.m_dwProperty = baseDataMgr.GetDiamond();
	singleMsg.m_dwPrizeID = prizeID;
	g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}

void BlessMoudle::_ProcessSingleDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类

	if (baseDataMgr.GetDiamond() < globalConfig.LotteryOnceGold) //( 付费祈福有足够金钱)
		return;
	//baseDataMgr.PlusDiamond(-globalConfig.LotteryOnceGold); //扣钱
	baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, globalConfig.LotteryOnceGold, GoodsWay::bless);
	int32_t prizeID = -1;
	if (pBlessDataMgr->IsFirstSingleDiamondBless()) //第一次元宝祈福 获取三星武将
	{
		pBlessDataMgr->DidFirstSingleDiamondBless();
		prizeID = m_upRandomObject->RandomThreeStarsHero();
		baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
	}
	else //第一次祈福后的普通祈福逻辑
	{
		BlessObjectType objectType = BlessObjectType::None;
		objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID); //调用祈福随机获取物品接口
		pBlessDataMgr->AddUpDiamondBlessCount(); //祈福次数累加

		if (objectType == BlessObjectType::Hero)
			pBlessDataMgr->ResetDiamondBlessCount(); //重置祈福次数

		if (objectType == BlessObjectType::Hero)
			baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
		else
			baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);
	}

	SASingleRequstBless singleMsg(SASingleRequstBless::DIAMOND_SINGLE_BLESS);
	singleMsg.m_dwProperty = baseDataMgr.GetDiamond();
	singleMsg.m_dwPrizeID = prizeID;
	g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);

}

void BlessMoudle::_ProcessMutipleDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	
	int lastHeroIndex = -1;
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //获取基础数据管理类
	if (baseDataMgr.GetDiamond() < globalConfig.LotteryTenTimesGold) //( 付费祈福有足够金钱)
		return;

	lite::Variant ret;//从lua获取多重祈福次数
	LuaFunctor(g_Script, "get_Exdata")[baseDataMgr.GetSID()][SG_ExDataType::MultiDiamondBlessCount](&ret);
	int allBlessCount = (int)ret; 
	lite::Variant ret2;//从lua获取多重祈福次数
	LuaFunctor(g_Script, "get_Exdata")[baseDataMgr.GetSID()][SG_ExDataType::BlessMaxThreeStarHeroCount](&ret2);//获取的到三星武将的次数条件
	int conditionCount = (int)ret2;

	//baseDataMgr.PlusDiamond(- globalConfig.LotteryTenTimesGold); //扣钱
	baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, globalConfig.LotteryTenTimesGold, GoodsWay::bless);

	SARequstMutipleBless msg(SARequstMutipleBless::DIAMOND_MUTIPLE_BLESS); //返回客户端消息


	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	int heroNum = 0;
	bool cancelBlessHero = false; //取消祈福获取武将接口
	bool onlyOneStarHero = true; //记录是否只获取1星武将
	int heroPrizeIndex = 0; //武将奖品的下标
	for (int32_t blessCount = 0; blessCount < MUTIPL_BLEES; blessCount++)
	{
		objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID, cancelBlessHero); //调用祈福随机获取物品接口
		pBlessDataMgr->AddUpDiamondBlessCount(); //祈福次数累加
		if (objectType == BlessObjectType::Hero) //由于武将需要做特别处理，所以不马上把武将加到英雄背包中
		{
			pBlessDataMgr->ResetDiamondBlessCount(); //重置祈福累计次数
			if (++heroNum >= 4)
				cancelBlessHero = true;

			if (allBlessCount <= conditionCount) //如果满足三星武将必出条件
			{
				if (onlyOneStarHero == true)
				{
					prizeID = m_upRandomObject->RandomThreeStarsHero();
					onlyOneStarHero = false;
				}
				else if (_GetHeroStarLevel(prizeID) == 3)
				{
					prizeID = m_upRandomObject->RandomTwoStarsHero();
				}
			}
			else if (onlyOneStarHero == true && _GetHeroStarLevel(prizeID) <= 1) //如果当前只有1星武将
			{
				prizeID = m_upRandomObject->RandomTwoStarsHero();
				onlyOneStarHero = false;
			}

			baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);

		}
		else
		{
			baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);
		}
		msg.m_dwPrizeIDs[blessCount] = prizeID;
	}
	_ProcessMutipleDiamondResult(msg.m_dwPrizeIDs);
	LuaFunctor(g_Script, "set_Exdata")[baseDataMgr.GetSID()][1][allBlessCount+1]();
	msg.m_dwProperty = baseDataMgr.GetDiamond();
	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SARequstMutipleBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///更新相关的成就完成状态
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve(MUTIPL_BLEES);
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(MUTIPL_BLEES);
}




const int32_t  BlessMoudle::_GetHeroStarLevel(const int32_t heroID)
{
	 const HeroConfig* heroConfig = CConfigManager::getSingleton()->GetHeroConfig(heroID);
	 if (heroConfig == nullptr)
		 return -1;
	 return heroConfig->Star;
}

void BlessMoudle::_ProcessMutipleDiamondResult(DWORD *prizeArray) //这里奖品做了随机处理
{
	int tempIndex = 0;
	int tempPrize;
	for (int i = 0; i < 5; i++)
	{
		tempIndex = CRandom::RandRange(i + 1, 9);
		tempPrize = prizeArray[tempIndex];
		prizeArray[tempIndex] = prizeArray[i];
		prizeArray[i] = tempPrize;
	}
	
}
