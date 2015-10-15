#pragma once

#include "NetModule.h"
// #include "���������//GameObjects/QuestCardPointModify.h"
#include "../pub/ConstValue.h"

// ���˺ŷ�����ͳ�Ʒ�����������������Ϣ�ķ���
// Ϊ�˷�ֹ�����г�����©������Ҫ�ܹ��жϷ������Ƿ�ƾ�����ӵ������Ƿ�����˸��ƣ�
// ���ڴ����ײ��ֵķ���Ϊ�����ֳ����������������Կ���ͳ���ܵĽ�����
// ��������������û��˵���ؼ�������������Ƚ��鷳������


// �Ѷ��ڵ㿨��ص�Ԫ�����ֵ���ϢҲͳһ�����
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �������ײ��ֵ���Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SPointModifyMsg, SMessage, SMessage::EPRO_POINTMODIFY_MESSAGE)
//{{AFX
EPRO_PREPARE_EXCHANGE,      // ���˺ŷ�������׼�����ף���ѯ�����Ƿ��㹻������һ�������ڽ��׵ĵ�������ʱ�ռ�
EPRO_CONFIRM_EXCHANGE,      // ���˺ŷ�������ȷ�Ͻ��ף�����ʱ�ռ�ĵ���ת�˵�ʵ��Ŀ������
EPRO_REFRESH_CARDPOINT,     // ֪ͨ���������ˢ��һ����������ĳ����ҵĵ�����Ϣ
EPRO_MONEYPOINT_BASE,		// ˢ��Ԫ�����ֺ�������Ԫ�������Ļ���
EPRO_EXCHANGEMC_BASE,		// ����һ��㿨�Ľ�����Ϣ
EPRO_EQUIVALENT_BASE,		// ����������׻��ࣨ������Ѱ汾��
EPRO_ENCOURAGEEMENT,        // �����齱
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



////=============================================================================================
//// ���˺ŷ�������׼�����ף���ѯ�����Ƿ��㹻������һ�������ڽ��׵ĵ�������ʱ�ռ�
//DECLARE_MSG(SPrepareExchangeMsg, SPointModifyMsg, SPointModifyMsg::EPRO_PREPARE_EXCHANGE)
//struct SQPrepareExchangeMsg : public SPrepareExchangeMsg
//{
//    SExchangeVerifyInfo vi;
//    dwt::stringkey<char [ACCOUNTSIZE]> SrcAccount;
//    dwt::stringkey<char [ACCOUNTSIZE]> DstAccount;
//    DWORD TradePoint;
//    DWORD SrcGID;
//    DWORD DstGID;
//};
//
//struct SAPrepareExchangeMsg : public SPrepareExchangeMsg
//{
//    enum
//    {
//        RET_SUCCESS,
//        RET_FAILED,
//        RET_NOTENOUGH_POINT,
//    };
//
//    BYTE result;
//    SExchangeVerifyInfo vi;
//};
////---------------------------------------------------------------------------------------------



////=============================================================================================
//// ���˺ŷ�������׼�����ף���ѯ�����Ƿ��㹻������һ�������ڽ��׵ĵ�������ʱ�ռ�
//DECLARE_MSG(SConfirmExchangeMsg, SPointModifyMsg, SPointModifyMsg::EPRO_CONFIRM_EXCHANGE)
//struct SQConfirmExchangeMsg : public SConfirmExchangeMsg
//{
//    SExchangeVerifyInfo vi;
//    dwt::stringkey<char [ACCOUNTSIZE]> SrcAccount;
//    dwt::stringkey<char [ACCOUNTSIZE]> DstAccount;
//    DWORD TradePoint;
//    DWORD SrcGID;
//    DWORD DstGID;
//
//    BOOL isConfirm; //OrCancel;
//};
////---------------------------------------------------------------------------------------------



//=============================================================================================
// ֪ͨ���������ˢ��һ����������ĳ����ҵĵ�����Ϣ
DECLARE_MSG(SRefreshCardPointMsg, SPointModifyMsg, SPointModifyMsg::EPRO_REFRESH_CARDPOINT)
struct SARefreshCardPointMsg : 
    SRefreshCardPointMsg
{
    DWORD dwGID;
    DWORD dwCardPoint;

	// ��������ADDHERE
};
//---------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////
// Ԫ�����ֵ���Ϣ
//=============================================================================================
DECLARE_MSG_MAP(SMoneyPointModifyMsg, SPointModifyMsg, SPointModifyMsg::EPRO_MONEYPOINT_BASE)
//{{AFX
EPRO_REFRESH_MONEYPOINT,	// ˢ��Ԫ������
EPRO_NOTIFY_MONEYPOINT,		// ͨ������ִ�����
//}}AFX
END_MSG_MAP()

// =============================================================
// ˢ��Ԫ������
//////////////////////////////////////////////////////////////////////
// Ԫ������ ��Ϊ���㿨������һ�� ����Ҳ�������
#include "MoneyPointDef.h"
DECLARE_MSG(SRefreshMoneyPointMsg, SMoneyPointModifyMsg, SMoneyPointModifyMsg::EPRO_REFRESH_MONEYPOINT)
struct SQRefreshMoneyPointMsg : public SRefreshMoneyPointMsg
{
	SMoneyPointNotify mpn;
	DWORD dwMoneyPoint;

	// char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

struct SARefreshMoneyPointMsg : public SRefreshMoneyPointMsg
{
	enum { R_INVALID_ACCOUNT, R_NOTENOUGH_POINT, R_OK, R_TIMEIN, R_TIMEOUT};
	
	DWORD dwGID;
	DWORD	dwRet;
    DWORD dwMoneyPoint;

	SMoneyPointNotify mpn;
};

DECLARE_MSG(SNotifyMoneyPointMsg, SMoneyPointModifyMsg, SMoneyPointModifyMsg::EPRO_NOTIFY_MONEYPOINT)
struct SQNotifyMoneyPointMsg : public SNotifyMoneyPointMsg
{
	enum { RET_OK, RET_FAIL };
	// char szAccount[ACCOUNTSIZE];
	DWORD dwOpeate;
	DWORD dwMoneyPoint;

    char streamData[MAX_ACCOUNT];
};
//---------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
// ����һ��㿨��Ϣ
//=============================================================================================
DECLARE_MSG_MAP(SExchangeMCMsg, SPointModifyMsg, SPointModifyMsg::EPRO_EXCHANGEMC_BASE)
//{{AFX
EPRO_REQUEST_MC,		// ��������һ��㿨/
EPRO_REQUEST_CM,		// �㿨�������
EPRO_REQUEST_MCINFO,	// ����쿴������Ϣ
EPRO_REQUEST_VALIDATE,  // ������֤�㿨������
//}}AFX
END_MSG_MAP()
// =============================================================
DECLARE_MSG(SRequestMCMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_MC)
struct SQRequestMCMsg : public SRequestMCMsg
{
	enum RMCType { RT_MC, RT_DELMEINFO };
	WORD wType;

	MCCLIENTINFO info;
};

DECLARE_MSG(SRequestCMMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_CM)
struct SQRequestCMMsg : public SRequestCMMsg
{
	MCCLIENTINFO info;
	char szCardNumber[30];
	char szCardPassword[30];
};

DECLARE_MSG(SRequestMCInfoMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_MCINFO)
struct SQRequestMCInfoMsg : public SRequestMCInfoMsg
{
	enum RType { RT_COM180, RT_COM600, RT_SZ50, RT_SZ100, RT_ME };
	WORD wType;
};

struct SARequestMCInfoMsg : public SRequestMCInfoMsg
{
	enum INFOType { IT_ALL, IT_ME };
	WORD	wType;
	WORD	wSize;
	WORD	wPage;			// ��������ݿ��ܻ��кܶ�ҳ��״�� ���ֵ����������ҳ
	MCCLIENTINFO	info[100];

	WORD  MySize()
	{
		return sizeof(SARequestMCInfoMsg) - sizeof(MCCLIENTINFO) * (100 - wSize);
	}
};

DECLARE_MSG(SRequestValidateMsg, SExchangeMCMsg, SExchangeMCMsg::EPRO_REQUEST_VALIDATE)
struct SQRequestValidateMsg : SRequestValidateMsg
{
	enum VTYPE { VT_CHECK, VT_COMPLETE };

	char szCardNumber[30];
	char szCardPassword[30];
	//char szAccount[ACCOUNTSIZE];			
	WORD  wOperateType;
	DWORD wCardType;       

    char streamData[MAX_ACCOUNT];
};

struct SARequestValidateMsg : SRequestValidateMsg
{
	enum STATE { ST_OK, ST_FAIL, ST_HADUSE, ST_INVALIDCARDORPWD, ST_ERRCARDTYPE, ST_COMPLETEERR };
	WORD wState;

	char szCardNumber[30];
	char szCardPassword[30];
	//char szAccount[ACCOUNTSIZE];
	DWORD wCardType;

    char streamData[MAX_ACCOUNT];
};

#include "networkmodule\eqvtypedef.h"

///////////////////////////////////////////////////////////
// �����������Ϣ
//=============================================================================================
DECLARE_MSG_MAP(SEquivalentModifyMsg, SPointModifyMsg, SPointModifyMsg::EPRO_EQUIVALENT_BASE)
//{{AFX
EPRO_GET_EQUIVALENT,	// ��ȡ��Ӧ����
EPRO_INC_EQUIVALENT,	// ���Ӷ�Ӧ����
EPRO_DEC_EQUIVALENT,	// �۵���Ӧ����
//}}AFX
END_MSG_MAP()

DECLARE_MSG(SGetEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_GET_EQUIVALENT)
struct SQGetEQVPointMsg : public SGetEQVPointMsg
{
	//EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

struct SAGetEQVPointMsg : public SGetEQVPointMsg
{
	SAGetEQVPointMsg() : dwEQVPoint(0)
	{
	}

//	EQUIVALENT type;
	DWORD dwEQVPoint;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SIncEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_INC_EQUIVALENT)
struct SQIncEQVPointMsg : public SIncEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	DWORD dwEQVPoint;
    char streamData[MAX_ACCOUNT];
};

struct SAIncEQVPointMsg : public SIncEQVPointMsg
{
//	EQUIVALENT type;
	DWORD dwEQVPoint;
	//char szAccount[ACCOUNTSIZE];
    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SDecEQVPointMsg, SEquivalentModifyMsg, SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT)
struct SQDecEQVPointMsg : public SDecEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	int dwEQVPoint;//���ѽ��

//	CONSUMABLETYPE consumable;
	int nIndex;
    int nNumber;
	int nLevel; //��ҵȼ�	

    char streamData[MAX_ACCOUNT];
};

struct SADecEQVPointMsg : public SDecEQVPointMsg
{
//	EQUIVALENT type;
	//char szAccount[ACCOUNTSIZE];

	int dwEQVPoint;//ʣ�����
	DWORD dwDecPoint;//�������ѿ۵��ĵ���

//	CONSUMABLETYPE consumable;
	int nIndex;
    int nNumber;

    char streamData[MAX_ACCOUNT];
};

DECLARE_MSG(SEncouragement, SPointModifyMsg, SPointModifyMsg::EPRO_ENCOURAGEEMENT)
struct SQEncouragement : public SEncouragement
{
    char m_szName[CONST_USERNAME];
	char m_szCardNumber[30];	
};
struct SAEncouragement: public SEncouragement
{
  char m_szName[CONST_USERNAME];
  BYTE m_iRet;
  enum
  {  
	  RET_INVALIDCARD = 2,
      RET_ISUSEED = 3,
	  RET_WEEKOK = 101,		//����OK
	  RET_MONTH = 102, //����OK
	  RET_ERROR = 200//��ҽ�ɫ������
  };

};
