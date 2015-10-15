//----------ChaseThiefTollgateManager.h------------------
//	------	Author : LiuWeiWei	------
//	------	Date : 04/10/2015	------
//	------	Desc : ׷�������ؿ�������
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
	///@brief ��ʼ���ؿ�
	void Init();
	///@brief ׼����ս�ؿ�,ѡ�������ȼ��Ĺؿ�
	bool PerpationChanllenge(int playerLevel);
	///@brief ��ʼ��ս ���������Ʒ
	bool Action(CBaseDataManager* pBaseDataManager);
	///@brief �ؿ�����
	void EarningClear(CBaseDataManager* pBaseDataManager);
	int GetCurTollgateID() const;
private:
	shared_ptr<CChaseThiefTollgate> m_pCurTollgate;	///<��ǰ��ս�Ĺؿ�
	shared_ptr<SThiefAnswerStartChanllenge> m_pDropOutMsg;
	map<int, shared_ptr<CChaseThiefTollgate>> m_mapTollgateContainer;
	//shared_ptr<SATollgateEarningClearedInfor> m_pMsgTollgateClearingAnswer;	///<�ؿ�����ɹ������ͻ��˵���Ϣ
};

