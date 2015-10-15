// ----- CExchangeEndurance.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   萌斗三国的宝石换体力的数据与逻辑处理,将来视情况是否整合到ExchangeManager中
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

	///@brief 初始化，建立逻辑和数据的链接
	virtual void Init(void* data);
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg);
	///@brief 重置元宝换体力的相关数据
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	virtual void ResetData(int sendMsg = 0);

private:
	///@brief 申请兑换一次
	void RequestExchange();

	int* m_ptrExchangedTimes;///<已经兑换的次数
	int m_iCurAvaliableCount;///<当前允许进行兑换的次数
	CBaseDataManager& m_baseDataMgr;
	shared_ptr<SAExchangeEnduranceInfor> m_ptrExchangeInforMsg;
	shared_ptr<SDiamondExchangeEnduranceMsg> m_ptrRefreshExchangeEnduranceInforMsg;
};

