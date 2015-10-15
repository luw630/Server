// ----- CTreasureHuntingDuplicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的秘境寻宝副本的管理类
// --------------------------------------------------------------------
//   --  管理副本的初始化以及验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include "DuplicateBaseManager.h"

class CTreasureHuntingDuplicateManager : public CDuplicateBaseManager
{
public:
	CTreasureHuntingDuplicateManager(CExtendedDataManager& dataMgr);
	virtual ~CTreasureHuntingDuplicateManager();

	virtual void InitMgr();
	virtual void Update();

protected:
	///@brief 处理客户端发来的消息
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);

private:
};

