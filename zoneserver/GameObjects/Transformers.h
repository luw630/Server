#pragma once
#include "NETWORKMODULE/TransformersMsg.h"
#include "ItemUser.h"
const __int32 IID_TRANSFORMERS = 0x117cdb40;
class CTransformers :public CItemUser
{
public:
	CTransformers();
	~CTransformers();
	void OnTransformersInit(SQTransformersInit *pTransformersMsg); //������������
	void OnTransformersInfo(SQTransformersInfo *pTransformersMsg);// ����������Ĳ�����Ϣ
	void OnTransformersSwitch(SQTransformersSwitch *pTransformersMsg);// ��������л�
	void OnTransformersSkillInfo(SQTransformersSkillInfo *pTransformersMsg); //�����������Ϣ
	BOOL ProcessTransformersMsg(STransformersMsgs *pTransformersMsg);// ������������Ϣ

	void  OnSendTransformersInfoMsg(SATransformersInfo* pTransformersMsg, WORD wmsglenth);//���ͱ������Ĳ�����Ϣ
	void SendTransformersSwitch(SATransformersSwitch *pTransformersMsg, WORD wmsglenth);// ���ͱ����л�
	void SendTransformersSkillInfo(SATransformersSkillInfo *pTransformersMsg, WORD wmsglenth);// ���ͱ�������Ϣ

	BYTE GetTransformersLevel(BYTE bTransformerIndex);//��ȡ״̬����ȼ�
	BOOL SetTransformersLevel(BYTE bTransformerIndex,BYTE blevel);//����״̬����ȼ�,״̬
};

