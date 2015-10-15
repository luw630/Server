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
	lite::Variant ret;//��lua��ȡ���ķ���ֵ
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroTrainingLimit && trainingCfg != nullptr)
	{
		///���ڵ�һ��ѵ������Ǯ�����ڴ˻�ȡһ�α���ѵ���Ƿ��Ǹ���ҵ�һ��ѵ��Ӣ��
		if (!bTrainedFlag)
		{
			///��ȡ�������ǰ�Ƿ�ѵ����Ӣ�۵�
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
			///�۵���Ӧ��ѵ������
			CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
			lite::Variant ret2;
			///��ʱpMsg�е�trainingFlag�����û�������������ѵ���ٶȣ��������뼸���ٵ�ѵ���ٶ�
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

			///��������ѵ���ɹ�
			if (m_ptrAllowStartTrainingMsg->trainingFlag != 0)
			{
				///��һ��ѵ������Ǯ
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

	m_ptrEarningsLiquidationMsg->trainingFlag = 0;///��0��Ϊ��
	int heroID = pMsg->heroID;
	int retValue = 0;
	m_ptrEarningsLiquidationMsg->heroID = heroID;
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_baseDataMgr.GetSID())->DynamicCast(IID_PLAYER);
	lite::Variant ret1;//��lua��ȡ���ķ���ֵ
	lite::Variant ret2;//��lua��ȡ���ķ���ֵ
	lite::Variant ret3;//��lua��ȡ���ķ���ֵ
	BOOL execResult = FALSE;
	if (m_baseDataMgr.GetMasterLevel() >= m_gameFeatureConfig.HeroTrainingLimit && pPlayer != nullptr)
	{
		int heroID = pMsg->heroID;
		CGlobalConfig& config = CConfigManager::getSingleton()->globalConfig;
		///��ȡ��ʼ���淨ʱ�����ĵȼ�
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
		///��ȡѵ�����佫��ģʽ
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
			///��ʱpMsg�е�trainingFlag�������û���û��Ǯ������ѵ���Ľ���
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
				count = (int)ret3;///lua�˷��ص��ǿ��Խ��ж��ٸ���λ�Ľ���
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
				///��õľ���ֵΪ����λ�� * ÿ10���ӻ�õľ���ֵ * ѵ��ģʽ�ļӳɱ���
				count = floor(count * trainingCfg->expProceeds * findResult->second.bonusScale);
				int level = 0;
				bool levelLimitted = false;
				///���õ��ľ���ֵ�����ͻ���
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
