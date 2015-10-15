#pragma once

class CPlayer;

class CDChatManager
{
    WORD wChatCount;

public:
    CDChatManager(void);
    ~CDChatManager(void);

    void Initialize();
    BOOL ProcessChatMsg(DNID dnidClient, struct SChatBaseMsg *pChatMsg, CPlayer *pPlayer);		    // 处理聊天相关信息
	void OnGolbalMsg();

    int SendSysMsg(DNID dnidClient, int iMsgID);                // 发送系统信息
    int SendSysCall(DNID dnidClient, char *szMsg);              // 发送系统公告
    int SendToSomeOne(DNID dnidClient, SChatBaseMsg *pMsg);	    // 发送聊天信息给单一玩家
    int SendToAll(SChatBaseMsg *pMsg);						    // 发送聊天信息给所有玩家
    int SendToFriend(SChatBaseMsg *pMsg);						// 发送聊天信息给好友
    int SendToGroup(SChatBaseMsg *pMsg);						// 发送聊天信息给队友
    int SendToFaction(SChatBaseMsg *pMsg);						// 发送聊天信息给帮会成员
};