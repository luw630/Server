#pragma once
//竞技场相关
#include "NETWORKMODULE/DyArenaMsg.h"
#include "player.h"
class CArenaMap
{
public:
	~CArenaMap();
	static CArenaMap& GetInstance()
	{
		static CArenaMap instance;
		return instance;
	}

	BOOL ProcessDyArenaMsg(DNID dnidClient, struct SArenaMsg *pSArenaMsg, CPlayer *pPlayer);		    // 处理竞技场相关信息

	BOOL OnRecvArenaInitMsg(DNID dnidClient, struct SQSArenaInit *pSArenaMsg, CPlayer *pPlayer);	//请求打开竞技场选择的界面
	BOOL OnRecvArenaRewardUIMsg(DNID dnidClient, struct SQSArenaRewardUI *pSArenaMsg, CPlayer *pPlayer);	//请求打开竞技场奖励界面
	BOOL OnRecvArenaStartMsg(DNID dnidClient, struct SQArenaStart *pSArenaMsg, CPlayer *pPlayer);	//请求进入竞技场
	BOOL OnRecvArenaQuestReward(DNID dnidClient, struct SQArenaQuestReward *pSArenaMsg, CPlayer *pPlayer);	//请求竞技场奖励

	bool SendArenaMsg(CPlayer *pPlayer, SArenaMsg *pSArenaMsg, WORD wmsglenth);//回复竞技场消息
// 	bool SendArenaInitMsg(CPlayer *pPlayer, SASArenaInit *pSArenaMsg, WORD wmsglenth); //回复打开竞技场选择的界面
// 	bool SendArenaRewardUIMsg(CPlayer *pPlayer, SASArenaRewardUI *pSArenaMsg, WORD wmsglenth); //回复打开竞技场奖励界面
};

