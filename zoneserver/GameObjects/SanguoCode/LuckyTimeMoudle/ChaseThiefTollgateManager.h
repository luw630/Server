//----------ChaseThiefTollgateManager.h------------------
//	------	Author : LiuWeiWei	------
//	------	Date : 04/10/2015	------
//	------	Desc : 追击盗贼关卡管理类
//--------------------------------------------------------------

#pragma once
#include <map>
#include <set>
#include <vector>
#include <memory>
using namespace std;
class CChaseThiefTollgate;
class CBaseDataManager;
//struct SATollgateEarningClearedInfor;
struct SThiefAnswerStartChanllenge;
enum CheckResult;

class CChaseThiefTollgateManager
{
public:
	CChaseThiefTollgateManager();
	~CChaseThiefTollgateManager();
	///@brief 初始化关卡
	void Init();
	///@brief 准备挑战关卡,选择贴近等级的关卡
	bool PerpationChanllenge(int playerLevel);
	///@brief 开始挑战 随机掉落物品
	bool Action(CBaseDataManager* pBaseDataManager);
	///@brief 关卡结算
	void EarningClear(CBaseDataManager* pBaseDataManager);
	int GetCurTollgateID() const;
private:
	shared_ptr<CChaseThiefTollgate> m_pCurTollgate;	///<当前挑战的关卡
	shared_ptr<SThiefAnswerStartChanllenge> m_pDropOutMsg;
	map<int, shared_ptr<CChaseThiefTollgate>> m_mapTollgateContainer;
	//shared_ptr<SATollgateEarningClearedInfor> m_pMsgTollgateClearingAnswer;	///<关卡结算成功后发往客户端的消息
};

