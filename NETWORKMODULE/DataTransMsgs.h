#pragma once
#include "NetModule.h"

// 账号数据库操作规则
// 登陆时会检测账号 [EPRO_CHECK_ACCOUNT] ，由于仅当账号开始记费时才认为客户端连接有效，所以检测成功的同时账号开始记费（实际上是记录登陆时间，等到退出的时候改数据库扣点）
// 记费过程中，随时可以发消息 [EPRO_REFRESH_CARD_POINT] 到账号服务器进行数据刷新，也就是根据游戏时间进行扣点，同时游戏时间清零，如果该账号还未启动记费，则自动开始记费
// 记费过程中，账号服务器随时可以询问游戏服务器 [ ] 某个账号是否仍然存在，如果询问失败（游戏服务器故障或该账号不存在于游戏服务器），记费停止
// 玩家退出时，游戏服务器通知账号服务器 [EPRO_ACCOUNT_LOGOUT] 账号退出， 记费停止

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 账号服务器相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SDataTransMsg, SMessage, SMessage::EPRO_DATATRANS_MESSAGE)
//{{AFX
EPRO_SRESET,
EPRO_SCRIPT_BULLETIN,
EPRO_TONGONE
//}}AFX
END_MSG_MAP()


// 重置服务器上脚本！
DECLARE_MSG(_SResetScriptMsg, SDataTransMsg, SDataTransMsg::EPRO_SRESET)
struct SResetScriptMsg : _SResetScriptMsg { BYTE flag; };

// 脚本的公告信息
DECLARE_MSG(SScriptBulletinMsg, SDataTransMsg, SDataTransMsg::EPRO_SCRIPT_BULLETIN)
#define MAX_SSYSCALL_LEN 500
struct SAScriptBulletinMsg  : public SScriptBulletinMsg
{
    char cChatData[MAX_SSYSCALL_LEN];
    WORD GetMySize()
    {
        _GetSSize(SAScriptBulletinMsg,cChatData);
    }

};

// 通告第一帮到其他服务器
DECLARE_MSG(STongOneMsg, SDataTransMsg, SDataTransMsg::EPRO_TONGONE)
struct SQTongOneMsg : public STongOneMsg
{
	char szTongName[CONST_USERNAME];
	DWORD dwFactionTitleID;
};