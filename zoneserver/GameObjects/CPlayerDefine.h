#ifndef CPLAYERDEFINE_H
#define CPLAYERDEFINE_H

class SBaseAttribute
{
public:
// 	WORD		m_FreePoint;			// ���ɼӵ�
// 	DWORD		m_BaseMp;				// ����
// 	DWORD		m_BaseSp;				// ����
// 	WORD		m_HpRecoverSpeed;		// �����ָ��ٶ�
// 	WORD		m_MpRecoverSpeed;		// �����ָ��ٶ�
// 	WORD		m_TpRecoverSpeed;		// �����ָ��ٶ�
// 	WORD		m_HpRecoverInterval;	// �����ָ����
// 	WORD		m_MpRecoverInterval;	// �����ָ����

	DWORD		m_BaseHp;				// ����
	DWORD		m_BaseJp;				// ����
	WORD		m_GongJi;				// ����
	WORD		m_FangYu;				// ����
	WORD  m_Hit;			//����
	WORD		m_ShanBi;				// ����
	WORD		m_BaoJi;				// ����
	WORD m_uncrit; //����
    WORD m_wreck;//�ƻ�
	WORD m_unwreck;	//����
	WORD m_puncture;	//����
	WORD m_unpuncture;	//����
	DWORD		m_BaseTp;				// ����
	WORD  m_TpRecover; //�����ָ�
	WORD		m_TpRecoverInterval;	// �����ָ����
	QWORD		m_Exp;					// ���辭��

};

struct XWZ_Attribute			// ��Ϊֵ����
{
	WORD		m_Atk;			// ����
	WORD		m_Defence;		// ����
	WORD		m_CtrAtk;		// ����
	WORD		m_Escape;		// ���
	WORD		m_Hp;			// ����
	WORD		m_Mp;			// ����
	WORD		m_Tp;			// ����
	WORD		m_SPEED;		// �ƶ��ٶ�
	WORD		m_ATKSPEED;		// �����ٶ�
	WORD		m_ABSDAMAGE;	// �����˺�
	WORD		m_REDUCEDAMAGE;	// �˺�����
	WORD		m_NONFANGYU;	// ���ӷ���
	WORD		KylinArmLevel;	// ����۵ȼ�
};

#endif // CPLAYERDEFINE_H