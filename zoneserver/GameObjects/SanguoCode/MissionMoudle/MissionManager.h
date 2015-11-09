// ----- CMissionManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ����������������߼��Ĺ���
// --------------------------------------------------------------------
//   --  ��Ҫ�ж�ĳһ�������Ƿ������ȡ���Ѿ���ȡ�����Ʒ��ӡ��Լ����ö�ʱ���ж���͵������Ƿ������ȡ���Ƿ������ʾ�ȡ�
//	 --  �򵥵������Ƿŵ��ͻ������ж��Ƿ��Ѿ���ɣ���VIP��ص��������Ƿŵ���������ж�
//---------------------------------------------------------------------  
#pragma once
#include "CMissionUpdate.h"

///@brief ������߼�������
struct SMessage;
struct SMissionMsg;
class CExtendedDataManager;
class CMissionDataManager;
class CMissionActiveness;

class CMissionManager : public CMissionUpdate
{
public:
	CMissionManager(CExtendedDataManager& missionDataMgr, CExtendedDataManager& missionActivenessMgr);
	~CMissionManager();

	void RecvMsg(const SMessage *pMsg);
	///@brief ���¸����淨�������������
	///@param type Ҫ���µĸ�������
	///@param completeTimes ��ս��ɵĴ���
	virtual void UpdateDungeionMission(InstanceType type, int completeTimes);
	///@brief �������淨�������������
	///@param blessTimes ���ɹ��Ĵ���
	virtual void UpdateBlessingMission(int blessTimes);
	///@brief ���¶����淨�������������
	///@param forgingTimes ����ɹ��Ĵ���
	virtual void UpdateForgingMission(int forgingTimes);
	///@brief ���¼��������������������
	///@param upgradeTimes �������ܳɹ��Ĵ���
	virtual void UpdateSkillUpgradeMission(int upgradeTimes);
	///@brief ����VIP�ȼ����������
	///@param vipLevel VIP��ǰ�ĵȼ�
	virtual void UpdateVipMission(int vipLevel);

private:
	///@brief �ͻ����������ĳһ������
	void AskToAccomplishMission(const SMissionMsg* msg);
	///@breif �ͻ������������Ծ�ȵĽ���
	void AskToClaimMissionActivenessRewards(const SMissionMsg* msg);

	CMissionDataManager * m_ptrMissionDataMgr;
	CMissionActiveness * m_ptrMissionActivenessMgr;
};

