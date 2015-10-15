#pragma once
#include "../NETWORKMODULE/SanguoPlayerMsg.h"
#include "../PUB/Singleton.h"
class CSkillUpgradeManager;
class CRandomAchieveUpdate;
struct CGlobalConfig;

class CSkillUpgradeMoudle
	: public CSingleton<CSkillUpgradeMoudle>
{
public:
	CSkillUpgradeMoudle();
	~CSkillUpgradeMoudle();

	void DispatchSkillUpgradeEvent(SSkillUpgrade* pMsg, CSkillUpgradeManager* pSkillUpgradeMgr);
private:
	void _SkillUpgradeProcess(SQUpgradeHeroSkill* pMsg, CSkillUpgradeManager* pSkillUpgradeMgr);
	void _BuySkillPointProcess(CSkillUpgradeManager* pSkillUpgradeMgr);
private:
	CGlobalConfig& m_GlobalConfig;
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator;
};

