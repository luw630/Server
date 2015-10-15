#pragma once
#include "Networkmodule\StallDef.h"

class CStall 
{
protected:
    void RecvMoveSaleItemMsg(struct SQMovetoSaleBoxMsg *pMsg);
    void RecvSeeSaleItemMsg(struct SQSeeSaleItemMsg *pMsg);
    void RecvBuySaleItemMsg(struct SQBuySaleItemMsg *pMsg);
    void RecvCloseSaleItemMsg(struct SQCloseSaleItemMsg *pMsg);
    void RecvSendSaleInfo(struct SQSendSaleInfoMsg *pMsg);
	void RecvChangePrice(struct SQChangeItemPriceMsg *pMsg);
	void RecvChangeStallName(struct SQChangeStallNameMsg *pMsg);
	void RecvChangeStallInfo(struct SQChangeStallInfoMsg *pMsg);
	void RecvSendMessage(struct SQSendMessageMsg *pMsg);
	void SendLogInfoToAll(struct SASendMessageMsg &logInfoMsg);

public:
    CStall(void);
    ~CStall(void);

	virtual void AddCheckID(void)=0;
	void DispatchSaleMessage(struct SSaleItemBaseMsg *pMsg);    
    
	WORD FindPos();
	BOOL FillStallPos(WORD pos, BYTE chk_value, BYTE fil_value);
	BOOL CheckPos(BYTE pos, BYTE chk_value) { return m_StallCells[pos] == chk_value ? TRUE : FALSE; }
	void AddCurPlayertoList(DWORD dwGID, DNID qwDNID);
	bool DelCurPlayerFromList(DWORD dwGID);
	WORD GetSaleItemNum(void) const { return m_ForSaleNumber; }
	SaleItem* GetItemList() { return m_StallItems; }
	void StallLoseStatus();
	
public:
	typedef std::map<DWORD, DNID> PLAYERMAP;

	struct SGIDDNID
	{
		DWORD gid;
		DNID dnid;
	};
	
	WORD		m_ForSaleNumber;				// ��������Ʒ�ĸ���
	BYTE		m_StallCells[MAX_SALEITEMNUM];
	SaleItem	m_StallItems[MAX_SALEITEMNUM];
	
	char		szStallName[MAX_STALLNAME];		// ̯λ��
	char		szSaleInfo[MAX_SALEINFO];		// ̯λ��Ϣ
	DWORD		ItemUpdateTime;					// �����ϴθ��µ�ʱ��
	PLAYERMAP	m_CurPlayerMap;					// �鿴�ҵ����
	DWORD		dwOtherGID;						// ���ڲ鿴�����

	// ������Ϣ
	DWORD					m_LogIndex;			// ���Եı��
	std::list<StallLogInfo>	m_Logs;				// ����
	std::map<DWORD, DWORD>	m_LogSendIndex;		// ����ĳ��������Ե������
};
