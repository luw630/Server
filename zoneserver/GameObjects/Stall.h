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
	
	WORD		m_ForSaleNumber;				// 想卖的物品的个数
	BYTE		m_StallCells[MAX_SALEITEMNUM];
	SaleItem	m_StallItems[MAX_SALEITEMNUM];
	
	char		szStallName[MAX_STALLNAME];		// 摊位名
	char		szSaleInfo[MAX_SALEINFO];		// 摊位信息
	DWORD		ItemUpdateTime;					// 道具上次更新的时间
	PLAYERMAP	m_CurPlayerMap;					// 查看我的玩家
	DWORD		dwOtherGID;						// 我在查看的玩家

	// 聊天消息
	DWORD					m_LogIndex;			// 留言的编号
	std::list<StallLogInfo>	m_Logs;				// 留言
	std::map<DWORD, DWORD>	m_LogSendIndex;		// 发给某个玩家留言的最后编号
};
