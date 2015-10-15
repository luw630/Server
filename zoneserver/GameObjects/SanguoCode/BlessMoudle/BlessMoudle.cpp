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
	case SBlessMsg::MONEY_FREE_BLESS: //�����
		_ProcessFreeMoneyBless(pBlessDataMgr); //���ͭǮ��
		break;
	case SBlessMsg::MONEY_SINGLE_BLESS:
		_ProcessSingleMoneyBless(pBlessDataMgr); //����ͭǮ��
		break;
	case SBlessMsg::MONEY_MUTIPLE_BLESS:
		_ProcessMutipleMoneyBless(pBlessDataMgr); //���ͭǮ��
		break;
	case SBlessMsg::DIAMOND_FREE_BLESS:
		_ProcessFreeDiamondBless(pBlessDataMgr); //��ѱ�ʯ��
		break;
	case SBlessMsg::DIAMOND_SINGLE_BLESS:
		_ProcessSingleDiamondBless(pBlessDataMgr); //���α�ʯ��
		break;
	case SBlessMsg::DIAMOND_MUTIPLE_BLESS:
		_ProcessMutipleDiamondBless(pBlessDataMgr); //��α�ʯ��
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
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	if (pBlessDataMgr->IsFirstMonyBless()) //��һ����
	{
		pBlessDataMgr->DiaFirstMoneyBless();
		SASingleRequstBless singleMsg(SASingleRequstBless::MONEY_FREE_BLESS);
		singleMsg.m_dwProperty = baseDataMgr.GetMoney();
		singleMsg.m_dwPrizeID = globalConfig.FristBlessHero;
		pBlessDataMgr->SetMoneyFreeBlessCount(pBlessDataMgr->GetMoneyFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeMoneyDate(time(nullptr));
		g_StoreMessage(baseDataMgr.GetDNID(), &singleMsg, sizeof(SASingleRequstBless));
		baseDataMgr.AddGoods_SG(GoodsType::hero, singleMsg.m_dwPrizeID, 1, GoodsWay::bless, false);
		/// �״ν�Ǯ�� ����������֪ͨ�ͻ���ǰ����һ��ָ��
		baseDataMgr.ProcessOperationOfGuide(FunctionMoudleType::Function_Bless);
		return;
	}

	if (pBlessDataMgr->GetMoneyFreeBlessCount() > 0 //ʣ���������
		&& time(nullptr) - pBlessDataMgr->GetLastFreeMoneyBlessDate() >= globalConfig.BlessFreeTimeForMoney) //����ϴ������ʱ�����5MIN
	{
		pBlessDataMgr->SetMoneyFreeBlessCount(pBlessDataMgr->GetMoneyFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeMoneyDate(time(nullptr));
	}
	else //������У��ʧ�� ֱ�ӷ���
	{
		return;
	}
	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //�����������ȡ��Ʒ�ӿ�
	pBlessDataMgr->AddUpMoneyBlessCount(); //�������ۼ�
	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetMoneyBlessCount(); //����������
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
	///������صĳɾ����״̬
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}


void BlessMoudle::_ProcessSingleMoneyBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	if (baseDataMgr.GetMoney() < globalConfig.LotteryOnceMoney)// ( ���������㹻��Ǯ)
		return;

	//baseDataMgr.PlusMoney(-globalConfig.LotteryOnceMoney); //��Ǯ
	baseDataMgr.DecGoods_SG(GoodsType::money, 0, globalConfig.LotteryOnceMoney, GoodsWay::bless);

	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //�����������ȡ��Ʒ�ӿ�
	pBlessDataMgr->AddUpMoneyBlessCount(); //�������ۼ�
	
	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetMoneyBlessCount(); //����������
	
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
	///������صĳɾ����״̬
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}

void BlessMoudle::_ProcessMutipleMoneyBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	if (baseDataMgr.GetMoney() < globalConfig.LotteryTenTimesMoney)//�������Ҫ�Ľ�Ǯ�Ƿ�����
		return;
	//baseDataMgr.PlusMoney(-globalConfig.LotteryTenTimesMoney);
	baseDataMgr.DecGoods_SG(GoodsType::money, 0, globalConfig.LotteryTenTimesMoney, GoodsWay::bless);
	//ѭ�����߼�
	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	SARequstMutipleBless msg(SARequstMutipleBless::MONEY_MUTIPLE_BLESS); //���ؿͻ�����Ϣ


	for (int count = 0; count < MUTIPL_BLEES; count++)
	{
		prizeID = -1;
		objectType = m_upRandomObject->MoneyBless(pBlessDataMgr->GetMoneyBlessCount(), prizeID); //�����������ȡ��Ʒ�ӿ�
		pBlessDataMgr->AddUpMoneyBlessCount(); //�ۼ�
		//��ȡ��ƷΪ��ɫ����Ʒ�ʣ���������������0
		if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
			pBlessDataMgr->ResetMoneyBlessCount();
		
		if (objectType == BlessObjectType::Hero) //����µ�����Ʒ
			baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
		else
			baseDataMgr.AddGoods_SG(GoodsType::item, prizeID, 1, GoodsWay::bless, false);

		msg.m_dwPrizeIDs[count] = prizeID;
	}
	msg.m_dwProperty = baseDataMgr.GetMoney();
	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SARequstMutipleBless));
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator = baseDataMgr.GetRandomAchievementUpdator();
	CMissionUpdate* m_ptrMissionUpdator = baseDataMgr.GetMissionUpdator();
	///������صĳɾ����״̬
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve(MUTIPL_BLEES);
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(MUTIPL_BLEES);
}


void BlessMoudle::_ProcessFreeDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
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
		/// �״�Ԫ���� ����������֪ͨ�ͻ���ǰ����һ��ָ��
		baseDataMgr.ProcessOperationOfGuide(FunctionMoudleType::Function_Bless);
		return;
	}

	if (time(nullptr) - pBlessDataMgr->GetLastFreeDiamondBlessDate() >= globalConfig.BlessFreeTime) //-���ʱ���Ƿ�����
	{
		//pBlessDataMgr->SetDiamondFreeBlessCount(pBlessDataMgr->GetDiamondFreeBlessCount() - 1);
		pBlessDataMgr->SetLastFreeDiamondDate(time(nullptr));
	}
	else  //�������������ֱ�ӷ���
	{
		return;
	}

	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID); //�����������ȡ��Ʒ�ӿ�
	pBlessDataMgr->AddUpDiamondBlessCount(); //�������ۼ�

	if (objectType == BlessObjectType::Blue_Equip || objectType == BlessObjectType::Purple_Equip || objectType == BlessObjectType::Hero)
		pBlessDataMgr->ResetDiamondBlessCount(); //����������

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
	///������صĳɾ����״̬
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);
}

void BlessMoudle::_ProcessSingleDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����

	if (baseDataMgr.GetDiamond() < globalConfig.LotteryOnceGold) //( ���������㹻��Ǯ)
		return;
	//baseDataMgr.PlusDiamond(-globalConfig.LotteryOnceGold); //��Ǯ
	baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, globalConfig.LotteryOnceGold, GoodsWay::bless);
	int32_t prizeID = -1;
	if (pBlessDataMgr->IsFirstSingleDiamondBless()) //��һ��Ԫ���� ��ȡ�����佫
	{
		pBlessDataMgr->DidFirstSingleDiamondBless();
		prizeID = m_upRandomObject->RandomThreeStarsHero();
		baseDataMgr.AddGoods_SG(GoodsType::hero, prizeID, 1, GoodsWay::bless, false);
	}
	else //��һ���������ͨ���߼�
	{
		BlessObjectType objectType = BlessObjectType::None;
		objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID); //�����������ȡ��Ʒ�ӿ�
		pBlessDataMgr->AddUpDiamondBlessCount(); //�������ۼ�

		if (objectType == BlessObjectType::Hero)
			pBlessDataMgr->ResetDiamondBlessCount(); //����������

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
	///������صĳɾ����״̬
	if (m_ptrRandomAchievementUpdator != nullptr)
		m_ptrRandomAchievementUpdator->UpdateBlessAchieve();
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateBlessingMission(1);

}

void BlessMoudle::_ProcessMutipleDiamondBless(CBlessDataMgr *pBlessDataMgr)
{
	
	int lastHeroIndex = -1;
	CBaseDataManager& baseDataMgr = pBlessDataMgr->GetBaseDataMgr(); //��ȡ�������ݹ�����
	if (baseDataMgr.GetDiamond() < globalConfig.LotteryTenTimesGold) //( ���������㹻��Ǯ)
		return;

	lite::Variant ret;//��lua��ȡ����������
	LuaFunctor(g_Script, "get_Exdata")[baseDataMgr.GetSID()][SG_ExDataType::MultiDiamondBlessCount](&ret);
	int allBlessCount = (int)ret; 
	lite::Variant ret2;//��lua��ȡ����������
	LuaFunctor(g_Script, "get_Exdata")[baseDataMgr.GetSID()][SG_ExDataType::BlessMaxThreeStarHeroCount](&ret2);//��ȡ�ĵ������佫�Ĵ�������
	int conditionCount = (int)ret2;

	//baseDataMgr.PlusDiamond(- globalConfig.LotteryTenTimesGold); //��Ǯ
	baseDataMgr.DecGoods_SG(GoodsType::diamond, 0, globalConfig.LotteryTenTimesGold, GoodsWay::bless);

	SARequstMutipleBless msg(SARequstMutipleBless::DIAMOND_MUTIPLE_BLESS); //���ؿͻ�����Ϣ


	int32_t prizeID = -1;
	BlessObjectType objectType = BlessObjectType::None;
	int heroNum = 0;
	bool cancelBlessHero = false; //ȡ������ȡ�佫�ӿ�
	bool onlyOneStarHero = true; //��¼�Ƿ�ֻ��ȡ1���佫
	int heroPrizeIndex = 0; //�佫��Ʒ���±�
	for (int32_t blessCount = 0; blessCount < MUTIPL_BLEES; blessCount++)
	{
		objectType = m_upRandomObject->DiamondBless(pBlessDataMgr->GetDiamondBlessCount(), prizeID, cancelBlessHero); //�����������ȡ��Ʒ�ӿ�
		pBlessDataMgr->AddUpDiamondBlessCount(); //�������ۼ�
		if (objectType == BlessObjectType::Hero) //�����佫��Ҫ���ر������Բ����ϰ��佫�ӵ�Ӣ�۱�����
		{
			pBlessDataMgr->ResetDiamondBlessCount(); //�������ۼƴ���
			if (++heroNum >= 4)
				cancelBlessHero = true;

			if (allBlessCount <= conditionCount) //������������佫�س�����
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
			else if (onlyOneStarHero == true && _GetHeroStarLevel(prizeID) <= 1) //�����ǰֻ��1���佫
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
	///������صĳɾ����״̬
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

void BlessMoudle::_ProcessMutipleDiamondResult(DWORD *prizeArray) //���ｱƷ�����������
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
