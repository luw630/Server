// ----- CRandomAchieveUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   �ȶ�����������ɾ͸����߼����������
// --------------------------------------------------------------------
//   --  ���ڳ�����ɾ͸��µĲ��ֽӿڣ�������ģ��ֻ�����ڸò����
//---------------------------------------------------------------------   
#pragma once
#include "AchieveUpdate.h"

class CRandomAchieveUpdate :
	public CAchieveUpdate
{
public:
	CRandomAchieveUpdate(){}
	virtual ~CRandomAchieveUpdate(){}

	///@brief �������ĳɾʹ����Ϣ
	virtual void UpdateBlessAchieve(int blessTimes = 1) = 0;
	///@brief ���¼��������ĳɾʹ����Ϣ
	virtual void UpdateSkillUpgradeAchieve(int upgradeTimes = 1) = 0;
	///@breif ���¶�������ɾ͵Ĵ�����
	virtual void UpdateForgingAchieve() = 0;
};

