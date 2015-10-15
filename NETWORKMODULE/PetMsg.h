#pragma once
#include "NetModule.h"
#include "../networkmodule/playertypedef.h"
#define MAX_PET_NEEDNUM 5
//=============================================================================================
DECLARE_MSG_MAP(SPetMsg, SMessage, SMessage::EPRO_FIGHTPET_MESSAGE)
//{{AFX
//2014/5/22 add by ly
EPRO_PET_EXTRACT,	//�����Ƴ�ȡ������Ϣ
EPRO_PET_CHIPCOMPOSE,	//������Ƭ�ϳɳ�����Ϣ
EPRO_PET_MERGER,	//�����̲���Ϣ
EPRO_PET_BREACH,	//����ͻ�Ƶ���һ���Ǽ�
EPRO_PET_RENAME,	//����������
EPRO_PET_SKILLUPGRADE,	//���＼������
EPRO_PET_SUISHENFIGHT,	//����������һ���ս
EPRO_PET_SHAPESHIFTFIGHT,	//��ұ�������ս
EPRO_PET_GETCARDPETINFO,	//��ȡ���Ƴ����б���Ϣ
EPRO_PET_USEPETITEM,	//ʹ�ó�����߻�ó���
EPRO_PET_GETPETCARDINFO,	//��ȡ���￨����Ϣ
EPRO_PET_SKILLCONFIG,	//���＼������
EPRO_PET_GETEXPINFO,	//��ȡ�������ؾ�����Ϣ
EPRO_PET_SYNDURABLE,	//ͬ�����������Ϣ
EPRO_PET_GLODBUY,	//���ֱ�ӹ��������Ϣ
EPRO_PET_RESUMEDURABLENEEDGOLD,		//�ָ�������Ļ�����Ҫ�Ľ����Ϣ
EPRO_PET_GOLDRESUMEDURABLE,	//��һָ�������Ļ�����Ϣ
EPRO_PET_STUDYSKILL,	//ͨ��������ѧϰ������¼���
EPRO_PET_OPENSTUDYSKILLPLAN,	//�򿪳��＼��ѧϰ���
EPRO_PET_SYNPETSKILL,	//ͬ�����＼��
//}}AFX
END_MSG_MAP()

////////////////////////////////////////////////////////
//�����Ƴ�ȡ������Ϣ
DECLARE_MSG(SExtractPetMsg, SPetMsg, SPetMsg::EPRO_PET_EXTRACT)
struct SQExtractPetMsg : public SExtractPetMsg
{
	BYTE m_ExtractMode;		//��ȡ��ʽ��1Ϊ��ѳ�ȡ 2Ϊ���ѳ�ȡһ�� 3Ϊ���ѳ�ȡ��� 4Ϊ�Ĳ���ȡ
};

struct SAExtractPetMsg : public SExtractPetMsg
{
	SNewPetData m_ExtractPet;	//��ȡ���ĳ���
};


//������Ƭ�ϳɳ�����Ϣ
DECLARE_MSG(SChipComposPetMsg, SPetMsg, SPetMsg::EPRO_PET_CHIPCOMPOSE)
struct SQChipComposPetMsg : public SChipComposPetMsg
{
	DWORD m_PetID;	//����ID
};

struct SAChipComposPetMsg : public SChipComposPetMsg
{
	SNewPetData m_ClipComposePet;	//�ϳɵĳ���
};

//�����̲���Ϣ
DECLARE_MSG(SMergerPetMsg, SPetMsg, SPetMsg::EPRO_PET_MERGER)
struct SQMergerPetMsg : public SMergerPetMsg
{
	BYTE m_SrcPetIndex;	//�̲����﷢���ߵĳ�������
	BYTE m_DestPetIndex[MAX_PET_NEEDNUM];	//���̲�������������
	DWORD m_ExpRateItemID;	//���ɳ���ʱ�ľ���ת�����ʵ���ID
};

struct SAMergerPetMsg : public SMergerPetMsg
{
	BYTE m_PetLevel;	//�̲���ĳ���ȼ��� Ϊ-1ʱ��ʾʧ��
	DWORD m_PetExp;	//�̲���ĳ��ﾭ��
	DWORD m_PetHp;	//�̲���ĳ���Ѫ��
	DWORD m_PetMp;	//�̲���ĳ��ﾫ��
	DWORD m_PetDurable;	//�̲���ĳ������
};

//����ͻ�Ƶ���һ���Ǽ�
DECLARE_MSG(SPetBreachStarMsg, SPetMsg, SPetMsg::EPRO_PET_BREACH)
struct SQPetBreachStarMsg : public SPetBreachStarMsg
{
	BYTE m_PetIndex;	//��������
};

struct SAPetBreachStarMsg : public SPetBreachStarMsg
{
	WORD m_PetStarAndMaxLevel;	//�����Ǽ������ȼ��������ֽ�Ϊ�Ǽ������ֽ�Ϊ���ȼ���
};

//����������
DECLARE_MSG(SRenamePetMsg, SPetMsg, SPetMsg::EPRO_PET_RENAME)
struct SQRenamePetMsg : public SRenamePetMsg
{
	char m_NewName[PETNAMEMAXLEN];		//������
	BYTE m_PetIndex;	//��������
};

struct SARenamePetMsg : public SRenamePetMsg
{
	BYTE m_Result;	//�ɹ���� 0��ʾʧ�� 1��ʾ�ɹ�
};

//���＼������
DECLARE_MSG(SPetSkillUpgradeMsg, SPetMsg, SPetMsg::EPRO_PET_SKILLUPGRADE)
struct SQPetSkillUpgradeMsg : public SPetSkillUpgradeMsg
{
	BYTE m_PetIndex;	//��������
	BYTE m_PetSkillIndex;	//���＼��������Ϊ0 - 9
};

struct SAPetSkillUpgradeMsg : public SPetSkillUpgradeMsg
{
	BYTE m_Result;	//�ɹ���� 0��ʾʧ�� 1��ʾ�ɹ�
};

//����������һ���ս
DECLARE_MSG(SSuiShenFightMsg, SPetMsg, SPetMsg::EPRO_PET_SUISHENFIGHT)
struct SQSuiShenFightMsg : public SSuiShenFightMsg
{
	BYTE m_PetIndex;	//��������
};

struct SASuiShenFightMsg : public SSuiShenFightMsg
{
	BYTE m_Res;
};

//��ұ�������ս
DECLARE_MSG(SShapeshiftFightMsg, SPetMsg, SPetMsg::EPRO_PET_SHAPESHIFTFIGHT)
struct SQShapeshiftFightMsg : public SShapeshiftFightMsg
{
	BYTE m_PetIndex;	//��������
};

struct SAShapeshiftFightMsg : public SShapeshiftFightMsg
{
	BYTE m_Res;
};

//��ȡ���Ƴ����б���Ϣ
DECLARE_MSG(SGetCardPetInf, SPetMsg, SPetMsg::EPRO_PET_GETCARDPETINFO)
struct SQGetCardPetInf : public SGetCardPetInf
{
	BYTE m_CardType;	//�������� 1��� 2һ�� 3��� 4�Ĳ�
};

struct SAGetCardPetInf : public SGetCardPetInf
{
	BYTE m_PetCount;
	DWORD *m_pPetID;
};

//ʹ�ó�����߻�ó���
DECLARE_MSG(SUsePetItem, SPetMsg, SPetMsg::EPRO_PET_USEPETITEM)
struct SAUsePetItem : public SUsePetItem
{
	SNewPetData m_GetPet;	//ʹ�õ��߻�ó���
};


//��ȡ���￨����Ϣ
DECLARE_MSG(SGetPetCardInf, SPetMsg, SPetMsg::EPRO_PET_GETPETCARDINFO)
struct SQGetPetCardInf : public SGetPetCardInf
{
};

struct SAGetPetCardInf : public SGetPetCardInf
{
	BYTE m_CardCount;	//��������
	BYTE *m_pCardCanExtractTimes;	//ʹ��һ�ο��ƿ��Գ�ȡ�ĳ���Ĵ���
	DWORD *m_pUsePrice;	//��ȡ�۸�
	DWORD *m_pCardCountDown;	//����ˢ�µ���ʱ ������ʱ��Ϊ0ʱ����ʾ�����Գ�ȡ����
};

//���＼������
DECLARE_MSG(SPetSkillCfgInf, SPetMsg, SPetMsg::EPRO_PET_SKILLCONFIG)
struct SQPetSkillCfgInf : public SPetSkillCfgInf
{
	BYTE m_PetIndex;	//��������
	BYTE m_SkillIndex[PETMAXUSESKILLNUM];	//������������
};

struct SAPetSkillCfgInf : public SPetSkillCfgInf
{
	BYTE m_Res;	//���ز������ 0��ʾʧ�� 1��ʾ�ɹ�
};


//��ȡ���ɳ�����Ի�ȡ�ľ���
DECLARE_MSG(SPetExpInfMsg, SPetMsg, SPetMsg::EPRO_PET_GETEXPINFO)
struct SQPetExpInfMsg : public SPetExpInfMsg
{
	BYTE m_DestPetIndex[MAX_PET_NEEDNUM];	//���̲�������������
	DWORD m_ExpRateItemID;	//���ɳ���ʱ�ľ���ת�����ʵ���ID
};

struct SAPetExpInfMsg : public SPetExpInfMsg
{
	DWORD m_PetAddExp;	//�̲��������ӵĳ��ﾭ��
};


//ͬ�����������Ϣ
DECLARE_MSG(SPetSynDurableMsg, SPetMsg, SPetMsg::EPRO_PET_SYNDURABLE)
struct SAPetSynDurableMsg : public SPetSynDurableMsg
{
	BYTE m_PetIndex;	//��������
	DWORD m_CurDurable;	//��ǰ����
};

//���ֱ�ӹ��������Ϣ
DECLARE_MSG(SGlodDirectBuyPetMsg, SPetMsg, SPetMsg::EPRO_PET_GLODBUY)
struct SQGlodDirectBuyPetMsg : public SGlodDirectBuyPetMsg
{
	DWORD m_PetID;	//����ID
};

struct SAGlodDirectBuyPetMsg : public SGlodDirectBuyPetMsg
{
	SNewPetData m_BuyPet;	//����ĳ���
};

//�ָ�������Ļ�����Ҫ�Ľ����Ϣ
DECLARE_MSG(SResumeNdGoldMsg, SPetMsg, SPetMsg::EPRO_PET_RESUMEDURABLENEEDGOLD)
struct SQResumeNdGoldMsg : public SResumeNdGoldMsg
{
	BYTE m_PetIndex;	//��������
};

struct SAResumeNdGoldMsg : public SResumeNdGoldMsg
{
	DWORD m_GoldNum;	//��Ҫ�Ľ����
};
//��һָ�������Ļ�����Ϣ
DECLARE_MSG(SResumePetDurableMsg, SPetMsg, SPetMsg::EPRO_PET_GOLDRESUMEDURABLE)
struct SQResumePetDurableMsg : public SResumePetDurableMsg
{
	BYTE m_PetIndex;	//��������
};

struct SAResumePetDurableMsg : public SResumePetDurableMsg
{
	BYTE m_Result;	//����Ľ��
};


//�������ǰ��ҵ�ս������
struct FightProperty
{
	// ��������
	DWORD		m_MaxHp;				// �������
	DWORD		m_CurHp;				// ��ǰ����
	WORD		m_GongJi;				// ����
	WORD		m_FangYu;				// ����
	WORD		m_BaoJi;				// ����
	WORD		m_Hit;					//����
	WORD		m_ShanBi;				// ����
	WORD m_uncrit; //����
 	WORD m_wreck;//�ƻ�
 	WORD m_unwreck;	//����
 	WORD m_puncture;	//����
 	WORD m_unpuncture;	//����
};


//ͨ��������ѧϰ������¼���
DECLARE_MSG(SPetStudySkillMsg, SPetMsg, SPetMsg::EPRO_PET_STUDYSKILL)
struct SQPetStudySkillMsg : public SPetStudySkillMsg
{
	BYTE m_PetIndex;	//��������
	DWORD m_PetSkillBookID;	//���＼����ID
};

struct SAPetStudySkillMsg : public SPetStudySkillMsg
{
	BYTE m_Result;	//ѧϰ���
};

//�򿪳��＼��ѧϰ���
DECLARE_MSG(SOpenPetStudySkillPlanMsg, SPetMsg, SPetMsg::EPRO_PET_OPENSTUDYSKILLPLAN)
struct SQOpenPetStudySkillPlanMsg : public SOpenPetStudySkillPlanMsg
{
};

struct SAOpenPetStudySkillPlanMsg : public SOpenPetStudySkillPlanMsg
{
	BYTE m_Result;
};

//ͬ�����＼��
DECLARE_MSG(SSynPetSkillMsg, SPetMsg, SPetMsg::EPRO_PET_SYNPETSKILL)

struct SASynPetSkillMsg : public SSynPetSkillMsg
{
	BYTE m_PetIndex;	//��������
	BYTE m_PetSkillIndex;	//���＼������
	BYTE m_PetSkillLevel;	//���＼�ܵȼ�
	DWORD m_PetSkillID;	//���＼��ID
};