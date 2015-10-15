#include "stdafx.h"
#include "HeroFateInterrogator.h"
#include "../Common/ConfigManager.h"
#include "HeroDataManager.h"


CHeroFateInterrogator::CHeroFateInterrogator()
	: m_FateConfigMap(CConfigManager::getSingleton()->GetFateConfigMap())
{
}


CHeroFateInterrogator::~CHeroFateInterrogator()
{
}

const vector<int32_t>* CHeroFateInterrogator::GetRelatedHeroes(int32_t heroID)
{
	auto fIter = m_RelatedHeroMap.find(heroID);
	if (fIter != m_RelatedHeroMap.end())
	{
		return &fIter->second;
	}

	vector<int32_t> heroes;
	FateConfig* pConfig = nullptr;
	for (auto & iter : m_FateConfigMap)
	{
		for (int i = 0, count = iter.second.size(); i < count; ++i)
		{
			pConfig = &iter.second[i];
			if (find(pConfig->vecRelationshipHero.begin(), pConfig->vecRelationshipHero.end(), heroID) != pConfig->vecRelationshipHero.end())
			{
				heroes.push_back(iter.first);
				break;
			}
		}
	}

	m_RelatedHeroMap.insert(make_pair(heroID, move(heroes)));
	return &heroes;
}

void CHeroFateInterrogator::GetActivatedFateConfigs(IN int32_t heroID, IN const CHeroDataManager* pHeroDataMgr, OUT vector<FateConfig*>& vecFateConfigs)
{
	auto fIter = m_FateConfigMap.find(heroID);
	if (fIter == m_FateConfigMap.end())
	{
		return;
	}

	bool activated = false;
	for (auto & config : fIter->second)
	{
		if (!pHeroDataMgr->ExistHero(config.heroID))
			continue;
		activated = true;
		for (int i = 0, count = config.vecRelationshipHero.size(); i < count; ++i)
		{
			if (!pHeroDataMgr->ExistHero(config.vecRelationshipHero[i]))
			{
				activated = false;
				break;
			}
		}

		if (activated)
			vecFateConfigs.push_back(&config);
	}
}
