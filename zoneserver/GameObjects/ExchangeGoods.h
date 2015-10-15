#pragma once
#include "Networkmodule\ItemTypedef.h"

// 交易过程说明
// [服务器]交易的启动由交易请求接受方确认后，根据返回消息启动，启动参数：（请求交易方[this]，接受交易方）
// [客户端]两个交易者弹出交易面板和物品栏

// [服务器]交易中，服务器会收到MoveItem的消息（介于目前的移动消息同时处理了移动到什么面板上，所以暂时沿用，以免有太大改动）
// [客户端]也是根据以前的实现原理，暂时会根据AddItem&DelItem来处理物品的移动，但实际上在服务器端物品是没有移动的

// [服务器]服务器会收到双方的交易确认消息，仅当双方都确认后，服务器处理相关的物品交换
// [客户端]根据AddItem&DelItem来响应服务器具体的操作

// [服务器]交易过程中，服务器可能随时收到某个交易对象的的取消操作，作相应的处理
// [客户端]根据AddItem&DelItem来响应撤销操作

#include "networkmodule\playertypedef.h"
//wk 20150427 优化玩家数据
//static const int MAX_EXCHANGE_ITEM_NUMBER = 12;
static const int MAX_EXCHANGE_ITEM_NUMBER = 1;

struct SExchangeBox
{
    SPackageItem    m_GoodsArray[MAX_EXCHANGE_ITEM_NUMBER];     // 交易的道具
	WORD			m_SellNumber;								// 已放入的道具数目
    DWORD           m_dwMoney;                                  // 交易的金钱
    DWORD           m_dwCardPoint;                              // 交易的元宝
	bool			m_bLocked;									// 是否被锁定
    bool            m_bCommit;                                  // 是否开始交易
};


class CExchangeGoods
{
public:
    void OnDispatchMsg(struct SExchangeBaseMsg *pMsg);

public:
	void RecvQuestExchangeMsg(struct SQQuestExchangeMsg *pMsg, bool Auto = false);
    void RecvPutItemToExchangeBoxMsg(struct SQPutItemToExchangeBoxMsg *pMsg);
    void RecvExchangeOperationMsg(struct SQExchangeOperationMsg *pMsg);
    void RecvRemoteExchangeOperationMsg(struct SQAQuestRemoteExchangeMsg *pMsg);

public:
    void QuestExchangeWith(DWORD dwGID);
    void StartExchange(CPlayer *pExchanger);
    void PutItemToExchangeBox(CPlayer *pDestPlayer, SPackageItem &item, WORD wDestX, WORD wDestY, bool isSrc);
    void ExchangeCancel(CPlayer *pDestPlayer, bool isSrc);
    void CommitExchange(CPlayer *pDestPlayer, bool isSrc, DWORD dwVerifyID4CardPoint = 0);
	void LockExchange(CPlayer *pDestPlayer, bool isSrc, bool isLock);
	void ExchgLoseStatus();
    void TheEnd();
    bool InExchange() { return m_dwOtherPlayerGID != 0; }
    void EndPrepareExchange(struct SExchangeFullInfo *lpEFI, int result);

public:
    CExchangeGoods(void);
    ~CExchangeGoods(void);

protected:
    DWORD           m_dwOtherPlayerGID;			// 交易对方的GID（针对于交易双方来说的）
    DWORD           m_dwSrcPlayerGID;			// 交易发起人的GID
    DWORD           m_dwDestPlayerGID;			// 交易目标人的GID

    SExchangeBox    m_MyBox;					// 自己的交易数据
    SExchangeBox    m_YouBox;					// 对方的交易数据

	bool			m_IsMoneyLocked;			// 金钱是否被锁定
};
