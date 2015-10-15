#pragma once
#include "pub\ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

#define     TEMP_SYSTEMMSG_CODE     -1
#define     MAX_CHAT_LEN            400
#define     GLOBAL_TALK_CODE        -2
#define     GM_TALK_CODE            -3
#define     SYS_TALK_CODE           -4
#define     GLOBAL_TALK_RUMOUR      -5
#define     GLOBAL_TALK_TRADE       -6
#define     GMCHECKMSG              -7
#define     GLOBAL_TALK_TONG        -8
#define     GLOBAL_SMALL_HORN       -9  // 小喇叭
#define     GLOBAL_TALK_SCHOOL      -13 // 门派
#define     GLOBAL_TALK_REGION      -15 // 场景
#define     MAX_HEAD_LEN            20 //头像长度
// 城主特殊
#define     GLOBAL_CTIYOWNER        -10
#define     GLOBAL_CTIYOWNERTRADE   -11
#define     GLOBAL_CTIYOWNERTRADE_ITEM   -12

//武林盟主特殊
#define		GLOBAL_WULINCHIEF		-14
#define     MAX_CHANNEL_PLAYER_NUMBER  16

enum    E_SYSTEM_ERROR
{
    ESE_NONE,
    // ...
    ESE_MAX,
};

struct SChannelPlayerInfo
{
    DWORD m_dwStaticID;
	char  m_szName[CONST_USERNAME];
};

#define _GetSize(classname, buffer)    WORD len; \
    len = (WORD)dwt::strlen(buffer, MAX_CHAT_LEN); \
    buffer[len] = 0; \
    return sizeof(classname) - (MAX_CHAT_LEN - len - 1) 
