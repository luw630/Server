#include "stdafx.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "HeroExtendsGameplayModel.h"
#include "HeroHungingHunting.h"
#include "HeroTraining.h"

CHeroExtendsGameplayModel::CHeroExtendsGameplayModel(CBaseDataManager& baseDataManager)
{
	m_ptrHeroTrainingMgr.reset(new CHeroTraining(baseDataManager));
	m_ptrHeroHuntingMgr.reset(new CHeroHungingHunting(baseDataManager));
}


CHeroExtendsGameplayModel::~CHeroExtendsGameplayModel()
{
}

void CHeroExtendsGameplayModel::OnRecvMsg(SSGPlayerMsg* pMsg)
{
	const SHeroExtendsGamplayMsg* ptrMsg = static_cast<const SHeroExtendsGamplayMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	switch (ptrMsg->_protocol)
	{
	case SHeroExtendsGamplayMsg::HERO_HUNGING:
		if (m_ptrHeroHuntingMgr != nullptr)
			m_ptrHeroHuntingMgr->DispachMsg(ptrMsg);
		break;
	case SHeroExtendsGamplayMsg::HERO_TRAINING:
		if (m_ptrHeroTrainingMgr != nullptr)
			m_ptrHeroTrainingMgr->DispachMsg(ptrMsg);
		break;
	}
}
