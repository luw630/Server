#include "stdafx.h"
#include "AchievementDataManager.h"
#include "..\BaseDataManager.h"
#include "../networkmodule/playertypedef.h"
#include "..\Common\ConfigManager.h"
#include <vector>

CAchievementDataManager::CAchievementDataManager(CBaseDataManager& baseDataManager)
	:CExtendedDataManager(baseDataManager)
{
	m_AchievementUnitData.clear();
	m_AchievementGroupData.clear();
	m_curUnaccomplishedAchievementGroup.clear();
	m_AchievementGroupCurUnaccomplishedID.clear();
}


CAchievementDataManager::~CAchievementDataManager()
{
	m_AchievementUnitData.clear();
	m_AchievementGroupData.clear();
	m_curUnaccomplishedAchievementGroup.clear();
	m_AchievementGroupCurUnaccomplishedID.clear();
}

bool CAchievementDataManager::InitDataMgr(void * pData)
{
	m_AchievementUnitData.clear();
	m_AchievementGroupData.clear();
	SAchievementData * data = static_cast<SAchievementData*>(pData);
	if (data == nullptr)
	{
		rfalse("�ɾ�����Ϊ��");
		return false;
	}

	if (m_pBaseDataMgr.IsNewPlayer())
	{
		vector<int32_t> achivementIDList;
		CConfigManager::getSingleton()->GetAchievementIDList(achivementIDList);
		if (achivementIDList.size() != ACHIEVEMENT_NUM)
		{
			rfalse("�ɾ͵ĸ�������");
			return false;
		}
		const AchievementConfig* config = nullptr;
		for (int i = 0; i < ACHIEVEMENT_NUM; ++i)
		{
			data->achievementsData[i].accompulished = false;
			data->achievementsData[i].completedTimes = 0;
			data->achievementsData[i].achievementID = achivementIDList[i];
			data->achievementsData[i].groupType = CConfigManager::getSingleton()->GetAchievementGroupType(achivementIDList[i]);

			m_AchievementUnitData.insert(make_pair(data->achievementsData[i].achievementID, &data->achievementsData[i]));
		}

		int newestAchivementID = 0;
		for (int i = 0; i < ACHIEVEMENT_GROUP_NUM; ++i)
		{
			///��ʱAchievementType��ʾ����𣬶��������Ӧ�ø�ACHIEVEMENT_GROUP_NUMһ�£��Ҵ�0��ʼ
			newestAchivementID = CConfigManager::getSingleton()->GetAchivementGroupInitID(i);
			if (newestAchivementID != -1)
				data->groupData[i].newestAchivementID = newestAchivementID;
			data->groupData[i].accompulished = false;
			data->groupData[i].GroupType = i;

			m_curUnaccomplishedAchievementGroup.insert(i);
			m_AchievementGroupData.insert(make_pair(i, &data->groupData[i]));
			m_AchievementGroupCurUnaccomplishedID.insert(make_pair(i, newestAchivementID));
		}
	}
	else
	{
		for (int i = 0; i < ACHIEVEMENT_GROUP_NUM; ++i)
		{
			m_AchievementGroupData.insert(make_pair(data->groupData[i].GroupType, &data->groupData[i]));
			if (!data->groupData[i].accompulished)
			{
				m_curUnaccomplishedAchievementGroup.insert(i);
			}
		}

		bool unaccomplishedAchievementIDCached = true;
		///�ٶ��ɾ͵����ö��û�д�-1��ʼ��ö��ֵ
		int groupTypeSerched = -1;
		for (int i = 0; i < ACHIEVEMENT_NUM; ++i)
		{
			m_AchievementUnitData.insert(make_pair(data->achievementsData[i].achievementID, &data->achievementsData[i]));

			///�������ɾ��Ѿ�ȫ������ˣ��򲻼�¼����ɾ���δ��ȫ����ɡ������
			if (m_curUnaccomplishedAchievementGroup.find(data->achievementsData[i].groupType) == m_curUnaccomplishedAchievementGroup.end())
				continue;

			if (data->achievementsData[i].groupType != groupTypeSerched)
			{
				unaccomplishedAchievementIDCached = false;
				groupTypeSerched = data->achievementsData[i].groupType;
			}

			if (!unaccomplishedAchievementIDCached)
			{
				const AchievementConfig* curAchievementConfog = CConfigManager::getSingleton()->GetAchievementConfig(data->achievementsData[i].achievementID);
				if (curAchievementConfog == nullptr)
					continue;

				///��Ѱ���µġ�δ��ɡ��ĳɾͣ��˴��ġ�δ��ɡ���ָ������ɴ�����δ�ﵽҪ��Ĵ���
				if (!data->achievementsData[i].accompulished && (curAchievementConfog->param2 == 0 || data->achievementsData[i].completedTimes < curAchievementConfog->param2))
				{
					unaccomplishedAchievementIDCached = true;
					m_AchievementGroupCurUnaccomplishedID.insert(make_pair(groupTypeSerched, data->achievementsData[i].achievementID));
					continue;
				}
			}
		}
	}

	return true;
}

bool CAchievementDataManager::ReleaseDataMgr()
{
	m_AchievementUnitData.clear();
	m_AchievementGroupData.clear();
	return true;
}

const SAchivementUnitData* CAchievementDataManager::SetAchievementCompleteTimes(DWORD achievementID, OUT bool& bAchievementAccomplished, int times /* = 1 */)
{
	bAchievementAccomplished = false;
	auto findResults = m_AchievementUnitData.find(achievementID);
	if (findResults == m_AchievementUnitData.end())
		return nullptr;

	///����Ѿ�����˸óɾͣ����衰�ٴ���ɡ�
	if (findResults->second->accompulished)
		return nullptr;

	const AchievementConfig* achievementConfig = CConfigManager::getSingleton()->GetAchievementConfig(achievementID);
	if (achievementConfig == nullptr)
		return nullptr;

	///����óɾ��Ѵ��ֻ��û�С���ȡ����������һ�εġ���ɡ�
	if (findResults->second->completedTimes != 0 && achievementConfig->param2 <= findResults->second->completedTimes)
	{
		return nullptr;
	}

	findResults->second->completedTimes += times;
	if (findResults->second->completedTimes != 0 && findResults->second->completedTimes >= achievementConfig->param2)
	{
		if (achievementConfig->param2 != 0)
			findResults->second->completedTimes = achievementConfig->param2;
		bAchievementAccomplished = true;
	}

	return findResults->second;
}

void CAchievementDataManager::SetAchievementAccompulished(DWORD achievementID)
{
	auto findResults = m_AchievementUnitData.find(achievementID);
	if (findResults != m_AchievementUnitData.end())
	{
		findResults->second->accompulished = true;
	}
}

void CAchievementDataManager::SetAchievementGroupAccompulished(AchievementType achievementType)
{
	auto findResult = m_AchievementGroupData.find(achievementType);
	if (findResult != m_AchievementGroupData.end())
	{
		findResult->second->accompulished = true;
	}
}

void CAchievementDataManager::SetAchievementGroupNewsetID(AchievementType achievementType, DWORD ID)
{
	auto findResult = m_AchievementGroupData.find(achievementType);
	if (findResult != m_AchievementGroupData.end())
	{
		findResult->second->newestAchivementID = ID;
	}
}

const SAchivementGroupData* CAchievementDataManager::GetAchievementGroupData(AchievementType achievementType)
{
	auto findResult = m_AchievementGroupData.find(achievementType);
	if (findResult != m_AchievementGroupData.end())
		return findResult->second;

	return nullptr;
}

const SAchivementUnitData* CAchievementDataManager::GetAchievementUnitData(DWORD achievementID)
{
	auto findResult = m_AchievementUnitData.find(achievementID);
	if (findResult != m_AchievementUnitData.end())
		return findResult->second;

	return nullptr;
}

void CAchievementDataManager::SetAchievementGroupCurUnaccomplishedID(AchievementType achievmentType, DWORD ID)
{
	auto findResult = m_AchievementGroupCurUnaccomplishedID.find(achievmentType);
	if (findResult == m_AchievementGroupCurUnaccomplishedID.end())
		return;

	findResult->second = ID;
}

int CAchievementDataManager::GetAchievemntGroupCurUnaccomplishedID(AchievementType achievmentType)
{
	auto findResult = m_AchievementGroupCurUnaccomplishedID.find(achievmentType);
	if (findResult == m_AchievementGroupCurUnaccomplishedID.end())
		return -1;

	return findResult->second;
}
