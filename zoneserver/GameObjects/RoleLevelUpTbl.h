#ifndef _ATTACK_AND_DEFENSE_H_
#define _ATTACK_AND_DEFENSE_H_
/*
	��ɫ������
	�����������Ž�ɫ��ͬ�ȼ��µ����ݡ�����ֻ������
*/
class CRoleLevelUpTbl
{
private:
	CRoleLevelUpTbl();
	~CRoleLevelUpTbl();
public:
	struct SDataTbl 
	{
		// Ѫ��ֵ
		INT32 m_Blood;		
		// ����ֵ
		INT32 m_InnerForce;		
		// ����ֵ
		INT32 m_VitalForce;	
		// ɱ��ֵ
		INT32 m_KillForce;	
		// �⹥���� �ڹ����� ������� �ڷ�����
//		INT32 m_wAttackDefenseBase[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX ];
		// ��ֵ
		INT32 m_BodyWork ;
		//��Ѩ��
		INT32 m_VenapointCount;
		// ������ ������
		INT32 m_wAddPoints[ 2 ];
		// ��������Ҫ�ľ���
		INT32 m_MaxExp;
	};

public:
	// �õ���ɫ������ĵ���
	static CRoleLevelUpTbl* GetSingleton();

	// ͨ�������ļ������ɫ������
	bool LoadRoleLevelUpTbl();

	// ȡ�ý�ɫ���ȼ�
	BYTE GetMaxLevel() { return (BYTE)m_totalLevel; }

	// ͨ������level�õ�����
	inline SDataTbl* GetPlayerDataByLevel(int level)
	{
        if (m_dataTbl == NULL )
            return NULL;
		if ((DWORD)level > m_totalLevel || level < 1)
			return &m_dataTbl[m_totalLevel - 1]; // ��֤�ڵȼ�Խ���Ҳ�ܹ�ȡ��ֵ
		return &m_dataTbl[level - 1];
	}
private:
	// ����������
	SDataTbl* m_dataTbl;
	// �ܵĵȼ�
	DWORD m_totalLevel;
};

#endif