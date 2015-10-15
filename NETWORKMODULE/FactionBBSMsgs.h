#pragma once

#include "TongMsgs.h"

#define BBSTEXT 180
#define MAX_BBSLIST 7

struct FactionBBS_Title
{
	BYTE    byType;
	WORD	wIndexID;
	char	szName[CONST_USERNAME];
	char	szTitle[CONST_USERNAME];
	DWORD	dwUpdataTime;
};

struct FactionBBS : public FactionBBS_Title
{
	char	szText[BBSTEXT];
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ����
//=============================================================================================
DECLARE_MSG_MAP(SFactionBBSMsg,STongBaseMsg,STongBaseMsg::EPRO_FACTIONBBS_MESSAGE)
//{{AFX
EPRO_FACTIONBBS_QUEST_GET,		// ������֪ͨ�ͷ��˷���EPRO_FACTIONBBS_GET��Ϣ
EPRO_FACTIONBBS_GET,			// ��������
EPRO_FACTIONBBS_SAVE,			// ��������
EPRO_FACTIONBBS_DELETE,			// ɾ������
EPRO_FACTIONBBS_GET_TEXT,		// ��������ı�
EPRO_FACTIONBBS_SET_TOP,		// ������������
//}}AFX
END_MSG_MAP()
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SQusetGetMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_QUEST_GET)
struct SQQusetGetMsg : public SQusetGetMsg
{
};
//=============================================================================================



//=============================================================================================
DECLARE_MSG(SGetFacBBSTextMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT)
struct SQGetFacBBSTextMsg : public SGetFacBBSTextMsg
{
	WORD    wIndexID;
	char	szName[CONST_USERNAME];
};

struct SAGetFacBBSTextMsg : public SGetFacBBSTextMsg
{
	WORD	wIndexID;
	size_t  nFreeSize;
	char	streamData[50+BBSTEXT];
};
//=============================================================================================


//=============================================================================================
DECLARE_MSG(SGetFacBBSMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_GET)
struct SQGetFacBBSMsg : public SGetFacBBSMsg
{
	BYTE byPage;
	__int32 nServerID;
	char	szFaction[CONST_USERNAME];
	char	szName[CONST_USERNAME];
	DWORD dwUpTime;
	DWORD dwDownTime;
};

struct SAGetFacBBSMsg : public SGetFacBBSMsg
{
	BYTE	byBBSNum;
	BYTE	byPage;
	char	szName[CONST_USERNAME];

	FactionBBS_Title stFactionBBS[MAX_BBSLIST];
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SSaveFacBBSMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_SAVE)
struct SQSaveFacBBSMsg : public SSaveFacBBSMsg
{
	size_t  nFreeSize;

	WORD	wIndexID;
	__int32 nServerID;
	char	szFaction[CONST_USERNAME];
	char	szName[CONST_USERNAME];

	char    streamData[BBSTEXT+50];
};

struct SASaveFacBBSMsg : public SSaveFacBBSMsg
{
	__int32 nServerID;
	char	szName[CONST_USERNAME];
	WORD	wIndexID;
	DWORD	dwUpdataTime;
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SSetFacBBSTopMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_SET_TOP)
struct SQSetFacBBSTopMsg : public SSetFacBBSTopMsg
{
	WORD wIndex;
	BYTE byTop;
};

struct SASetFacBBSTopMsg : public SSetFacBBSTopMsg
{
	WORD wIndex;
	BYTE byTop;
};
//=============================================================================================

//=============================================================================================
DECLARE_MSG(SDelFacBBSMsg, SFactionBBSMsg, SFactionBBSMsg::EPRO_FACTIONBBS_DELETE)
struct SQDelFacBBSMsg : public SDelFacBBSMsg
{
	WORD wIndex;
	BYTE isRight;
};

struct SADelFacBBSMsg : public SDelFacBBSMsg
{
	WORD wIndex;
	BYTE byResult;
};
//=============================================================================================