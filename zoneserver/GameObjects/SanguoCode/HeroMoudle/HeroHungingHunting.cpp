#include "stdafx.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "HeroHungingHunting.h"
#include "..\BaseDataManager.h"
#include "HeroExtendsGameplayModel.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
#include "..\..\Random.h"

CHeroHungingHunting::CHeroHungingHunting(CBaseDataManager& baseDataManager)
	:m_baseDataMgr(baseDataManager),
	m_gameFeatureConfig(CConfigManager::getSingleton()->GetGameFeatureActivationConfig())
{
	m_ptrAllowToHungingHuntingMsg.reset(new AnswerHeroHuntingApplyMsg(HeroHungingHuntingMsg::ANSWER_HUNTING_PERMISSION));
	m_ptrEarningsLiquidationSucceedMsg.reset(new AnswerHeroHuntingApplyMsg(HeroHungingHuntingMsg::ANSWER_HUNTING_LIQUIDATION_RESULT));
	m_ptrSynSurprisePresentMsg.reset(new AnswerSynSurprisePresentsMsg());
}

CHeroHungingHunting::~CHeroHungingHunting()
{
}

void CHeroHungingHunting::DispachMsg(const SHeroExtendsGamplayMsg* pMsg)
{
	const HeroHungingHuntingMsg* ptrMsg = static_cast<const HeroHungingHuntingMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	switch (ptrMsg->_protocol)
	{
	case HeroHungingHuntingMsg::REQUEST_START_HUNTING:
		RequestHunging(ptrMsg);
		break;
	case HeroHungingHuntingMsg::REQUEST_HUNTING_LIQUIDATION:
		RequestLiquidation(ptrMsg);
		break;
	case HeroHungingHuntingMsg::REQUEST_SURPRISE_PRESENTS:
		RequestSurprisePresents(ptrMsg);
		break;
	}
}

void CHeroHungingHunting::RequestHunging(const HeroHungingHuntingMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	m_ptrAllowToHungingHuntingMsg->bSucceed = false;
	m_ptrAllowToHungingHuntingMsg->heroID = pMsg->heroID;
	lite::Variant ret;//从lua获取到的返回值
	BOOL execResult = FALSE;
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroHungingHuntingLimit)
	{
		CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		int heroID = pMsg->heroID;
		if (g_Script.PrepareFunction("StartHunging"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(config.HeroHuntingCountLimit);
			g_Script.PushParameter(m_baseDataMgr.GetMasterLevel());
			///获取申请挂机寻宝成功与否
			execResult = g_Script.Execute(&ret);
		}

		if (!execResult || ret.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowToHungingHuntingMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		int succeedState = 0;
		try
		{
			succeedState = (int)ret;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowToHungingHuntingMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		m_ptrAllowToHungingHuntingMsg->bSucceed = succeedState != 0 ? true : false;
	}

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowToHungingHuntingMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
}

void CHeroHungingHunting::RequestSurprisePresents(const HeroHungingHuntingMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	int heroID = pMsg->heroID;
	int retValue = 0;
	BOOL execResult = FALSE;
	lite::Variant ret1;//从lua获取到的返回值
	lite::Variant ret2;//从lua获取到的返回值
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroHungingHuntingLimit)
	{
		if (g_Script.PrepareFunction("GetSpecifyExBaseData"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(HeroExDataType::cachedLevel);
			///获取开始该玩法时君主的等级
			execResult = g_Script.Execute(&ret1);
		}

		if (!execResult || ret1.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			return;
		}

		try
		{
			retValue = (int)ret1;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			return;
		}

		const HeroHungingHuntingCfg* hungingHuntingCfg = CConfigManager::getSingleton()->GetHeroHungingHuntingCfg(retValue);
		if (hungingHuntingCfg != nullptr)
		{
			CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
			int maxRequestTimes = config.HeroHuntingTime / config.HeroHuntingSurpriseUnitTime;
			int heroID = pMsg->heroID;
			if (g_Script.PrepareFunction("RequestSurprisePresents"))
			{
				g_Script.PushParameter(m_baseDataMgr.GetSID());
				g_Script.PushParameter(heroID);
				g_Script.PushParameter(maxRequestTimes);
				g_Script.PushParameter(config.HeroHuntingSurpriseUnitTime);
				///获取开始该玩法时君主的等级
				execResult = g_Script.Execute(&ret2);
			}

			if (!execResult || ret2.dataType == LUA_TNIL)
			{
				rfalse(2, 1, "ConsumeFinalDamageValue Faile");
				return;
			}

			try
			{
				retValue = (int)ret2;
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return;
			}

			if (retValue > 0)
			{
				///开始掉落会掉落的物品
				for (int i = 0; i < retValue; ++i)
					RandomSurprise(heroID, hungingHuntingCfg);

				g_Script.CleanCondition();
				return;
			}
		}
	}

	m_ptrSynSurprisePresentMsg->heroID = heroID;
	m_ptrSynSurprisePresentMsg->itemType = 0;
	m_ptrSynSurprisePresentMsg->itemID = 0;
	m_ptrSynSurprisePresentMsg->itemNum = 0;
	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrSynSurprisePresentMsg.get(), sizeof(AnswerSynSurprisePresentsMsg));
}

void CHeroHungingHunting::RequestLiquidation(const HeroHungingHuntingMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	m_ptrEarningsLiquidationSucceedMsg->bSucceed = false;
	m_ptrEarningsLiquidationSucceedMsg->heroID = pMsg->heroID;
	int heroID = pMsg->heroID;
	int retValue = 0;
	BOOL execResult = FALSE;
	lite::Variant ret1;//从lua获取到的返回值
	lite::Variant ret2;//从lua获取到的返回值
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroHungingHuntingLimit)
	{
		///获取开始该玩法时君主的等级
		if (g_Script.PrepareFunction("GetSpecifyExBaseData"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(HeroExDataType::cachedLevel);
			///获取开始该玩法时君主的等级
			execResult = g_Script.Execute(&ret1);
		}
		if (!execResult || ret1.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationSucceedMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		try
		{
			retValue = (int)ret1;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationSucceedMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		if (g_Script.PrepareFunction("RequestHungingLiquidation"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(config.HeroHuntingTime);
			g_Script.PushParameter(config.HeroHuntingGoldClearingUnitTime);
			///获取开始该玩法时君主的等级
			execResult = g_Script.Execute(&ret2);
		}
		if (!execResult || ret2.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationSucceedMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		int liquidationResult = 0;
		try
		{
			//lua端返回的是过了多少个时间单位
			liquidationResult = (int)ret2;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationSucceedMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
			return;
		}

		const HeroHungingHuntingCfg* hungingHuntingCfg = CConfigManager::getSingleton()->GetHeroHungingHuntingCfg(retValue);
		if (hungingHuntingCfg != nullptr)
		{
			if (liquidationResult > 0)
				m_baseDataMgr.AddGoods_SG(GoodsType::money, 0, liquidationResult * hungingHuntingCfg->fixedProceeds, GoodsWay::hungingHunting);

			m_ptrEarningsLiquidationSucceedMsg->bSucceed = true;
		}
	}

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationSucceedMsg.get(), sizeof(AnswerHeroHuntingApplyMsg));
}

void CHeroHungingHunting::RandomSurprise(int heroID, const HeroHungingHuntingCfg* config)
{
	m_ptrSynSurprisePresentMsg->heroID = heroID;
	m_ptrSynSurprisePresentMsg->itemType = 0;
	m_ptrSynSurprisePresentMsg->itemID = 0;
	m_ptrSynSurprisePresentMsg->itemNum = 0;

	int randomResult = CRandom::RandRange(1, 100);
	///如果不会掉落东西，则退出
	if (config == nullptr || randomResult > config->fLuckyOdds)
	{
		g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrSynSurprisePresentMsg.get(), sizeof(AnswerSynSurprisePresentsMsg));
		return;
	}

	randomResult = CRandom::RandRange(1, config->totalWeight);
	if (randomResult <= config->goldWeight)
	{
		m_ptrSynSurprisePresentMsg->itemType = GoodsType::money;
		m_ptrSynSurprisePresentMsg->itemNum = config->goldProceeds;
	}
	else if (randomResult <= config->goldWeight + config->diamondWeight)
	{
		m_ptrSynSurprisePresentMsg->itemType = GoodsType::diamond;
		m_ptrSynSurprisePresentMsg->itemNum = config->diamondProceeds;
	}
	else if (randomResult <= config->goldWeight + config->diamondWeight + config->propsWeight)
	{
		m_ptrSynSurprisePresentMsg->itemType = GoodsType::item;
		m_ptrSynSurprisePresentMsg->itemNum = 1;
		m_ptrSynSurprisePresentMsg->itemID = config->extraPropsProceeds[CRandom::RandRange(0, config->extraPropsProceeds.size() - 1)];
	}
	else if (randomResult <= config->goldWeight + config->diamondWeight + config->propsWeight + config->equipWeight)
	{
		m_ptrSynSurprisePresentMsg->itemType = GoodsType::item;
		m_ptrSynSurprisePresentMsg->itemNum = 1;
		m_ptrSynSurprisePresentMsg->itemID = config->extraEqupProceeds[CRandom::RandRange(0, config->extraEqupProceeds.size() - 1)];
	}
	else if (randomResult <= config->goldWeight + config->diamondWeight + config->propsWeight + config->equipWeight + config->debrisWeight)
	{
		m_ptrSynSurprisePresentMsg->itemType = GoodsType::item;
		m_ptrSynSurprisePresentMsg->itemNum = 1;
		m_ptrSynSurprisePresentMsg->itemID = config->extraDebrisProceeds[CRandom::RandRange(0, config->extraDebrisProceeds.size() - 1)];
	}

	m_baseDataMgr.AddGoods_SG(m_ptrSynSurprisePresentMsg->itemType, m_ptrSynSurprisePresentMsg->itemID, m_ptrSynSurprisePresentMsg->itemNum, GoodsWay::hungingHunting);
	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrSynSurprisePresentMsg.get(), sizeof(AnswerSynSurprisePresentsMsg));
}
