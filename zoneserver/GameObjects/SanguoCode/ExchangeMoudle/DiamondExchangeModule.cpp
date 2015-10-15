#include "stdafx.h"
#include "DiamondExchangeModule.h"
#include "DiamondExchangeProcess.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include "../networkmodule/playertypedef.h"


CDiamondExchangeModule::CDiamondExchangeModule()
{
}


CDiamondExchangeModule::~CDiamondExchangeModule()
{
}

void CDiamondExchangeModule::AddProcesser(int processerType, shared_ptr<CDiamondExchangeProcess> processer)
{
	if (processer == nullptr)
		return;

	m_ProcesserList.insert(make_pair(processerType, processer));
}

void CDiamondExchangeModule::RecvMsg(const SMessage *pMsg)
{
	const SDiamondExchangeMsg* ptrMsg = static_cast<const SDiamondExchangeMsg*>(pMsg);
	if (ptrMsg == nullptr)
		return;

	auto findResult = m_ProcesserList.find(ptrMsg->_protocol);
	if (findResult != m_ProcesserList.end() && findResult->second != nullptr)
		findResult->second->RecvMsg(ptrMsg);
}

void CDiamondExchangeModule::RefreshData()
{
	if (m_ProcesserList.size() <= 0)
		return;

	for (auto iter : m_ProcesserList)
	{
		///默认刷新后往服务器发送消息
		if (iter.second != nullptr)
			iter.second->ResetData(1);
	}
}
