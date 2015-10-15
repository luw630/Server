#pragma once
#include"../PUB/Singleton.h"

class CBaseDataManager;
struct SQAskToForging;
struct SQAskToMoneyForging;
struct SAForgingPermission;

///@brief 服务器端锻造的相关逻辑处理
class CForgingManager : public CSingleton<CForgingManager>
{
public:
	CForgingManager();
	~CForgingManager();

	void OnRecvMsg(const SMessage *pMsg, CBaseDataManager *pBaseDataMgr);

private:
	///@brief 申请元宝祈福
	void AskToDiamondForging(const SQAskToForging *pMsg, SAForgingPermission& permissionMsg);
	///@brief 申请金钱祈福
	void AskToMoneyForging(const SQAskToForging* pMsg, SAForgingPermission& permissionMsg);

	CBaseDataManager *m_ptrBaseDataManager;
};

