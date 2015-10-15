// ----- CAchieveUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/05
//   --  Desc:   �ȶ������ĳɾ͸����߼����������
// --------------------------------------------------------------------
//   --  ���ڳ�����ɾ͸��µĲ��ֽӿڣ�������ģ��ֻ�����ڸò����
//---------------------------------------------------------------------   
#pragma once
class CAchieveUpdate
{
public:
	CAchieveUpdate(){}
	virtual ~CAchieveUpdate(){}

	///@brief ������ͨ��������Ӣ����������˵���ؾ�Ѱ���ĳɾ���ص���Ϣ
	///@param curConquredTollgateID ��ǰ�����ĸ����ؿ�ID
	///@param completedTimes ��ս��ɵĴ���
	virtual void UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes = 1) = 0;
	///@brief �����佫�ռ��ĳɾ�
	virtual void UpdateHeroCollectionAchieve() = 0;
	///@brief �����佫���׵ĳɾ�
	///@param curHeroRank ��ǰ�佫���׵��ĵȼ�
	virtual void  UpdateHeroRankRiseAchieve(int curHeroRank) = 0;
	///@brief ���¾��������ĳɾ�
	///@param curMasterLevel ��ǰ�ľ����ȼ�
	///@param levelUpgradeAmount ���������ĵȼ�����һ����˵Ϊ1��Ҳ���������һ����������
	virtual void UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount) = 0;
};

