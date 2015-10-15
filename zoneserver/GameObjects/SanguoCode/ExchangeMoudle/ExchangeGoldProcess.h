// ----- CExchangeGoldProcess.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/11
//   --  Desc:   萌斗三国的宝石换金钱的数据与逻辑处理,将来视情况是否整合到ExchangeManager中
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

	///@brief 初始化，建立逻辑和数据的链接
	virtual void Init(void* data);
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg);
	///@brief 重置元宝换金钱的相关数据
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	virtual void ResetData(int sendMsg = 0);

private:
	
	///@brief 申请兑换一次
	void RequestExchangeOnce();
	///@brief 申请连续的兑换
	void RequestExchangeContinuous();

	int* m_ptrExchangedTimes;///<已经兑换的次数
	int m_iCurAvaliableCount;///<当前允许进行兑换的次数
	CBaseDataManager& m_baseDataMgr;
	shared_ptr<SAExchangeGoldInfor> m_ptrExchangeGoldInforMsg;
	shared_ptr<SDiamondExchangeGoldMsg> m_ptrRefreshExchangeGoldInforMsg;
};

