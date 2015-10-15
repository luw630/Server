// ----- CDiamondExchangeModule.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   萌斗三国的元宝兑换其他物件的模块类，现在暂时主管消息的分发
// --------------------------------------------------------------------
//	 --  独立出来因为现在就有兑换金钱、兑换体力的模块，将来可能会有其他模块，继续加进来。且消息可在EPRO_SANGUO_DIAMONDEXCHANGE之下，不会造成SSGPlayerMsg的消息过于繁杂
//---------------------------------------------------------------------  
#pragma once
#include <unordered_map>
#include <memory>

class CDiamondExchangeProcess;

class CDiamondExchangeModule
{
public:
	CDiamondExchangeModule();
	~CDiamondExchangeModule();

	///@brief 添加元宝兑换其他物件的操作
	void AddProcesser(int processerType, shared_ptr<CDiamondExchangeProcess> processer);
	///@brief 到刷新点了刷新数据
	void RefreshData();
	void RecvMsg(const SMessage *pMsg);

private:
	unordered_map<int, shared_ptr<CDiamondExchangeProcess>> m_ProcesserList;///<当前的宝石兑换操作的集合
};

