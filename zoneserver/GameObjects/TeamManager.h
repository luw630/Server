#pragma once

class CPlayer;
struct SMessage;
struct SQChatTeam;

class TeamManager
{
public:
	void Run();
	void TeamLoseStatus(CPlayer *pPlayer, BYTE bResult);	
	int OnRecvTeamMessage(DNID dnidClient, DWORD dwDestGID, bool IsJoinMe, CPlayer *pSource, CPlayer *pDst = 0);
	int OnDispatch(DNID dnidClient, SMessage *pMsg, CPlayer *pPlayer);
	int OnDispatch(DNID dnidClient, SQChatTeam *pMsg, CPlayer *pPlayer);
    
	~TeamManager();
};