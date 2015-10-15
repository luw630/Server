#include "stdafx.h"
#include "SkillUpgradeManager.h"
#include "NETWORKMODULE/SanguoPlayer.h"
#include "../BaseDataManager.h"
#include "../Common/PubTool.h"
#include "../MissionMoudle/CMissionUpdate.h"
#include "Player.h"
#include "ScriptManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"
#include <time.h>
#include "CMystring.h"

CSkillUpgradeManager::CSkillUpgradeManager(CBaseDataManager& BaseDataMgr)
	: CExtendedDataManager(BaseDataMgr)
	, m_GlobalConfig(CConfigManager::getSingleton()->globalConfig)
	, m_LastCalcSkillPointTime(0)
	, m_CurrentTime(0)
	, m_iSkillPointLimit(0)
{
	m_ptrMissionUpdator = m_pBaseDataMgr.GetMissionUpdator();
}


CSkillUpgradeManager::~CSkillUpgradeManager()
{
	m_ptrMissionUpdator = nullptr;
}

bool CSkillUpgradeManager::InitDataMgr(void * pData)
{
	if (nullptr == pData)
		return false;

	m_pSkillPointData = static_cast<SSkillPointData*>(pData);
	
	return true;
}

bool CSkillUpgradeManager::InitSkillPointData()
{
	if (nullptr == m_pSkillPointData)
		return false;

	lite::Variant ret;
	BOOL result = FALSE;
	if (g_Script.PrepareFunction("SI_vip_getDetail"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.PushParameter(VipLevelFactor::VF_Skill_Num);
		result = g_Script.Execute(&ret);
	}

	if (!result || ret.dataType == LUA_TNIL)
	{
		rfalse("SetSkillPointForVIP Failed");
	}

	try
	{
		m_iSkillPointLimit = static_cast<int>(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
		m_iSkillPointLimit = m_GlobalConfig.SkillPointLimit;
	}

	if (m_pBaseDataMgr.IsNewPlayer())
	{
		m_pSkillPointData->learnedSkill = false;
		m_pSkillPointData->m_dwSkillPoint = m_iSkillPointLimit;
		m_LastCalcSkillPointTime = m_pBaseDataMgr.GetLoginTime();
		ConverTool::ConvertInt64ToBytes(m_LastCalcSkillPointTime, m_pSkillPointData->m_LastCalculateSkillPointTime);
	}
	else
	{
		m_LastCalcSkillPointTime = *((__time64_t*)(m_pSkillPointData->m_LastCalculateSkillPointTime));
		_time64(&m_CurrentTime);
		_CalculateSkillPoint(m_LastCalcSkillPointTime, m_CurrentTime);
	}

	return true;
}

bool CSkillUpgradeManager::ReleaseDataMgr()
{
	m_pSkillPointData = nullptr;
	return true;
}

bool CSkillUpgradeManager::UpgradeSkill(const int heroID, const int skillID, const int skillLevel)
{
	if (m_pBaseDataMgr.UpgradeHeroSkill(heroID, skillID, skillLevel) == false)
		return false;

	///��������������ص������߼�
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateSkillUpgradeMission(1);

	m_pSkillPointData->m_dwSkillPoint--;
	SetLearnedSkill();
	return true;
}

bool CSkillUpgradeManager::MultiUpgradeSkill(const int heroID, const int skillID, const int destLevel, DWORD& skillPointCost)
{
	if (!CanLearnSkill())
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("NOT_ENOUGH_SKILL_POINT")); // ���ܵ㲻��
		return false;
	}
	DWORD curSkillLevel = m_pBaseDataMgr.GetHeroSkillLevel(heroID, skillID);
	if (curSkillLevel == 0)
		return false;
	DWORD needSkillPoint = destLevel - curSkillLevel;
	DWORD goldCost = ((curSkillLevel + destLevel - 1) * needSkillPoint / 2) * m_GlobalConfig.SkillCostMoney;
	if (needSkillPoint > m_pSkillPointData->m_dwSkillPoint || m_pBaseDataMgr.GetMoney() < goldCost)
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("NOT_ENOUGH_SKILL_POINT_AND_MONEY")); //"���ܵ���Ǯ����"); 
		return false;
	}
	if (m_pBaseDataMgr.MultiUpgradeHeroSkill(heroID, skillID, destLevel) == false)
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("SKILL_LEVEL_HIGHER_THAN_HERO_LEVEL"));//���ܵȼ������佫�ȼ�
		return false;
	}
	skillPointCost = needSkillPoint;

	///��������������ص������߼�
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateSkillUpgradeMission(skillPointCost);

	m_pSkillPointData->m_dwSkillPoint -= needSkillPoint;
	m_pBaseDataMgr.DecGoods_SG(GoodsType::money, 0, goldCost, GoodsWay::addskill);
	SetLearnedSkill();
	return true;
}

bool CSkillUpgradeManager::MultiUpgradeSkill(const int heroID, DWORD skillIDs[], DWORD destLevel[], DWORD validNum, DWORD& skillPointCost)
{
	if (validNum > MAX_SKILLNUM || CanLearnSkill() == false)
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("NOT_ENOUGH_SKILL_POINT")); // ���ܵ㲻��
		return false;
	}

	DWORD curSkillLevel;
	DWORD destSkillLevel;
	DWORD levelDistance;
	DWORD needSkillPoint = 0;
	DWORD goldCost = 0;
	for (int i = 0; i < validNum; ++i)
	{
		curSkillLevel = m_pBaseDataMgr.GetHeroSkillLevel(heroID, skillIDs[i]);
		if (curSkillLevel == 0)
		{
			TalkToDnid(m_pBaseDataMgr.GetDNID(), "����ID����");//
			return false;
		}
		destSkillLevel = destLevel[i];
		levelDistance = destSkillLevel - curSkillLevel;
		needSkillPoint += levelDistance;
		goldCost += ((curSkillLevel + destSkillLevel - 1) * levelDistance / 2) * m_GlobalConfig.SkillCostMoney;
	}

	skillPointCost = needSkillPoint;
	if (m_pSkillPointData->m_dwSkillPoint < needSkillPoint || m_pBaseDataMgr.GetMoney() < goldCost)
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("NOT_ENOUGH_SKILL_POINT_AND_MONEY")); //"���ܵ���Ǯ����"); 
		return false;
	}

	if (m_pBaseDataMgr.MultiUpgradeHeroSkill(heroID, skillIDs, destLevel, validNum) == false)
	{
		TalkToDnid(m_pBaseDataMgr.GetDNID(), CMyString::GetInstance().GetFormatString("SKILL_LEVEL_HIGHER_THAN_HERO_LEVEL"));//���ܵȼ������佫�ȼ�
		return false;
	}

	///��������������ص������߼�
	if (m_ptrMissionUpdator != nullptr)
		m_ptrMissionUpdator->UpdateSkillUpgradeMission(needSkillPoint);

	m_pSkillPointData->m_dwSkillPoint -= needSkillPoint;
	m_pBaseDataMgr.DecGoods_SG(GoodsType::money, 0, goldCost, GoodsWay::addskill);
	SetLearnedSkill();
	return true;
}

bool CSkillUpgradeManager::BuySkillPoint()
{
	//if (CanLearnSkill() == true)
	//	return false;
	_time64(&m_CurrentTime);
	_CalculateSkillPoint(m_LastCalcSkillPointTime, m_CurrentTime);
	m_pSkillPointData->m_dwSkillPoint = m_GlobalConfig.SkillPointLimit;
	m_pSkillPointData->m_dwBuySkillPointTime++;
	if (m_GlobalConfig.SkillPointLimit == m_iSkillPointLimit)
	{
		m_pSkillPointData->m_dwRemainingSeconds = 0;
	}

	return true;
}

bool CSkillUpgradeManager::CanLearnSkill()
{
	_time64(&m_CurrentTime);
	_CalculateSkillPoint(m_LastCalcSkillPointTime, m_CurrentTime);
	return m_pSkillPointData->m_dwSkillPoint > 0;
}

void CSkillUpgradeManager::_CalculateSkillPoint(const __time64_t& lastTime, const __time64_t& currentTime)
{
	ConverTool::ConvertInt64ToBytes(currentTime, m_pSkillPointData->m_LastCalculateSkillPointTime);
	//���㵱ǰʱ����ϴμ��㼼�ܵ�ʱ��ļ��(��Ϊ��λ)
	DWORD gapSeconds = (DWORD)difftime(currentTime, lastTime);
	//�����ϴμ�����ʣ���CDʱ��(��Ϊ��λ)
	gapSeconds += m_pSkillPointData->m_dwRemainingSeconds;
	//����Ӧ�ûظ��ļ��ܵ�
	DWORD recoverPoint = gapSeconds / m_GlobalConfig.SkillPointRecoverTime;
	//�����ǰ���ܵ���ϻظ�����ڻ���ڼ��ܵ�����������ʣ��CDʱ��Ϊ0
	if ((m_pSkillPointData->m_dwSkillPoint + recoverPoint) >= m_iSkillPointLimit)
	{
		m_pSkillPointData->m_dwSkillPoint = m_iSkillPointLimit;
		m_pSkillPointData->m_dwRemainingSeconds = 0;
	}
	else
	{
		m_pSkillPointData->m_dwSkillPoint += recoverPoint;
		m_pSkillPointData->m_dwRemainingSeconds = gapSeconds % m_GlobalConfig.SkillPointRecoverTime;
	}
	//���浱ǰ���㼼�ܵ�ʱ��
	m_LastCalcSkillPointTime = currentTime;
}

DWORD CSkillUpgradeManager::RemainingSkillPoint() const
{
	return m_pSkillPointData->m_dwSkillPoint;
}

DWORD CSkillUpgradeManager::RecoverRemainingTime() const
{
	return m_pSkillPointData->m_dwRemainingSeconds;
}

void CSkillUpgradeManager::SetLearnedSkill()
{
	if (m_pSkillPointData->learnedSkill == false)
		m_pSkillPointData->learnedSkill = true;
}

DWORD CSkillUpgradeManager::PurchasedSkillPointTimes() const
{
	return m_pSkillPointData->m_dwBuySkillPointTime;
}

bool CSkillUpgradeManager::SetSkillPointForVIP(int vipLevel)
{
	lite::Variant ret;
	BOOL result = FALSE;
	if (g_Script.PrepareFunction("SI_vip_getDetail"))
	{
		g_Script.PushParameter(m_pBaseDataMgr.GetSID());
		g_Script.PushParameter(VipLevelFactor::VF_Skill_Num);
		result = g_Script.Execute(&ret);
	}
	else
	{
		return false;
	}

	if (!result || ret.dataType == LUA_TNIL)
	{
		rfalse("SetSkillPointForVIP Failed");
		return false;
	}

	int pointLimit = 0;
	try
	{
		pointLimit = static_cast<int>(ret);
	}
	catch (lite::Xcpt &e)
	{
		rfalse(2, 1, e.GetErrInfo());
		return false;
	}

	m_iSkillPointLimit = pointLimit;
	m_pSkillPointData->m_dwSkillPoint = m_iSkillPointLimit;
	m_pSkillPointData->m_dwRemainingSeconds = 0;
	_time64(&m_CurrentTime);
	m_LastCalcSkillPointTime = m_CurrentTime;
	ConverTool::ConvertInt64ToBytes(m_LastCalcSkillPointTime, m_pSkillPointData->m_LastCalculateSkillPointTime);

	return true;
}

void CSkillUpgradeManager::AddSkillPoint(int Num)
{
	///�ȼ����Ѿ����˶��ټ��ܵ�
	_time64(&m_CurrentTime);
	_CalculateSkillPoint(m_LastCalcSkillPointTime, m_CurrentTime);

	///��ʱ���������ܼ��ܵ����˲��ӵ����,���˲��ӵ������ȫ�����ڿͻ��˵��ж�
	/////�Ѿ����˵Ļ��Ͳ��ܼӼ��ܵ���
	//if (m_pSkillPointData->m_dwSkillPoint >= m_iSkillPointLimit)
	//{
	//	///֪ͨ�ͻ����޷��Ӽ��ܵ�
	//	g_StoreMessage(m_pBaseDataMgr.GetDNID(), &addSkillPointMsg, sizeof(SAAddSkillPoitResult));
	//	return;
	//}
	
	///��ʼ�Ӽ��ܵ�
	m_pSkillPointData->m_dwSkillPoint += Num;
	if (m_pSkillPointData->m_dwSkillPoint >= m_iSkillPointLimit)
	{
		m_pSkillPointData->m_dwRemainingSeconds = 0;
	}
}
