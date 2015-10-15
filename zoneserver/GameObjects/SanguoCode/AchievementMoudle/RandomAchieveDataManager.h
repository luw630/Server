// ----- CRandomAchieveDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   �ȶ�������������ɵĿɴ�ɵĳɾ͵����������ݵĹ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <unordered_map>
#include <unordered_set>
#include "..\ExtendedDataManager.h"

struct SAchivementUnitData;
enum RandomAchievementType;

class CRandomAchieveDataManager :
	public CExtendedDataManager
{
public:
	CRandomAchieveDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CRandomAchieveDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief ���óɾ͵���ɴ���
	///@param missionID �ɾͶ�Ӧ��ID
	///@return �ɹ��Ż��޸ĺ�ĳɾ����ݣ�ʧ�ܷ���null
	const SAchivementUnitData* SetAchievementCompleteTimes(DWORD achievementID, int times = 1);
	///@brief ����ĳһ�����Ѿ���ȡ�ˣ�������ɡ��ĸ�����Щ����
	void SetAchievementAccompulished(DWORD achievementID);
	///@brief �����µĿɴ�ɵ�һϵ�еĳɾ�
	///@param achievementIDs �µĳɾ͵�ID�б�
	void SetNewAchievement(DWORD achievementID);
	///@brief ����������ɵĿɴ�ɵĳɾ͵��������ʣ��ʱ������뿪ʼ��ʣ��ʱ��
	///@param bLuckyTime �Ƿ��ڳɾͼ����״̬��Ϊfalse��Ϊ�����족�������û��ж೤ʱ�䵽��ɾ͵ļ���ʱ�䣨��ҹ��
	///@param remainingTime ʣ��ʱ��
	void SetRemainingTimes(bool bLuckyTime, DWORD remainingTime);

	///@brief ��ȡ��Ӧ�ɾ͵�����
	const SAchivementUnitData* GetAchievementUnitData(DWORD achievementID);
	///@brief ����������ɵĳɾ͵���������ȡ����ĵ�ǰ�����õĵĳɾ�
	///@param type �ɾͶ�Ӧ������
	///@param achievementID ���ڵĳɾ͵�ID
	bool IsAchievementExist(RandomAchievementType type, OUT int& achievementID);
	///@brief ����ˢ��Զ���ؿ���Ʊ��
	void RefreshData(int sendMsgFlag = 0);
	///@brief �ڡ����족��ʱ����������ɾ͵�����
	void LogoutProcess();

private:
	BYTE* m_ptrLuckyTimeFalling;
	DWORD* m_ptrCurRandomAchievementRemainingTime;
	DWORD* m_ptrAccomplishedTimes;
	SAchivementUnitData* m_ptrAchievementUnitData;
};

