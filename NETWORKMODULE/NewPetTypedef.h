#pragma once


/////add by ly 2014/5/24   �³���ϵͳ������ݽṹ����/////////
#define PETNAMEMAXLEN 32	//����������󳤶�
#define PETSKILLMAXNUM 20	//�������ӵ�еļ��������
#define PETMAXUSESKILLNUM 4  //����ͬʱ������װ��ʹ�õļ���

//���＼��
typedef struct newpetskill
{
	DWORD m_PetSkillID;		//���＼��ID
	BYTE m_Level;	//���ܵȼ�
	newpetskill()
	{
		m_PetSkillID = 0;
		m_Level = 0;
	}
}SNewPetSkill, *LPSNewPetSkill;

typedef struct newpetdata
{
	BYTE m_PetLevel;	//����ȼ�
	BYTE m_CurUseSkill[PETMAXUSESKILLNUM];	//��ǰ����ʹ�õļ���
	char m_PetName[PETNAMEMAXLEN];		//��������
	WORD m_PetStarAndMaxLevel;	//�����Ǽ������ȼ��������ֽ�Ϊ�Ǽ������ֽ�Ϊ���ȼ���
	DWORD m_PetID;	//����ID
	DWORD m_CurPetExp;	//��ǰ���ﾭ��
	DWORD m_CurPetHp;	//��ǰ����Ѫ��
	DWORD m_CurPetMp;	//��ǰ���ﾫ��
	DWORD m_CurPetDurable;	//��ǰ�������
	DWORD m_DurableResumeNeedTime;	//�����ָ�����Ҫ��ʱ��
	SNewPetSkill m_PetSkill[PETSKILLMAXNUM];	//��ǰӵ�еļ���
	newpetdata()
	{
		m_PetLevel = 0;
		m_PetStarAndMaxLevel = 0;
		m_PetID = 0;
		m_CurPetExp = 0;
		m_CurPetHp = 0;
		m_CurPetMp = 0;
		m_CurPetDurable = 0;
		memset(m_CurUseSkill, 0xff, PETMAXUSESKILLNUM);
		memset(m_PetName, 0, PETNAMEMAXLEN);
		memset(m_PetSkill, 0, PETSKILLMAXNUM);
	}

}SNewPetData, *LPSNewPetData;