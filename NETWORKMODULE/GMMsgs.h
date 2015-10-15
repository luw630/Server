#pragma once

#include "NetModule.h"
#include "itemtypedef.h"
#include "../pub/ConstValue.h"

#define GM_MODE 1       // ʹ��GMģʽ�����ڲ���

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// GM�����Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SGameMngMsg, SMessage, SMessage::EPRO_GAMEMNG_MESSAGE)
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================

// Ψһ��һ��GM��Ϣ��������
struct SQGameMngMsg	:
    public SGameMngMsg
{
    enum E_GM_LIST
    {
        GM_MOVETO,
        GM_SETMONEY,
        GM_SETHPMPSP,
        GM_ADDGOODS,
        GM_SETFIGHT,
		GM_SETFLY,
		GM_SETPROTECT,
        GM_TEST,
        GM_OTHERMOVETO,
        GM_ONLINECHECK,
        GM_DELMEMBER,
        GM_RENAME,
        GM_CHANGEFIGHT,
        GM_SETMUTATE,           // ���ñ���
        GM_SETSCAPEGOAT,        // ��������
        GM_SETICON,
        GM_UPLOAD,
        GM_SRESET,
        GM_ADD_GMC,             // ������װ����ɱ������ļ���
		GM_RLOADMC,				// ���¶���㿨���������Ϣ
		GM_RECOVERPLAYER,
		GM_MOVEBUILDTO,			//
		GM_MOVEBUILDIDTO,		// ������UID�ƶ�
        GM_LOADGMCHECKURL,      // ����GM check URL
        GM_ADDGOODSEX,          // ����A�еȼ���װ��
		GM_ADDEXP,				// ���Ӿ���
		GM_ADDTELERGY,			// �����ķ�������
		GM_ADDSKILLPROF,		// �����书������
		GM_ADDFLYPROF,			// �����Ṧ������
		GM_ADDPROTECTPROF,		// ���ӻ���������
		GM_ADDMOUNTS,			// ������
		GM_ADDPETS,				// �ӷ�ս������
		GM_ADDMOUNT_PROPERTY,	// ����������
		GM_ADDBUFF,				// Ϊ�Լ����һ��BUFF
		GM_OPENVENA,			// ��Ѩ��
		GM_SET_PLAYER_PROPERTY,	// ���ý�ɫ����
		GM_UPDATETELERGY,		// ����ĳ���ķ���ĳ���ȼ����������ݣ�����+�ȼ���
		GM_SendMsg,
        GM_MAX,
		
    };

	DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    WORD    wGMCmd;             // GMָ��
    int     iNum[5];            // ��������

	char    m_Message[256];
	char	szUserpass[CONST_USERPASS];		// ��ɾ�����ɣ���Ա��ʱ����������ز�����Ҫ����������
};

struct SQGameMngStrMsg : public SQGameMngMsg
{
    char szName[CONST_USERNAME];         // ���ɫ���ִ�С��ͬ
};

struct SQGameMngStr2Msg : public SQGameMngStrMsg
{
    char szName2[CONST_USERNAME];        // ���ɫ���ִ�С��ͬ
};

struct SQGMUploadFileMsg : public SQGameMngMsg
{
    DWORD dwVerinfo;        // У������
    DWORD dwOffset;         // ƫ����
    WORD wTransDataSize;    // ��ǰ���ݿ��С
    BYTE byTransFlag;       // ���ͱ��
    BYTE byDestFilenameLen; // �ļ�������
    BYTE pDataEntry[1];     // �������
};

//---------------------------------------------------------------------------------------------
