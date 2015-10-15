#pragma once
#include "../PUB/Singleton.h"
#include "../../../NETWORKMODULE/SanguoPlayerMsg.h"
class CChaseThiefDataManager;
class CChaseThiefTollgateManager;

class CChaseThiefMoudle :
	public CSingleton < CChaseThiefMoudle >
{
public:
	CChaseThiefMoudle();
	~CChaseThiefMoudle();
	void OnRecvive(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SChaseThiefMsg* pMsg);
private:
	void PerpationChanllengeProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestPerpationChanllenge* pMsg);
	void StartChanllengeProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestStartChanllenge* pMsg);
	void ClearingProcess(CChaseThiefDataManager* pThiefDataMgr, CChaseThiefTollgateManager* pThiefTollgateMgr, SThiefRequestClearing* pMsg);
};

