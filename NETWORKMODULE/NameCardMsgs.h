#pragma once

#include "NetModule.h"
#include "playertypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 名片相关
//=============================================================================================
DECLARE_MSG_MAP(SNameCardBaseMsg, SMessage, SMessage::EPRO_NAMECARD_BASE)
//{{AFX
EPRO_NAMECARD_QUESTINFO,    // 请求获取一个玩家的名片数据
EPRO_NAMECARD_UPDATEINFO,   // 请求刷新名片数据（包括自己的和别人的）
EPRO_NAMECARD_SETMASK,      // 请求涂改名片的数据（只能是自己的）
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 请求获取一个玩家的名片数据
DECLARE_MSG(SNameCardQuestInfoMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_QUESTINFO)
struct SQNameCardQuestInfoMsg : 
    public SNameCardQuestInfoMsg
{
    DWORD dwDestGID;    // 目标的GID
    DWORD dwSelfGID;    // 自己的GID
};

struct SANameCardQuestInfoMsg : 
    public SNameCardQuestInfoMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    DWORD dwDestGID;    // 目标的GID

    // SPlayerComment Info;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 请求刷新名片数据（包括自己的和别人的）
DECLARE_MSG(SNameCardUpdateInfoMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_UPDATEINFO)
struct SQNameCardUpdateInfoMsg : 
    public SNameCardUpdateInfoMsg
{
    DWORD dwDestGID;    // 目标的GID

    DWORD dwSelfGID;    // 自己的GID

    BYTE byInfoNumber;  // 修改的目的编号 1：自我介绍 2：路人评价 3：队友评价 4：帮主评价

    char szSelfIntroduction[200]; // 修改内容
};

struct SANameCardUpdateInfoMsg : 
    public SNameCardUpdateInfoMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    DWORD dwSrcGID;                 // 源GID （修改的人）

    DWORD dwDestGID;                // 目标的GID （被修改的人）

    BYTE byInfoNumber;              // 修改的目的编号 1：自我介绍 2：路人评价 3：队友评价 4：帮主评价

    BYTE byIndex;                   // 当类型为 （路人评价/队友评价）的时候，指定的位置

    SYSTEMTIME UpdateTime;          // 当前的修改时间

    char szSelfIntroduction[200];   // 修改的内容
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 请求涂改名片的数据（只能是自己的）
DECLARE_MSG(SNameCardSetMaskMsg, SNameCardBaseMsg, SNameCardBaseMsg::EPRO_NAMECARD_SETMASK)
struct SQNameCardSetMaskMsg : 
    public SNameCardSetMaskMsg
{
    DWORD dwSelfGID;    // 自己的GID

    BYTE byInfoNumber;  // 修改的目的编号 1：自我介绍 2：路人评价 3：队友评价 4：帮主评价

    BYTE byMaskNumber;  // 第几个字
};

struct SANameCardSetMaskMsg : 
    public SNameCardSetMaskMsg
{
    enum ERetCode
    {
        ERC_OK,
        ERC_SERVER_DISCARD,
    };

    BYTE byRetCode;

    BYTE byMaskNumber;  // 第几个字

    BYTE byInfoNumber;  // 修改的目的编号 1：自我介绍 2：路人评价 3：队友评价 4：帮主评价

    BYTE byLeftMask;    // 还剩下的Mask次数
};
//---------------------------------------------------------------------------------------------
