#pragma once
#include "Networkmodule\ItemTypedef.h"

class CWareHouse
{
public:
    void OnDispatchMsg(struct SWareHouseBaseMsg *pMsg);

public:
    void OnRecvOpenItemWareHouseMsg(struct SQOpenItemWareHouseMsg *pMsg);
    void OnRecvCloseWareHouseMsg(struct SQCloseWareHouseMsg *pMsg);
    void OnRecvMoveItemInWareHouseMsg(struct SQMoveItemInWareHouseMsg *pMsg);
    void OnRecvMoveItemOutWareHouseMsg(struct SQMoveItemOutWareHouseMsg *pMsg);
    void OnRecvMoveItemSelfWareHouseMsg(struct SQMoveSelfItemWareHouseMsg *pMsg);
	void OnRecvExChangeSelfWareHouseMsg(struct SQExChangeItemWareHouseMsg *pMsg);
    void OnRecvLockWareHouseMsg(struct SQLockWareHouseMsg *pMsg);
	void OnRecvActiveWareHouseMsg(struct SQActiveWareHouseMsg *pMsg);
	void WRLostStatus();

	WORD GetValidPosEnd(BYTE byActiveTimes) const;

public:
    void TheEnd(BOOL backupOnly = FALSE);

public:
    void ArrangeWareHouse();

public:
    CWareHouse(void);
    ~CWareHouse(void);

protected:
	DWORD	m_dwOperationSerial;			// 本次操作的校验序列号
	DWORD	m_WHArrangeIntervalTime;		// 整理间隔时间

private:
	enum
	{
		WHT_ITEM = 1,	// 道具
		WHT_MOUNT,		// 骑乘
		WHT_PET,		// 宠物

		WHT_MAX,
	};

	BYTE	m_WareHouseType;				// 打开的仓库类型
};
