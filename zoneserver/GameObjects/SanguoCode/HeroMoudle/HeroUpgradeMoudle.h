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
	///@breif 装备武将
	bool _EquipHero(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	/**@brief 武将进阶*/
	bool _HeroRankRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	/**@brief 武将升星*/
	bool _HeroStarLevelRise(SBaseHeroUpgrade *pMsg, CBaseDataManager* pBaseDataMgr);
	///@brief 招募武将
	bool _ConscribeHero(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr);
	///@brief 武将升级
	bool _HeroUpgradeLevel(SBaseHeroUpgrade* pMsg, CBaseDataManager* pBaseDataMgr);
private:
	const CGlobalConfig &globalConfig;
	CAchieveUpdate* m_ptrAcievementUpdate; ///<更新武将升阶、武将收集的相关终身成就逻辑
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator;///<更新武将升阶、武将收集的相关随机生成的成就逻辑
};

