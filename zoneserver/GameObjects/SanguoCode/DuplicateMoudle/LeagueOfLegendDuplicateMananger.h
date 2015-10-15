// ----- CLeagueOfLegendDuplicateMananger.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的将神传说副本的管理类
// --------------------------------------------------------------------
//   --  管理副本的初始化以及验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include "DuplicateBaseManager.h"

class CLeagueOfLegendDuplicateMananger : public CDuplicateBaseManager
{
public:
	CLeagueOfLegendDuplicateMananger(CExtendedDataManager& dataMgr);
	virtual ~CLeagueOfLegendDuplicateMananger();

	virtual void InitMgr();
	virtual void Update();

	///判断本类型副本的某些副本的激活状态
	void JudgeDuplicateAcitiveState();

protected:
	///@brief 处理客户端发来的消息
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);
	///@brief 设置激活某一系列的副本的消息的排他性状态，该消息的排他性状态是指指定的某类型的所有副本只有本消息中指定的可开放的副本才能开放
	virtual void GetActiveDuplicateExclusivityState(bool& exclusivityState) const;
};

