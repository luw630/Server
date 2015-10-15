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
	DWORD	m_dwOperationSerial;			// ���β�����У�����к�
	DWORD	m_WHArrangeIntervalTime;		// ������ʱ��

private:
	enum
	{
		WHT_ITEM = 1,	// ����
		WHT_MOUNT,		// ���
		WHT_PET,		// ����

		WHT_MAX,
	};

	BYTE	m_WareHouseType;				// �򿪵Ĳֿ�����
};
