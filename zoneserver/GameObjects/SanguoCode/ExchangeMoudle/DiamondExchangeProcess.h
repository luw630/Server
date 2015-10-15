// ----- CDiamondExchangeProcess.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   萌斗三国的元宝兑换其他物件的处理抽象类
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once

struct SDiamondExchangeMsg;
class CBaseDataManager;

class CDiamondExchangeProcess
{
public:
	CDiamondExchangeProcess(){};
	virtual ~CDiamondExchangeProcess(){};

	///@brief 初始化，建立逻辑和数据的链接
	virtual void Init(void* data) = 0;
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg) = 0;
	///@brief 重置元宝置换其他物件的相关数据
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	virtual void ResetData(int sendMsg = 0) = 0;
};

