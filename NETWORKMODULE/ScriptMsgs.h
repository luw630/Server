#pragma once

#include "NetModule.h"
#include "playertypedef.h"

#define STRING_LENTH 366
#define RANK_LIST_MAX 50
#define MAX_PRAYER_SHOPITEM 8
#define MAX_GIFTCODE_LENTH 16
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �ű������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SScriptBaseMsg, SMessage, SMessage::EPRO_SCRIPT_MESSAGE)
//{{AFX
EPRO_CLICK_OBJECT,		    // ��������ϵĶ���
EPRO_CLICK_MENU,            // ѡ��ѡ��
EPRO_FORGE_GOODS,           // ������Ʒ
EPRO_BRON_MSG,              // ��ʦ
EPRO_BACK_MSG,          
EPRO_SHOW_TASK,             // ������ʾ
EPRO_SEND_SCORE,            // ���а�
EPRO_SEND_KILLINFO,         // ׷ɱ��
EPRO_SEND_ITEMINFO,         // ��Ʒ
EPRO_HELP_PINFO,            // ����������Ϣ
EPRO_ROLETASK_INFO,         // �������
EPRO_MULTIME_INFO,			// �౶����ʱ��
EPRO_SCORE_LIST,			// ���£��������а�
EPRO_SCORE_LIST_EX,			// �����⣩�������а�
EPRO_CUSTOM_WND,			// ������涨����Ϣ��
EPRO_SCRIPT_TRIGGER,		// �ű��߼���������
EPRO_LUACUSTOM_MSG,         // �ű�������Ϣ
EPRO_LUATIPS_MSG,			// �ű���ʾ��Ϣ
EPRO_DOCTORTIME_INFO,		// ҽ��ʣ��ʱ��
EPRO_OTHEREQUIPMENTNAME,    // ͨ�����ֲ鿴װ��
EPRO_SYNCTASKDATA,			// ͬ���������ݵ��ͻ���
EPRO_SYNNPCTASKDATA,		// ͬ��NPC��������Ϣ���ͻ��ˣ�������ʾNPCͷ������ʾ��ǣ�
EPRO_NOTIFY_STATUS,			// ��ʾ�û���Ϣ
EPRO_PLAY_CG,				// ����CG����Ϣ
EPRO_CLICK_MENUNAME,        // �õ�NPC������Ʒ��ҳ����
EPRO_RANK_LIST,				// XYD3���а�
EPRO_RANK_GETOTHEREQUIPMENT,// XYD3���а�鿴�������װ��
EPRO_CLIENT_REQUEST_DB,		// �ͻ��������������DB��ȡ����
EPRO_QUEST_TREASURE,//�����ȡ�۱���
EPRO_INIT_PRAYER,//����ʼ����Ϣ
EPRO_START_PRAYER,//����Ϣ
EPRO_SHOP_REFRESH,//refreshshop �̵�ˢ��
EPRO_SHOP_BUSINESS,//refreshshop �̵꽻��
EPRO_GIFTCODE_ACTIVITY,//���������
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//EPRO_SEND_CLIENT        // Ҫ��ͻ���ִ�е���Ϣ


DECLARE_MSG(SPlayCG, SScriptBaseMsg, SScriptBaseMsg::EPRO_PLAY_CG)
struct SAPlayCG : public SPlayCG
{
	WORD wCgID;		 // CG��ID
};

struct SQPlayCG : public SPlayCG
{
	WORD wCgID;		 // �Ѿ�������ɵ�CG ID
};



DECLARE_MSG(SNotifyStatus, SScriptBaseMsg, SScriptBaseMsg::EPRO_NOTIFY_STATUS)
struct SANotifyStatus : public SNotifyStatus
{
	enum
	{
		// �������
		NS_EXG_SELFINEXGING = 1,		// �Լ����ڽ����У��޷�����˽���
		NS_EXT_DESTEXGING,				// �Է����ڽ����У��޷����㽻��
		NS_EXG_CANTEXGSELF,				// ���ܺ��Լ�����
		NS_EXT_CANTFIND_SRC,			// ����Դ������
		NS_EXG_CANTFIND_DEST,			// ����Ŀ�겻����
		NS_EXT_WAITDEST_RESPOND,		// ���������ѷ������ȴ���Ӧ
		NS_EXT_REQUEST_REJECT,			// ���ױ��ܾ�
		NS_EXT_ALREADY_IN_LOCK,			// �Ѵ�������״̬��
		NS_EXT_ALREADY_IN_COMMIT,		// �Ѵ��ڽ���״̬��
		NS_EXT_MONEY_INVALID,			// ����Ľ�Ǯ��Ŀ���Ϸ�
		NS_EXT_MONEY_LACK,				// ��Ǯ���벻��
		NS_EXT_ITEM_CANT_FIND,			// ������ĵ��߲�����
		NS_EXT_ITEM_LOCKORBIND,			// ������ĵ��߱�����/��
		NS_EXT_ITEM_CANTEXG,			// ������ĵ����޷�������		
		NS_EXT_EXTBOX_FULL,				// ����������
		NS_EXT_NOTHING,					// û�н����κζ���
		NS_EXT_DEST_PACKAGE_LACK,		// �Է���������
		NS_EXT_SELF_PACKAGE_LACK,		// �Լ���������
		NS_EXT_ITEM_BIND,				// �����޷�������

		// ��ֵ������
		NS_SPLITITEM_BIND = 201,		// ���߱��󶨣��޷����
		NS_SPLITITEM_LOCKED,			// ���߱��������޷����
		NS_SPLITITEM_TASKNOTALLOWED,	// ��������޷������
		NS_SPLITITEM_PACKAGEFULL,		// �����������޷����
		NS_SPLITITEM_COOLING,			// ���ڲ����ȴ

		// �ֿ����
		NS_WH_ALREADY_OPEN = 401,		// ��ǰ�Ѿ�����һ���ֿ�
		NS_WH_MONEY_FULL,				// �ֿ��Ǯ����
		NS_WH_ITEMCANT_STORAGABLE,		// ���߲���������ֿ�
		NS_WH_ITEM_LOCKED,				// ���߱��������޷�����ֿ�
		NS_WH_PLAYER_MONEY_FULL,		// ������ϵĽ�Ǯ����

		// ����������
		NS_ZL_COOLING,					// ����������ȴ
	};

	WORD wIndex;					// Ҫ��ʾ��Ϣ������
};

// added by xhy
DECLARE_MSG(SSyncTaskData, SScriptBaseMsg, SScriptBaseMsg::EPRO_SYNCTASKDATA)
struct SQASyncTaskData : public SSyncTaskData 
{
    char streamData[8*1024];
};

DECLARE_MSG(SSyncNpcTaskData, SScriptBaseMsg, SScriptBaseMsg::EPRO_SYNNPCTASKDATA)
struct SASyncNpcTaskData : public SSyncNpcTaskData
{
	WORD	NpcCount;			// Npc����
	DWORD	TaskData[128];		// NpcID + TaskCount + TaskStatus
};

//=============================================================================================
// �ű��Զ�����Ϣ
// ע�⣬���ڸ���Ϣ���ܻ�ֱ���ɿͻ��˷������Է������������޷�ֱ���ж�����Ч�Եģ���Ҫ�ڽű���������ȷ�Ĵ�����У�����
DECLARE_MSG(SLuaCustomMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_LUACUSTOM_MSG)
struct SQALuaCustomMsg : public SLuaCustomMsg 
{
    BYTE flags;
    char streamData[2048];
};
DECLARE_MSG(SLuaTipsMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_LUATIPS_MSG)
struct SQALuaTipsMsg : public SLuaTipsMsg 
{
    BYTE flags;	//��������
	char streamData[2048];	//��Ϣ
};

// �ͻ��˵����NPC������������������Ӧ
DECLARE_MSG(SClickObjectMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_OBJECT)
struct SQClickObjectMsg : public SClickObjectMsg
{
	DWORD	dwGlobalID;			// ����ߵ�ID
	DWORD	dwDestGlobalID;		// ������ߵ�ID
	BYTE	dymmy[12];			// ����Ĳ���
};

struct SAClickNpcMsg : public SClickObjectMsg
{
	DWORD		verifier;			// ��֤��Ϣ
	DWORD		wScriptID;			// �ű�ִ�д���
	WORD		wTaskNum;			// ������Ϣ����
	WORD		wMenuIndex[20];		// Ҫ��ʾ�Ĳ˵�����
	WORD		wTaskData[64];		// �������Ϣ�����跢�ͣ�
};

struct SAClickObjectMsg : public SClickObjectMsg
{
	DWORD	verifier;			// ��֤��Ϣ
	DWORD	wScriptID;			// �ű�ִ�д���
	WORD	wMenuCount;			// �˵�����
	WORD	wTaskGroupCount;	// ����Ⱥ����
	DWORD	wParam[5];			// ���������ظ��ͻ��˵Ĳ���
};

// �ͻ��������Ի���ѡ�������������˴���
DECLARE_MSG(SChoseMenuMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_MENU)
struct SQChoseMenuMsg : public SChoseMenuMsg
{
    DWORD	verifier;			//	����Ч����
	DWORD	byChoseID;			//  ��ҵ�ѡ������
};

struct SAChoseMenuMsg : public SChoseMenuMsg
{
    enum 
	{
        CM_BUYPANEL = 1,            // �����������
        CM_FORGEPANEL,              // ����������� 
        CM_FUSEPANEL,               // �����ں����
        CM_STORAGEPANEL,            // �����ֿ����
        CM_FAMEPANEL,               // �������������

        CM_SHOWPICTURE,             // ����ͼƬ
        CM_KILLINFOPANEL,           // ����׷ɱ�����

		CM_COLOURPANEL,				// ����Ⱦ�����
		CM_INPUTPANEL,				// �����������
        CM_CHECKITEMPANEL,			// �����������
		CM_BACKITEMPANEL,			// �����������
		CM_INPUTMCPANEL,			// ������������һ��㿨���
		CM_MCINFOPANEL,				// ��������һ��㿨��Ϣ���
		CM_OWNMCINFOPANEL,			// �����Լ�������һ��㿨��Ϣ���

		CM_UPDATEITEM,				// �����������
		CM_XIDIAN,					// ����ϴ�����
		CM_DAKONG,					// ������׽���
		CM_XIANGQIAN,				// ������Ƕ���
		CM_ZHAICHU,					// ����ժ�����
    };

	WORD	m_Index;				// ѡ���Ҫ��ͻ������Ĵ���
	BYTE	m_count;				// ��Ʒ����			
	DWORD	m_Goods[128];			// NPC������Ʒ��64������ƷID+������
};
//---------------------------------------------------------------------------------------------


enum SItemType { ST_CHECKONE, ST_BACKITEMS };
//=============================================================================================
// �ͻ�������������Ʒ��Ϣ  -- �ں�Ҳ����һ����Ϣ
DECLARE_MSG(SForgeGoodsMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_FORGE_GOODS)
struct SQForgeGoodsMsg : public SForgeGoodsMsg
{
    enum {
        SP_FORGE = 1,
        SP_FUSE,
        SP_FAME,
        SP_MONEY,
        SP_SITEMS
    };

	BYTE byWhat;      // ���ƻ����ں�? 1:����  2:�ں�  3: ����   4:Ǯ����� 5: �ű�����
    DWORD wSuccess;
};

struct SAForgeGoodsMsg : public SForgeGoodsMsg
{
	BYTE  byWhat;
    WORD  wSuccess;        // �ɹ���
   	WORD  wEquipment;      // װ���з�
	WORD  wDrinkNum;       // �Ƶ�����
	DWORD wMoney;           
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// һЩ�ű�ִ�е���Ϣ����
DECLARE_MSG(SBackMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_BACK_MSG)
struct SABackMsg : public SBackMsg
{
	enum EBType
	{
        B_FAIL = 0,
		B_FORGE = 1,             
		B_NOFORGE,
		B_FUSE,
		B_NOFUSE,
		B_NOBIJOU,
		B_FULLBAG,
        B_FARFIGHT,
		B_NOLEARNGEST,    
        B_POSABLE,              // ��ռ��λ�ò��ܹ���
        B_SKILLABLE,            // û��ѧ�ᵱǰ��ʽ
        B_MONSTERABLE,          // ���ܶԹ��︨������
        B_NOTTARGET,            // û��Ŀ���Ŀ���Ѿ�����
        B_ATTACKTEAM,           // ���ܹ�������
        B_LEVELABLE,            // �ȼ���������PK
        B_LIMITATTACK,          // �㱻�����ˣ�
        B_LIMITHP,              // Ѫ������
        B_LIMITMP,              // ���ﲻ��
        B_LIMITSP,              // ��������
		B_HADLEARNED,           // �Ѿ�ѧϰ�������书
        B_LIMITUSE,             // ��ǰ����ʹ�õ��书
        B_BEVITALS,             // ����Ѩ
        B_BEDIZZY,              // ��ѣ��

        B_NOTSCHOOL,            // ���ɲ�ͬ
        B_LV_NOTENOUGH,         // �ȼ�����
        B_EN_NOTENOUGH,         // ��������
        B_IN_NOTENOUGH,         // ���Բ���
        B_ST_NOTENOUGH,         // ���ǲ���
        B_AG_NOTENOUGH,         // ������

        B_LIMIT_TALK,           // ��ֹˢ��

        SYS_LEVEL_NOT_ENOUGH,   // ���볡���ĵȼ�����
        SYS_CAN_NOT_DO_PK,      // �������Ʋ��ܽ���PK

        EXM_FAIL,                       // �ڽ����г���ʧ��
        EXM_CANT_FIND_OTHERPLAYER,      // �Ҳ������׶���
        EXM_OTHERPLAYER_BUSY,           // �Է�æ
        EXM_WAIT_OTHERPLAYER_RESPOND,   // ��ȴ��Է�����Ӧ
        EXM_MONEY_NOT_ENOUGH,           // Ǯ���㹻
        EXM_CANT_FIND_ITEM,             // �޷��ҵ���Ӧ�ĵ���
        EXM_CANT_PUT_ITEM,              // �޷����¸���Ʒ
        EXM_REJECT,                     // ���ױ��ܾ�
        EXM_CANT_ADDSELF_ITEM,          // �޷�Ϊ�Լ���ӵ���
        EXM_CANT_ADDDEST_ITEM,          // �޷�Ϊ�Է���ӵ���
        EXM_CANT_DELSELF_ITEM,          // �޷�ɾ���Լ��ĵ���
        EXM_CANT_DELDEST_ITEM,          // �޷�ɾ���Է��ĵ���
		EXM_ITEMEX_DISABLED,			// �����޷�������

        ERR_GMLIMIT,                    // û��GMȨ�ޣ��򲻹�

        B_ITEM_PROTECTED,               // ��Ʒ����ʱ��
        B_TALKMASK,                     // ������
        // ...
        B_NOTFORGELEVEL,                // ���Ʊ�����3������
        B_NOTFORGEITEM,                 // ����������Ʒ

        B_ATTERR_NOTTELERGY,            // ����ʹ�õ�ǰ�ķ�
        B_FORCE_LOGOUT,                 // �㱻����Աǿ�ƶϿ����ӣ�
        B_ACCOUNT_KICK,                 // ����ʹ������˺������ˣ�

        // �ķ����
        B_FULLTELERGY,                  // ���Ѿ�װ���������ķ�

        // �������
        B_BECUREING,                    // �����ڽ�������
        B_CUREING,                      // ������Ϊ��������
        B_CUREROVER,                    // Ϊ���β������Ѿ���ʧ��������ֹ
        B_CURERGOOUT,                   // ���β��ڼ䣬�κ��˶����ܶ����㿴�������ֹ�ˣ�
        B_CURE_NOZAZEN,                 // Ҫ�������˶�������������
        B_CURE_FARAWAY,                 // ����̫Զ����������
        B_CURE_END,                     // �������
        B_CURER_END,                    // �������������

        // Ѩ�����
        B_VENA_PASSED,                  // "������Ѩ���Ѿ���ͨ�˵�"
        B_VENA_SETPPASS,                // "���������ϵ�Ѩ������Ҫһ��һ������"
        B_VENA_PASSNOT6,                // "�����ͨǰ����������������ɣ�"
        B_VENA_COUNTNOT,                // "��ĳ�Ѩ��������"
        B_VENA_ITEMNOT,                 // "��û��Ѩ����Ӧ��ָ����"
        B_VENA_LOWTELERGY,              // ����ķ��������ڴ�ͨ���Ѩ��
        B_VENA_PASSVENASO,              // �����ܵ���ѨӰ��
		B_VENA_NOTITEM,					// û�ж�Ӧͭ��

        B_LONEMANSELF,                  // ������״̬(myself)
        B_LONEMANOTHER,                  // ������״̬(other)

        B_MUTATE_CANOTRUN,              // ����״̬�Ͳ����ܺ�����
        B_MUTATE_CANOTZAZEN,            // ��Ҫ����״̬�ͱ������
        B_MUTATE_CANOTONZAZEN,          // ��վ�����ű���
        B_MUTATE_CANOTSKILL,            // �㻹û��ѧ�����֮��ʹ���书

        //B_UPASSWORD_ERROR,              // �����������

        B_TESTMSG,                      // ���ز�����Ϣ

        B_ASK_DOCTORBUFFADD,            // ѯ���Ƿ�ͬ�⸽��ҽ��BUFF
        B_AGREE_ADDDOCTORBUFF,          // ͬ�⸽��ҽ��BUFF
        B_BECURED,                      // �Է����ڱ�����
        B_CUREOTHER,                    // ������������
        B_REDUCEWERA,                   // ����֪ͨ�ͻ��˼����;�ֵ
        B_CANCELLOCK,                   // ֪ͨ�ͻ���ȡ���������
        B_STOPLOCK,                     // �����Ϣ��ʾ����������ͬ��״̬�ˣ��ͻ��˿��Ա��

		B_NOT_ENOUGH_SPACE,				// �����ռ䲻��
        B_SUCCESS = 255,
	};

	BYTE byType;
};

struct SAAddDoctorBUFFMsg : public SABackMsg
{
    DWORD dwGID;
};

struct SAReduceEquipWearMsg : public SABackMsg
{
    enum 
    {
        ATTACK_EUIP,    // ����װ���� ��������Ʒ ��
        DEF_EQUIP       // ����
    };

    BYTE byEquipType;
};
//=============================================================================================
// ��ʦ������ out school & out teacher -_______-bb
DECLARE_MSG(SOutSchoolMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_BRON_MSG)
struct SAOutSchoolMsg : public SOutSchoolMsg
{
	DWORD  dwGlobalID;
	BYTE   byBRON;
	BYTE   bySchool;
	BYTE   bySex;
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SShowTaskInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOW_TASK)
struct SAShowTaskInfoMsg : public SShowTaskInfoMsg
{
    enum 
    {
        EO_ADD_TASK,  // ��ǰ�ɵ�����ϵͳ
		//EO_ADD_TASK2,  // �������⴦�������
        EO_DEL_TASK,
        EO_CLEAN_TASK
    };

    BYTE byOperate;
	BYTE byTaskType;
    WORD wTaskID;
    DWORD wTaskState;
};
/*
struct SAShowTaskInfoMsgEx : public SShowTaskInfoMsg
{
	WORD wTaskID;               // �����   ����ſɸ�NPC�������Ľű���һ��  Ҳ����ר��ָ��������� 
	DWORD wTaskState;           // ������\״ֵ̬
	BYTE byOperate;
	BYTE byTaskType;            // ��������
};
*/
// --------------------------------------------------------------------------------------------
// ���а�
DECLARE_MSG(SSendScoreInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_SCORE)
struct SASendScoreInfoMsg : public SSendScoreInfoMsg
{
    SScoreTable::ScoreType type;
    SScoreTable::SScoreRecode Score[MAX_SCORE_NUMBER];
};

// --------------------------------------------------------------------------------------------
// EPRO_SEND_KILLINFO
DECLARE_MSG(SSendKillInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_KILLINFO)
struct SQSendKillInfoMsg : public SSendKillInfoMsg
{
    DWORD dwMoney;
	char szName[CONST_USERNAME];
};

struct SASendKillInfoMsg : public SSendKillInfoMsg
{
//    KILLINFO stKillInfo[MAX_KILLINFO];
};

//--------------
DECLARE_MSG(SSendSItemInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_ITEMINFO)
struct SQSendSItemInfoMsg : public SSendSItemInfoMsg
{
//     enum STYPE {
//         ST_CHECKITEM,
//         ST_BACKITEMS
//     };
// 
//     BYTE type;
//     SITEMINFO info;
};

struct SQSendSItemDelMsg : public SQSendSItemInfoMsg
{   
};

///------------------------------
DECLARE_MSG(SHelpPInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_HELP_PINFO)
struct SQHelpPInfoMsg : public SHelpPInfoMsg
{
    enum PTYPE {
        PT_MASTER,
        PT_PRENTIS 
    };

    WORD wPPType;
	char szName[CONST_USERNAME];
    DWORD dwStaticID;
};

// ���������
DECLARE_MSG_MAP(SRoleTaskBaseMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_ROLETASK_INFO)
EPRO_SENDROLETASKINFO,				// �������
EPRO_ADDROLETASK,					// ��������
EPRO_HANDINTASK,					// ������
EPRO_FINISHTALK,					// ��ɶԻ�
EPRO_FINISHGIVE,					// ��ɸ���
EPRO_FINISHGET, 					// ��ɻ�ȡ
EPRO_UPDATETASKFLAG,				// �������״̬
EPRO_SENDROLETASKELEMENT,			// ����������
EPRO_SET_SPEF,						// ��������
EPRO_CLEAR_ALL,						// �������
EPRO_GIVEUPTASK,					// ��������
EPRO_FINISHTADDTASK,				//���������ĳ������
END_MSG_MAP()

DECLARE_MSG(SSendRoleTaskInfoMsg, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SENDROLETASKINFO)
struct SASendRoleTaskInfoMsg : public SSendRoleTaskInfoMsg
{
	WORD	wTaskNum;			// ������Ϣ����
	WORD	wTaskData[64];		// �������Ϣ�����跢�ͣ�
};

DECLARE_MSG(SAddRoleTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_ADDROLETASK)
struct SQAddRoleTask : public SAddRoleTask
{
	DWORD	dwGlobalID;				// ���ID
	WORD	wTaskID;				// ����ID
};

// �����һ������
struct SAAddRoleTask : public SAddRoleTask
{
	DWORD		dwGlobalID;			// ���ID
	WORD		wTaskID;			// ����ID�����ӵ�����ID��
	WORD		wFlagCount;			// �����Ŀ
	QWORD		m_CreateTime;		// ����Ĵ���ʱ��
	STaskFlag	w_flags[TASK_MAX];			// ������꣨���跢�ͣ�
};

DECLARE_MSG(SHandInRoleTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_HANDINTASK)
struct SQHandInRoleTask : public SHandInRoleTask
{
	DWORD	dwGlobalID;				// ���ID
	WORD	wTaskID;				// ����ID
	WORD	wAwardCount;			// ������Ʒ�ı�ţ����û�п�ѡ��������Ϊ0
};

// ���������
struct SAHandInRoleTask : public SHandInRoleTask
{
	enum
	{
		HTR_SUCCESS = 1,
		HTR_FULLBAG,				// �������ˣ��޷����ɽ���
	};

	DWORD	dwGlobalID;				// ���ID
	WORD	wTaskID;				// ����ID
	WORD	wResult;
};

// ��������
DECLARE_MSG(SGiveUpTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_GIVEUPTASK)
struct SQGiveUpTask : public SGiveUpTask
{
	DWORD dwGlobalID;
	WORD  wTaskID;
};

// �����������
struct SAGiveUpTask : public SGiveUpTask
{
	enum
	{
		QUT_SUCCESS = 1,
		QUT_DONT_EXIST,
		QUT_ALREADY_HANDIN,
	};

	DWORD	dwGlobalID;				// ���ID
	WORD	wTaskID;				// ����ID
	WORD	wResult;
};

DECLARE_MSG(SFinishTalk, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHTALK)
struct SQFinishTalk : public SFinishTalk
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wTalkDes;
};

DECLARE_MSG(SFinishGive, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHGIVE)
struct SQFinishGive : public SFinishGive
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wGiveDes;
};

DECLARE_MSG(SFinishGet, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHGET)
struct SQFinishGet : public SFinishGet
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wGetDes;
};

// �����������Ϣ
DECLARE_MSG(SUpdateTaskFlag, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_UPDATETASKFLAG)
struct SAUpdateTaskFlag : public SUpdateTaskFlag
{
	STaskFlag flag;
};

DECLARE_MSG(SSendRoleTaskElementMsg, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SENDROLETASKELEMENT)
struct SASendRoleTaskElementMsg : public SSendRoleTaskElementMsg
{
	DWORD		m_Index;			// ������������16λ�������񣬵�16λ�����������
	BYTE		m_Type;				// ��������
	BYTE		m_Complete;			// �Ƿ����
	DWORD		m_Value;			// ������ǰ״̬��һ��ֵ����ͬ�������в�ͬ�ĺ���
};

DECLARE_MSG(SSetSpefTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SET_SPEF)
struct SQSetSpefTask : public SSetSpefTask
{
	DWORD	dwPlrID;				// ���ID
	WORD	wTaskID;				// ����ID
};

DECLARE_MSG(SClearAllTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_CLEAR_ALL)
struct SAClearAllTask : public SClearAllTask
{
	DWORD	dwPlrID;				// ���ID
};

//�������ĳ��������
DECLARE_MSG(SFinishAddTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHTADDTASK)
struct SQFinishAddTask : public SFinishAddTask
{
	BYTE btype;
	WORD wCount;
	WORD	wTaskID;
	DWORD dIndex;
};

//=============================================================================================
// �౶�����ʱ��
DECLARE_MSG(SMulTimeInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_MULTIME_INFO)
struct SAMulTimeInfoMsg : public SMulTimeInfoMsg
{
	double	dTime;
	WORD	wMulTime;    // ��λ�洢�౶����
};
// --------------------------------------------------------------------------------------------

// ҽ��BUFFʣ��ʱ��
DECLARE_MSG(SDoctorTimeInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_DOCTORTIME_INFO )
struct SADoctorTimeInfoMsg : public SDoctorTimeInfoMsg
{
    QWORD	dLeaveTime; // �����ŵ�����BUFF�ķ��������Ӹ�λ����λ��AGI,POW,DAM,DEF
};
// --------------------------------------------------------------------------------------------
// ############################################################################################
// --------------------------------------------------------------------------------------------
// ���£��������а�
DECLARE_MSG( SUpdateScoreListMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCORE_LIST )
struct SQUpdateScoreListMsg : public SUpdateScoreListMsg
{
    enum CMD_TYPE 
    { 
        QUERY_TITLE, 
        QUERY_SCORE, 
        SCORE_BEGIN, 
        SCORE_PREV, 
        SCORE_NEXT, 
        SCORE_END,
        UPDATE_APPRAISE,
        SCORE_AWARD,

    }   commandType;

	int param;
	int page;
};

struct SAUpdateScoreListMsg : public SUpdateScoreListMsg
{
    // ���ֻ��2�����ݣ���ȡ�����б�������������
    // serialized RPC data
    char streamData[1024];
};

// --------------------------------------------------------------------------------------------
// ############################################################################################
// --------------------------------------------------------------------------------------------
// �����⣩�����������а� 
DECLARE_MSG( SUpdateScoreListExMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCORE_LIST_EX )
struct SQUpdateScoreListExMsg : public SUpdateScoreListExMsg
{
    WORD wScoreType;
};

struct SAUpdateScoreListExMsg : public SUpdateScoreListExMsg
{
    WORD wScoreType;
    char streamData[1024];
};
// --------------------------------------------------------------------------------------------


// ############################################################################################
// ������涨����Ϣ��
// --------------------------------------------------------------------------------------------
DECLARE_MSG( SCustomWndMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CUSTOM_WND )
struct SQCustomWndMsg : public SCustomWndMsg
{
    // ����Ч��ͻ��˷���ֵ�Ĳ���
    // ͬʱ�����verifier����һ�ε���ͬ����������β������ڴ��ڸ��£����������´�������
    DWORD verifier;

	enum OperateType
	{
		CustomWndShow = 0,
		CustomWndClose = 1,
		CustomWndUpdate = 2,
		CustomWndJUSuccess = 3,
		CustomWndJUFailed = 4
	};

	BYTE operate;

    // MAX10K���ȵ���涨����Ϣ��
    char streamData[10240];
};

struct SACustomWndMsg : public SCustomWndMsg
{
    DWORD verifier;
    DWORD endFlag;          // �ͻ��˷�����Ϣʱ��֪ͨ���������β�����������ȫ�رջ���ֻ�ǲ������󣨷���ȫ�رգ�

    // �ͻ��˷�������Ӧ�ò��࣬1K����Ѿ��㹻
    char streamData[1024];
};
// --------------------------------------------------------------------------------------------

DECLARE_MSG( SScriptTriggerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCRIPT_TRIGGER )
struct SQScriptTriggerMsg : public SScriptTriggerMsg
{
    // �ű��������������߼���ֱ�ӽ���ű��ڲ����д���
    char streamData[1024];
};

// --------------------------------------------------------------------------------------------
DECLARE_MSG( SOthereQuipmentNameMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_OTHEREQUIPMENTNAME )
struct SQOthereQuipmentNameMsg : public SOthereQuipmentNameMsg
{
	char szDestName[CONST_USERNAME];
};//EPRO_CLICK_MENUNAME


DECLARE_MSG( SGetMenuName, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_MENUNAME)
struct SAGetMenuName : public SGetMenuName
{
	enum MENUINDEX
	{
		MI_INDEX = 6,
	};

	DWORD		verifier;					// ��֤��Ϣ
	DWORD		wScriptID;					// �ű�ִ�д���
	DWORD		wIndex;						// ����
	DWORD		wMenuIndex[MI_INDEX];		// Ҫ��ʾ�Ĳ˵�����
};

// XYD3�鿴�������װ����Ϣ
DECLARE_MSG(SRankListEquipMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_RANK_GETOTHEREQUIPMENT)
struct SQRankListEquipMsg : public SRankListEquipMsg
{
	char szDestName[CONST_USERNAME];
};

/************************************************************************/
// �����а���Ϣ
// һ����Ϣ�ǿͻ����ڵ㿪[���а����]��ʱ���͵ģ������Ϣ֪ͨ����������DB����
// ��ȡ���а�����
// ��һ����Ϣ��Ҫ�Ǹ��ݲ�ͬ�����������а����ݣ���������ǰ����ȫ�����ͣ������Ƿֿ�
// ����
/************************************************************************/

// (1)�ͻ��˴򿪽��淢�͵���Ϣ��֪ͨ��������Ҫ��DB��ȡ���а�����
DECLARE_MSG(SClientQuestRankFromDB, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLIENT_REQUEST_DB)
struct SQClientQuestRankFromDB : public SClientQuestRankFromDB
{
};

struct SAClientQuestRankFromDB : public SClientQuestRankFromDB
{
	enum IF_UPDATE
	{
		IU_UPDATE,
		IU_UNUPDATE,
	};

	BYTE		type;									// �Ƿ���£����º�ŷ����ݸ��ͻ���
	BYTE		RankNum[RT_MAX];						// ���а�ǰ����
	Rank4Client RankList[RT_MAX][MAX_RANKLIST_NUM];		// ��ǰ�������а�����
};

// (2)�ͻ��������������а���Ϣ�����ʱ���Ǿ��������������Ժ�ġ����跢�ͣ���ȫ������
DECLARE_MSG(SRankListMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_RANK_LIST)
struct SQRankListMsg : public SRankListMsg
{
	BYTE	bType;		// ���а�����
};

struct SARankListMsg : public SRankListMsg
{
	WORD		bMyRank;		// �ҵ�����
	Rank4Client	ranklist[MAX_RANKLIST_NUM];
};

DECLARE_MSG(STreasureMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_QUEST_TREASURE)
struct SQTreasureMsg : public STreasureMsg
{
	BYTE  bTreasureType;		//  0   ����ǰ����    1  ������ǰ�����ľ۱���
};

struct SATreasureMsg : public STreasureMsg
{
	BYTE bTreasureIndex;	//��ǰ��ʾ�ľ۱�������
	BYTE  bResule;	//�Ƿ�ɹ�  1 �ɹ�  2 ����Ԫ������ 3 �Ѿ��ﵽ�������� 4 �۱����Ѿ��ر�
	WORD wTreasureNum;//����ɹ���õ���Ԫ��
};

typedef struct PRAYERPOINT    //���������
{
	char playername[CONST_USERNAME];  //�������
	WORD  wPoint;		//��ҷ���
}PrayerPoint;

typedef struct PRAYERSHOPITEM    //������̳ǵ���
{
	DWORD  ditemIndex;	//����ID
	DWORD  ditemprice;	//�۸�
	BYTE			bnum;	//����
}PrayerShopItem;

//����ʼ����Ϣ
DECLARE_MSG(SInitPrayerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_INIT_PRAYER)
struct SQInitPrayerMsg : public SInitPrayerMsg
{
};
struct SAInitPrayerMsg : public SInitPrayerMsg
{
	PrayerPoint  pPlayerPoint[RANK_LIST_MAX];//�������а�			
	WORD  wCurrentPoint;					//��ҵ�ǰʣ�����
	WORD  wHistoryPoint;					//�����ʷ����
	WORD   wRemainDay;					//�ʣ�࿪��ʱ��
	DWORD  dfreetime;						//��ǰ������ѳ�ȡʱ����
	DWORD  dclosetime;						//��������̳ǵĹر�ʱ��
	BYTE		 bShopOpened;					//�̳��Ƿ���
};

DECLARE_MSG(SPrayerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_START_PRAYER)
struct SQPrayerMsg : public SPrayerMsg
{
	BYTE  bcount;			//������10���� ��10
};
const WORD bufflen = (sizeof(BYTE)  + sizeof(DWORD)* 2 ) * 10;
struct SAPrayerMsg : public SPrayerMsg
{
	BYTE  bresult;   //��� ���ɹ�����ʧ�ܵĶ���	
	WORD wItemNum;	//��������
	BYTE   bBuffer[bufflen];
};

DECLARE_MSG(SShopRefresh, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOP_REFRESH)
struct SQShopRefresh : public SShopRefresh  //ˢ���̵����
{
	BYTE  brefreshtype; //  0 ���̵�,��ȡ�����б�   1  ʹ�� ����ˢ�� 
};
struct SAShopRefresh : public SShopRefresh  //ˢ���̵����
{
	BYTE  bmoneyType; //��������
	WORD  bRefreshPoint; //ˢ�»���
	DWORD  dRefreshTime; //���ˢ��ʱ����
	PrayerShopItem  m_PrayerItem[MAX_PRAYER_SHOPITEM];
};

DECLARE_MSG(SShopBusiness, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOP_BUSINESS)
struct SQShopBusiness : public SShopBusiness  //ˢ���̵����
{
	BYTE  btype;	//0�������1���� �ֽ׶�ֻ�й���   
	BYTE  bIndex;	//�ڵ����̵��е����������ǵ���ID
};
struct SAShopBusiness : public SShopBusiness  //ˢ���̵����
{
	BYTE  bresult;	//  1�ɹ�  0 ������������ 2 ���ֲ��� 3 �۳�����ʧ�� 4 û��ˢ���̵��еĵ���
};


DECLARE_MSG(SGiftcodeActivity, SScriptBaseMsg, SScriptBaseMsg::EPRO_GIFTCODE_ACTIVITY)
struct SQGiftcodeActivity : public SGiftcodeActivity  //��������ȡ����
{
	char  strgiftcode[MAX_GIFTCODE_LENTH];
};
struct SAGiftcodeActivity : public SGiftcodeActivity  //��������ȡ����
{
	BYTE  bresult;	//  1�ɹ�  �������������
	WORD wItemNum;	//��������
	BYTE   bBuffer[bufflen];
};