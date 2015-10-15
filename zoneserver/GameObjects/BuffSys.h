#pragma once

#include "EventBase.h"
#include "networkmodule/playertypedef.h"
#include "BaseObject.h"
class CFightObject;

// Buff�����޸ĵ�����/����б�
enum BuffActionProperty
{
	// ��������
	BAP_MHP = 1,			// ��������
	BAP_MMP,				// ��������
	BAP_MTP,				// ��������
	BAP_GONGJI,				// ����
	BAP_FANGYU,				// ����
	BAP_BAOJI,				// ����
	BAP_SHANBI,				// ����
	BAP_SPEED,				// �ƶ��ٶ�	
	BAP_HIT,				//����
	BAP_ATKSPEED,			// �����ٶ�
	BAP_SORBDAM,			// �˺�����ֵ
	BAP_SORBDAM_PER,		// �˺����հٷֱ�
	BAP_BACK_DAMAGE,		// �˺�����ֵ
	BAP_BACK_DAMAGE_PER,	// �˺������ٷֱ�
	BAP_EXTRA_EXP_PER,		// ���⾭��ٷֱ�
	BAP_UNCRIT, //����
	BAP_WRECK,//�ƻ�
	BAP_UNWRECK,	//����
	BAP_PUNCTURE,	//����
	BAP_UNPUNCTURE,	//����
	BAP_ATTRI_MAX,

	// ���������ԣ���Ϊ�����壬���Բ��÷�����룬�����ڿ�������֮��
	BAP_CHP,				// ��ǰ����
	BAP_CMP,				// ��ǰ����
	BAP_CTP,				// ��ǰ����

	//********ս��״̬*********/
	BAP_DIZZY = 1000,		// ����(���ᱻ����)
	BAP_LIMIT_SKILL,		// ����
	BAP_HYPNOTISM,			// ����(Ŀǰͬ����)
	BAP_DINGSHENG ,			// ����
	BAP_WUDI,				// �޵�
	BAP_ADDBUFF,				// �޷������κ�BUFF
	BAP_JITUI,				// ������
	BAP_STATE_MAX,

	//********���￹��*********/
//	BAP_RESIST_CHP = 2001,	// ����Ѫ
//	BAP_RESIST_SPEED,		// ������
//	BAP_RESIST_DIZZY,		// ��ѣ��
//	BAP_RESIST_LIMIT_SKILL,	// ������
//	BAP_RESIST_HYPNOTISM,	// ������
//	BAP_RESIST_POISION,		// ���ж�
	BAP_RESIST_SUCKHP = 2001,	// ����Ѫok
	BAP_RESIST_SUCKMP,			// ������OK
	BAP_RESIST_SUCKTP,			// ��������OK
	BAP_RESIST_DEDUCTHP,		// ����ѪOK
	BAP_RESIST_DEDECTMP,		// ������OK
	BAP_RESIST_DEDUCTTP,		// ��������OK
	BAP_RESIST_JITUI,			// ������ok
	BAP_RESIST_DINGSHEN,		// ������ok
	BAP_RESIST_DEDUCTSPEED,		// ������OK
	BAP_RESIST_MAX,

	//********����Buff*********/
	BAP_SP_SUCKHP = 3001,	// ������
	BAP_SP_SUCKMP,			// ������
	BAP_SP_SUCKTP,			// ������
	BAP_SP_DEDUCTHP,		// ������OK
	BAP_SP_DEDUCTMP,		// ������OK
	BAP_SP_DEDUCTTP,		// ������OK

	BAP_SP_MAX,				

	BAP_NULL = 0xffff,		// ʲô�����޸�
};

// Buff�¼��Ĵ�����
enum BuffActionTriggerSpot
{
	BATS_ADD = 0,		// ���ӵ��������ʱ����
	BATS_REMOVE,		// ����������Ƴ�ʱ����
	BATS_CANCEL,		// ����ϵ�ʱ�򴥷�
	BATS_INTERVAL,		// �������

	BATS_MAX,			// Max
};

// Buff�ı�ֵ��ʽ
enum BuffActionMode
{
	BAM_POT = 0,		// �ı�ֵ
	BAM_PRECENT,		// �ı�ٷֱ�
	BAM_MAX,			// Max
};

// Buff�¼�Ԫ
class BuffActionEx
{
public:
	WORD	m_TriggetSpot;	// ����ʱ��
	int		m_Value;		// ֵ
};


// Buff����
class BuffModify : public EventBase
{
	friend class SBuffBaseData;
	friend class BuffManager;

	typedef std::vector<BuffActionEx*> BuffActionVector;

public:
	BuffModify(CFightObject *owner, CFightObject *pusher) : m_ID(0), m_GroupID(0), m_Weight(0), m_InturptRate(0), m_Margin(0), m_WillActTimes(0), 
		m_ActedTimes(0), m_DeleteType(0), m_SaveType(0), m_curStep(0), m_CanDropBuff(false), m_Type(0), m_CanBeReplaced(false), m_ScriptID(0),
		m_PropertyID(0), m_ActionType(BAM_MAX)
	{
		m_owner		= owner;
		m_pusher	= pusher;
	}

	//��ȡ������buff�¼������ݣ���Ҫ�������߱������߻ָ�
	void GetSaveBuffData(TSaveBuffData *pdat_t) const;
	void SetSaveBuffData(const TSaveBuffData *pdat_t);
private:
	virtual void OnCancel();				// Buff�ж�
	virtual void OnActive();				// Buff����
	virtual void SelfDestory();				// ��������

	void SendNotify(bool init);				// ����֪ͨ
	void OnTimer(int step);					// ��ʱ����
	void OnTimerActived(int step);      //buff��ʱ����
	bool InitBuffData(const SBuffBaseData *pData);
	bool ModifyProperty(WORD ProID, WORD Mode, int val, int *ret, bool isEnd);
	bool CreateReginBuff();

	void CheckAddList(check_list<LPIObject> *objectList, std::list<CFightObject *> *destList);
	bool IsCanDamageByBuff(CFightObject* desObject);//Buff����
public:
	CFightObject	*m_owner;				// Buffӵ����
	CFightObject	*m_pusher;				// Buff�ͷ���

	DWORD			m_ID;					// ���
	DWORD			m_DeleteType;			// ɾ������
	DWORD			m_SaveType;				// ��������
	DWORD			m_ScriptID;				// �ű�ID
	DWORD			m_Margin;				// ����ʱ����
	int				m_curStep;				// ��ǰ�Ľ׶�
	WORD			m_WillActTimes;			// Ҫ����Ĵ�����0��ʾ���ã�1��ʾ˲����2��ʾ������2���ϱ�ʾ�����
	WORD			m_ActedTimes;			// Buff�Ѿ�����Ĵ���
	WORD			m_GroupID;				// Buff����
	WORD			m_Weight;				// BuffȨ��
	WORD			m_InturptRate;			// ��ϼ���
	BYTE			m_Type;					// ����/����
	bool			m_CanBeReplaced;		// �ܷ��滻
	bool			m_CanDropBuff;			// �ɷ�ɾ��

	WORD			m_PropertyID;			// ����ID
	WORD			m_ActionType;			// �޸�ģʽ
	BuffActionEx	m_BuffAction[BATS_MAX];	// Buff����
	WORD		m_ReginBuffType;//����Buff����0 ����Ϊԭ�� 1 Ŀ��Ϊԭ��
	WORD		m_ReginBuffShape;//������״ 0 Բ�� 1����
	WORD		m_ReginBuffHeight;//���򳤶ȣ����ΪԲ�ξ�����Ϊ����뾶��
	WORD		m_ReginBuffWidth;//�����ȣ����ΪԲ�ν���ʹ�����������
	float			 m_fCenterPosX;
	float			m_fCenterPosY;
	
};