#pragma once
//���������
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

	BOOL ProcessDyArenaMsg(DNID dnidClient, struct SArenaMsg *pSArenaMsg, CPlayer *pPlayer);		    // �������������Ϣ

	BOOL OnRecvArenaInitMsg(DNID dnidClient, struct SQSArenaInit *pSArenaMsg, CPlayer *pPlayer);	//����򿪾�����ѡ��Ľ���
	BOOL OnRecvArenaRewardUIMsg(DNID dnidClient, struct SQSArenaRewardUI *pSArenaMsg, CPlayer *pPlayer);	//����򿪾�������������
	BOOL OnRecvArenaStartMsg(DNID dnidClient, struct SQArenaStart *pSArenaMsg, CPlayer *pPlayer);	//������뾺����
	BOOL OnRecvArenaQuestReward(DNID dnidClient, struct SQArenaQuestReward *pSArenaMsg, CPlayer *pPlayer);	//���󾺼�������

	bool SendArenaMsg(CPlayer *pPlayer, SArenaMsg *pSArenaMsg, WORD wmsglenth);//�ظ���������Ϣ
// 	bool SendArenaInitMsg(CPlayer *pPlayer, SASArenaInit *pSArenaMsg, WORD wmsglenth); //�ظ��򿪾�����ѡ��Ľ���
// 	bool SendArenaRewardUIMsg(CPlayer *pPlayer, SASArenaRewardUI *pSArenaMsg, WORD wmsglenth); //�ظ��򿪾�������������
};

