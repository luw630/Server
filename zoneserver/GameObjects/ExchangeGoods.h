#pragma once
#include "Networkmodule\ItemTypedef.h"

// ���׹���˵��
// [������]���׵������ɽ���������ܷ�ȷ�Ϻ󣬸��ݷ�����Ϣ�����������������������׷�[this]�����ܽ��׷���
// [�ͻ���]���������ߵ�������������Ʒ��

// [������]�����У����������յ�MoveItem����Ϣ������Ŀǰ���ƶ���Ϣͬʱ�������ƶ���ʲô����ϣ�������ʱ���ã�������̫��Ķ���
// [�ͻ���]Ҳ�Ǹ�����ǰ��ʵ��ԭ����ʱ�����AddItem&DelItem��������Ʒ���ƶ�����ʵ�����ڷ���������Ʒ��û���ƶ���

// [������]���������յ�˫���Ľ���ȷ����Ϣ������˫����ȷ�Ϻ󣬷�����������ص���Ʒ����
// [�ͻ���]����AddItem&DelItem����Ӧ����������Ĳ���

// [������]���׹����У�������������ʱ�յ�ĳ�����׶���ĵ�ȡ������������Ӧ�Ĵ���
// [�ͻ���]����AddItem&DelItem����Ӧ��������

#include "networkmodule\playertypedef.h"
//wk 20150427 �Ż��������
//static const int MAX_EXCHANGE_ITEM_NUMBER = 12;
static const int MAX_EXCHANGE_ITEM_NUMBER = 1;

struct SExchangeBox
{
    SPackageItem    m_GoodsArray[MAX_EXCHANGE_ITEM_NUMBER];     // ���׵ĵ���
	WORD			m_SellNumber;								// �ѷ���ĵ�����Ŀ
    DWORD           m_dwMoney;                                  // ���׵Ľ�Ǯ
    DWORD           m_dwCardPoint;                              // ���׵�Ԫ��
	bool			m_bLocked;									// �Ƿ�����
    bool            m_bCommit;                                  // �Ƿ�ʼ����
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
    DWORD           m_dwOtherPlayerGID;			// ���׶Է���GID������ڽ���˫����˵�ģ�
    DWORD           m_dwSrcPlayerGID;			// ���׷����˵�GID
    DWORD           m_dwDestPlayerGID;			// ����Ŀ���˵�GID

    SExchangeBox    m_MyBox;					// �Լ��Ľ�������
    SExchangeBox    m_YouBox;					// �Է��Ľ�������

	bool			m_IsMoneyLocked;			// ��Ǯ�Ƿ�����
};
