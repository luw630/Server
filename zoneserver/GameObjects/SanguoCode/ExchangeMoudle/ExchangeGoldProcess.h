// ----- CExchangeGoldProcess.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/11
//   --  Desc:   �ȶ������ı�ʯ����Ǯ���������߼�����,����������Ƿ����ϵ�ExchangeManager��
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "DiamondExchangeProcess.h"
#include <functional>
#include <memory>
typedef function<void(int)> FunctionPtr;

struct SDiamondExchangeMsg;
struct SAExchangeGoldInfor;
struct SDiamondExchangeGoldMsg;
class CBaseDataManager;
class TimerEvent_SG;

class CExchangeGoldProcess : public CDiamondExchangeProcess
{
public:
	CExchangeGoldProcess(CBaseDataManager& baseDataMgr);
	virtual ~CExchangeGoldProcess();

	///@brief ��ʼ���������߼������ݵ�����
	virtual void Init(void* data);
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg);
	///@brief ����Ԫ������Ǯ���������
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	virtual void ResetData(int sendMsg = 0);

private:
	
	///@brief ����һ�һ��
	void RequestExchangeOnce();
	///@brief ���������Ķһ�
	void RequestExchangeContinuous();

	int* m_ptrExchangedTimes;///<�Ѿ��һ��Ĵ���
	int m_iCurAvaliableCount;///<��ǰ������жһ��Ĵ���
	CBaseDataManager& m_baseDataMgr;
	shared_ptr<SAExchangeGoldInfor> m_ptrExchangeGoldInforMsg;
	shared_ptr<SDiamondExchangeGoldMsg> m_ptrRefreshExchangeGoldInforMsg;
};

