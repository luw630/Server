#pragma once
#include"../PUB/Singleton.h"
#include "Networkmodule\SanguoPlayerMsg.h"

class CBaseDataManager;
class CHeroDataManager;
class CGlobalConfig;
class CAchieveUpdate;
class CRandomAchieveUpdate;
class CHeroUpgradeMoudle : public CSingleton<CHeroUpgradeMoudle>
{
public:
	CHeroUpgradeMoudle();
	~CHeroUpgradeMoudle();
	void DispatchHeroUpgradeMsg(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);

private:
	///@breif װ���佫
	bool _EquipHero(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	/**@brief �佫����*/
	bool _HeroRankRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	/**@brief �佫����*/
	bool _HeroStarLevelRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	///@brief ��ļ�佫
	bool _ConscribeHero(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr);
	///@brief �佫����
	bool _HeroUpgradeLevel(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr);
private:
	const CGlobalConfig &globalConfig;
	CAchieveUpdate* m_ptrAcievementUpdate; ///<�����佫���ס��佫�ռ����������ɾ��߼�
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator;///<�����佫���ס��佫�ռ������������ɵĳɾ��߼�
};

