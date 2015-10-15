#pragma once
#include"../PUB/Singleton.h"

class CBaseDataManager;
struct SQAskToForging;
struct SQAskToMoneyForging;
struct SAForgingPermission;

///@brief �������˶��������߼�����
class CForgingManager : public CSingleton<CForgingManager>
{
public:
	CForgingManager();
	~CForgingManager();

	void OnRecvMsg(const SMessage *pMsg, CBaseDataManager *pBaseDataMgr);

private:
	///@brief ����Ԫ����
	void AskToDiamondForging(const SQAskToForging *pMsg, SAForgingPermission& permissionMsg);
	///@brief �����Ǯ��
	void AskToMoneyForging(const SQAskToForging* pMsg, SAForgingPermission& permissionMsg);

	CBaseDataManager *m_ptrBaseDataManager;
};

