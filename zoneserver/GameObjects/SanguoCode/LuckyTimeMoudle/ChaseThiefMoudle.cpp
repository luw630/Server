#include "stdafx.h"
#include "ChaseThiefMoudle.h"
#include "ChaseThiefDataManager.h"
#include "ChaseThiefTollgateManager.h"
#include "../BaseDataManager.h"
#include "../MailMoudle/MailMoudle.h"
#include "CMystring.h"

CChaseThiefMoudle::CChaseThiefMoudle()
{
}


CChaseThiefMoudle::~CChaseThiefMoudle()
{
}

void CChaseThiefMoudle::OnRecvive(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SChaseThiefMsg* pMsg)
{
	if (nullptr == pThiefDataMgr || nullptr == pThiefTollgateMgr || nullptr == pMsg)
		return;

	switch (pMsg->_protocol)
	{
	case SChaseThiefMsg::CHASETHIEF_PERPATIONCHANLLENGE:
		{
			SThiefRequestPerpationChanllenge* pPerpation = static_cast<SThiefRequestPerpationChanllenge*>(pMsg);
			if (nullptr == pPerpation)
				return;
			PerpationChanllengeProcess(pThiefDataMgr, pThiefTollgateMgr, pPerpation);
		}
		break;
	case SChaseThiefMsg::CHASETHIEF_STARTCHANLLENGE:
		{
			SThiefRequestStartChanllenge* pStart = static_cast<SThiefRequestStartChanllenge*>(pMsg);
			if (nullptr == pStart)
				return;
			StartChanllengeProcess(pThiefDataMgr, pThiefTollgateMgr, pStart);
		}
		break;
	case SChaseThiefMsg::CHASETHIEF_CLEARING:
		{
			SThiefRequestClearing* pClearing = static_cast<SThiefRequestClearing*>(pMsg);
			if (nullptr == pClearing)
				return;
			ClearingProcess(pThiefDataMgr, pThiefTollgateMgr, pClearing);
		}
		break;
	default:
		break;
	}
}

void CChaseThiefMoudle::PerpationChanllengeProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestPerpationChanllenge* pMsg)
{
	SThiefAnswerPerpationChanllenge msg;
	CBaseDataManager& baseDataMgr = pThiefDataMgr->GetBaseDataMgr();

	if (pThiefDataMgr->CheckEffectiveness())
	{
		msg.permission = pThiefTollgateMgr->PerpationChanllenge(baseDataMgr.GetPlayerLevel());
		msg.tollgateID = pThiefTollgateMgr->GetCurTollgateID();
	}

	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefAnswerPerpationChanllenge));
}

void CChaseThiefMoudle::StartChanllengeProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestStartChanllenge* pMsg)
{
	SThiefRefusalEnter msg;
	msg.loseEffectiveness = false;
	CBaseDataManager& baseDataMgr = pThiefDataMgr->GetBaseDataMgr();
	/// 如果追击无效 或已经开始追击 则退出
	if (pThiefDataMgr->CheckEffectiveness() == false)
	{
		msg.loseEffectiveness = true;
		g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefRefusalEnter));
		return;
	}

	if (pThiefDataMgr->GetChaseFlag())
	{
		g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefRefusalEnter));
		return;
	}

	if (pThiefTollgateMgr->Action(&baseDataMgr))
	{
		pThiefDataMgr->SetChasing(true);
		pThiefDataMgr->SetChasingHeroes(pMsg->heroes, MAX_THIEF_HERO_LIMIT);
		pThiefDataMgr->RandomReward();
		return;
	}

	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefRefusalEnter));
}

void CChaseThiefMoudle::ClearingProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestClearing* pMsg)
{
	///<不论怎样 先将追击小偷做失效处理
	pThiefDataMgr->LoseEffectiveness();

	SThiefAnswerClearing msg;
	CBaseDataManager& baseDataMgr = pThiefDataMgr->GetBaseDataMgr();
	if (pThiefDataMgr->GetChaseFlag() == false)
	{
		msg.complete = false;
		g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefAnswerClearing));
	}

	//pThiefDataMgr->Clearing(pMsg->win);
	msg.complete = pMsg->win;

	g_StoreMessage(baseDataMgr.GetDNID(), &msg, sizeof(SThiefAnswerClearing));

	LPCSTR text = CMyString::GetInstance().GetFormatString("THIEF_TIP");
	TalkToDnid(baseDataMgr.GetDNID(), text);

	MailInfo mailInfo;
	memset(&mailInfo, 0, sizeof(MailInfo));
	text = CMyString::GetInstance().GetFormatString("THIEF_TITLE");
	char* utf8Str = g_AnsiToUtf8(text);
	dwt::strcpy(mailInfo.title, (LPCSTR)utf8Str, sizeof(mailInfo.title));
	if (nullptr != utf8Str)
		delete[] utf8Str;
	//strcpy_s(mailInfo.title, text);
	text = CMyString::GetInstance().GetFormatString("THIEF_CONTENT");
	utf8Str = g_AnsiToUtf8(text);
	dwt::strcpy(mailInfo.content, (LPCSTR)utf8Str, sizeof(mailInfo.content));
	if (nullptr != utf8Str)
		delete[] utf8Str;
	//strcpy_s(mailInfo.content, text);
	text = CMyString::GetInstance().GetFormatString("THIEF_SENDERNAME");
	utf8Str = g_AnsiToUtf8(text);
	dwt::strcpy(mailInfo.senderName, (LPCSTR)utf8Str, sizeof(mailInfo.senderName));
	if (nullptr != utf8Str)
		delete[] utf8Str;
	//strcpy_s(mailInfo.senderName, text);
	mailInfo.szItemInfo[0].itype = GoodsType::item;
	pThiefDataMgr->FillReward(pMsg->win, mailInfo.szItemInfo[0].id, mailInfo.szItemInfo[0].num);
	MailMoudle::getSingleton()->SendMail_SanGuo(baseDataMgr.GetSID(), 1440/*1440分钟后失效*/, &mailInfo);
}