// ----- CAchievementDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   �ȶ������ĳɾ͵�������ݵĹ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

///@brief �ɾ͵����ݹ�����
enum AchievementType;
struct SAchivementUnitData;
struct SAchivementGroupData;
class CAchievementDataManager :
	public CExtendedDataManager
{
public:
	CAchievementDataManager(CBaseDataManager& baseDataManager);
	virtual ~CAchievementDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief ���óɾ͵���ɴ���
	///@param missionID �ɾͶ�Ӧ��ID
	///@param bAchievementAccomplished �ɾ��Ƿ񡰴�ɡ�
	///@return �ɹ��Ż��޸ĺ�ĳɾ����ݣ�ʧ�ܷ���null
	const SAchivementUnitData* SetAchievementCompleteTimes(DWORD achievementID, OUT bool& bAchievementAccomplished, int times = 1);
	///@brief ����ĳһ�����Ѿ���ȡ�ˣ�������ɡ��ĸ�����Щ����
	void SetAchievementAccompulished(DWORD achievementID);
	///@brief ����ĳһ��ɾ��Ƿ�ȫ����ȡ��ϣ�����ȫ����ɡ��ĸ����������
	void SetAchievementGroupAccompulished(AchievementType achievementType);
	///@brief ����ĳһ��ɾ͵����µġ�δ��ȡ���ĳɾ�ID������δ��ɡ��ĸ�����Щ����
	void SetAchievementGroupNewsetID(AchievementType achievementType, DWORD ID);
	///@brief ���õ�ǰ��ĳһ���ĳɾ͵� ���µġ�����ɡ��ĳɾ�ID���˴���"�����"��������ȡ���ĸ���������,��ɺ������ȡ
	void SetAchievementGroupCurUnaccomplishedID(AchievementType achievmentType, DWORD ID);

	///@brief ��ȡ��Ӧ���ĳɾ͵�����
	const SAchivementGroupData* GetAchievementGroupData(AchievementType achievementType);
	///@brief ��ȡ��Ӧ�ɾ͵�����
	const SAchivementUnitData* GetAchievementUnitData(DWORD achievementID);
	///@brief ��ȡ��ǰĳһ���ĳɾ͵� ���µġ�����ɡ��ĳɾ�ID���˴���"�����"��������ȡ���ĸ���������,��ɺ������ȡ
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetAchievemntGroupCurUnaccomplishedID(AchievementType achievmentType);

private:
	unordered_map<DWORD, SAchivementUnitData*> m_AchievementUnitData;
	unordered_map<DWORD, SAchivementGroupData*> m_AchievementGroupData;
	unordered_map<DWORD, DWORD> m_AchievementGroupCurUnaccomplishedID; ///<ĳһ��ɾ͵�ǰ�Ĵ���ɵĳɾ�ID
	unordered_set<DWORD> m_curUnaccomplishedAchievementGroup;///<��ǰ��δȫ����ɵĳɾ���
};

