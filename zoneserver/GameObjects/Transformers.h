#pragma once
#include "NETWORKMODULE/TransformersMsg.h"
#include "ItemUser.h"
const __int32 IID_TRANSFORMERS = 0x117cdb40;
class CTransformers :public CItemUser
{
public:
	CTransformers();
	~CTransformers();
	void OnTransformersInit(SQTransformersInit *pTransformersMsg); //请求变身激活，升级
	void OnTransformersInfo(SQTransformersInfo *pTransformersMsg);// 请求变身消耗材料信息
	void OnTransformersSwitch(SQTransformersSwitch *pTransformersMsg);// 请求变身切换
	void OnTransformersSkillInfo(SQTransformersSkillInfo *pTransformersMsg); //请求变身技能信息
	BOOL ProcessTransformersMsg(STransformersMsgs *pTransformersMsg);// 处理变身相关信息

	void  OnSendTransformersInfoMsg(SATransformersInfo* pTransformersMsg, WORD wmsglenth);//发送变身消耗材料信息
	void SendTransformersSwitch(SATransformersSwitch *pTransformersMsg, WORD wmsglenth);// 发送变身切换
	void SendTransformersSkillInfo(SATransformersSkillInfo *pTransformersMsg, WORD wmsglenth);// 发送变身技能信息

	BYTE GetTransformersLevel(BYTE bTransformerIndex);//获取状态变身等级
	BOOL SetTransformersLevel(BYTE bTransformerIndex,BYTE blevel);//设置状态变身等级,状态
};

