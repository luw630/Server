#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"

#define MAX_ITEM_DATA		256
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// ���������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(STransformersMsgs, SMessage, SMessage::EPRO_TRANSFORMERS_MESSAGE)
EPRO_TRANSFORMERS_INIT,				//���������
EPRO_TRANSFORMERS_INFO,				//������Ϣ
EPRO_TRANSFORMERS_SKILLINFO,				//�����������Ϣ
EPRO_TRANSFORMERS_SWITCH,				//�����л�
EPRO_TRANSFORMERS_SKILL,				//������
EPRO_TRANSFORMERS_UNDOSHAPE,	//ȡ����ҵı�����̬
END_MSG_MAP()

// ������������
DECLARE_MSG(STransformersInit, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_INIT)
struct SQTransformersInit : public STransformersInit
{
	BYTE		bTransformerIndex;			//�����������
};
struct SATransformersInit : public STransformersInit
{
	BYTE		bresult;			//������
};

// ����������Ĳ�����Ϣ
DECLARE_MSG(STransformersInfo, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_INFO)
struct SQTransformersInfo : public STransformersInfo
{
	BYTE		bTransformerIndex;			//�����������
};
struct SATransformersInfo : public STransformersInfo
{
	BYTE   dmaterialnum;//��������
	DWORD dmoney;//��Ҫ��Ǯ
	DWORD dmaterialIndex;//��������
	DWORD dcurrentHP;//��ǰ�ȼ�HP
	DWORD dnextHP;//���������Ժ��HP

};


// �������
DECLARE_MSG(STransformersSwitch, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_SWITCH)
struct SQTransformersSwitch : public STransformersSwitch
{
	BYTE		bTransformerIndex;			//�����������
};
struct SATransformersSwitch : public STransformersSwitch
{
	BYTE bresult;
};

//�����������Ϣ
DECLARE_MSG(STransformersSkillInfo, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_SKILLINFO)
struct SQTransformersSkillInfo : public STransformersSkillInfo
{
	BYTE		bTransformerIndex;			//�����������
};
struct SATransformersSkillInfo : public STransformersSkillInfo
{
	WORD	skillCurrentdamage;//��ɵ��˺�
	WORD	skillLevelUpdamage;//��������ɵ��˺�
	WORD CurrentTriggerate;//��ǰ�������� 10000
	WORD LevelUpTriggerate;//�����󴥷����� 10000
	WORD wSkillIndex; //�����Ӽ���index
	DWORD currentSkillExp;//�����ܵ�ǰ������
	DWORD levelUpSkillExp;//������������Ҫ������
	DWORD silveritem; //ʹ�ü���������ͨ����
	DWORD goditem; //ʹ�ü������Ľ�ҵ���
	BYTE  bCurrentLevel;//��ǰ���ܵȼ�
	
};

//ȡ����ҵı�����̬
DECLARE_MSG(SUnTransformers, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_UNDOSHAPE)
struct SAUnTransformers : public SUnTransformers
{
};
