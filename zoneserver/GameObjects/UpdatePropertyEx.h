#pragma once

// �����3����ͬ�����ƣ�����ͬ������Լ��������
// ����ö��
enum XYD3_ATTRI
{
	////////// ս������ ////////////	
	XA_GONGJI,			// ����
	XA_FANGYU,			// ����
	XA_BAOJI,			// ����
	XA_SHANBI,			// ����

	XA_MAX_HP,			// �������
	XA_MAX_MP,			// �������
	XA_MAX_TP,			// �������

	XA_CUR_HP,			// ��ǰ����
	XA_CUR_MP,			// ��ǰ����
	XA_CUR_TP,			// ��ǰ����

	XA_LEVEL,			// �ȼ�
	XA_SPEED,			// �ƶ��ٶ�
	XA_ATKSPEED,		// �����ٶ�

	XA_HIT, //����
	XA_UNCRIT, //����
	XA_WRECK,//�ƻ�
	XA_UNWRECK,	//����
	XA_PUNCTURE,	//����
	XA_UNPUNCTURE,	//����

	XA_REDUCEDAMAGE,	// �˺�����
	XA_ABSDAMAGE,		// �����˺�
	XA_NONFANGYU,		// ���ӷ���
	XA_MULTIBAOJI,		// ��������

// 	XA_ICE_DAMAGE,		// ���˺�
// 	XA_ICE_DEFENCE,		// ������
// 	XA_FIRE_DAMAGE,		// ���˺�
// 	XA_FIRE_DEFENCE,	// ����
// 	XA_XUAN_DAMAGE,		// ���˺�
// 	XA_XUAN_DEFENCE,	// ������
// 	XA_POISON_DAMAGE,	// ���˺�
// 	XA_POISON_DEFENCE,	// ������

	////////// ���ר�� ////////////
	XA_MAX_EXP,			// �����
	XA_CUR_EXP,			// ��ǰ����
	XA_MAX_JP,			// �����
	XA_CUR_JP,			// ��ǰ����
	XA_MAX_SP,			// �������
	XA_CUR_SP,			// ��ǰ����

// 	XA_BIND_MONEY,		// �󶨻���
// 	XA_UNBIND_MONEY,	// �ǰ󶨻���
	XA_UNBIND_MONEY,
	XA_BIND_MONEY,

	XA_ZENGBAO,			// ����
	XA_YUANBAO,			// Ԫ��

	XA_TF_JINGGONG,		// ����
	XA_TF_FANGYU,		// ����
	XA_TF_QINGSHEN,		// ����
	XA_TF_JIANSHEN,		// ����
	XA_REMAINPOINT,		// ʣ�����

	XA_FIGHTPOWER,	//���ս����ͬ��

	XA_MAX,
};

// �����������ͣ�������
enum XYD3_ATTRI_TYPE
{
	XAT_WORD,		// 16�޷���
	XAT_DWORD,		// 32�޷���	
	XAT_QWORD,		// 64�޷���

	XAT_MAX,		
};

inline WORD GetAttriType(XYD3_ATTRI attri, WORD &size)
{
	WORD type = XAT_MAX;
	size = 0;

	switch (attri)
	{
	case XA_GONGJI:
	case XA_FANGYU:
	case XA_BAOJI:
	case XA_SHANBI:
	case XA_LEVEL:
	case XA_ATKSPEED:
	case XA_TF_JINGGONG:
	case XA_TF_FANGYU:
	case XA_TF_QINGSHEN:
	case XA_TF_JIANSHEN:
	case XA_REMAINPOINT:

	case XA_REDUCEDAMAGE:
	case XA_ABSDAMAGE:
	case XA_NONFANGYU:
	case XA_MULTIBAOJI:
	case XA_UNCRIT:
	case XA_WRECK:
	case XA_UNWRECK:
	case XA_PUNCTURE:
	case XA_UNPUNCTURE:
	case XA_HIT:
		type = XAT_WORD;
		size = sizeof(WORD);
		break;

	case XA_MAX_HP:
	case XA_MAX_MP:
	case XA_MAX_TP:
	case XA_MAX_JP:
	case XA_MAX_SP:
	case XA_CUR_HP:
	case XA_CUR_MP:
	case XA_CUR_TP:
	case XA_CUR_JP:
	case XA_CUR_SP:
	case XA_BIND_MONEY:
	case XA_UNBIND_MONEY:
	case XA_ZENGBAO:
	case XA_YUANBAO:
	case XA_FIGHTPOWER:
		type = XAT_DWORD;
		size = sizeof(DWORD);
		break;

	case XA_MAX_EXP:
	case XA_CUR_EXP:
		type = XAT_QWORD;
		size = sizeof(QWORD);
		break;

	default:
		break;
	}

	return type;
}