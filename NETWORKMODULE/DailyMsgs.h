#pragma once
#include "NetModule.h"
//����ճ�������Ϣ
//=============================================================================================
DECLARE_MSG_MAP(SDailyMsg, SMessage, SMessage::EPRO_DAILY_MESSAGE)
//{{AFX
	//����ճ����Ϣ
EPRO_DAILY_INIT,	//��ʼ���ճ���������
EPRO_DAILY_ENTRY,	//��������ճ����������ȡ�ճ������
EPRO_DAILY_AWARD,	//���ؽ�����ϸ��Ϣ
EPRO_DAILY_LEAVE,	//�ͻ��������뿪�ճ������
EPRO_DAILY_CURBATCHMONSTERDIE,	//֪ͨ��ҵ�ǰ��������������һ�����ж�ó��֣�������Ѿ��ﵽ���ٲ�����

//��һ��Ϣ
EPRO_FIRSTPAY_UPDATEFLAGE,	//֪ͨ����״γ�ֵ��ʶ�ı�
EPRO_FIRSTPAY_AWARD,		//����״γ�ֵ����
EPRO_EVERYDAYPAY_AWARD,		//�����ȡÿ�ճ�ֵ����
EPRO_EVERYDAYPAY_GETAWARDSTATE,	//��ȡÿ�ճ�ֵ����״̬

EPRO_DAILY_NPCBUYSP,	//��е��NPC�����ع�������֮����Ʒ����Ϣ
EPRO_DAILY_OPENFLAG,	//����ʧ��԰����ű�־(��ҵ�½ʱҪ������ң��ͱ�־����ʱҪͬ���������������)

EPRO_DAILY_JULONGSHANRISK,	//��������ھ���ɽ��̽��

EPRO_TAROT_INITTAROTDATA,	//��ʼ���������������
EPRO_TAROT_TURNONONECARD,	//������󷭿�һ��������
EPRO_TAROT_GETAWARD,	//������������ƽ���
EPRO_TAROT_RESETTAROTDATA,	//�����������������
EPRO_TAROT_UPTPLAYANIMATEFLAG,	//���������Ʋ��Ŷ�����־

//��������ʱ�������
EPRO_CURDAYONLINE_INIT,	//��ʼ�����ÿ������ʱ��������Ϣ
EPRO_CURDAYONLINE_TIMELONG,		//�������ÿ������ʱ�����ҵ�������ʱ��
EPRO_CURDAYONLINE_GETAWARD,		//��ȡ��Ӧʱ��εĽ���
EPRO_CURDAYONLINE_RESET,	//֪ͨ��һ����
EPRO_CURDAYONLINE_ADDONEAWARD,		//��Ҵﵽһ���µĿ�����ȡ�Ľ���

//�ۼƵ�½�
EPRO_ADDUPLOGIN_INIT,	//��ʼ���ۼ����ߵ�½����
EPRO_ADDUPLOGIN_ADDUPDAY,	//���ҵ����ۼƵ�½����
EPRO_ADDUPLOGIN_ADDNEWAWARD,	//��Ҵﵽ������ȡ�µĽ���
EPRO_ADDUPLOGIN_GETAWARD,	//�����ȡ�ۼƵ�½��Ӧ����ID�Ľ���

//��ֵ�����
EPRO_RFB_INIT,	//��ʼ����ֵ��������
EPRO_RFB_GETAWARD,	//��ȡ��ֵ������Ӧ�Ľ���
EPRO_RFB_ADDNEWAWARD,	//֪ͨ������µĽ���������ȡ

//�ȼ������
EPRO_LR_INIT,	//��ʼ����ҵȼ���������
EPRO_LR_GETAWARD,	//��ȡ�ȼ���������
EPRO_LR_NOTIFYCANGETNEWAWARD,	//֪ͨ��ҿ�����ȡ�µĽ���
EPRO_LR_NOTIFYUPTRESIDUETIMES,	//֪ͨ�����������ʣ������ı�

//����Ħ���ֻ
EPRO_FW_LOGININIT,	//��ȡ����Ħ���ֵ�½��ʼ��������
EPRO_FW_GETOPENRESIDUETIME,	//��ȡ������Ħ���ֵĿ���ʣ��ʱ������
EPRO_FW_STARTGAME,	//��ʼ����Ħ������Ϸ
EPRO_FW_BROADCASTRECORDUPT,	//�㲥����Ħ������Ҽ�¼��Ϣ�������
EPRO_FW_GETAWARD,	//��ȡ����

EPRO_RECHARGE_SYNTATOLGLOD,		//ͬ����ҳ�ֵ�������

//}}AFX
END_MSG_MAP()

//��ʼ���ճ���������
DECLARE_MSG(SInitDailyMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_INIT)
struct SQInitDailyMsg : public SInitDailyMsg
{
};

struct SAInitDailyMsg : public SInitDailyMsg
{
	BYTE m_DailyArrSize;
	unsigned short *m_pDailyDetail;		//������1�����λ��ʾ�ճ����״̬��2�����ֽں����7��λ��ʾ�ճ����ʣ�������3�����ֽڱ�ʾ�ճ�ID
	BYTE *m_pDailyTotalTimes;	//�ճ���ܵ���Ѵ���
};

//�����ճ������
DECLARE_MSG(SEntryDailyMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_ENTRY)
struct SQEntryDailyMsg : public SEntryDailyMsg
{
	BYTE m_DailyID;
	BYTE m_IsFreeDaily;		//�Ƿ�Ϊ����ճ����ʶ��0Ϊ����ճ���1Ϊ�����ճ�
};

struct SAEntryDailyMsg : public SEntryDailyMsg
{
	BYTE m_Result;	// ����״̬  0�����������ʧ��  1����ɹ�  2�������ʧ��   4�ճ������ 5��ҵȼ��㹻�����ܽ����ճ��
	WORD m_NextSpendGlord;	//��һ�θ�����Ҫ�Ľ����
};

//���ؽ�����ϸ��Ϣ
DECLARE_MSG(SDailyAwardMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_AWARD)
struct SQDailyAwardMsg : public SDailyAwardMsg
{
	BYTE m_DailyID;
};

struct SADailyAwardMsg : public SDailyAwardMsg
{
	//BYTE m_AwardType;	//�������ͣ�0��Ӧ��ȡ����ʧ�ܣ�1��Ӧ�������ϵ��ߣ�2��Ӧ����������3��Ӧ�������飬4��Ӧ�������ң�5��Ӧ������ң�
	enum
	{
		AW_ITEMID,
		AW_ITEMNUM,
		AW_EXTRAITEMID,
		AW_EXTRAITEMNUM,
		AW_SP,
		AW_EXP,
		AW_JINBI,
		AW_YINBI,
	};
	DWORD m_AwardValue[8];	//������ֵ��0��Ӧ�������ϵ���ID��1��Ӧ�������ϵ�������, 2��Ӧ���⽱�����ϵ���ID��3��Ӧ���⽱�����ϵ�������,4��Ӧ����������5��Ӧ�������飬6��Ӧ�������ң�7��Ӧ������ң�
};

//�ͻ��������뿪�ճ������
DECLARE_MSG(SDailyLeaveMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_LEAVE)
struct SQDailyLeaveMsg : public SDailyLeaveMsg
{
};

//֪ͨ��ҵ�ǰ��������������һ�����ж�ó��֣�������Ѿ��ﵽ���ٲ�����
DECLARE_MSG(SNextMonsterRefreshMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_CURBATCHMONSTERDIE)
struct SANextMonsterRefreshMsg : public SNextMonsterRefreshMsg
{
	BYTE m_CurBatchMonster;	//��һ����Ϊ�ڼ�������
	WORD m_NextBatchRefreshTime;	//��һ���ֻ��ж��ˢ��
	WORD m_CurDailyID;	//��ǰ�����ճ��ID
};

//��һ��Ϣ
//֪ͨ����״γ�ֵ��ʶ�ı�
DECLARE_MSG(SUptFirstPayFlagMsg, SDailyMsg, SDailyMsg::EPRO_FIRSTPAY_UPDATEFLAGE)
struct SAUptFirstPayFlagMsg : public SUptFirstPayFlagMsg
{
	BYTE m_NewValue;
};

//����״γ�ֵ����
DECLARE_MSG(SFirstPayAwardMsg, SDailyMsg, SDailyMsg::EPRO_FIRSTPAY_AWARD)
struct SQFirstPayAwardMsg : public SFirstPayAwardMsg
{
};
struct SAFirstPayAwardMsg : public SFirstPayAwardMsg
{
	BYTE m_Result;
};


//�����ȡÿ�ճ�ֵ����
DECLARE_MSG(SEveryDayPayAwardMsg, SDailyMsg, SDailyMsg::EPRO_EVERYDAYPAY_AWARD)
struct SQEveryDayPayAwardMsg : public SEveryDayPayAwardMsg
{
	BYTE m_AwardIndex;	//����������1��ʼ
};
struct SAEveryDayPayAwardMsg : public SEveryDayPayAwardMsg
{
	BYTE m_Result;
};

//��ȡÿ�ճ�ֵ����״̬
#define EVERYDAYAWARDNUM 4

DECLARE_MSG(SGetEveryDayAwardStateMsg, SDailyMsg, SDailyMsg::EPRO_EVERYDAYPAY_GETAWARDSTATE)
struct SQGetEveryDayAwardStateMsg : public SGetEveryDayAwardStateMsg
{
};
struct SAGetEveryDayAwardStateMsg : public SGetEveryDayAwardStateMsg
{
	BYTE m_State[EVERYDAYAWARDNUM];
};


//��е��NPC�����ع�������֮����Ʒ����Ϣ
DECLARE_MSG(SClickNpcGetSpGoodsInfMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_NPCBUYSP)
struct SAClickNpcGetSpGoodsInfMsg : public SClickNpcGetSpGoodsInfMsg
{
	BYTE m_DefaultNum;	//Ĭ��һ�ο��Թ��������
	GoodsData m_GoodsData;	//��Ʒ��Ϣ
};

//���ػ���ű�־(��ҵ�½ʱҪ������ң��ͱ�־����ʱҪͬ���������������)
DECLARE_MSG(SDailyOpenFlagMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_OPENFLAG)
struct SADailyOpenFlagMsg : public SDailyOpenFlagMsg
{
	BYTE m_DailyType;
	BYTE m_Flag;
};


//��������ھ���ɽ��̽��
#define PLOTDIALOGUEMAXSIZE 400

DECLARE_MSG(SJuLongShanRiskMsg, SDailyMsg, SDailyMsg::EPRO_DAILY_JULONGSHANRISK)
struct SQJuLongShanRiskMsg : public SJuLongShanRiskMsg
{
};
struct SAJuLongShanRiskMsg : public SJuLongShanRiskMsg
{
	BYTE m_AwardType;		//�������ͣ�Ϊ1��ʾ�������ߣ�Ϊ2��ʾ�������飬Ϊ3�������ң�Ϊ4��������֮����Ϊ5��ʾ�������κν�����
	char m_PlotDialogueInf[PLOTDIALOGUEMAXSIZE];	//̽�յľ���Ի�
	DWORD m_AwardItemID;	//�����������Ϊ1����Ϊ����ID������Ϊ0
	DWORD m_AwardNum;		//���Ϊ1���ͽ�������Ϊ���ߵ�����������Ϊ����Ľ���ֵ����
};


//�������������
#define TAROTMAXNUM 12

//���������ֵ���󳤶�
#define TAROTMAXNAMELEN 32
//�������������
#define MAXTURNONTAROTNUM 5

//�����ƶԼҽ���
struct TarotAwardData
{
	DWORD m_ItemID;
	BYTE m_ItemNUm;
};

//�����ƻ���������Ϣ
struct TarotBaseData
{
	BYTE m_TarotID;
	char m_TarotName[TAROTMAXNAMELEN];
	TarotAwardData m_TarotAwardData;
};

//��ҷ�����һ������������
struct TurnOnTarotData
{
	BYTE m_TarotID;
	BYTE m_TarotPos;
};

//��ʼ���������������

DECLARE_MSG(SInitTarotDataMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_INITTAROTDATA)
struct SAInitTarotDataMsg : public SInitTarotDataMsg
{
	BYTE m_CanPlayTurnOffCard;	//����ܷ񲥷ſ��ƶ�����Ϊ1��ʾ�ᣬΪ0��ʾ����
	TarotBaseData m_TarotBaseData[TAROTMAXNUM];  //�����ƻ�������
	TurnOnTarotData m_TurnOnTarotData[MAXTURNONTAROTNUM];	//��ҷ�������������Ϣ
};

//������󷭿�һ��������
DECLARE_MSG(STurnOnOneTarotCardMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_TURNONONECARD)
struct SQTurnOnOneTarotCardMsg : public STurnOnOneTarotCardMsg
{
	BYTE m_TurnOnTarotPos;	//��ҷ����Ǹ�λ�õ�������
};
struct SATurnOnOneTarotCardMsg : public STurnOnOneTarotCardMsg
{
	BYTE m_InsertPos;	//�·�����������λ��
	TurnOnTarotData m_TurnOnTarotData;	//�·�������������Ϣ
};

//������������ƽ���
DECLARE_MSG(SGetTarotAwardMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_GETAWARD)
struct SQGetTarotAwardMsg : public SGetTarotAwardMsg
{
};
struct SAGetTarotAwardMsg : public SGetTarotAwardMsg
{
	BYTE m_Ret;
};

//�����������������
DECLARE_MSG(SResetPlayerTarotMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_RESETTAROTDATA)
struct SAResetPlayerTarotMsg : public SResetPlayerTarotMsg
{
};


//���������Ʋ��Ŷ�����־
DECLARE_MSG(SUptTarotPlayAnimateFlagMsg, SDailyMsg, SDailyMsg::EPRO_TAROT_UPTPLAYANIMATEFLAG)
struct SQUptTarotPlayAnimateFlagMsg : public SUptTarotPlayAnimateFlagMsg
{
};


//��������ʱ�������
//���ÿ���������Ϣ
struct CurDayOnlineData
{
	BYTE m_CurDayOnlineLevelID;		//��ҵ������߼���ID
	BYTE m_AwardFlag;	//����״̬��0��ʶ������ȡ��1��ʶ������ȡ��2��ʶ�Ѿ���ȡ��
	DWORD m_NeedTime;	//��Ҫ����ʱ��ʱ��
	DWORD m_AwardItemID;	//�����ĵ���ID
	DWORD m_AwardItemNum;	//����������Ϊ���ߣ������顢����������֮��������֮��ĵ��ߣ�Ϊ���ߵ�����������Ϊʵ�ʵĽ���ֵ��

};

//��ʼ�����ÿ������ʱ��������Ϣ
DECLARE_MSG(SCurDayOnlineInitMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_INIT)
struct SACurDayOnlineInitMsg : public SCurDayOnlineInitMsg
{
	BYTE m_CurDayOnlineAwardNum;
	CurDayOnlineData* m_lpCurDayOnlineData;
};

//�������ÿ������ʱ�����ҵ�������ʱ��
DECLARE_MSG(SCurDayOnlineLongMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_TIMELONG)
struct SQCurDayOnlineLongMsg : public SCurDayOnlineLongMsg
{
};
struct SACurDayOnlineLongMsg : public SCurDayOnlineLongMsg
{
	DWORD m_CurDayOnlineTime;	//��ҵ���������ʱ��
};

//��ȡ��Ӧʱ��εĽ���
DECLARE_MSG(SCurDayOnlineGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_GETAWARD)
struct SQCurDayOnlineGetAwardMsg : public SCurDayOnlineGetAwardMsg
{
	BYTE m_CurDayOnlineLevelID;		//��ҵ������߼���ID��Ϊ0��ʾ��ȡ��ǰ���п�����ȡ�Ľ���
};
struct SACurDayOnlineGetAwardMsg : public SCurDayOnlineGetAwardMsg
{
	BYTE m_Ret;
};

//֪ͨ��һ����
DECLARE_MSG(SCurDayOnlineResetMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_RESET)
struct SACurDayOnlineResetMsg : public SCurDayOnlineResetMsg
{
};

//��Ҵﵽһ���µĿ�����ȡ�Ľ���
DECLARE_MSG(SCurDayOnlineAddAwardMsg, SDailyMsg, SDailyMsg::EPRO_CURDAYONLINE_ADDONEAWARD)
struct SACurDayOnlineAddAwardMsg : public SCurDayOnlineAddAwardMsg
{
	BYTE m_OnlineLevelID;		//��ҵ������߼���ID
};

//�ۼƵ�½�
#define ADDUPLOGINMAXAWARDNUM 5
//�ۼƵ�½��������
struct AddUpLoginBaseData
{
	BYTE m_AddUpLoginLevelID;	//����ID
	BYTE m_AwardFlag;	//������ʶ
	BYTE m_AwardItemNum[ADDUPLOGINMAXAWARDNUM];	//�����ĵ�������
	WORD m_NeedDay;	//�ﵽ������
	DWORD m_AwardItem[ADDUPLOGINMAXAWARDNUM];	//�����ĵ���ID
};

//��ʼ���ۼ����ߵ�½����
DECLARE_MSG(SAddUpLoginInitMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_INIT)
struct SAAddUpLoginInitMsg : public SAddUpLoginInitMsg
{
	BYTE m_AddUpLevelNum;
	AddUpLoginBaseData* m_lpAddUpLoginBaseData;
};

//���ҵ����ۼƵ�½����
DECLARE_MSG(SAddUpLoginDayMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_ADDUPDAY)
struct SQAddUpLoginDayMsg : public SAddUpLoginDayMsg
{
};
struct SAAddUpLoginDayMsg : public SAddUpLoginDayMsg
{
	WORD m_AddUpLoginDay;
};

//��Ҵﵽ������ȡ�µĽ���
DECLARE_MSG(SAddUpLoginAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_ADDNEWAWARD)
struct SAAddUpLoginAddNewAwardMsg : public SAddUpLoginAddNewAwardMsg
{
	BYTE m_AddUpLoginLevelID;
};

//�����ȡ�ۼƵ�½��Ӧ����ID�Ľ���
DECLARE_MSG(SAddUpLoginGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_ADDUPLOGIN_GETAWARD)
struct SQAddUpLoginGetAwardMsg : public SAddUpLoginGetAwardMsg
{
	BYTE m_AddUpLoginLevelID;
};
struct SAAddUpLoginGetAwardMsg : public SAddUpLoginGetAwardMsg
{
	BYTE m_Ret;
};

//��ֵ�����
#define RFBMAXAWARDITEMNUM 6
struct RFBAwardInf
{
	BYTE m_AwardID;	//����ID
	BYTE m_IsCanGetAwardFlag;	//�Ƿ������ȡ������ʶ
	BYTE m_AwardItemNum[RFBMAXAWARDITEMNUM];	//�������߶�Ӧ������
	DWORD m_NeedRechargeGlodNum;	//��Ҫ��ֵ�������
	DWORD m_AwardItem[RFBMAXAWARDITEMNUM];		//��������
};

//��ʼ����ֵ��������
DECLARE_MSG(SRFBInitMsg, SDailyMsg, SDailyMsg::EPRO_RFB_INIT)
struct SARFBInitMsg : public SRFBInitMsg
{
	BYTE m_AwardAllNum;
	RFBAwardInf* m_lpRFBAwardInf;
};

//��ȡ��ֵ������Ӧ�Ľ���
DECLARE_MSG(SRFBGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_RFB_GETAWARD)
struct SQRFBGetAwardMsg : public SRFBGetAwardMsg
{
	BYTE m_AwardID;
};
struct SARFBGetAwardMsg : public SRFBGetAwardMsg
{
	BYTE m_Ret;
};
//֪ͨ������µĽ���������ȡ
DECLARE_MSG(SRFBAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_RFB_ADDNEWAWARD)
struct SARFBAddNewAwardMsg : public SRFBAddNewAwardMsg
{
	BYTE m_AwardID;
};

//�ȼ������
#define LEVELRACEMAXAWARDNUM 4
//��ҵȼ�������������
struct PlayerLevelRaceBaseData
{
	BYTE m_DataID;	//����ID
	BYTE m_NeedLevel;  //��Ҫ�ﵽ�ĵȼ�
	BYTE m_IsCanGetAwardFlag;	//�Ƿ������ȡ������ʶ��0Ϊ��������ȡ��1Ϊ������ȡ��2Ϊ�Ѿ���ȡ��
	WORD m_MaxPeopleNum;	//�������
	WORD m_ResiduePeopleNum;	//ʣ������
	WORD m_AwardItemNum[LEVELRACEMAXAWARDNUM];	//��Ӧ�������ߵ�����
	DWORD m_AwardItem[LEVELRACEMAXAWARDNUM];	//�����ĵ�����
};

//��ʼ����ҵȼ���������
DECLARE_MSG(SLRInitMsg, SDailyMsg, SDailyMsg::EPRO_LR_INIT)
struct SALRInitMsg : public SLRInitMsg
{
	BYTE m_LevelRaceNum;
	PlayerLevelRaceBaseData* m_lpPlayerLevelRaceBaseData;
};

//��ȡ�ȼ���������
DECLARE_MSG(SLRGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_LR_GETAWARD)
struct SQLRGetAwardMsg : public SLRGetAwardMsg
{
	BYTE m_DataID;
};
struct SALRGetAwardMsg : public SLRGetAwardMsg
{
	BYTE m_Ret;
};

//֪ͨ��ҿ�����ȡ�µĽ���
DECLARE_MSG(SLRNotifyAddNewAwardMsg, SDailyMsg, SDailyMsg::EPRO_LR_NOTIFYCANGETNEWAWARD)
struct SALRNotifyAddNewAwardMsg : public SLRNotifyAddNewAwardMsg
{
	BYTE m_DataID;
};

//֪ͨ�����������ʣ������ı�
DECLARE_MSG(SLRBroadCastResidueTimesMsg, SDailyMsg, SDailyMsg::EPRO_LR_NOTIFYUPTRESIDUETIMES)
struct SALRBroadCastResidueTimesMsg : public SLRBroadCastResidueTimesMsg
{
	BYTE m_DataID;
	WORD m_LRResidueTimes;
};


//����Ħ���ֻ
#define FWMAXNUM 12 //����Ħ�����������
#define FWMAXRECORDPLAYERNUM 5 //����Ħ��������¼��ҵ�����
//����Ħ���ֻ���������Ϣ
struct FWBaseInf
{
	BYTE m_SerialID;	//����Ħ���ֵ����ID
	BYTE m_AwardItmeNum;	//�����ĵ�������
	DWORD m_AwardItemID;	//�����ĵ���
};

//����Ħ���ּ�¼���������Ϣ
struct FWRecordPlayerInf
{
	char m_PlayerName[CONST_USERNAME];	//��¼��ҵ�����
	DWORD m_AwardItemID;	//��¼��һ�õĽ���
};

//��ȡ����Ħ���ֵ�½��ʼ��������
DECLARE_MSG(SFWLoginInitMsg, SDailyMsg, SDailyMsg::EPRO_FW_LOGININIT)
struct SAFWLoginInitMsg : public SFWLoginInitMsg
{
	FWBaseInf m_FWBaseInf[FWMAXNUM];
	FWRecordPlayerInf m_FWRecordPlayerInf[FWMAXRECORDPLAYERNUM];
};

//��ȡ������Ħ���ֵĿ���ʣ��ʱ������
DECLARE_MSG(SFWOpenResidueTimeMsg, SDailyMsg, SDailyMsg::EPRO_FW_GETOPENRESIDUETIME)
struct SQFWOpenResidueTimeMsg : public SFWOpenResidueTimeMsg
{
};
struct SAFWOpenResidueTimeMsg : public SFWOpenResidueTimeMsg
{
	WORD m_CanUseTimes;	//����ʹ�õĴ���
	DWORD m_OpenResidueTime;
};

//��ʼ����Ħ������Ϸ
DECLARE_MSG(SFWStartGameMsg, SDailyMsg, SDailyMsg::EPRO_FW_STARTGAME)
struct SQFWStartGameMsg : public SFWStartGameMsg
{
};
struct SAFWStartGameMsg : public SFWStartGameMsg
{
	BYTE m_SerialID;	//����Ħ���ֵ����ID
};

//�㲥����Ħ������Ҽ�¼��Ϣ�������
DECLARE_MSG(SFWRecordUptMsg, SDailyMsg, SDailyMsg::EPRO_FW_BROADCASTRECORDUPT)
struct SAFWRecordUptMsg : public SFWRecordUptMsg
{
	FWRecordPlayerInf m_NewFWRecordPlayerInf[FWMAXRECORDPLAYERNUM]; //�µ���Ҽ�¼��Ϣ
};

//��ȡ����
DECLARE_MSG(SFWGetAwardMsg, SDailyMsg, SDailyMsg::EPRO_FW_GETAWARD)
struct SQFWGetAwardMsg : public SFWGetAwardMsg
{
	BYTE m_SerialID;	//����Ħ���ֵ����ID
};
struct SAFWGetAwardMsg : public SFWGetAwardMsg
{
	BYTE m_Ret;	
};

//ͬ����ҳ�ֵ�������
DECLARE_MSG(SSynRechargeTatolGlodMsg, SDailyMsg, SDailyMsg::EPRO_RECHARGE_SYNTATOLGLOD)
struct SASynRechargeTatolGlodMsg : public SSynRechargeTatolGlodMsg
{
	DWORD m_TatolRechargeNum;	//�ܵĳ�ֵ����
};