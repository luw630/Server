#pragma once
//场景关卡相关
#include "NETWORKMODULE/SceneMsg.h"
#include "player.h"
class CSceneMap
{
public:
	~CSceneMap(void);
	static CSceneMap& GetInstance()
	{
		static CSceneMap instance;
		return instance;
	}

public:
	 BOOL ProcessSceneMsg(DNID dnidClient, struct SSceneMsg *pSceneMsg, CPlayer *pPlayer);		    // 处理章节关卡相关信息
	 BOOL OnRecvGetSceneMsg(DNID dnidClient, struct SQSceneInit *pSceneMsg, CPlayer *pPlayer);	//请求章节关卡信息
	 BOOL OnRecvSelectSceneMsg(DNID dnidClient, struct SQSceneSelect *pSceneMsg, CPlayer *pPlayer);	//请求关卡数据
	 BOOL OnRecvEnterSceneMsg(DNID dnidClient, struct SQSceneEnter *pSceneMsg, CPlayer *pPlayer);	//请求进入关卡
	 BOOL OnRecvGetRewardMsg(DNID dnidClient, struct SQPickCards *pSceneMsg, CPlayer *pPlayer);	//请求抽牌
	 BOOL OnRecvStopSDMsg(DNID dnidClient, struct SQStopSD *pSceneMsg, CPlayer *pPlayer);	//请求停止扫荡
	 BOOL OnRecvSDRewardMsg(DNID dnidClient, struct SQSDReward *pSceneMsg, CPlayer *pPlayer);	//请求扫荡奖励
	 BOOL OnRecvSQSceneLeave(DNID dnidClient, struct SQSceneLeave *pSceneMsg, CPlayer *pPlayer);	//请求离开关卡
	 BOOL OnRecvSceneQuestReward(DNID dnidClient, struct SQSceneQuestReward *pSceneMsg, CPlayer *pPlayer);	//请求关卡奖励
	 BOOL OnRecvQuestDebrisinfo(DNID dnidClient, struct SQDebrisinfo *pSceneMsg, CPlayer *pPlayer);	//请求碎片收集信息
	 BOOL OnRecvQuestDebrisAward(DNID dnidClient, struct SQDebrisAward *pSceneMsg, CPlayer *pPlayer);	//请求徽章奖励
	 BOOL OnRecvSendGroupMonsterDie(DNID dnidClient, struct SQGroupMonsterDie *pSceneMsg, CPlayer *pPlayer);	//当前分组的怪物已经死亡(用于单机模式客户端通知服务器)

	 bool SendSceneMsg( CPlayer *pPlayer,SASceneInit* pSceneMsg,WORD wmsglenth);
	 bool SendSelectSceneMsg( CPlayer *pPlayer,SASceneSelect* pSceneMsg,WORD wmsglenth);
	 bool SendEnterSceneMsg( CPlayer *pPlayer,SASceneEnter* pSceneMsg);
	 bool SendRewardMsg( CPlayer *pPlayer,SAWinrewardInfo* pSceneMsg,WORD wmsglenth);  //发送胜利结算
	 bool SendWinRewardMsg( CPlayer *pPlayer,SAPickCards* pSceneMsg,WORD wmsglenth); //发送抽牌的奖励
	 bool SendPassFailMsg( CPlayer *pPlayer,SAPassFail* pSceneMsg,WORD wmsglenth); //发送闯关失败消息
	 bool SendSDMsg( CPlayer *pPlayer,SAStopSD* pSceneMsg,WORD wmsglenth); //发送扫荡相关消息
	 bool SendSDReward( CPlayer *pPlayer,SASDReward* pSceneMsg,WORD wmsglenth); //发送扫荡奖励领取相关消息
	 bool SendSceneListMsg( CPlayer *pPlayer,SASceneList* pSceneMsg,WORD wmsglenth); //发送扫荡列表消息
	 bool SendSceneFinishMsg(CPlayer *pPlayer, SASceneFinish* pSceneMsg, WORD wmsglenth); //发送关卡完成消息
	 bool SendDebrisinfo(CPlayer *pPlayer, SADebrisinfo* pSceneMsg, WORD wmsglenth);//发送碎片收集信息
	 bool SendDebrisAward(CPlayer *pPlayer, SADebrisAward* pSceneMsg, WORD wmsglenth);//通知客户端领取徽章奖励
	 bool SendBatchesMonster(CPlayer *pPlayer, SABatchesMonster* pSceneMsg, WORD wmsglenth);//通知客户端怪物已经刷新或者死亡
	 bool SendEPSceneMsg(CPlayer *pPlayer, SSceneMsg* pSceneMsg, WORD wmsglenth);
private:
	// 禁止拷贝构造和拷贝赋值
	CSceneMap(CSceneMap &);
	CSceneMap& operator=(CSceneMap &);
	CSceneMap(){}
};

