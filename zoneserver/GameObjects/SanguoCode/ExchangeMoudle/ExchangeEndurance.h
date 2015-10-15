// ----- CExchangeEndurance.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   �ȶ������ı�ʯ���������������߼�����,����������Ƿ����ϵ�ExchangeManager��
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "DiamondExchangeProcess.h"
#include <functional>
#include <memory>
typedef function<void(int)> FunctionPtr;

struct SDiamondExchangeMsg;
struct SAExchangeEnduranceInfor;
struct SDiamondExchangeEnduranceMsg;
class CBaseDataManager;
class TimerEvent_SG;

class CExchangeEndurance : public CDiamondExchangeProcess
{
public:
	CExchangeEndurance(CBaseDataManager& baseDataMgr);
	virtual ~CExchangeEndurance();

	///@brief ��ʼ���������߼������ݵ�����
	virtual void Init(void* data);
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg);
	///@brief ����Ԫ�����������������
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	virtual void ResetData(int sendMsg = 0);

private:
	///@brief ����һ�һ��
	void RequestExchange();

	int* m_ptrExchangedTimes;///<�Ѿ��һ��Ĵ���
	int m_iCurAvaliableCount;///<��ǰ������жһ��Ĵ���
	CBaseDataManager& m_baseDataMgr;
	shared_ptr<SAExchangeEnduranceInfor> m_ptrExchangeInforMsg;
	shared_ptr<SDiamondExchangeEnduranceMsg> m_ptrRefreshExchangeEnduranceInforMsg;
};

