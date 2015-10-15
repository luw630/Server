#include "stdafx.h"
#include"..\Common\ConfigManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "HeroTraining.h"
#include "..\BaseDataManager.h"
#include "HeroExtendsGameplayModel.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"

extern LPIObject GetPlayerBySID(DWORD dwStaticID);

CHeroTraining::CHeroTraining(CBaseDataManager& baseDataManager)
	:m_baseDataMgr(baseDataManager),
	m_gameFeatureConfig(CConfigManager::getSingleton()->GetGameFeatureActivationConfig())
{
	bTrainedFlag = false;
	m_ptrAllowStartTrainingMsg.reset(new HeroTrainingMsg());
	m_ptrAllowStartTrainingMsg->_protocol = HeroTrainingMsg::ANSWER_TRAINING_PERMISSION;

	m_ptrEarningsLiquidationMsg.reset(new HeroTrainingMsg());
	m_ptrEarningsLiquidationMsg->_protocol = HeroTrainingMsg::ANSWER_TRAINING_LIQUIDATION_RESULT;
}


CHeroTraining::~CHeroTraining()
{
}

void CHeroTraining::DispachMsg(const SHeroExtendsGamplayMsg* pMsg)
{
	const HeroTrainingMsg* ptrMsg = static_cast<const HeroTrainingMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	switch (ptrMsg->_protocol)
	{
	case HeroTrainingMsg::REQUEST_START_TRAINING:
		RequestStartTraining(ptrMsg);
		break;
	case HeroTrainingMsg::REQUEST_TRAINING_LIQUIDATION:
		RequestLiquidation(ptrMsg);
		break;
	}
}

void CHeroTraining::RequestStartTraining(const HeroTrainingMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	m_ptrAllowStartTrainingMsg->trainingFlag = 0;
	int heroID = pMsg->heroID;
	m_ptrAllowStartTrainingMsg->heroID = heroID;
	int masterLevel = m_baseDataMgr.GetMasterLevel();
	const HeroTrainingCfg* trainingCfg = CConfigManager::getSingleton()->GetHeroTrainingCfg(masterLevel);
	BOOL execResult = FALSE;
	lite::Variant ret;//从lua获取到的返回值
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroTrainingLimit && trainingCfg != nullptr)
	{
		///由于第一次训练不扣钱，故在此获取一次本次训练是否是该玩家第一次训练英雄
		if (!bTrainedFlag)
		{
			///获取该玩家以前是否训练过英雄的
			if (g_Script.PrepareFunction("get_Exdata"))
			{
				g_Script.PushParameter(m_baseDataMgr.GetSID());
				g_Script.PushParameter(SG_ExDataType::TrainedFlag);
				execResult = g_Script.Execute(&ret);
			}

			if (!execResult || ret.dataType == LUA_TNIL)
			{
				rfalse(2, 1, "ConsumeFinalDamageValue Faile");
				bTrainedFlag = true;
			}
			else
			{
				try
				{
					bTrainedFlag = (int)ret;
				}
				catch (lite::Xcpt &e)
				{
					rfalse(2, 1, e.GetErrInfo());
					bTrainedFlag = true;
				}
			}
		}

		auto findResult = trainingCfg->gainsAndCost.find((HeroTrainingType)pMsg->trainingFlag);
		if (findResult != trainingCfg->gainsAndCost.end() && (!bTrainedFlag || m_baseDataMgr.CheckGoods_SG(findResult->second.consumeType, 0, findResult->second.payment)))
		{
			///扣掉对应的训练消耗
			CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
			lite::Variant ret2;
			///此时pMsg中的trainingFlag代表用户是申请正常的训练速度，还是申请几倍速的训练速度
			if (g_Script.PrepareFunction("RequestStartTraining"))
			{
				g_Script.PushParameter(m_baseDataMgr.GetSID());
				g_Script.PushParameter(heroID);
				g_Script.PushParameter(pMsg->trainingFlag);
				g_Script.PushParameter(masterLevel);
				g_Script.PushParameter(config.HeroTrainingCountLimit);
				execResult = g_Script.Execute(&ret2);
			}

			if (!execResult || ret2.dataType == LUA_TNIL)
			{
				rfalse(2, 1, "ConsumeFinalDamageValue Faile");
				g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowStartTrainingMsg.get(), sizeof(HeroTrainingMsg));
				return;
			}

			try
			{
				m_ptrAllowStartTrainingMsg->trainingFlag = (BYTE)(int)(ret2);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				m_ptrAllowStartTrainingMsg->trainingFlag = 0;
				g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowStartTrainingMsg.get(), sizeof(HeroTrainingMsg));
				return;
			}

			///代表申请训练成功
			if (m_ptrAllowStartTrainingMsg->trainingFlag != 0)
			{
				///第一次训练不扣钱
				if (bTrainedFlag)
					m_baseDataMgr.DecGoods_SG(findResult->second.consumeType, 0, findResult->second.payment, GoodsWay::heroTraining);
				else
				{
					bTrainedFlag = true;
					if (g_Script.PrepareFunction("set_Exdata"))
					{
						g_Script.PushParameter(m_baseDataMgr.GetSID());
						g_Script.PushParameter(SG_ExDataType::TrainedFlag);
						g_Script.PushParameter(1);
						execResult = g_Script.Execute();
					}
				}
			}
		}
	}

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrAllowStartTrainingMsg.get(), sizeof(HeroTrainingMsg));
}

void CHeroTraining::RequestLiquidation(const HeroTrainingMsg* pMsg)
{
	if (pMsg == nullptr)
		return;

	m_ptrEarningsLiquidationMsg->trainingFlag = 0;///非0即为真
	int heroID = pMsg->heroID;
	int retValue = 0;
	m_ptrEarningsLiquidationMsg->heroID = heroID;
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	lite::Variant ret1;//从lua获取到的返回值
	lite::Variant ret2;//从lua获取到的返回值
	lite::Variant ret3;//从lua获取到的返回值
	BOOL execResult = FALSE;
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroTrainingLimit && pPlayer != nullptr)
	{
		int heroID = pMsg->heroID;
		CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		///获取开始该玩法时君主的等级
		if (g_Script.PrepareFunction("GetSpecifyExBaseData"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(HeroExDataType::cachedLevel);
			execResult = g_Script.Execute(&ret1);
		}

		if (!execResult || ret1.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
			return;
		}

		try
		{
			retValue = (int)ret1;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
			return;
		}

		const HeroTrainingCfg* trainingCfg = CConfigManager::getSingleton()->GetHeroTrainingCfg(retValue);
		///获取训练该武将的模式
		if (g_Script.PrepareFunction("GetSpecifyExBaseData"))
		{
			g_Script.PushParameter(m_baseDataMgr.GetSID());
			g_Script.PushParameter(heroID);
			g_Script.PushParameter(HeroExDataType::trainingType);
			execResult = g_Script.Execute(&ret2);
		}
		if (!execResult || ret2.dataType == LUA_TNIL)
		{
			rfalse(2, 1, "ConsumeFinalDamageValue Faile");
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
			return;
		}

		try
		{
			retValue = (int)ret2;
		}
		catch (lite::Xcpt &e)
		{
			rfalse(2, 1, e.GetErrInfo());
			g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
			return;
		}

		if (trainingCfg != nullptr)
		{
			g_Script.SetCondition(0, pPlayer, 0);
			///此时pMsg中的trainingFlag仅代表用户用没用钱来进行训练的结算
			if (g_Script.PrepareFunction("RequestTrainingLiquidation"))
			{
				g_Script.PushParameter(m_baseDataMgr.GetSID());
				g_Script.PushParameter(heroID);
				g_Script.PushParameter(pMsg->trainingFlag);
				g_Script.PushParameter(config.HeroTrainingNeedTime);
				g_Script.PushParameter(config.HeroTrainingUnitTime);
				g_Script.PushParameter(trainingCfg->costOfSpeedUp);
				execResult = g_Script.Execute(&ret3);
			}
			g_Script.CleanCondition();

			if (!execResult || ret3.dataType == LUA_TNIL)
			{
				rfalse(2, 1, "ConsumeFinalDamageValue Faile");
				g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
				return;
			}

			int count = 0;
			try
			{
				count = (int)ret3;///lua端返回的是可以进行多少个单位的结算
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
				return;
			}

			m_ptrEarningsLiquidationMsg->trainingFlag = count == -1 ? 0 : 1;
			auto findResult = trainingCfg->gainsAndCost.find((HeroTrainingType)retValue);
			if (count > 0 && findResult != trainingCfg->gainsAndCost.end())
			{
				///获得的经验值为，单位数 * 每10分钟获得的经验值 * 训练模式的加成比例
				count = floor(count * trainingCfg->expProceeds * findResult->second.bonusScale);
				int level = 0;
				bool levelLimitted = false;
				///将得到的经验值发给客户端
				if (m_baseDataMgr.PlusHeroExp(heroID, count, level, levelLimitted))
				{
					SAHeroUpgradeLevelResult resultMsg;
					resultMsg.m_HeroID = heroID;
					resultMsg.dwLevel = level;
					resultMsg.dwExperienceAmount = count;

					g_StoreMessage(m_baseDataMgr.GetDNID(), &resultMsg, sizeof(SAHeroUpgradeLevelResult));
				}
			}
		}
	}

	g_StoreMessage(m_baseDataMgr.GetDNID(), m_ptrEarningsLiquidationMsg.get(), sizeof(HeroTrainingMsg));
}
