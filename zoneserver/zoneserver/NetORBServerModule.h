#pragma once
#include "networkmodule\netconsumermodule\netconsumermodule.h"
#include "networkmodule\orbmsgs.h"

class CNetORBServerModule :
    public CNetConsumerModule
{
public:
    void Execution(void);	            // 这里做基本的套接字操作，数据的接收与发送，相应函数的调用

    BOOL SendMsgToORBSrv(SORBMsg *pMsg, int iSize);

    bool OnDispatch( void *data, size_t size );     // 因为这里要做类型检查，所以需要从最底层的消息开始做起
	enum { AUTOCONNECTORBTIME= 1000 * 60 * 3 };
public:
    void OnDispatchCtrlChatMsg(SORBChatBaseMsg *pMsg);
    void OnDispatchCtrlSimplayerMsg(SCtrlSimPlayer *pMsg);
    void OnDispatchCtrlCheckMsg(SCtrlCheckMsg * pMsg);
    void OnDispatchCtrlFactionMsg(SCtrlFactionMsg *pMsg);
    void OnDispatchCtrlTranDataMsg(SQAORBDataTransMsg *pMsg);

public:
    // base talk operation
    void OnRecvTalkGlobalMsg(SAORBChatGlobalMsg * pMsg);

	void OnRecvFactionChatMsg(SAORBChatFactionMsg * pMsg);

public:
    CNetORBServerModule(void);
    ~CNetORBServerModule(void);

private:
    DWORD m_prevCheckTime;

    // base team operation
    void OnDispatchCtrlTeamGroupMsg(SCtrlTeamGroup *pMsg);
    void OnDispatchTeamOperationMsg(STeamOperationMsg *pMsg);
    void OnDispatchCtrlChannelGroupMsg(SORBChannelBaseMsg *pMsg);
    int  OnRecvTeamPassbyMsg(SATeamPassbyMsg *pMsg);
    void OnRecvTeamCreateMsg(SATeamCreateMsg *pMsg);
    void OnRecvDeleteTeamMsg(SATeamDeleteMsg *pMsg);
    void OnRecvTeamSomeoneJoinMsg(SATeamSomeoneJoinMsg *pMsg);
    void OnRecvTeamSomeoneLeaveMsg(SATeamSomeoneLeaveMsg *pMsg);

    // check team operation
    void OnRecvTeamChangeSkillMsg(SATeamChangeSkillMsg *pMsg);
    void OnRecvTeamShareExpMsg(SATeamShareExpMsg *pMsg);
    void OnRecvTeamTalkMsg(SATeamTalkMsg *pMsg);
    void OnRecvTeamGetMember(SAGetTeamMemberMsg * pMsg);
};
