// ----- CStoryDulicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的普通故事副本的管理类
// --------------------------------------------------------------------
//   --  管理副本的初始化以及验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include <functional>
#include "DuplicateBaseManager.h"
#include <vector>
#include <memory>

typedef std::function<void()> EmptyFunction;
class CDuplicateExtraRewards;

class CStoryDulicateManager : public CDuplicateBaseManager
{
public:
	CStoryDulicateManager(CExtendedDataManager& dataMgr);
	virtual ~CStoryDulicateManager();

	virtual void InitMgr();
	virtual void Update();
	///@brief 用来激活普通副本（又名故事副本）到最新的副本和关卡
	virtual void ActiveTo(int tollgateID);
	virtual void BindClearingEvent(EmptyFunction& func) final;

protected:
	///@brief 特异化的初始化逻辑
	virtual void SpecificInitialization();
	///@brief 处理客户端发来的消息
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);
	///@brief 当客户端申请挑战某一个关卡的时候，子类会做的一些操作
	virtual void OnAskToEnterTollgate();
	///@brief 当客户端准备进入战斗场景刷怪的时候，要将对应关卡的掉落信息给发给客户端
	virtual void OnStartToChallenge();
	///@brief 检测是否能进行扫荡操作
	virtual void CheckMopUpOperationPermission(const SDuplicateMsg *pMsg) final;
	///@brief 副本结算的一些额外操作
	virtual void EarningClearingExtraProgress() final;
	///@brief 副本相关的活动在副本结算时要做的操作
	virtual void ActivityClearingProgress();

	bool m_bMopUpOperation;		///<是否为扫荡操作
	bool m_bDuplicateActivated;	///<副本是否已经激活了
	int  m_iMopUpTicketID;
	std::vector<EmptyFunction> m_funcClearing;	///<结算回调事件
	unique_ptr<CDuplicateExtraRewards> m_ptrDuplicateExtraRewards; ///<关卡额外的奖励管理
};

