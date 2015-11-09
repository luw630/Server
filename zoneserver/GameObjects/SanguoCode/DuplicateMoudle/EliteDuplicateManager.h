// ----- CEliteDuplicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的精英副本管理类
// --------------------------------------------------------------------
//   --  管理副本的初始化以及验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include <memory>
#include "StoryDulicateManager.h"
using namespace std;

struct SAActivateDuplicate;

class CEliteDuplicateManager : public CStoryDulicateManager
{
public:
	CEliteDuplicateManager(CExtendedDataManager& dataMgr);
	virtual ~CEliteDuplicateManager();

protected:
	///@brief 特异化的初始化逻辑
	virtual void SpecificInitialization();
	///@brief 处理客户端发来的消息
	virtual void DispatchMsg(const SDuplicateMsg* pMsg) final;
	///@brief 当客户端申请挑战某一个关卡的时候，子类会做的一些操作
	virtual void OnAskToEnterTollgate();
	///@brief 副本相关的活动在副本结算时要做的操作
	virtual void ActivityClearingProgress();
	///@brief 激活本副本，让玩家可以挑战
	void Activate();
	///@brief "重置某一个关卡的挑战次数"功能的元宝花费获取
	///@return 成功放回对应的值，失败返回-1
	virtual int GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID);

private:
};

