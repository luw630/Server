#pragma once
//�����ؿ����
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
	 BOOL ProcessSceneMsg(DNID dnidClient, struct SSceneMsg *pSceneMsg, CPlayer *pPlayer);		    // �����½ڹؿ������Ϣ
	 BOOL OnRecvGetSceneMsg(DNID dnidClient, struct SQSceneInit *pSceneMsg, CPlayer *pPlayer);	//�����½ڹؿ���Ϣ
	 BOOL OnRecvSelectSceneMsg(DNID dnidClient, struct SQSceneSelect *pSceneMsg, CPlayer *pPlayer);	//����ؿ�����
	 BOOL OnRecvEnterSceneMsg(DNID dnidClient, struct SQSceneEnter *pSceneMsg, CPlayer *pPlayer);	//�������ؿ�
	 BOOL OnRecvGetRewardMsg(DNID dnidClient, struct SQPickCards *pSceneMsg, CPlayer *pPlayer);	//�������
	 BOOL OnRecvStopSDMsg(DNID dnidClient, struct SQStopSD *pSceneMsg, CPlayer *pPlayer);	//����ֹͣɨ��
	 BOOL OnRecvSDRewardMsg(DNID dnidClient, struct SQSDReward *pSceneMsg, CPlayer *pPlayer);	//����ɨ������
	 BOOL OnRecvSQSceneLeave(DNID dnidClient, struct SQSceneLeave *pSceneMsg, CPlayer *pPlayer);	//�����뿪�ؿ�
	 BOOL OnRecvSceneQuestReward(DNID dnidClient, struct SQSceneQuestReward *pSceneMsg, CPlayer *pPlayer);	//����ؿ�����
	 BOOL OnRecvQuestDebrisinfo(DNID dnidClient, struct SQDebrisinfo *pSceneMsg, CPlayer *pPlayer);	//������Ƭ�ռ���Ϣ
	 BOOL OnRecvQuestDebrisAward(DNID dnidClient, struct SQDebrisAward *pSceneMsg, CPlayer *pPlayer);	//������½���
	 BOOL OnRecvSendGroupMonsterDie(DNID dnidClient, struct SQGroupMonsterDie *pSceneMsg, CPlayer *pPlayer);	//��ǰ����Ĺ����Ѿ�����(���ڵ���ģʽ�ͻ���֪ͨ������)

	 bool SendSceneMsg( CPlayer *pPlayer,SASceneInit* pSceneMsg,WORD wmsglenth);
	 bool SendSelectSceneMsg( CPlayer *pPlayer,SASceneSelect* pSceneMsg,WORD wmsglenth);
	 bool SendEnterSceneMsg( CPlayer *pPlayer,SASceneEnter* pSceneMsg);
	 bool SendRewardMsg( CPlayer *pPlayer,SAWinrewardInfo* pSceneMsg,WORD wmsglenth);  //����ʤ������
	 bool SendWinRewardMsg( CPlayer *pPlayer,SAPickCards* pSceneMsg,WORD wmsglenth); //���ͳ��ƵĽ���
	 bool SendPassFailMsg( CPlayer *pPlayer,SAPassFail* pSceneMsg,WORD wmsglenth); //���ʹ���ʧ����Ϣ
	 bool SendSDMsg( CPlayer *pPlayer,SAStopSD* pSceneMsg,WORD wmsglenth); //����ɨ�������Ϣ
	 bool SendSDReward( CPlayer *pPlayer,SASDReward* pSceneMsg,WORD wmsglenth); //����ɨ��������ȡ�����Ϣ
	 bool SendSceneListMsg( CPlayer *pPlayer,SASceneList* pSceneMsg,WORD wmsglenth); //����ɨ���б���Ϣ
	 bool SendSceneFinishMsg(CPlayer *pPlayer, SASceneFinish* pSceneMsg, WORD wmsglenth); //���͹ؿ������Ϣ
	 bool SendDebrisinfo(CPlayer *pPlayer, SADebrisinfo* pSceneMsg, WORD wmsglenth);//������Ƭ�ռ���Ϣ
	 bool SendDebrisAward(CPlayer *pPlayer, SADebrisAward* pSceneMsg, WORD wmsglenth);//֪ͨ�ͻ�����ȡ���½���
	 bool SendBatchesMonster(CPlayer *pPlayer, SABatchesMonster* pSceneMsg, WORD wmsglenth);//֪ͨ�ͻ��˹����Ѿ�ˢ�»�������
	 bool SendEPSceneMsg(CPlayer *pPlayer, SSceneMsg* pSceneMsg, WORD wmsglenth);
private:
	// ��ֹ��������Ϳ�����ֵ
	CSceneMap(CSceneMap &);
	CSceneMap& operator=(CSceneMap &);
	CSceneMap(){}
};

