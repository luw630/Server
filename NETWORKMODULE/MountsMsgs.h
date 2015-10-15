#pragma once

#include "NetModule.h"
#include "PlayerTypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// ������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SMountsMsg, SMessage, SMessage::ERPO_MOUNTS_MESSAGE)
//{{AFX
EPRO_ADD_MOUNTS,			// �������
EPRO_EQUIP_MOUNTS,			// װ������
EPRO_UNEQUIP_MOUNTS,		// ȡ��װ������
EPRO_UP_MOUNTS,				// ����
EPRO_DOWN_MOUNTS,			// ����
EPRO_FEED_MOUNTS,			// ιʳ
EPRO_ADD_POINT,				// �ӵ�
EPRO_DELETE_MOUNTS,			// ����
EPRO_USE_SKILL,				// ʹ�ü���
EPRO_SYNPLAYMOUNT_ACTION,	// �㲥����״̬
EPRO_ADD_PETS,				// �������
EPRO_CALL_OUT_PET,			// �ٻ�����
EPRO_CALL_BACK_PET,			// �ջ�
EPRO_DELETE_PETS,			// ����
EPRO_ADD_FIGHTPET,			// �������
EPRO_DEL_FIGHTPET,			// ��������
EPRO_CALL_OUT_FIGHTPET,		// ���ͳ�ս
EPRO_CALL_BACK_FIGHTPET,	// ������Ϣ
EPRO_FIGHTPET_STATUSCHG,	// ����״̬�л�
EPRO_FIGHTPET_STARTATK,		// ���͹���
EPRO_FIGHTPET_MOVETOMSG,	// ����˲����Ϣ
EPRO_FIGHTPET_MOVEFINSHMSG,	// ����˲�������Ϣ
EPRO_FIGHTPET_DEAD,		// ����������ʬ�壩
EPRO_FIGHTPET_IMPEL,		// ��������
EPRO_FIGHTPET_CHANGEATTTYPE,				//�����л�����ģʽ
//}}AFX
END_MSG_MAP()

// ���͹���
DECLARE_MSG(SFightPetStartAtk, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_STARTATK)
struct SAFightPetStartAtk : public SFightPetStartAtk
{
	DWORD dwFightPetID;
	DWORD dwTargetID;
};

// ����״̬�л�
DECLARE_MSG(SFightPetStatusChg, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_STATUSCHG)
struct SAFightPetStatusChg : public SFightPetStatusChg
{
	enum
	{
		SFPS_FOLLOW,	// ����
		SFPS_FIGHT,		// ս��
	};

	BYTE  bCurStatus;
	DWORD dwPetGID;		// ����ID
};

// ���ͳ�ս
DECLARE_MSG(SCallOutFightPet, SMountsMsg, SMountsMsg::EPRO_CALL_OUT_FIGHTPET)
struct SQCallOutFightPet : public SCallOutFightPet
{
	BYTE index;
	float x;	///��������
	float y;
};

struct SACallOutFightPet : public SCallOutFightPet
{
	BYTE  index;			// ����
	DWORD fpGID;			// ����ID
	float x;	///��������
	float y;
};

// �����ս
DECLARE_MSG(SCallOutPet, SMountsMsg, SMountsMsg::EPRO_CALL_OUT_PET)
struct SQCallOutPet : public SCallOutPet
{
	BYTE index;
};

struct SACallOutPet : public SCallOutPet
{
	DWORD dwOwnerID;		// ��������ID
	DWORD dwPetGlobalID;	// �����ΨһID
	WORD  wPetID;			// �ٻ����ĳ���ID������Ϊ0
};

// ������Ϣ
DECLARE_MSG(SCallBackFightPet, SMountsMsg, SMountsMsg::EPRO_CALL_BACK_FIGHTPET)
struct SQCallBackFightPet : public SCallBackFightPet
{
};

struct SACallBackFightPet : public SCallBackFightPet
{
	BYTE  index;			// ����
	DWORD fpGID;			// ����ID
};

// ������Ϣ
DECLARE_MSG(SCallBackPet, SMountsMsg, SMountsMsg::EPRO_CALL_BACK_PET)
struct SQCallBackPet : public SCallBackPet
{
};

struct SACallBackPet : public SCallBackPet
{
	DWORD dwOwnerID;		// ��������ID
	DWORD dwPetGlobalID;	// �����ΨһID
	WORD  wPetID;			// �ٻ����ĳ���ID������Ϊ0
};

// �������
DECLARE_MSG(SAddMounts, SMountsMsg, SMountsMsg::EPRO_ADD_MOUNTS)
struct SAAddMounts : public SAddMounts 
{
	SPlayerMounts::Mounts mounts;
};

// ��ӳ���
DECLARE_MSG(SAddPets, SMountsMsg, SMountsMsg::EPRO_ADD_PETS)
struct SAAddPets : public SAddPets
{
	SPlayerPets::Pets pet;
};

// �������
DECLARE_MSG(SAddFightPet, SMountsMsg, SMountsMsg::EPRO_ADD_FIGHTPET)
struct SAAddFightPet : public SAddFightPet
{
	SFightPetExt fightpet;
};

// װ������
DECLARE_MSG(SEquipMounts, SMountsMsg, SMountsMsg::EPRO_EQUIP_MOUNTS)
struct SQEquipMounts : public SEquipMounts 
{
	BYTE index;
};

struct SAEquipMounts : public SEquipMounts
{
	BYTE index;
};

// ȡ��װ������
DECLARE_MSG(SUnEquipMounts, SMountsMsg, SMountsMsg::EPRO_UNEQUIP_MOUNTS)
struct SQUnEquipMounts : public SUnEquipMounts 
{
	BYTE index;
};

struct SAUnEquipMounts : public SUnEquipMounts 
{
	BYTE index;
};

// ����
DECLARE_MSG(SUpMounts, SMountsMsg, SMountsMsg::EPRO_UP_MOUNTS)
struct SQUpMounts : public SUpMounts 
{
	BYTE index;
};

struct SAUpMount : public SUpMounts 
{
	BYTE index;
	BYTE result;
};

// ����
DECLARE_MSG(SDownMounts, SMountsMsg, SMountsMsg::EPRO_DOWN_MOUNTS)
struct SQDownMounts : public SDownMounts 
{
};

struct SADownMounts : public SDownMounts 
{
	BYTE index;	
	BYTE result;
};

// ιʳ
DECLARE_MSG(SFeedMounts, SMountsMsg, SMountsMsg::EPRO_FEED_MOUNTS)
struct SQFeedMounts : public SFeedMounts
{
	BYTE mountIndex;		// �������
	WORD foodIndex;			// ʳ������
};

struct SAFeedMounts : public SFeedMounts
{
	BYTE  mountIndex;		// �������
	BYTE  mountLevel;		// ��˵ȼ�
	DWORD curExp;			// ��˾���
	WORD  Attri[SPlayerMounts::SPM_MAX];	// �������
};

// �ӵ�
DECLARE_MSG(SMountAddPoint, SMountsMsg, SMountsMsg::EPRO_ADD_POINT)
struct SQMountAddPoint : public SMountAddPoint
{
	BYTE mountIndex;	// �������
	WORD attri[5];		// Ҫ�ӵ����Ե�
};

struct SAMountAddPoint : public SMountAddPoint
{
	BYTE mountIndex;	// �������
	WORD value[5];		// ��ǰ������ֵ
	WORD remainPoint;	// ʣ�����
};

// ɾ������
DECLARE_MSG(SDeletePets, SMountsMsg, SMountsMsg::EPRO_DELETE_PETS)
struct SQDeletePets : public SDeletePets
{
	BYTE index;
};

struct SADeletePets : public SDeletePets
{
	BYTE index;
};

// ɾ������
DECLARE_MSG(SDeleteMounts, SMountsMsg, SMountsMsg::EPRO_DELETE_MOUNTS)
struct SQDeleteMounts : public SDeleteMounts
{
	BYTE index;
};

// ɾ���������
struct SADeleteMounts : public SDeleteMounts
{
	BYTE index;
};

// ɾ������
DECLARE_MSG(SDeleteFightPet, SMountsMsg, SMountsMsg::EPRO_DEL_FIGHTPET)
struct SQDeleteFightPet : public SDeleteFightPet
{
	BYTE index;
};

// ɾ�����ͻ���
struct SADeleteFightPet : public SDeleteFightPet
{
	BYTE index;
};

// ʹ�����＼��
DECLARE_MSG(SUseMountSkill, SMountsMsg, SMountsMsg::EPRO_USE_SKILL)
struct SQUseMountSkill : public SUseMountSkill
{
	short int index;	// ��������
	short int skillType;// ��������(��������)
	short int skillIdx;	// �����������	
};

// ͬ������״̬
DECLARE_MSG(SMountAction, SMountsMsg, SMountsMsg::EPRO_SYNPLAYMOUNT_ACTION)
struct SAMountAction : public SMountAction
{
	enum
	{
		MA_READY,		// ׼������
		MA_CANCEL_READY,// ȡ��׼��
		MA_UP,			// ������
		MA_DOWN,		// ������
	};

	DWORD	dwGID;		// ���ID
	WORD	index;		// ���ID
	BYTE	level;		// ��ĵȼ�
	BYTE	bAction;	// �������� 0 ��ʾ׼�� 1 ��ʾ���϶���
};

// ����˲����Ϣ
DECLARE_MSG(SFightPetMoveto, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_MOVETOMSG)
struct SQFightPetMoveto : public SFightPetMoveto
{
	DWORD	dwGID;		// ���ID
	//DWORD	fpGID;		// ����ID
	float     PosX;			//�ƶ�Ŀ������
 	float     PosY;
	//float		direction;	// ����
};

// ����˲�����
DECLARE_MSG(SFightPetMoveFinsh, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_MOVEFINSHMSG)
struct SAFightPetMoveFinsh : public SFightPetMoveFinsh
{
	DWORD	dwGID;		// ���ID
	//DWORD	fpGID;		// ����ID
// 	float     PosX;			//�ƶ�Ŀ������
// 	float     PosY;
	//float		direction;	// ����
};

// ����������ʬ�壩
DECLARE_MSG(SFightPetDead, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_DEAD)
struct SQFightPetDead : public SFightPetDead
{
	//DWORD	dwGID;		// ���ID
		BYTE		m_index;		//��������		
		float     PosX;			//����
	 	float     PosY;
};

//���ͼ���
DECLARE_MSG(SFightPetImpel, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_IMPEL)
struct SQFightPetImpel : public SFightPetImpel
{
		BYTE		m_index;		//��������		
		WORD	m_impelValue;	//ʹ�õļ���ֵ.
};

struct SAFightPetImpel : public SFightPetImpel
{
	WORD	m_impelValue;	//����ʣ�༤��ֵ
};


//�����л�����ģʽ
DECLARE_MSG(SfpchangeatttypeMsg, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_CHANGEATTTYPE)
struct SQfpchangeatttypeMsg : public SfpchangeatttypeMsg
{
	BYTE		atttype;			//�����л���ģʽ
};

//�����л�����ģʽ
struct SAfpchangeatttypeMsg : public SfpchangeatttypeMsg
{
	BYTE		atttype;			//�л����ģʽ
};



