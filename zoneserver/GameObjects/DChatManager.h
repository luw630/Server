#pragma once

class CPlayer;

class CDChatManager
{
    WORD wChatCount;

public:
    CDChatManager(void);
    ~CDChatManager(void);

    void Initialize();
    BOOL ProcessChatMsg(DNID dnidClient, struct SChatBaseMsg *pChatMsg, CPlayer *pPlayer);		    // �������������Ϣ
	void OnGolbalMsg();

    int SendSysMsg(DNID dnidClient, int iMsgID);                // ����ϵͳ��Ϣ
    int SendSysCall(DNID dnidClient, char *szMsg);              // ����ϵͳ����
    int SendToSomeOne(DNID dnidClient, SChatBaseMsg *pMsg);	    // ����������Ϣ����һ���
    int SendToAll(SChatBaseMsg *pMsg);						    // ����������Ϣ���������
    int SendToFriend(SChatBaseMsg *pMsg);						// ����������Ϣ������
    int SendToGroup(SChatBaseMsg *pMsg);						// ����������Ϣ������
    int SendToFaction(SChatBaseMsg *pMsg);						// ����������Ϣ������Ա
};