#include "stdafx.h"
#include "ForgingManager.h"
#include "..\BaseDataManager.h"
#include"..\Common\ConfigManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "..\StorageMoudle\StorageManager.h"
#include "../AchievementMoudle/RandomAchieveUpdate.h"
#include "..\MissionMoudle\CMissionUpdate.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include "Player.h"
#include "ScriptManager.h"
extern LPIObject GetPlayerBySID(DWORD dwStaticID);


CForgingManager::CForgingManager()
{
	m_ptrBaseDataManager = nullptr;
}

CForgingManager::~CForgingManager()
{
	m_ptrBaseDataManager = nullptr;
}

void CForgingManager::OnRecvMsg(const SMessage *pMsg, CBaseDataManager *pBaseDataMgr)
{
	m_ptrBaseDataManager = pBaseDataMgr;
	if (m_ptrBaseDataManager == nullptr)
	{
		rfalse("�������ݹ�����CBaseDataManagerΪ��");
		return;
	}

	static SAForgingPermission permissionMsg;
	permissionMsg.permitted = false;
	const SQAskToForging* ptrMsg = static_cast<const SQAskToForging*>(pMsg);
	if (ptrMsg == nullptr)
	{
		rfalse("����ת����Ϣʧ��");
		///�������
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		return;
	}

	///��������Ӣ�۲����ڻ��߶����װ�������ڣ��򷵻�
	if (!m_ptrBaseDataManager->IsHeroEquipmentAttached(ptrMsg->heroID, ptrMsg->forgingEquipID))
	{
		///�������
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		rfalse("����ѡ���Ӣ�۲����ڻ��߶����װ��������");
		return;
	}

	switch (ptrMsg->_protocol)
	{
	case SQAskToForging::REQUEST_DIAMOND_FORGING:
		AskToDiamondForging(ptrMsg, permissionMsg);
		break;
	case SQAskToForging::REQUEST_MONEY_FORGING:
		AskToMoneyForging(ptrMsg, permissionMsg);
		break;
	default:
		break;
	}
}

void CForgingManager::AskToMoneyForging(const SQAskToForging* pMsg, SAForgingPermission& permissionMsg)
{
	const SQAskToMoneyForging* ptrMsg = static_cast<const SQAskToMoneyForging*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	try
	{
		int curMaterialForgingExp = 0;
		const EquipmentConfig* equipConfig = nullptr;
		const ItemConfig* itemConfig = nullptr;
		for (int i = 0; i < ptrMsg->materialTypeNum; ++i)
		{
			///�ж�ѡ��Ĳ����Ƿ���ڻ����㹻
			if (!m_ptrBaseDataManager->GetStorageManager().ItemExistOrEnough(ptrMsg->materialInfor[i], ptrMsg->materialInfor[i + ptrMsg->materialTypeNum]))
			{
				///�������
				g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
				rfalse("����ѡ��Ĳ��ϲ�����");
				return;
			}

			///������ϻ�����ľ���ֵ
			equipConfig = CConfigManager::getSingleton()->GetEquipment(ptrMsg->materialInfor[i]);
			if (equipConfig != nullptr)
				curMaterialForgingExp += equipConfig->refiningExp * ptrMsg->materialInfor[i + ptrMsg->materialTypeNum];
			else
			{
				itemConfig = CConfigManager::getSingleton()->GetItemConfig(ptrMsg->materialInfor[i]);
				if (itemConfig != nullptr)
				{
					curMaterialForgingExp += itemConfig->RefiningExp * ptrMsg->materialInfor[i + ptrMsg->materialTypeNum];
				}
			}
		}

		int equipStarLevel = 0;
		///�ж��ܷ����Ӧ��װ������
		int increasedExp = m_ptrBaseDataManager->IncreaseHeroEquipmentForgingExp(ptrMsg->heroID, ptrMsg->forgingEquipID, curMaterialForgingExp, equipStarLevel);
		if (increasedExp != -1)
		{
			int moneyCost = increasedExp * CConfigManager::getSingleton()->globalConfig.ForgeMoneyCostScale;///��ʱ����д��Ϊ120

			if (m_ptrBaseDataManager->CheckGoods_SG(GoodsType::money,0, moneyCost))
			{
				///������Ӧ������
				m_ptrBaseDataManager->DecGoods_SG(GoodsType::money, 0, moneyCost, GoodsWay::forgeMall);
				permissionMsg._protocol = SForgingMsg::ANSWER_MONEY_FORGING_PERMITTED;
			}
			else
			{
				///�ͻ��˲������
				g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
				return;
			}

			///��������ɾ͵��������
			CRandomAchieveUpdate*  m_ptrRandomAchievementUpdator = m_ptrBaseDataManager->GetRandomAchievementUpdator();
			CMissionUpdate* ptrMissionUpdator = m_ptrBaseDataManager->GetMissionUpdator();
			if (m_ptrRandomAchievementUpdator != nullptr)
				m_ptrRandomAchievementUpdator->UpdateForgingAchieve();
			if (ptrMissionUpdator != nullptr)
				ptrMissionUpdator->UpdateForgingMission(1);

			permissionMsg.permitted = true;
			permissionMsg.heroID = pMsg->heroID;
			permissionMsg.forgedEquipID = pMsg->forgingEquipID;
			permissionMsg.curEquipStarLevel = equipStarLevel;
			///�ͻ��˿��Խ��ж���Ĳ�����
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));

			///���Զ���Ļ��Ƴ�������Ӧ�Ĳ���
			m_ptrBaseDataManager->DecGoods_SG(GoodsType::item, ptrMsg->materialTypeNum, (int*)ptrMsg->materialInfor + ptrMsg->materialTypeNum, ptrMsg->materialInfor, GoodsWay::forgeMall);

			return;
		}
		else
		{
			///�������
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
			rfalse("�����޷�����");
			return;
		}
	}
	catch (const std::exception& oor)
	{
		rfalse("������Ϣ���쳣���˳�");
		///�������
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		return;
	}
}

void CForgingManager::AskToDiamondForging(const SQAskToForging *pMsg, SAForgingPermission& permissionMsg)
{
	if (pMsg == nullptr)
		return;

	const ForgingConfig* forgingConfig = CConfigManager::getSingleton()->GetForgingConfig(pMsg->forgingEquipID);
	if (forgingConfig == nullptr)
		return;

	///��ʱ��VIP�ȼ��Ļ�ȡ�����ŵ�����,��ΪVIP�ĵȼ�����ʱ������䣬���ԡ�ʵʱ����ȡ
	CPlayer* pPlayer = (CPlayer *)GetPlayerBySID(m_ptrBaseDataManager->GetSID())->DynamicCast(IID_PLAYER);
	int forgingAvaliableState = 0;
	if (pPlayer != nullptr)
	{
		g_Script.SetCondition(0, pPlayer, 0);
		lite::Variant ret;//��lua��ȡ�����ܴ���
		LuaFunctor(g_Script, "SI_vip_getDetail")[g_Script.m_pPlayer->GetSID()][VipLevelFactor::VF_OneKey_Num](&ret);
		forgingAvaliableState = (int)(ret);
		g_Script.CleanCondition();
	}
	else
		rfalse("��ȡ����CPlayer��ָ��");

	///�жϵ�ǰ��VIP�ȼ��Ƿ񿪷���һ��Ԫ������
	if (forgingAvaliableState <= 0)
	{
		///�ͻ��˲������
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
	}

	///Ԫ����ֱ�ӵ���
	auto findResult = --forgingConfig->levelExp.end();
	int equipStarLevel = 0;
	int increasedExp = m_ptrBaseDataManager->IncreaseHeroEquipmentForgingExp(pMsg->heroID, pMsg->forgingEquipID, findResult->first, equipStarLevel);
	if (increasedExp != -1)
	{
		int diamondCost = increasedExp * CConfigManager::getSingleton()->globalConfig.ForgeDiamondCostScale;///��ʱд��Ԫ������Ϊ����3
		if (m_ptrBaseDataManager->CheckGoods_SG(GoodsType::diamond,0, diamondCost))
		{
			///������Ӧ������
			m_ptrBaseDataManager->DecGoods_SG(GoodsType::diamond,0, diamondCost,GoodsWay::forgeMall);
			permissionMsg._protocol = SForgingMsg::ANSWER_DIAMOND_FORGING_PERMITTED;
		}
		else
		{
			///�ͻ��˲������
			g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
			return;
		}

		///��������ɾ͵��������
		CRandomAchieveUpdate*  m_ptrRandomAchievementUpdator = m_ptrBaseDataManager->GetRandomAchievementUpdator();
		CMissionUpdate* ptrMissionUpdator = m_ptrBaseDataManager->GetMissionUpdator();
		if (m_ptrRandomAchievementUpdator != nullptr)
			m_ptrRandomAchievementUpdator->UpdateForgingAchieve();
		if (ptrMissionUpdator != nullptr)
			ptrMissionUpdator->UpdateForgingMission(1);

		///���»ظ���Ϣ
		permissionMsg.permitted = true;
		permissionMsg.heroID = pMsg->heroID;
		permissionMsg.forgedEquipID = pMsg->forgingEquipID;
		permissionMsg.curEquipStarLevel = equipStarLevel;

		///�ͻ��˿��Խ��ж���Ĳ�����
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
	}
	else
	{
		///�������
		g_StoreMessage(m_ptrBaseDataManager->GetDNID(), &permissionMsg, sizeof(SAForgingPermission));
		rfalse("�����޷�����");
		return;
	}
}
