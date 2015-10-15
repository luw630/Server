// ----- CMissionUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/06/19
//   --  Desc:   �ȶ���������������߼����������
// --------------------------------------------------------------------
//   --  ���ڳ�����ɾ͸��µĲ��ֽӿڣ�������ģ��ֻ�����ڸò����
//---------------------------------------------------------------------   
#pragma once

enum InstanceType;

class CMissionUpdate
{
public:
	CMissionUpdate(){}
	virtual ~CMissionUpdate(){}

	///@brief ���¸����淨�������������
	///@param type Ҫ���µĸ�������
	///@param completeTimes ��ս��ɵĴ���
	virtual void UpdateDungeionMission(InstanceType type, int completeTimes) = 0;
	///@brief �������淨�������������
	///@param blessTimes ���ɹ��Ĵ���
	virtual void UpdateBlessingMission(int blessTimes) = 0;
	///@brief ���¶����淨�������������
	///@param forgingTimes ����ɹ��Ĵ���
	virtual void UpdateForgingMission(int forgingTimes) = 0;
	///@brief ���¼��������������������
	///@param upgradeTimes �������ܳɹ��Ĵ���
	virtual void UpdateSkillUpgradeMission(int upgradeTimes) = 0;
	///@brief ����VIP�ȼ����������
	///@param vipLevel VIP��ǰ�ĵȼ�
	virtual void UpdateVipMission(int vipLevel) = 0;
};