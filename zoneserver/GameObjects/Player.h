#pragma once

#include <stack>
#include <hash_map>
#include <set>
#include "ItemUser.h"
#include "WareHouse.h"
#include "TreasureShop.h"
#include "ExchangeGoods.h"
#include "count.h"
#include "stall.h"
#include "relation.h"
#include "DTelergyData.h"
#include "VenapointData.h"
#include "Monster.h"
#include "scriptmanager.h"
#include "networkmodule\tongmsgs.h"
#include "networkmodule/playertypedef.h"
#include "networkmodule\itemmsgs.h"
#include "pub/ConstValue.h"
#include "Networkmodule\GMMsgs.h"
#include "networkmodule\ScriptMsgs.h"
#include "networkmodule/MountsMsgs.h"
#include "networkmodule\logmsgs.h"
#include "networkmodule\BiguanMsgs.h"
#include "Networkmodule\DataMsgs.h"
#include "NETWORKMODULE\MoveMsgs.h"
#include "BuffJump.h"
#include "BuffUpSpeed.h"
#include "BuffAddAgile.h"
#include "BuffReduceCD.h"
#include "BuffReduceDelay.h"
#include "BuffAbsoluteParry.h"
#include "BuffOutDefense.h"
#include "BuffInnerDefense.h"
#include "BuffCounterAttackRate.h"
#include "BuffReduceAbsoluteHurt.h"
#include "BuffReduceHurtRate.h"
#include "BuffUpHPLimit.h"
#include "CItemService.h"
#include "CMountDefine.h"
#include "CPlayerDefine.h"
#include "CJingMaiDefine.h"
#include "Item.h"
#include ".\EventHandle\BatchReadyEvent.h"
#include "PlayerData.h"
#include "Transformers.h"
#include "SanguoCode\SanguoPlayerBase.h"
#include <time.h>
#include <list>
#include <bitset>
#include "CGloryATitleBaseData.h"

#include "Networkmodule\ShopMsg.h"
#include "Networkmodule\VipMsg.h"
#include "Networkmodule\PetMsg.h"
#include "Networkmodule\SanguoPlayerMsg.h"
//#include "ranklist_singleton.h"
class CPlayerData;
//using namespace std;

#define MAX_LEVEL_DIST	30

const __int32	IID_PLAYER = 0x110ef9d3; 

const   int     MAX_MONSTER_ONGOAT  = 81;      // 81���ֽ�����
const   int     AUTO_ADDMC_ONGOAT   = 72;      // 1.2�����Զ����� 1%
const   int     MAX_SKILL_STATE     = 6;       // �书��󾳽�

const DWORD DEFAULT_HP_RES_INTERVAL = 10 * 1000;
const DWORD DEFAULT_MP_RES_INTERVAL = 10 * 1000;
const DWORD DEFAULT_SP_RES_INTERVAL = 10 * 1000;
const BYTE	MAX_FIGHTPETACTIVED = 4 ;// ���͸�����󼤻����
#define MOUNT_HUNGER_TIME (1000 * 10)
#define CHECKTIMELIMIT_TIME (1000 * 10)
#define PLAYER_SP_RESTIME 20
#define PLAYER_SP_RESVALUE 4		///�����ָ�ֵ
#define PLAYER_PKVALUERISE 100  //ɱ������ֵ

typedef multimap<DWORD, Rank4Client> RankMap;

struct SQAutoUseItemMsg;

#include "sfuncmanager.h"

struct ClickLimitChecker
{
	// ����&�������������
	// ˵�������ÿ5���ڣ����������յ���ָ�������Ŀǰ�����ƶ�������������Ϣ������15�Σ�
	// �������[�򲻶϶���ȡ������ָ��]
	// �����ڼ���1������෢3��ָ��Ļ�����
	DWORD clickLimitCheckTime;
	DWORD clickCount;

	ClickLimitChecker() : clickLimitCheckTime( timeGetTime() ), clickCount( 0 ) {};

	BOOL isValid();
	void Increment() { clickCount++; }
	void Update();
};

class CPlayer : public CSanguoPlayer, public CWareHouse, public CExchangeGoods, public CStall, public CRelations, public CPlayerData
{
public:
	static TreasureShop tsSingle;
public:
	friend class ReadyUseItemEvent;
public:
	IObject *VDC(const __int32 IID) 
	{ 
		if ((IID == IID_ACTIVEOBJECT) || (IID == IID_FIGHTOBJECT) || (IID == IID_SANGUOPLAYER) )
			return this; 
		return NULL; 
	}

	CPlayer(void);
	virtual ~CPlayer();

public:
	void OnClose();

public:
	DWORD		m_BaseMaxHp;		// �����������
	DWORD		m_BaseMaxMp;		// �����������
	DWORD		m_BaseMaxTp;		// �����������
	WORD		m_BaseAtk;			// ��������
	WORD		m_BaseDefence;		// ��������
	WORD		m_BaseCtrAtk;		// ��������
	WORD		m_BaseEscape;		// �������
	BYTE		m_nPkRule;			// PKģʽ
	DWORD		m_nChagePkRuleTime;	// �л�PKģʽʱ��
protected:
	int OnCreate(_W64 long pParameter);

public:
	void SendData(BOOL isSave);					// ���ݱ��棨������½��������
	void Backup( SFixProperty &data );			// �������ݵ�data��
	void Logout(bool SaveAndLogOut = false);	// ����˳�
	bool CheckDisWait();						// ��Ҷ��ߵȴ�����ʱ����
	void OnReconnect();							// �������
	void OnDisconnect();						// ��Ҷ���
	void ClearLogoutStatus();					// �˳�����״̬
	void ClearDisConnectStatus();				// ��������״̬
	void SaveLogoutBuffList();					// �˳���Ϸʱ����Buff�б�

	void OnClickSomebody(LPIObject pObj);		// ��ҵ��һ������

	

	void ResetState();							// ת������ʱ�������״̬

	void MyTitleChanged(char* pNewTitle);
	
	void ClearPlayerStatus(bool IsCommon);

	void OnDead( CFightObject *PKiller );		// ��������ʱ��Ļص�
	bool isDead(){ return m_Property.m_CurHp <= 0; }
	void OnRegionChanged(CRegion *newReigon, bool Before);
	void RecoverPlayer(CPlayer *pPlayer); //�ָ�����
	void SaveData();							// �����������
public:
	BOOL CheckAction(int eCA);					// ��⶯��ִ�е�����
	LPCSTR GetName() { return m_FixData.m_Name; }
	LPCSTR GetAccount() { return m_szAccount.c_str(); }
	LPCSTR GetNotPrefixAccount() { return m_szAccount_without_prefix.c_str(); }
	LPCSTR GetAccountPrefix( int lv = 0 ) { return lv ? m_szAccount_lv2_prefix.c_str() : m_szAccount_lv1_prefix.c_str(); }
	DNID	GetMac();
	BYTE	GetSex() { return m_Property.m_Sex ? 1 : 2; }
	// -----------------------------------
	// ActiveObject & FightObject �麯��
private:
	DWORD m_checkTick;

	void CheckOnRunTime();		// �������ÿ֡���
	void OnRun();               // �̳л����������
	
	//void CheckRunOnTime(void);
	BOOL EndPrevAction();       // ��һ����������ʱ�Ļص�
	BOOL DoCurAction();         // ִ�е�ǰ�Ķ���
	BOOL CheckTrapTrigger();

private:
	void FillSelectMsg(struct SASelPlayerMsg *pMsg);	// ��������Ϣ
public :
	void OnEveryDayManagerRun(int nIndex);

	void AddValueByVenaDataEx( LPSVenapointDataEx pVenaData );
	bool IsPassVenationEx( BYTE byVenaNum, BYTE byPointNum );
	int CheckPlusPoints();
	// -----------------------------------
	// ��Ϣ������
public:
	void OnRecvMoveMsg(struct SQSynPathMsg *pMsg);
	void OnRecvSynPosMsg(struct SQSynPosMsg *pMsg);
	void OnRecvSynZMsg(struct SQSynZMsg *pMsg);
	void OnRecvJumpMsg(struct SAQSynJumpMsg *pMsg);
	void OnRecvUnity3DJumpMsg(struct SAQUnity3DJumpMsg *pMsg);

	void OnJumpMsg(struct SQASynWayJumpMsg *pMsg);
	void OnMoveMsg(struct SQSynWayTrackMsg *pMsg);
	struct SASynPlayerMsg *GetStateMsg();

	void OnRecvLevelUp(struct SQLevelChangeMsg *msg);		// ���յ����������Ϣ
	void OnRecvSetPoint(struct SQSetPointMsg *pMsg);        // ���յ���ҷ������
	void OnRecvResetPoint(struct SQResetPointMsg* pMsg);	// ���յ����ϴ����Ϣ
	void OnRecvClearPoint(struct SQResetPointMsg* pMsg);	// ���յ����ϴ����Ϣ

	void OnReceiveClickMsg(struct SQClickMsg);	            // ���յ���ҵ��ĳ�������Ϣ��������Ա�����Ϊ������Ϊ
	void OnRecvEquipColor(struct SQAEquipColorMsg *pMsg);   // ���յ���Ҹı�װ����ɫ
	void OnRecvZazenMsg( struct SQSetZazenMsg *pMsg );      // ���յ�������ô���״̬
	void OnWifeRemitMsg();                                  // ���޴�����Ϣ����
	void OnRevDoctorAddBuffMsg( struct SQDoctorAddBuffMsg* pMsg, CPlayer* pDoctor );  // ���յ�ҽ����ҽ��BUFF����Ϣ

	void FightOnSomebody(struct SFightAction *pMsg, LPIObject DestObject);	// ����ҹ���ĳ����

	void OnTaskMsg(struct SRoleTaskBaseMsg *pMsg);
	void StoreClickMsg(struct SQClickObjectMsg *pMsg);
	bool OnUpdateTask(struct SRoleTaskBaseMsg *pMsg);//������������״̬

	void SetAccount(const char * szSetAccount);
	BOOL SetFixProperty(SFixProperty *pData);
	BOOL CPlayer::SetFixData(SFixData *pData); //���������������
	BOOL UpdateProperty();

	BOOL GetTempData(SPlayerTempData *pData);       // ������ʱ����
	BOOL SetTempData(SPlayerTempData *pData);       // ������ʱ����

	BOOL AddPKiller(CPlayer *pPKiller);
	BOOL IsPKiller(CPlayer *pPKiller);
	BOOL CheckPKiller(CPlayer *pDestPlayer,BOOL );

	void UpdatePkMap();

	BOOL AddPlayerPoint(WORD type, WORD point);
	BOOL ResetPlayerPoint(BYTE type, WORD point);

	// ����ȡֵ�ӿ�
	BYTE	GetAmuck(void);
	short	GetXValue(void);
	BYTE    GetPKValue(void);

	// ��ȡ���װ���ϸ��ӵ�����
	WORD    GetEquipDAM(void);
	WORD    GetEquipPOW(void);
	WORD    GetEquipDEF(void);
	WORD    GetEquipAGI(void);

	// ��ȡѪ���������ٶ�
	WORD    GetAddHPSpeed(void);
	WORD    GetAddMPSpeed(void);
	WORD    GetAddSPSpeed(void);

	bool    SetMutate( BYTE byMutateType, WORD	wMutateID );        // ���ñ���״̬
	void    SetScapegoat( WORD wScapegoatID, WORD wGoatEffID );     // ��������״̬

	void UpdateEquipWear(int pos, int value, bool IfMaxWear = false);			// �ı�װ�����;ö�

	// ����һ�������ķ�
	bool ReduceTelergyVal( BYTE byTelergyID, DWORD dwUseVal, BOOL bIsReduce, LPCSTR info );   
	// �����ķ���ѧ���ķ���
	BOOL ActivaTelergy( WORD wTelergyID, LPCSTR info  = NULL);
	// �����Ṧ��ѧ���Ṧ��
	BOOL ActiveFly(WORD wFlyID, LPCSTR info, bool bForce = false);
	// �����ķ���ѧ�Ụ�壩
	BOOL ActiveProtected(WORD wProtected, LPCSTR info , bool bForce = false );
	// ɾ��װ�����ķ�(��װ����λ��0-5)
	BOOL DeleteTeletgy( BYTE byTelergyNum );      
	// ��ȡ��ǰ�����ķ�����
	int GetTelergyLevel( void );   
	// ��ȡ�ķ�����
	int GetTelergyLevel( WORD wTelergyID );   
	// �����ķ�
	BOOL AddTelergy( int iAddVal, LPCSTR info = NULL);
	// �����Ṧ 
	BOOL AddFly( int id, int iAddVal, LPCSTR info = NULL );   
	// ��������
	BOOL AddProtected( int id, int iAddVal, LPCSTR info=  NULL );   
	// �����ķ�������
	BOOL SetTelergyProfic( WORD wTelergyID, DWORD dwProfic );   
	// �����ķ�������
	BOOL SetTelergyProficM( WORD wTelergyID, DWORD dwProfic, LPCSTR info );   
	// ���õ�ǰ�������ķ�
	BOOL SetCurTelergyNum( BYTE byCurTelergyNum );       

	bool GetTelergyLevelEx(int index,int* level);
	// ����ID��õ�ǰ�ķ��ĵȼ� --anhuajie
	WORD GetTelerLevelByIndex(WORD index);

	// �ķ��лָ���ҵĹ�����������
	void ListenProperty();

	//�����Ҫ�����BUFF����
	void SetBuffSaveData();
	
	//���һ��弼��
//	SSkill* GetProtectSkillById( int id, EProtectedSkillType& type );
	//�����Ṧ����
//	SSkill* GetFlySkillById( int id, EFlySKillType& type );

	// ���õ�ǰ�����书
	BOOL SetCurSelSkills( WORD cur );	
	// ��ȡ�ķ�����ϵ��
	int GetTelergyCoef( BOOL bIsAttack );         
	// �����ķ����ͼ��㵱ǰװ���ķ�����Ӧֵ
	int GetTelergyCalculate( E_TOTAL_TELERGY iDataType );    

	// �����ķ����ͼ��㵱ǰװ���ķ�����Ӧֵ
	int GetTelergyEffect( E_TOTAL_TELERGY iDataType );          
	void CalculateTelergyEffects();

	// �������
	BOOL    DamageToVenation( CPlayer *pAttacker, int iDamage, BOOL bIsDead,
		BYTE &byWhoVenation, BYTE &byVenationState );// �����˺�ֵ���㾭�����ܵ����˺�
	BOOL    DamageToVenation( CPlayer *pAttacker, BYTE &byWhoVenation, BYTE &byVenationState, DWORD extraPos );

	BOOL    BeCureVenation( CPlayer *pCurer );                      // �����ƾ�������
	BOOL    CureSomebodyVenation( DWORD dwDestGID );                // ����ĳ�˾�������
	void    UpdateCureVenation( void );                             // ÿ��20�θ������ƾ�����ʱ��

	// �����ķ��ı���Ϣ����
	void	SendCurTelChange(SXYD3Telergy *pTempTel, BYTE pos, bool bAddNewTel);
	
	/******************************************* 
	�������� 
	*/
	void SendUpSpeed();

	// �����ӳɵ���������
	int m_VenaAddFactor[SVenapointData::VENA_PROPERTY_MAX];
	bool InitVenaAddFactors();
	int GetVenaAddFactor(SVenapointData::VENA_PROP_DEF type);

	// �ж�һ�������Ƿ��Ѿ���ͨ
	bool IsPassVenation( BYTE byVenaNum );									
	// ����Ѩ��������������ֵ
	void AddValueByVenaData( LPSVenapointData pVenaData );												
	// ȡ���Ѿ���ͨ�ľ�������
	int  GetPassVenationCount( BYTE byCount );
	int  m_iPassVenationCount;
	// �򿪾���
	bool SetPassVenapoint( BYTE byVenaNum, BYTE byPointNum );    // ����һ��Ѩ������ͨ
	bool OpenVena( LPSVenapointData pVenaData, BYTE byVenaNum, BYTE byPointNum );		
	bool GmOpenVena( BYTE byVenaNum, BYTE byPointNum );		
	// �ķ��ӳɵ���������
	INT32 m_TelergyAddFactor[ TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE ];
	bool InitAddTelergyAddFactorTbl();
	INT32 GetTelergyFactorByIndex(TelergyDataXiaYiShiJie::TELE_PROP_DEF index);

	// �����ķ��ȼ���������õĵȼ��ȵ�ǰ�ĵͣ���ô��ֵ�����ȥ��
	bool SetTelergyLevel(int index,int level);

	// ����ͻ�������ӵ����Ϣ
	void OnRecvQuestAddPoints(struct SQuestAddPoint_C2S_MsgBody* pMsg);

	// װ�����Լӳɣ���ֵ�ͼӰٷֱȣ�
	int m_EquipFactorValue[SEquipDataEx::EEA_MAX];

	// ������Լӳ�
	WORD m_MountAddFactor[SPlayerMounts::SPM_MAX];

	// �������Լӳ�
	WORD m_JingMaiFactor[JMP_MAX];
	//�������Լӳ�
	WORD m_XinYangFactor[XYP_MAX];

	//add by ly 2014/2/25 �ƺ����Լӳ�
	WORD m_TitleFactor[TitleAddType::TITLE_AMAX];

	// �ķ��������Լӳɣ�ֵ��ٷֱȣ�
	WORD m_TelergyFactor[TET_MAX];
	// �ķ�������������
	WORD m_TelergyActiveRate[TET_MAX - TET_SUCKHP_PER];

	// 1 װ�� 2 ���� 3 жװ 4 ����
	void ChangeMountFactor(SPlayerMounts::Mounts *pMount, WORD type);
	WORD GetMountFactor(WORD index);

	void InitEquipJewelData(int EquipIndex);
	void InitEquipAttribute(const SItemFactorData* pItemFactorData);
	void InitEquipAttriData(int EquipIndex);
	void InitEquipmentData();	
	BOOL InitEquipSuitAttributes_New(); // ������װ������
	BOOL InitEquipSuitAttributes();		// װ����װ�����Խӿ�
	BOOL MinusAllSuitAttibutes();		// ��ȥ������װ���ԣ�����װ����ȫ�������
	BOOL ModifySuitAttributes(WORD wMark);		// �޸���װ��������

	// ------test----------
	BOOL ModifySuitAttriValue(WORD wMark);		// �޸�����ֵ
	// ---------------------

// 	//���͵�װ�����Ա仯
// 	void _fpInitEquipJewelData(int EquipIndex);
// 	void _fpInitEquipAttribute(const SItemFactorData* pItemFactorData);
// 	void _fpInitEquipRandAttriData(int EquipIndex);
// 	void _fpInitEquipmentData();
// 	///������Ҫװ�����ߵ���������
// 	BOOL _fpSetEquipFightPet(BYTE index);
// 	///�����Ҫװ�����ߵ���������
// 	void _fpInitEquipFightPetIndex(){m_nEquipfpIndex = -1;}
// 	int	_fpgetEquipFightPetIndex(){return m_nEquipfpIndex;}
// 	///�������Ը���
// 	void _fpUpdateAllProperties();

	BOOL RecvCloseGroundItem();
	BOOL RecvCheckGroundItem(SQCheckGroundItemMsg *pMsg);
	BOOL RecvStartTakeChance(SQGroundItemChanceBack *pMsg);

	//��½ʱ���������������BUFF
	void InitBeginBuffData();	

	//��������һ����ͼ����ӵ�BUFF
	void DropRegionBuffList(WORD regionId);

	//�����ķ�״̬
	bool SetTelergyState(SQSetTelergyStateMsg *pMsg);

	// �������Լӳ�
	virtual void UpdateAllProperties();
	void UpdateBaseProperties();
	void UpdataPlayerCounterpartProperties();//������Ҹ�������
	long FindpartData(short Index);
	// ���跢�͸��º������
	virtual void SendPropertiesUpdate();
	virtual void SwitchFightState(bool IsIn);

	// ��һ�λظ�HP,MP,SP��ʱ��
	DWORD m_dwLastHPResTime;
	DWORD m_dwLastMPResTime;
	DWORD m_dwLastTPResTime;
	DWORD m_HpRSInterval;
	DWORD m_MpRSInterval;
	DWORD m_TpRSInterval;
	DWORD m_HpRecoverSpeed;
	DWORD m_MpRecoverSpeed;
	DWORD m_TpRecoverSpeed;

	///��һ�λظ�������ʱ��
	DWORD m_dwLastSPResTime;
	/// �������ߺ��ѵĶ��ٶ�ʱ�;���,��¼ʱ��
	DWORD m_nOnlineFriendTime;
	/// �������ߺ��ѵĶ��ٶ�ʱ�;���,��¼����
	DWORD m_nOnlineFriendExp;
	/// �������ߺ��ѵĶ��ٶ�ʱ�;���,��¼�ۼƵľ������
	WORD m_nOnlineExpCount;
	/// �������ߺ��ѵĶ��ٶ�ʱ�;���,��¼���һ�����ӵľ���������֤
	DWORD m_nOnlineFriendLastExp;

	///����װ�����ߵ���������
	int m_nEquipfpIndex;

	/// �������ߺ��ѵĶ��ٶ�ʱ�;���
	void  OnCheckFriendNum(DWORD dwCurTick);

	// �����Զ��ָ�
	void __OnCheckAutoRestore(DWORD dwCurTick);
	bool __OnCheckHPAutoRestore(DWORD dwCurTick);
	bool __OnCheckMPAutoRestore(DWORD dwCurTick);
	bool __OnCheckSPAutoRestore(DWORD dwCurTick);///�����ָ�
	bool __OnCheckTPAutoRestore(DWORD dwCurTick);

	bool AddSkillProficiency(WORD wSkillID, int iAddVal, LPCSTR info = NULL);    // �����书������

// 	DWORD GetSkillProficiency( DWORD dSkillID );                  // ��ȡ�书������
// 	bool AddSkillProficiency(DWORD dSkillID,  int iAddVal);// �����书������

	// װ���;öȼ���
	void OnWeaponWearReduce();
	void OnEquipWearReduce( WORD point);
	int __OnEquipWearReduce( EQUIP_POSITION position, WORD point);		// ����1��ʾ�;öȱ仯������2��ʾ�;öȱ仯����Ϊ0��ʧЧ��
	void __SendEquipWearChangeMsg(EQUIP_POSITION position);

	/*****************************************************/

	// ��������أ�û�л������ˣ�
	double  GetSkillCoef( BYTE bySkillID, BOOL bIsAttack );         // ��ȡ�书ϵ��
	BOOL    CheckUpdataAttackLevel( BYTE bySkillID, BYTE byLevel ); // ����书��������

	void    SendTelergyChange( BOOL extraOnly = FALSE );            // �����ķ����ݵĸı�
	void    SendUpdataSkill( BYTE bySkillPosID );                   // ������ʽ������Ϣ
	void    SendTemplateSkill(WORD index,WORD Flag);
	void    SendVenationState( BYTE byWhichvena, BYTE byState, BYTE byIsUpdate=1 );    // �����ķ�״̬���������ˡ�����
	void    SendOnCureing(DWORD dwGID);                             // �����ķ����Ƶ�ȷ����Ϣ�����ܰ������ƶ����������ǹ㲥
	void    SendExtraState(void);                                   // ���͸���״̬����̯=0x01��û�ã������ƾ���=0x02�������ƾ���=0x04

	void    SendPassVenapoint( BYTE byVenaNum, BYTE bypointNum );   // ���ʹ�ͨѨ��ȷ����Ϣ
	void    SendChangeMutate( BYTE byMutateType, WORD	wMutateID );// ���ͱ�����Ϣ
	void    SendScapegoat( WORD	wScapegoatID, WORD wGoatEffID );    // ����������Ϣ
	void    SendOnGoatMonsterCount( WORD wCount, WORD wMutateTime );	// ��������״̬�������
	void    SendRefreshOnUp(void);										// ���ߺ��������ݸ���

	void    OnRecvPassVenapointAsk( BYTE byVenaNum, BYTE bypointNum );	// ���ܴ�ͨѨ��������Ϣ

	void	OnRecvUpdateTelegry(SQTelergyUpdate *TelMsg);				// ���Ҫ�����ķ�
	void	OnRecvUpdateSkill(SQSkillUpdate *SkillMsg);					// ���Ҫ��������
	void UpdateSingleSkill(SQSkillUpdate *SkillMsg);	//����������������
	void UpdateGroupSkill(SQSkillUpdate *SkillMsg);		//��������һ�鼼��

public:
	BOOL	IsMoneyEnough(DWORD type, DWORD value);
	DWORD	AddPlayerMoney(DWORD type, DWORD value);
	BOOL	CheckPlayerMoney(DWORD type, DWORD value, bool OnlyCheck);
	WORD	GetPlayerReginID();
public:
	DWORD	AddPlayerSp(DWORD value);
	BOOL	CheckPlayerSp(DWORD value, bool OnlyCheck);
	bool	CheckPkRuleAttk(DWORD gid,BYTE AtkType=0);	// ���PKģʽ�ܷ񹥻����ж��Ƿ��ڳ���б���
	
	void	SendPlayerPkValue();						// ������ҵ�ɱ��ֵ
	void	SendPlayerNameColor();						// ���͸ı���ҵ�������ɫ��Ϣ
	BYTE	GetPlayerNameColor();						// ɱ��ֵ�õ����������ɫ
	void	OnCheckNameColor();							// ����ʱ��������ɱ��ֵ
	bool    OnCheckNameColorByVen();                    //���ݾ�����ͨ��Ŀ���ж�������ɫ
	DWORD	CalculatePkValueDecrease();					// ÿ1��ɱ��ֵ���ٵ�ʱ��
	void	CheckPkPlayerName(CFightObject *fightobj);	// ����Ƿ�Ӧ�û���
	void	OnDropItem();								// ����������Ʒ
	int		GetRandomEquipmentIndex();					// �õ�װ�������ϵ�һ�����װ��
	int		GetEquipmentNum();							// �õ�װ�������ϵ�װ������
	int		RandomDropItemToGroundBag(std::vector<SRawItemBuffer> &itembag,BYTE dropType);//�����������װ��������Ʒ,0װ��1��Ʒ
	void	OnChangePKValue(int pvalue);				// �ı���ҵ�ɱ��ֵ
	// һ��������������ڴ��������趨
public:
	ITEMDROPIDTABLE m_vTaskDropIdTable;					// ������Ҫ������ƷID��
public:
	BOOL UpdatePlayerProperty(SFixBaseData * pFixBaseData);     // GM ָ������������

	/************************************************************************/
	/*              OnRun�����Ĺ����Ӻ���                            */
	/************************************************************************/
	void UpdateUseItemTime();
	bool CheckAndUpdateOnlineState();
	bool CheckUseRose();
	void SpecialProcessDeadState();
	void TimeToRelive(WORD regionID, WORD wX, WORD wY, WORD type);
	void Relive();
	void SendTestMsg();
	void CheckStatisticsFlag();
	void ProcessAutoUseItem();
	void QuestToRelive(BYTE type);

public:
	// network
	DNID m_ClientIndex;
	BOOL SendMsg( void *msg, size_t size );
	void SetPUID( QWORD puid ) { m_Property.puid = puid; }

public:
	struct SParameter
	{
		DNID    dnidClient;     // ���ӱ��
		DWORD   dwVersion;      // �������
	};

public:
	struct SFluxProperty : public SFixProperty
	{
		SFluxProperty() {}
		
		QWORD puid;
	};

	SFluxProperty	m_Property;			// ��һ�������
	BYTE			m_aClickParam[12];	// ����Ĳ���
	SFixData m_FixData; //������ҳ�Ա����
	std::map<DWORD , DWORD> m_StroageIndex; //�ֿ���������
public:
	// �����3������Ը���
	const SBaseAttribute *m_PlayerAttri;				// ��������
	DWORD m_PlayerFightPower;	//���ս����
	void	*m_PlayerAttriRefence[XA_MAX-XA_MAX_EXP];	// �����Ż�
	bool	m_PlayerPropertyStatus[XA_MAX-XA_MAX_EXP];
	bool	m_IsPlayerUpdated;

	QWORD	m_MaxExp;		// ��ǰ�ȼ������ֵ
	DWORD	m_MaxJp;		// ��ǰ�ȼ������ֵ
	DWORD	m_MaxSp;		// ��ǰ�ȼ��������ֵ

	DWORD		m_dwTeamID;			// ����ID
	WORD		m_wTeamMemberCount;	// ���鵱ǰ����
	bool		m_bIsTeamLeader;	// �Ƿ�Ϊ�ӳ�

	WORD		m_wPKValue;				// PKֵ,ɱ��ֵ
	BYTE		m_bNameColor;			// ������ɫ
	DWORD		m_dChangeColorTime;		// �ı�������ɫ��ʱ
	BOOL IsTeamLeader();


	void ChangeTeamSkill(BYTE byNewTeamSkill,WORD wMemberCount);
	BOOL CheckCanTalk(DWORD type, DWORD time);

	LPCSTR GetTeamLeaderName(BYTE index); ///��ȡ�������������
	DWORD GetTeamLeaderRegionGID();
	DWORD GetTeamCurPartRegionGID();
// ********************************���а����**************************************
public:
	BYTE		m_RankNum[RT_MAX];				// ���а�ǰ����
	Rank4Client m_RankList[RT_MAX][MAX_RANKLIST_NUM];

    BOOL QuestDB2GetRankList(struct SQClientQuestRankFromDB *pMsg);		// ����DB��������
	BOOL GetRankListInfo(struct SQRankListMsg *pMsg);					// XYD3�鿴���а���Ϣ�ӿ�

// ==================================================================================

private:
	long	m_DeadWaitTime;	// �����ȴ�ʱ��
	DWORD	m_DeadTime;		// ����ʱ�䣬������֤

	WORD px, py;

	DWORD   m_dwOneSecondTick;		// 1��ļ��
	DWORD   m_dwSecondTick;			// 10��ļ��
	DWORD   m_dwMountTimer;			// 1�ֵļ��
	DWORD   m_dwMinuteTick;			// 1��һ�εĶ���
	DWORD   m_dwHellionBuffTick;	// 3��ļ��
	DWORD   m_dwDecDurTime;			// 5���Ӽ��

	BOOL    m_bCountFlag;			// ͳ�Ƶı��
	WORD    m_wScriptState;			// �ű�״ֵ̬
	WORD    m_wScriptIcon;			// �ű�ͼ����

	struct MultipleTime
	{
		BYTE m_byMultipleTime;		// ��ǰ˫��ʱ��
		BYTE m_byLeaveTime;			// ʣ��ʱ��
		DWORD m_dwLastTime;			// ���һ����ȡʱ�� 
		BYTE m_byCurWeekUseTime;	// ��ǰʹ��ʱ��
		BYTE m_fMultiple;           // ����
		BYTE m_reserve1;		
	};

	MultipleTime multipletime;

	DWORD   m_dwUseTelergyVal;  // ���γ�Ѩ�����ĵ��ķ�����

	WORD    m_SportsState;      // ����״̬
	DWORD   m_SportsRoomID;     // ����

public:
	DWORD   m_ProRefTime;      // ����������ʼʱ��
	bool    m_bProtect;       // ����״̬
	BYTE	m_DynamicRegionState;		// 0,1�Ƿ��ڶ�̬����
public:
	void EnableProtect(void);
	bool GetProtect(void) const { return m_bProtect; }

	_PLAYERDATA m_CountData;   //��Ҫ��ͳ�Ƶ�����

	bool IsOnSportsArea(void);
	void SetSportsRoomID(DWORD dwID) { m_SportsRoomID = dwID; }
	DWORD GetSportsRoomID(void) const  { return m_SportsRoomID; }
	void SetSportsState(WORD wState, WORD wLeader = 0) 
	{ 
		m_dwExtraState &=  0xFFFF0FFF;
		m_dwExtraState |= wState << 12;
		m_SportsState = wState;
		if(wLeader)
		{
			if(wState == 0)
				wLeader = 0;
			m_dwExtraState &=  0xFFF0FFFF;
			m_dwExtraState |= wLeader << 16;
		}
		SendExtraState();
	}
	WORD GetSportsState(void)   const {  return m_SportsState; }

	void SetCountFlag(BOOL bPrint);
	void SetScriptState(WORD wState, WORD wScriptIcon) 
	{
// 		m_wScriptState = wState;
// 		m_wScriptIcon = wScriptIcon;
// 
// 		if(m_wScriptState > 0) 
// 			m_dwExtraState |= SHOWEXTRASTATE_SCRIPT;
// 		else
// 			m_dwExtraState &= ~SHOWEXTRASTATE_SCRIPT;
	}

	inline void SendStatusMsg(WORD msgID)
	{
		SANotifyStatus msg;
		msg.wIndex = msgID;
		SendMsg(&msg, sizeof(SABackMsg));
	}

	WORD GetScriptState(void) const { return m_wScriptState; }
	WORD GetScriptIcon(void) const { return m_wScriptIcon; }

	// �౶�����顢��Ǯ���
	BYTE  GetMultipleTime(void) const { return multipletime.m_byMultipleTime; }
	WORD  GetLeaveTime(void) const {  return  multipletime.m_byLeaveTime; }
	BYTE  GetNMultiple(void) const { return multipletime.m_fMultiple; }
	DWORD  GetMulStartTime(void) const { return multipletime.m_dwLastTime; }
	BYTE  GetCurWeekUseTime( void ) const { return multipletime.m_byCurWeekUseTime; }

	void  SetMultipleTime(BYTE byTime) { multipletime.m_byMultipleTime = byTime; }
	void  SetMulStartTime( DWORD dwTime ) {multipletime.m_dwLastTime = dwTime; SendMultimeInfo(); }
	void  SetLeaveTime(BYTE byTime) { multipletime.m_byLeaveTime = byTime; }
	void  SetNMultiple(BYTE fMultiple) { multipletime.m_fMultiple = fMultiple; }
	void  AddCurWeekUseMulTime( BYTE bytime ) { multipletime.m_byCurWeekUseTime += bytime; }
	void  SetCurWeekUseMulTime( BYTE bytime ) { multipletime.m_byCurWeekUseTime = bytime; }

	void  SendMultimeInfo(void);
	void  SetPlayerConsumePoint(DWORD dwPoint);

	/*DWORD  GetPlayerConsumePoint() const { return m_Property.m_dwConsumePoint;}*/
	//---------------------------------------
	BOOL	m_bIsTalkGM; //Add By Lovelonely
	//---------------------------------------

	// ��Ϸ����
	BOOL    AddMoney(DWORD dwCoin, DWORD dwSilver, DWORD dwGold);
	BOOL    AddMoney(int MoneyType, DWORD dwValue);

	BOOL    DelMoney(DWORD dwCoin, DWORD dwSilver, DWORD dwGold);
	BOOL    DelMoney(int MoneyType, DWORD dwValue);

	void    UpdateMoneyToC();
	DWORD   GetMoney(BYTE type);

	void SetScoresState(DWORD dwScore, DWORD byWhich) 
	{
// 		if(dwScore == 0)
// 		{
// 			switch(byWhich)
// 			{
// 			case SHOWEXTRASTATE_SCORES_LEVEL:
// 				m_dwExtraState &= ~SHOWEXTRASTATE_SCORES_LEVEL;
// 				break;
// 			case SHOWEXTRASTATE_SCORES_MONEY:
// 				m_dwExtraState &= ~SHOWEXTRASTATE_SCORES_MONEY;
// 				break;
// 			case SHOWEXTRASTATE_SCORES_FAMEG:
// 				m_dwExtraState &= ~SHOWEXTRASTATE_SCORES_FAMEG;
// 				break;
// 			case SHOWEXTRASTATE_SCORES_FAMEX:
// 				m_dwExtraState &= ~SHOWEXTRASTATE_SCORES_FAMEX;
// 				break;
// 			case SHOWEXTRASTATE_SCORES_MEDICALETHICS:
// 				m_dwExtraState &= ~SHOWEXTRASTATE_SCORES_MEDICALETHICS;
// 				break;
// 
// 			default:
// 				break;
// 			}
// 		}
// 		else
// 		{
// 			m_dwExtraState |= dwScore;
// 		}
// 
// 		AddCheckID();
// 		SendExtraState();
	}

	void UpdateDoctorBuffTime( bool loginCheck = false ); // ����ҽ��BUFFʣ��ʱ��

protected:
	void UpdateMultipleTime(void);

	WORD	m_wMutateID;		    // �����ͼ�����
	WORD    m_wScapegoatID;         // �����ͼ����� =0 ������
	WORD    m_wGoatEffectID;        // �����Ч�����

	WORD    m_DropItemBaseRand; // ����������Ʒ�ı仯����

public:
	void CheckMulTime(void);

// 	bool	IsMonsterMutate(void)	{	return	(m_dwExtraState&(SHOWEXTRASTATE_ONMOSMUTATE|SHOWEXTRASTATE_ONGOATMUTATE|SHOWEXTRASTATE_ONESPMUTATE)) != 0;	}
// 	bool	IsGoatMutate(void)	    {	return	(m_dwExtraState&SHOWEXTRASTATE_ONGOATMUTATE) != 0;	}
// 	bool	IsEspMutate(void)	    {	return	(m_dwExtraState&SHOWEXTRASTATE_ONESPMUTATE) != 0;	}
// 	bool	IsNpcMutate(void)		{	return	(m_dwExtraState&SHOWEXTRASTATE_ONNPCMUTATE) != 0;	}
// 	bool	IsNewModel(void)		{	return	(m_dwExtraState&SHOWEXTRASTATE_NEWMODEL)	!= 0;	}
	WORD    GetScapegoat(void)      {   return  m_wScapegoatID;                                 }//(m_dwExtraState&SHOWEXTRASTATE_ONGOAT     );	}
	WORD    GetGoatEffect(void)     {   return  m_wGoatEffectID;                                 }//(m_dwExtraState&SHOWEXTRASTATE_ONGOAT     );	}
	WORD    GetMutateID(void)       {   return  m_wMutateID;                                    }

	BOOL	IsBetting();			//�Ƿ�������Ϸ����

	void    AddCheckID(void)        {   m_wCheckID++;                                           }

	// ����������Ʒ�仯���ʵĿ���
	WORD    GetDropItemBaseRand(void)   {   return  m_DropItemBaseRand;                         }
	void    InitDropItemBaseRand(void);
	void    RdcDropItemBaseRand(void);

	void    DoAddDoctorBuff( CPlayer* pDoctor, bool useItem );
	void    UpdateWhineBuffTime( bool loginCheck = false);
	BOOL    AddWhineBuff(WORD buffType);

	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);


	DWORD   GetSpouseSID(void);
private:
	// ���崥�����������
	WORD    m_PrevTrapID;

public:
	// !!!!!!!!!!
	std::map<DWORD, DWORD> m_PkMap;      // PK����ӳ���
	std::list< DWORD > m_LookMeList;
	DWORD              m_dwLookGID;
	DWORD              m_dwLookStartTime;
	DWORD              m_dwLookDisable;

	// ==== ��ʱ����==== 
	struct tempAddItem
	{
		DWORD dwItemCoolingTime; // ��ȴʱ��
		BYTE  itemBuf[68];
	};
	std::list<tempAddItem> m_tempItem;
	void OnClearTempItem();
	void UpdateTempItemToClient( BOOL nullUpdate = false );
	// =================

	bool m_bInit;
	long m_DisWaitTime;		// ���ߵȴ�ʱ�䣬frame
	
//	bool m_isLockedTarget;  // �Ƿ��ڹ�������״̬

	DWORD m_dwSaveTime;     // �������ݵ�ʱ��
	DWORD m_dwHalfHourTime; // ��Сʱһ�ε�ʱ��
	DWORD m_dwOneHourTime;

	// ################������ϵͳ################

	/*1������2010��8��1��ʵʩ�ġ�������Ϸ�������а취����������Ϸ�û���ʹ����Ч���֤������ʵ��ע�ᡣ

		2������δ���������Ľ�����δ��18����û����ܵ�������ϵͳ�����ƣ�
		��Ϸ���̣�����ʾ�����ۼ�����ʱ�䡣
		�ۼ���Ϸʱ�䳬��3Сʱ����Ϸ���棨���飬��Ǯ�����롣
		�ۼ���Ϸʱ�䳬��5Сʱ����Ϸ����Ϊ0��

	*/
	void NotifyTimeLimit( DWORD m_OnlineTime );
	void InitTimeLimit( DWORD limitedState, DWORD online, DWORD offline );
	void OnCheckTimeLimit(DWORD dwCurTick);
	void ShowLimitTips();//����������Ե���ҽ�����ʾ

	DWORD getShowStep      ()         { return m_bStrShowStep; } 
	void  putShowStep      ( DWORD v ) { m_bStrShowStep = (BYTE)v; }
	__declspec( property( get = getShowStep, put = putShowStep ) ) DWORD showStep;         // ��ʾ��ʾ�Ѿ����е��ĸ��׶���!

	DWORD m_OnlineTime;       // ����ʱ��!!!
	DWORD offlineTime;
	DWORD limitedState;
	DWORD	m_bStrShowStep;	
	int GetElapsedState()
	{ 
// 		if ( limitedState == 0 ) 
// 			return 0;
// 		if ( showStep < 3 ) 
// 			return 0; 
// 		else if ( showStep < 7 ) 
// 			return 1; 
// 		return 2; 
		return 0;
	}
	enum limitedType
	{
		LIMITED_NULL,			//δ���������
		LIMITED_ALREADY,	//�Ѽ��������
		LIMITED_HALF,			//�������
		LIMITED_ZERO,			//����Ϊ0
	};
	// ##########################################

	// �˺��������
	std::string m_szAccount;
	std::string m_szAccount_without_prefix;
	std::string m_szAccount_lv1_prefix;
	std::string m_szAccount_lv2_prefix;

	enum ChatType { CT_SLOW, CT_NORMAL, CT_FAST ,CT_GMVEERYSLOW};

	DWORD m_dwChatSkipTime[sizeof(ChatType)+1];   // ˢ�����ʱ��

	DWORD   m_dwExtraState;     // ��ҵ�һЩ����״̬����̯=0x01��û�ã������ƾ���=0x02�������ƾ���=0x04

	unsigned    int m_iCurerGID;
	DWORD   m_dwCureDAMTime;    // ������̫��ʱ�� ��ʼ��=0
	DWORD   m_dwCurePOWTime;    // ����������ʱ�� ��ʼ��=0
	DWORD   m_dwCureDEFTime;    // ������̫��ʱ�� ��ʼ��=0
	DWORD   m_dwCureAGITime;    // ����������ʱ�� ��ʼ��=0

	WORD    m_wMonsterCountOnGoat;  // ��װ����ɱ������ļ���
	WORD    m_wGoatMutateID;        // ��ǰ����������װ���(1-11)
	DWORD   m_dwAutoAddMCountTime;  // �Զ����ǣ�ÿ1.2������1%�Ľ��������ۼ�ʱ��
	WORD    m_wDoctorBuffTime;     // ҽ�¼Ӿ����ĳ���ʱ��

	WORD    m_wDoctorDefBuff;   // ҽ�߷�������BUFF
	WORD    m_wDoctorDamBuff;   // ҽ���⹦����BUFF
	WORD    m_wDoctorPowBuff;   // ҽ���ڹ�����BUFF
	WORD    m_wDoctorAgiBuff;   // ҽ��������BUFF

	BYTE    m_byDoctorDefBUFF;  // δ���õ�def BUFF
	BYTE    m_byDoctorDamBUFF;  // δ���õ�dam BUFF
	BYTE    m_byDoctorPowBUFF;  // δ���õ�pow BUFF
	BYTE    m_byDoctorAgiBUFF;  // δ���õ�agi BUFF

	DWORD   m_dwDoctorGID;      // ҽ��BUFFҽ����GID

	WORD    m_wCheckID;

	WORD    m_wWhineBUFFType;		// �Ҿ�BUFF����
	WORD    m_wWhineBUFFData[6];    // �Ҿ�����BUFF���� 0:�⹦ 1���ڹ� 2������ 3���� 4��Ѫ�� 5��ɱ��

public:
	BOOL SendMyState();
	BOOL LoginChecks();
	BOOL LogoutChecks();

public:
	typedef std::vector<WORD> MENUINFO;
	typedef std::list<std::pair<WORD, WORD> > TASKINFO;
	typedef std::list<SAddItemInfo> ITEMINFO;

	TASKINFO			m_TaskInfo;
	MENUINFO			m_MenuInfo;
	ITEMINFO			m_ItemInfo;
	CRoleTaskManager	m_TaskManager;

	// ������<----->������꣩
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKKILL;
	TASKKILL m_KillTask;

	// ������<----->������꣩
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKITEM;
	TASKITEM m_ItemTask;

	// ������<----->������꣩
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKUSEITEM;
	TASKUSEITEM m_UseItemTask;

	// ������<----->������꣩
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKGIVEITEM;
	TASKGIVEITEM m_GiveItemTask;

	// ������<----->������꣩
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKGETITEM;
	TASKGETITEM	m_GetItemTask;

	// ����ͼ<----->������꣩
	typedef std::hash_map<WORD, std::list<DWORD> > TASKMAP;
	TASKMAP m_MapTask;

	// ���ؿ�<----->������꣩
	typedef std::hash_map<WORD, std::list<DWORD> > TASKSCENE;
	TASKSCENE m_SceneTask;


	// ������<----->������꣩
	typedef std::hash_map<WORD, std::list<DWORD> > TASKADD;
	TASKADD m_AddTask;

public:
	void DeleteTaskItemMap(WORD taskID);									// ɾ��һ������������������ϱ��ӳ��
	void DeleteTaskFlagMap(const STaskFlag& flag);							// ɾ��һ���������ϱ��ӳ��
	void SendUpdateTaskFlagMsg(STaskFlag &flag, bool updateStatus = true);	// ���͸��������Ϣ
	int  GetTaskStatus(WORD TaskID);										// ��ȡ����״̬
	void OnTimeLimitTask();
	void DeleteCycData(WORD taskID,BYTE flag = 0);										// ���2����������ݶ�����ֻ��һ����������
	void UpDataCycData();													// ����ѭ������
	void ClearCycData(std::vector<long> indexVec,BYTE flag = 0);
	void SendCycData(WORD taskID);
	inline void ParaseTOstring(const std::string str , std::vector<long> & vec)
	{
		std::string temp = "";
		long len = str.size();
		for (size_t i = 0; i < str.size(); ++i)
		{
			char ch = str[i];
			if (ch != ':' )
			{
				temp.append(1,ch);
				if (i == len - 1)
				{
					vec.push_back(atol(temp.c_str()));

				}
			}
			else
			{
				vec.push_back(atol(temp.c_str()));
				temp = "";
			}
		}
	}
	inline int  Systime2String( const SYSTEMTIME& time ,std::string& strTime )
	{
		char timeStamp[30];
		sprintf(timeStamp,"%u-%02u-%02u -%02u- %02u:%02u:%02u"
			,time.wYear
			,time.wMonth
			,time.wDay
			,time.wDayOfWeek
			,time.wHour
			,time.wMinute
			,time.wSecond);
		strTime = timeStamp;

		return 0;
	}
	inline int SystemYearMd( const SYSTEMTIME& time ,std::string& strTime )
	{
		char timeStamp[30];
		sprintf(timeStamp,"%u-%02u-%02u"
			,time.wYear
			,time.wMonth
			,time.wDay);
		strTime = timeStamp;

		return 0;
	}
	inline int SystemHourMm( const SYSTEMTIME& time ,std::string& strTime )
	{
		char timeStamp[30];
		sprintf(timeStamp,"%02u:%02u:%02u"
			,time.wHour
			,time.wMinute
			,time.wSecond);
		strTime = timeStamp;

		return 0;
	}
	inline void GetSysHourMm( std::string& timeTick )
	{
		SYSTEMTIME sysTime;
		GetLocalTime( &sysTime );
		SystemHourMm( sysTime,timeTick );
	}
	inline void GetSysYearMd( std::string& timeTick )
	{
		SYSTEMTIME sysTime;
		GetLocalTime( &sysTime );
		SystemYearMd( sysTime,timeTick );
	}
	inline void GetSysTimeTick( std::string& timeTick )
	{
		SYSTEMTIME sysTime;
		GetLocalTime( &sysTime );
		Systime2String( sysTime,timeTick );
	}
	inline void GetSysTimeStruct(const std::string & CompleteDayTime , SYSTEMTIME & timestruct)
	{
		DWORD year,month,day,hour,minute,second,week;

		int ret = sscanf(CompleteDayTime.c_str(),"%u-%02u-%02u -%02u- %02u:%02u:%02u",&year,&month,&day,&week,&hour,&minute,&second);

		timestruct.wYear      = year;				//��
		timestruct.wMonth     = month;				//��
		timestruct.wDay       = day;				//��
		timestruct.wHour      = hour;				//Сʱ
		timestruct.wMinute    = minute;			//��
		timestruct.wSecond    = second;			//��
		timestruct.wMilliseconds = 0;
		timestruct.wDayOfWeek    = week;
	}
	inline bool IsCompleteDayTime(const std::string & OverTime)
	{
		SYSTEMTIME curtimeTick;//��ǰʱ��
		GetLocalTime(&curtimeTick);

		SYSTEMTIME ctimestruct;//���������ɵ�ʱ��
		GetSysTimeStruct(OverTime,ctimestruct);
		if (curtimeTick.wYear > ctimestruct.wYear)//�����������������ճ�
		{
			return true;
		}

		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth > ctimestruct.wMonth)//ͬһ�� ���Ҵ�����
		{
			return true;
		}

		if (	
			curtimeTick.wYear == ctimestruct.wYear && 
			curtimeTick.wMonth == ctimestruct.wMonth && 
			curtimeTick.wDay > ctimestruct.wDay
			)
		{
			return true;
		}

		return false;
	}
	inline void   FileTimeToTime_t(  const FILETIME  & ft,   time_t   *t   )   
	{   
		LONGLONG   ll;   

		ULARGE_INTEGER ui;   
		ui.LowPart =   ft.dwLowDateTime;   
		ui.HighPart =   ft.dwHighDateTime;   

		ll =   (ft.dwHighDateTime   <<   32)   +   ft.dwLowDateTime;   

		*t =   ((LONGLONG)(ui.QuadPart   -   116444736000000000)   /   10000000);   
	} 
	long GetDiffDays(const SYSTEMTIME & ctimestruct,const SYSTEMTIME & curtimeTick)
	{
		FILETIME fTime1;   
		FILETIME fTime2;   

		time_t tt1;   
		time_t tt2;   

		SystemTimeToFileTime(   &ctimestruct,   &fTime1   );   
		SystemTimeToFileTime(   &curtimeTick,   &fTime2   );   

		FileTimeToTime_t(   fTime1,   &tt1   );   
		FileTimeToTime_t(   fTime2,   &tt2   );   

		time_t difftime =   tt2   -   tt1; 
		long dif = difftime   /   (24*3600L);
		return dif;
	}

	inline bool IsCompleteWeekTime(const std::string & OverTime)
	{
		SYSTEMTIME ctimestruct;//���������ɵ�ʱ��
		GetSysTimeStruct(OverTime,ctimestruct);

		SYSTEMTIME curtimeTick;//��ǰʱ��
		GetLocalTime(&curtimeTick);
		long tk = GetDiffDays(ctimestruct,curtimeTick);
		WORD dayofweek = curtimeTick.wDayOfWeek;//���ڼ�

		long changeValue = 0;

		if (dayofweek == 0)
		{
			changeValue = 1;
		}
		else if (dayofweek > 0)
		{
			changeValue = 7 - dayofweek + 1;
		}
		else
		{
			return false;
		}
		long value = tk - changeValue;
		if (value >= 0)
		{
			return true;
		}
		return false;
	}
	long GetSpaceDays(const std::string & OverTime)
	{
		long k = 0;
		SYSTEMTIME ctimestruct;//���������ɵ�ʱ��
		GetSysTimeStruct(OverTime,ctimestruct);

		SYSTEMTIME curtimeTick;//��ǰʱ��
		GetLocalTime(&curtimeTick);
		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth == ctimestruct.wMonth)//ͬ��ͬ��
		{
			return curtimeTick.wDay - ctimestruct.wDay;
		}
		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth > ctimestruct.wMonth)//ͬ�겻ͬ��
		{
			return GetDiffDays(ctimestruct,curtimeTick) + 1;
		}
		if (curtimeTick.wYear > ctimestruct.wYear)//��ͬ��
		{
			return GetDiffDays(ctimestruct,curtimeTick) + 1;
		}

		return 0;
	}
	void UpDataPlayerRdTaskInfo();//��������ճ���������
	void InsertPlayerRdTaskInfo(WORD NpcID,WORD taskID);
	WORD GetRdTasksNum(WORD NpcID);
	WORD GetCurrentNpcTaskID(WORD NpcID);
	void RecvRdData(WORD taskID);
	WORD GetRdNpcNum();
	void UpDataPlayerXKL();//����������
	long OnXKLProcess();
	void AddXKLUseNum(short num);
	short GetXKLUseNum();
	void OnHandleCycTask(WORD npcID);
	WORD GetHeadIDByStr(std::string  headHail);
	void PrintXKLLOG(WORD num,long ItemID);
	void OnPlayerBlessTime();
	long GetPlayerMoney(long type) const;
	void ChangeMoney(long type,long value);	
	long GetPlayerAllMoney() const; // ��ȡ������������е�Ǯ
	bool ReducedCommonMoney( long lMoneyValue, TPlayerMoneyType ePriorityReduced = EMoneyType_Bind); // [��Ǯ-�󶨺ͷǰ�Ǯ�����Կ�ȡ��ע�ⲻ�ܼ�Ǯ]
	void UpdataBless();//������
	void SendPlayerBlessOpen();
	//�ճ�������¼�������
	long OnPartEnterNum(DWORD RegionID,BYTE numLimit);//���븱���ɹ���ʱ���������
	long OnTeamPartEnterNum(DWORD RegionID,BYTE numLimit);//���븱���ɹ���ʱ���������
	long IsPersonalEnterPart(DWORD RegionID);
	long IsTeamEnterPart(DWORD RegionID);
	void UpdataPartEnterNum();
	DWORD m_PartDyRegionID;		//���븱������һ�θ���ID
	DWORD m_PartDyRegionGID;
	long IsNonTeamPartEnter(DWORD RegionGid);
	//
	std::string m_ServerRelationName;
	long m_CountTA;
	std::vector<TemplateSkillInfo> m_TemplateSkillInfo;
	long m_ClickTaskFlag; //0��ֱ�ӵ��NPC 1�ǽ�������
public:
	// �������
	SimFactionData::SimMemberInfo m_stFacRight;
	//BOOL  m_bisFaction;	// �Ƿ��ǰ���
	DWORD m_dwFactionTitleID;
	void  UpdateFaction();
	void  UpdateMemberData( SimFactionData::SimMemberInfo *member, LPCSTR factionName, WORD factionId, BOOL doSynchro );
	// ��¼��ǰλ�� �Ա����»���
	WORD m_wBackRegionID;
	WORD m_wBackPosX;
	WORD m_wBackPosY;

	//20150126 wk �������Ը���λ��
//	DWORD GetSID(void) const { return m_Property.m_dwStaticID; }
	DWORD GetSID(void) const { return m_FixData.m_dwStaticID; }
	
public:
	DWORD m_dwPKNumber;

public:
	enum ONLINE_STATE
	{
		OST_NOT_INIT,
		OST_NORMAL,
		OST_LINKBREAK,
		OST_HANGUP
	} m_OnlineState;

public:
	BOOL RecvAutoUseItem(SQAutoUseItemMsg* pMsg);

	// ��һ�ν�����Ϸʱ��Ҫ�ĳ�ʼ��������
	BOOL FirstLoginInit();

	// �һ�ʱ������Ϸʱ��Ҫ�ĳ�ʼ��������
	BOOL HangupLoginInit();

	// �ͻ��˻�û���κ����ݵ�ʱ�򣬷������б�Ҫ����Ϣ��
	BOOL SendInitMsgs();

	void RecoverTask();

public:
	BOOL isWaitingLockedResult( BOOL useAutoUnlock, BOOL showDialog );

private:
	SQAutoUseItemMsg m_stAutoUseItemSet;								// �Զ���ҩ����
	WORD             m_dUpXMValTimes;                                   // ������ħֵ���� 20��һ��
	bool             m_bHaveXMBuff;                                     // �Ƿ�����ħBUFF

public:
	WORD selected_item_x;
	WORD selected_item_y;

public:
	DWORD	temporaryVerifier;		// Ч�����

	// ��ATM����صĽű��������ڲ�������ִ�нű��ص�
	// ������ڽű��ص������ջ�׵ı����ض��ǽű����������ַ�����������ı�����Ϊ�ú����Ĳ���
	DWORD                       ATM_temporaryVerifier;      // Ч�����
	std::stack< lite::Variant > ATM_temporaryScriptVariant;

	// ������Զ�̽�����Ϣ
	std::map< DWORD, time_t >    excInfoMap; // ��չ������Ϣӳ�����߼�¼�˷����ĵ��ߺ���󷢲���ʱ��

	// �ͽ���ϵͳ���У������, ������ҽ��������޸�!
	DWORD                       buildVerify;      // Ч�����

	// ɱ����������Ĺ���ӳ���( key=��������,value=list<����ID> )
	// ������������ݿ�,����Ҽ������ݵ�ʱ��,��Ҫ��ֵһ��
	std::map< std::string, std::list<DWORD> > taskKillMonsters;

public:
	// ʱ�����������ڼ���������������������ڿͻ��ˣ�
	ClickLimitChecker           clickLimitChecker;

	//��չ�ķ����
	void    UpdateExtraTelerge();
	void    OnExtraTelergyModify( struct SQSetExtraTelergyMsg *pMsg );
	void    OnOpenExtraTelergy( struct SQOpenExtraTelergyMsg *pMsg );
	// ��������
	void    ArrangePackage(WORD type);

	// ��̬�ֿ�ͱ���
	//WORD    packageId[3];         // 
	//WORD    goodNumber[3];        // 0:�������� 1 & 2���ֿ�����

	// ����ת�̵Ľ�����
	BYTE            dialEndIndex;

	STaskExBuffer   taskEx;             // Ϊ�½ű�ϵͳ׼�������ݿռ�
	BOOL            isErrPlayer;        // �Ƿ�Ϊ������
	BOOL            isAutoFight;        // �Ƿ�����ʹ���Զ�ս������
	DWORD           timeOutChk;         // ������ȡ�����Ϣ�ĳ�ʱ�����߼��жϡ�����
	BOOL            isUseRose;

	SPackageItem    *m_pItemXQD;        // ���ڿ��ٲ���Ѫ����
	SPackageItem    *m_pItemXTBL;       // ���ڿ��ٲ������챦¼
	SPackageItem    *m_pItemLZ[6];      // ���ڿ��ٲ��һ�ˮľ����������
	int             m_nLZEffectVal[6];  // ���������ֵ����ʱ��¼������

	void    UseItemXTBL();              // ʹ�����챦¼

	// GMȨ����֤���
	void    SendGetGMIDInf();           // ���ͻ�ȡGM�����Ϣ����
	struct  GMIDCheckInf
	{
		DWORD   dwRand;                 // �����
		SQGameMngMsg SQGMMsg;           // GM��Ϣ
	};

	GMIDCheckInf m_GMIDCheckInf;

	// ����ҽ�µ�
	WORD    GetAddMedicalEthicPoint( WORD wCureType );
	void    AddMedicalEthics( WORD wAddPoint, WORD wDesPLevel = 0 );
	WORD    CalculateMaxPerDayME( WORD wLevel );

	void	SendFirstUseQXD();
	void    SendFirstUseLZ();
	bool	IsWuLinChief();
	BOOL    IsInFactionFirst();

	void    RecvGetNewXYDScoresInf( SQUpdateScoreListExMsg * pMsg );
	void    CheckAndSendHellioMaskBUFF( BOOL bIsLogin = FALSE );
	BOOL    GetAutoUseItemSetLZ() { return m_stAutoUseItemSet.bSetLZ; }

	// ��ħ���
	void    UpdateXinMoVaule();
	void    SendUpdateXMVauleMsg();
	void    AddXinMoBuff();

	// ����ÿ����µ�������ݵ����Ͳ�ͬ�����˺�������Ϊģ�庯������֤���ݰ�ȫ����ֹ�������ݵ��������
	template<class T, class _Ty>
		void	UpdatePlayerDataValsPerDay( DWORD& dwLastTime, T* pValue1, _Ty* pValue2 = NULL )
	{
		if ( pValue1 == NULL )
			return;

		DWORD dwCurTime = ( DWORD )time( NULL );

		if ( (( DWORD )( ( dwLastTime + 8*60*60 ) / ( 24*60*60 ) ) != ( DWORD )( ( dwCurTime + 8*60*60 ) / ( 24*60*60 ) ))
			|| ( dwCurTime - dwLastTime ) > 24*60*60 )
		{
			*pValue1 = 0;
			if ( pValue2 != NULL )
				*pValue2 = 0;
			dwLastTime = dwCurTime;
		}
	}

	void CheckUseXQD( WORD itemIndex, DWORD dwHPMin, DWORD dwHPMax, BOOL &autoUseItemRuslet );

	void SendUpdateDecDurTimeMsg();

	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);

	virtual INT32 GetSkillIDBySkillIndex(INT32 index);
	virtual INT32 GetCurrentSkillLevel(DWORD dwSkillIndex);
	INT32 SetCurrentSkillLevel(DWORD dwSkillIndex,BYTE bskillLevel);

	INT32 GetSkillLevel(DWORD dwSkillIndex);
	INT32 SetSkillLevel(DWORD dwSkillIndex, BYTE bskillLevel);
	

	DWORD GetSkillProficiency(DWORD dSkillID);                  // ��ȡ�书������
	bool AddSkillProficiency(DWORD dSkillID, int iAddVal);// �����书������

	// ��ǰ��״̬�Ǽ��ܹ���״̬���ڼ��ܹ���״̬������л��������״̬��ʱ��
	virtual void OnChangeState(EActionState newActionID);
	virtual void OnRunEnd();

	void Move(INT32 moveTileX,INT32 moveTileY);

	void OnDamage(INT32 *nDamage, CFightObject *pFighter);

	void UpdateSKillBox(SUpdateSkillBox_C2S_MsgBody* msg);
	void UpdatePracticeBox(SUpdatePracticeBox_C2S_MsgBody * msg);

	void ProcessForceQusetEnemyInfo();

	// ���ļ���
	virtual bool ConsumeHP(int consume);
	virtual bool ConsumeMP(int consume);
	virtual	bool ConsumeTP(int consume);
	virtual SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);
	WORD GetMpConsume(BYTE type, WORD baseMPConsume,WORD telergyLevel);
	// �ڲ����Խӿڣ���Ҫʹ��
	void _SetPropertyTemp(int type, int data);

	//����һ���б�����Ŀǰѡ���Լ�����ұ��
	std::vector<DWORD>  m_LookAtPlayer;

	bool StartJumpBuff();
public:
	void LockeOperator( );
	void UnLockeOperator( );
public:
	class	LockEvent : public EventMgr::EventBase
	{
	public:
		LockEvent( ){ mPlayer = 0; }
		~LockEvent( ){ mPlayer = 0; }

		void OnSetup( ){ if(mPlayer) mPlayer->LockeOperator(); }
		void OnCancel( EventMgr *mgr );
		void OnActive( EventMgr *mgr );
		void SetPlayer( CPlayer* player ){ mPlayer = player; }
	private:
		CPlayer* mPlayer;
	};
	AutoEvent<CPlayer::LockEvent>	mLockEvent;

public://����
	// ����Ŀ��ȫ��ID
	void StoreTarget(CFightObject* pObj);
	void CALLBACK OnCloseListener(CFightListener* p);

public:
	CFightListener *m_pAttribListener;	

protected:
	virtual BOOL _UseItem(SPackageItem *pItem,DWORD useType=0);
public://����,������ڻ����ж���
	int 	GetEquipMount( );									//�õ���ǰװ������������
	int 	GetRidingMount( );									//�ȵ���ǰ��˵���������
	void	InitMounts( SPlayerMounts* pFixMounts );			//��ʼ��һ������,����ӵ���ɫ�������,
	bool	CreateMounts( int mountId, BYTE quality );			//����һ��ָ��ID,ָ��Ʒ�ʵ�����
	bool	CreatePets(int petId, BYTE quality);				//����һ��ָ��ID,ָ��Ʒ�ʵĳ���
	bool	CreateFightPet(WORD fpID);							//����һ��ָ��ID������
	void	EquipMount(BYTE mountIndex);						//װ������,ָ����������
	void	UnEquipMount(BYTE mountIndex);						//ȡ��װ������
	void	DeleteMounts(BYTE mountIndex );						//ɾ������
	void	DeletePets(BYTE petIndex);							//ɾ������
	void	DeleteFightPet(BYTE fpIndex);						//ɾ������
	SPlayerMounts::Mounts*	GetMounts( int index );				//ͨ�������õ���������
	SPlayerPets::Pets* GetPets(int index);						//��ȡ��������

	//����
	void	PreUpMounts(BYTE mountIndex);						//׼������
	void	UpMounts( int mountIndex );							//����
	void	DownMounts( );										//����
	
	void	OnMountsMsg(SMountsMsg* pMsg);						//����������Ϣ
	void	UseMountSkill(short mountIndex,short skillIdx);		//ʹ�����＼��
	void	FeedMounts( struct SQFeedMounts* pMsg);				//������ιʳ
	void	AddMountPoint(SQMountAddPoint *msg);
	
	void	SkillNumUpdate();									//���＼�������ȸ��� ��ʱ���ӿ�
	void	InitMountSkillColdTime();							//��ʼ�����＼����ȴʱ��
	void	ReleaseMountSkillCD();								//�ͷ����＼����ȴʱ�����				

	DWORD	m_RidingStatus;										// �����״̬�����Ϊ0����ʾû������
	BYTE			m_InRidingevens;										//�����������״̬	
	BYTE			m_InUseItemevens;										//�ڵ��߶�����״̬

	std::map<int,EventMgr::EventBase*> m_MountSkillColdTimeTbl; //���＼����ȴʱ���.

public:
	void	StartCollect(DWORD npcid, DWORD time, int x, int y, const string& sFinishFunc);
	void	CancelCollect();
	void	OnFinishCollect();
	bool	UpdateCollect();

	void	SendCollectMsg(DWORD npcid, DWORD time, int x, int y);
	void	SendCancelCollectMsg();

	DWORD	m_nCollectTimeLen;
	DWORD	m_nCollectBeginTime;
	DWORD	m_nCollectNpcId;
	string	m_sCollectFunc;

public:
	void AddToSaledItemList(SPackageItem &item, DWORD value);
	void BuyBackSaledItem(SQBuySaledItem *pMsg);

public:
	void OpenVenPoint(WORD VenId);
	bool CheckFrontOpened(WORD FontId);
	BOOL IsAllVenOpened(WORD MaiID, bool AddExtra = false);
	BOOL RecvOpenVenMsg(SQOpenVenMsg *pMsg);
	void InitJingMaiData();

	//add by 2014/3/25 ��ʼ���ƺŸ�������
	void InitTitleData();
	
	void OnShareExpWhenJingMaiOpenVenSuccess(WORD wXueID); // /// ��������Ѩ�ɹ��ľ������

public:
	inline BOOL IsInStall()		// �Ƿ��ڰ�̯״̬
	{
	//	return m_Property.m_ShowState & PSE_STALL_SALE;
		return FALSE;
	}

public:	//�һ�
	bool	m_bIsStarPractice;	//��ʾ�һ��Ƿ�����
	DWORD	m_lStarTime;		//��ʼʱ��
	DWORD	m_lEndTime;			//����ʱ��
	WORD	m_wCurSkillLevel;
	
	DWORD	m_dwSkillProNum;	//��¼��ͨ�书������������
	void	OnGetPracticeRequest(SAPracRequesttMsg * pMsg);//�õ�����
	void	SendPracticeResult(bool bSuccess,WORD wType);	//����������
	bool	CheckPlayPracticeCondition(DWORD dwGID,WORD wType,WORD wID); //�ж��Ƿ���������������Ŀǰֻ�л���
	void	AccountPracticeResult(WORD wType,WORD wSKillID);	//�����������
	//void	UpDatePracticeBox(SUpdatePracticeBox_C2S_MsgBody *msg);
	WORD	GetCurPracticeSkillLevel(WORD wType,WORD wID);
	DWORD	GetSkillProForMP(WORD wType,WORD wSkillID,DWORD dwPro);	//ͨ���������ʵ�ʵ����������յ�������
	SSkill*	GetSkillInfoforID(WORD wskillID);
	bool	CheckBoxInfo(WORD wID,DWORD wstOldBox[]);
	void	LogOutUnPractice();			//���߹һ�����
	void	SendSkillPro(WORD wType,WORD wSkillID,DWORD dwPro);			// ͬ������������
	void	SendPlayerPracticeResult();
	void	UnPractice();
	void	SendSynPlayMount(WORD index, BYTE level, BYTE bAction);		// ������

public:
	enum COPYSCENE_STATE
	{
		COPYSCENS_NONE,		// û�и�������
		COPYSCENS_TASK,		// ���ܸ�������
		COPYSCENE_PREPARE,	// ׼�����븱��
		COPYSCENE_IN,		// ������
		COPYSCENE_LOCK,		// ����������
	};
	void SetCopySceneState( COPYSCENE_STATE eState){ m_nCopySceneStata = eState;}
	COPYSCENE_STATE GetCopySceneState(){ return m_nCopySceneStata;}

private:
	COPYSCENE_STATE m_nCopySceneStata;

//-------------------------------------------------����------------------------------------//
public:
	BOOL CanUpMount(BYTE mountIndex);
	void CallOutFightPet(BYTE index);
	void CallOutFightPet(SQCallOutFightPet *pmsg);
	void CallBackFightPet(bool IsTemp = true);//�Ƿ�ӳ������Ƴ�
	void ShowPet(BYTE index);	// ��ʾ����Ϊindex�ĳ���
	void CallBackPet();			// ���յ�ǰ�ĳ���
	void CloseGroundItem();		// �Ӵ��鿴������ӳ��
	void MoveFightPet(SQFightPetMoveto *pmsg); //˲������

	void _fpUpdateExtraProperties(BYTE changetype,BYTE index);//�������Ͷ�������Ա仯
	void _fpChangeExtraProperties(BYTE changetype,int changevalue,BYTE index);//�������Ͷ�������Ա仯

	void _fpMakeItem(BYTE index);//����ת��Ϊ����
	BOOL _fpItemTofp(SPackageItem *pItem);//���߱������
	void _fpChangeAttType(SQfpchangeatttypeMsg *msg);//�л���ǰ��ս�����͵Ĺ���ģʽ
	//���͵�װ�����Ա仯
	void _fpInitEquipJewelData(int EquipIndex);
	void _fpInitEquipAttribute(const SItemFactorData* pItemFactorData);
	void _fpInitEquipRandAttriData(int EquipIndex);
	void _fpInitEquipmentData();
	///������Ҫװ�����ߵ���������
	BOOL _fpSetEquipFightPet(BYTE index);
	///�����Ҫװ�����ߵ���������
	void _fpInitEquipFightPetIndex(){m_nEquipfpIndex = -1;}
	int	_fpgetEquipFightPetIndex(){return m_nEquipfpIndex;}
	///�������Ը���
	void _fpUpdateAllProperties(int index = -1);
	SFightPetExt *_fpGetFightPetByIndex(BYTE index);//����������ȡ����
	void _fpUpdateProperties(BYTE type,WORD propertyvalue,BYTE index);
	void _fpUpdateProperties();
	//����װ��������
	BOOL _fpRecvEquipSkillBookMsg(BYTE index,SPackageItem *pItem);

	bool _fpIsEquipSkillItem(DWORD itemid);//�Ƿ�Ϊ����װ�������ͼ�����

	int   lua_fpGetItemToSkillID(DWORD itemid);//���ݵ�ǰ��װ���ļ����鷵����Եļ���ID
	int   lua_fpGetItemToSkillLevel(DWORD itemid);//���ݵ�ǰ��װ���ļ����鷵����Եļ��ܵȼ�

	int  _fpGetSkillSlot(WORD wSkillID,BYTE index);//��ȡ���ͼ���λ�ã����Ϊ-1��û��ѧ���������,-2û���ҵ��������
	int _fpLearnSkillByfp(SQfpSkillUpdate *pMsg);//��������������ѧϰ����

	int _fpGetSkillNum(BYTE index);//��ȡ�����Ѿ�ѧϰ�ļ�������

	bool _fpIsInfightPet(BYTE index);//�Ƿ��ǳ�ս�е�����

	bool IsInJump();
	
	///��ȡ��ǰ��̬����ID��Ϊ0����ͨ����
	//DWORD GetDynamicRegionID();
	//��������
	void ImpelFightPet(SQFightPetImpel *PetImpelmsg);
	void GetImpelValueOntime(DWORD dwCurTick);//��������ʱ���ȡ����ֵ,�������͵�һЩ����ʱ��ı�Ķ���
	void ClearMetrialRelation();
	DWORD	m_nImpelFightPetTime;	// ���ͼ���ֵ��ȡʱ��
	WORD	m_ImpelValue;				// ���ͼ���ֵ
	DWORD	m_nRestoreTiredTime[MAX_FIGHTPET_NUM];		//���ͻظ�ƣ��ʱ��
	DWORD	m_nfpLearnSkillTime;				//���Ϳ�ʼѧϰ���ܵ�ʱ��
	DWORD	m_nchecklivetime;						//���ڼ������Ƿ����ߵļ�¼
	//BYTE			m_fpAtkType;						//���͹���ģʽ
public:	
	WORD  m_PetStatus;			// ��ǰ�ĳ���״̬(������// [2012-7-27 16-35 gw: +])��0��ʾû�� 
	DWORD m_PetGID;				// �����ΨһID

	// �����3��������ϵͳ	
	SPackageItem	*m_UpdateItem;	
	SPackageItem	*m_LJS_Metrial;		// ����ʯ	
	SPackageItem	*m_XMTJ_Metrial;// �����쾫	
	BYTE			m_UpdateItemType;	

	BYTE			m_MaxUpdateTimes;	// ���������������
	BYTE			m_MaxUseTimes;		// �������ʹ�ô���

	// �����3װ������ϵͳ
	//SPackageItem	*m_forgeItem;			// ��ǰ�������װ��
	SPackageItem	*m_forgeMetrial;		// ԭ����
	BYTE			m_forgeType;			// �ϱ�ʯ
	DWORD			m_MakeHoleTime;			// ��ʼ��׵�ʱ��

	// װ��ǿ��ϵͳ����������������
	// ���������ǡ����ף���װ����ԭ���ϡ����ʲ��ϣ��������׻��б�������
	// ��������Ʒ�ʡ�������	
	SPackageItem				*m_strengthenMetrial;			// ǿ��ԭ����
	std::vector<WORD>			m_vecStrengthenEquip;			// ǿ��װ��λ���б���¼λ�ø�����һЩ	
	std::vector<WORD>			m_vecStrengthenEquipAssist;		// ǿ����������
	std::vector<WORD>			m_vecStrengthenMetrial;			// ǿ��ԭ�����б�
	
	BYTE						m_refineType;					// ǿ������
	DWORD						m_MetrialItemID;	// ���ϵ���ID
	// ϴ�����
	SEquipment					*m_pIdentifyEquipBackUpNeedDel;		// ϴ�����ɵ���װ����ʱ���ݣ������Ϊ0��Ҫ�ֶ�ɾ��

	/***********************************
	 * XYD3��װ���Լӳ���ر���
	 ***********************************/
	WORD						m_ModifyTypeMark;				// ��Ҫ�޸ĵ���װ���Ա��

	// ��ʱ������װ���Ե�ֵ��ʹ����װ��ʱ�������ֵ����һ����
	WORD						m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAX];	

	// �����3����֧�Ż���
	typedef std::map<DWORD, DWORD> CHXQuestList;
	CHXQuestList	m_ChxSentOut;				// �ͳ�������
	CHXQuestList	m_ChxGetIn;					// �յ�������

	// �����3���֧�Ż���
	typedef std::map<DWORD, std::pair<DWORD, BYTE> > TeamQuestList;
	typedef std::hash_map<DWORD, DWORD> SyncFindTeamList;
	typedef std::set<DWORD> SyncTeamQuestList;

	TeamQuestList		m_TeamSentOut;					// �ͳ�����������ȡ����
	TeamQuestList		m_TeamGetIn;					// �յ����������ھܾ�/ͬ�⣩

	DWORD				m_SendRequestID;				// ���������������
	SyncTeamQuestList	m_LastSynTeamRequestIDList;		// ͬ�����ͻ��˵���������б�
	SyncFindTeamList	m_LastSynFindTeamIDList;		// ͬ�����ͻ��˵Ķ����б�

	// �����3����ʰȡҡ��֧�Ż���
	typedef std::map<DWORD, WORD> OwnedChanceItems;
	OwnedChanceItems	m_OwnedChangeItems;				// �����Ҫҡ�ŵ��б�

	SaledItemType	m_buyBackItem[CONST_BUYBACK_COUNT];	// ���߻ع�ϵͳ
	WORD			m_curSaledItemCount;				// ��ǰ�ع��б�ĸ���
	CItem			*m_pCheckItem;						// �鿴���ϵİ���
	CSingleItem		*m_pSingleItem;						// �鿴���ϵİ���
	DWORD			m_dwFactionLimitTime;				// ��������ʱ����
	DWORD			m_dwPublicLimitTime;
	DWORD			m_dwSchoolLimitTime;
	DWORD			m_dwTeamLimitTime;
	DWORD			m_dwRumorLimitTime;
	DWORD			m_dwWhisperLimitTime;
	DWORD			m_dwGlobalLimitTime;

	class CFightPet	*m_pFightPet;						// ��ǰ���ڳ�ս������
	DWORD			m_FightPetTimeLimit;				// ���ͳ�սʱ������

	BYTE			m_PreFPIndex;						// ���ڽ�������л���ͼ������

	// �����3��Ծ��������
	DWORD			m_StartJumpTime;					// ��ʼ��Ծʱ��
	DWORD			m_LastingTime;						// ��Ծ����ʱ��

	// ��ҵľ���ɱ�������ж�
	int				m_NineWordsKill;
	// �ж��Ƿ��͵ȼ����ķ�������Ϣ
	BOOL			m_updateSkill;
	BOOL			m_updateTelergy;

	// �Ƿ���������˺�
	BOOL			m_bKrlinArmDamge;

	//����ʱ�䣬�������������
	INT64 m_dwOnlineBeginTime;	//�ۼ����߿�ʼʱ��
	INT64 m_dwLoginTime; //��ǰ�ĵ�½ʱ�䡣���ڼ�¼
	BOOL m_AddOnceTpFlag;	//ÿ��16���ӻָ���������������ߺ��½ʱ��ֻ����һ������ʱ��������ҵ�TPֵ
	INT64 m_SecondOnceAddTp;	//ÿ��ֻ��������ҵ�����һ�Σ���Ϊ��ѭ��ʽ�������ʱ����ͬ���������ɶ��ִ��
	//����������ģ������,���Ĺ���
	typedef std::map<short,long> TemplateMap;
	TemplateMap m_TemplateMap;

	BYTE		m_bFashionMode; //ʱװģʽ������ͨģʽ
	bool		m_bFirstLogIn;	// ��һ�ε�½ȫ�������ķ�����
	BYTE		m_bTempLevel;	// ��½����ķ��ȼ�

	WORD	m_bCurDeadreginID;		//��ǰ������ͼID
	WORD	m_wCurDeadCount;		//�ڵ�ǰ��ͼ��������
	WORD wReliveTime[4];
	WORD wReliveMoney[4];
	DWORD m_dUpdateState;			//���ڼ��ͻ��˵ĸ���״̬�����Ϣ����
	BYTE	m_HoleType;
	BYTE m_PlayerOpenClose;
	DWORD m_dLoginCheckTime;	//��¼���ʱ�䣬�����쳣ʱ����
public://�չؽ���
	void OnRecvBiGuanMsg(SBiguanMsg* pMsg);
	void OnRecvBiGuanLingquMsg(SQBiguanLingquMsg* pMsg);
	void calculateSpAndExpByTime(INT64 sec,DWORD &sp,DWORD &exp);
public://���ʳɳ�
	bool tizhiUpgrade(DWORD itemId);		//��������
	void SendTiZhiUpgradeResult();
	bool tizhiGrow(DWORD itemId);			//�������ʣ���������
	void SendTiZhiGrowResult();								
	void SendTiZhiData();					//��ͻ��˷�����������
	void OnRecvTiZhiUpdateMsg(SQTiZhiUpdateMsg *pMsg);			
	void OnRecvTizhiTupoMsg(SQTiZhiTupoMsg *pMsg);
	void OnRecvKylinArmUpgradeMsg(SQKylinArmUpgradeMsg* pMsg);
public://�����
	bool activeKylinArm();
	bool kylinArmUpgrade();
	void SendKylinArmData();
	virtual void OnAttack(CFightObject *pFighter);
	void addPlayerYuanqi(WORD value);
	
public:
	void ShowChangeProperty(BYTE type, WORD value);
	void UpdatePlayerXwzMsg();//���͸��������Ϊֵ
	void OnRecvQuestUpdateBossMsg(SQBossDeadUpdatemsg* pMsg);
	void OnRecvGetGiftMsg(SQGetOnlienGiftMsg *pMsg);	
	void _L_SafeCallLuaFunction(const char *funcnam,...);	
	int  _L_GetLuaValue(const char *funcnam);
	int  _L_GetLuaValue(const char *funcnam,lite::Variant const &lvt,lite::Variant const &lvt2);
	int  _L_GetLuaValue(const char *funcnam,lite::Variant const &lvt,lite::Variant const &lvt2,lite::Variant const &lvt3);
	int  _L_GetLuaValueParmer(const char *funcnam,WORD parmernum,...);
	void _L_CheckOnRunTime();
	//void _L_StopTime();
	//void _L_StartTime();
	//void _L_CleanTime();
	//bool _L_IsStopTime();
	void _L_UpdateCountDownGiftState();			// ���µ���ʱ�����ȡ�Ľ׶�
	int _L_GetCountDownGiftState();				// �õ�����ʱ��������׶�
	BYTE _L_GetOnlineGiftState(int index);	// �õ���ǰӦ����ȡ�����
	void _L_UpdateOnlineGiftState(int index, BYTE state);
	void _L_ResetCountDownGiftState();			// ���õ���ʱ�����ȡ�Ľ׶�
	void _L_ResetOnlineGiftState();
	void _L_ResetCountDownGiftTime();	//���õ���ʱ���ʱ��
	INT64 _L_GetCountDownGiftBeginTime();//�õ�����ʱ��ʼʱ��
	INT64 _L_GetOnlineBeginTime();			// �õ���ǰ����ʱ��
	INT64 _L_GetNowTime();				// �õ���ǰʱ��
	void OnRecvQuestChangeFashion(SQFashionChangeMsg *pmsg);
	void OnClinetReady();				// �ڿͻ��˽�������ȡ���Ժ�ʼ����һЩ��Ϣ������Ϊ��ʼ������һ�νű�����Ϣ
	void SentClinetGiftSucceed(BYTE bType,BYTE bResult, BYTE index = 0);
	void OnProcessDead();//�����������
	void OnRecvQuestActivityNotice(SQBossDeadUpdatemsg* pMsg);
	float GetWalkSpeed();


	INT64 _L_GetLastLoginTime(){return m_Property.m_dLoginTime64;} //��ȡ�ϴεĵ�¼��Ϸʱ��	
	INT64 _L_GetLastLeaveTime(){return m_Property.m_dLeaveTime64;}//��ȡ�ϴε�����ʱ��	
	INT64 _L_GetCurrentLoginTime(){return m_dwLoginTime;} //��ȡ��ǰ�ĵ�¼��Ϸʱ��	

	void _L_SetPlayerVipLevel(BYTE level){m_Property.m_bVipLevel = level <= 10 ?  level : m_Property.m_bVipLevel;}
	void OnSetPlayerLevel(BYTE Level);

	bool _L_calculateSpAndExpByTime(INT64 sec,DWORD &sp,DWORD &exp);//�¹�Ԫ�����㣬��lua�з���

	DWORD	m_PartprevRegID;			// ���֮ǰ�ĳ���ID
	WORD	m_PartprevX;				// ����
	WORD	m_PartprevY;				
	WORD  m_XwzValue;			//�����Ϊֵ
	WORD    m_CurFollowIndex;
public:
	void InitTelergyAndSkill();			// �ķ���ʼ�������ﲻ�ô������ݿ⣬ֱ���ڵ�½��ʱ�����¼���һ�ξ�OK��
	void OnRecvTaskMovemsg(SQtaskmove *pmsg);//���������͵�NPC

	////add by ly 2014/3/17  ������Ǵ���
	void OnHandlePlayerRiseStar(struct SQXinYangRiseStarmsg *pMsg);

	//add by ly 2014/3/25
	void SendSAGloryMsg(struct SAGloryMsg *pMsg);		//������ҫ������Ӧ����Ϣ
	void GetPlayerGloryInfo(struct SQGloryMsg *pMsg);	//�����ҫ������
	BOOL JudgeCanGetGloryAward(const SGloryBaseData *lpGloryBaseData);		//�ж��Ƿ������ȡ��ҫ����
	void GetGloryAward(struct SQGetGloryMsg *pMsg);		//��ȡ��ҫ����
	void OnHandleTitleMsg(struct SQTitleMsg *pMsg);		//����ƺ���Ϣ

	//add by ly 2014/4/16
	void SendDailyMsgToPlayer(struct SAInitDailyMsg *pMsg);	//�����ճ���б���Ϣ
	BOOL DispatchDailyMessage(struct SDailyMsg *pMsg);	//�ճ�������Ϣ
	void OnHandleInitDailyMsg(struct SQInitDailyMsg *pMsg);	//��ʼ���ճ����Ϣ
	void OnHandleEntryDaily(struct SQEntryDailyMsg *pMsg);	//�����ճ����Ϣ
	void OnHandleAwardDaily(struct SQDailyAwardMsg *pMsg);	//�����ȡ�ճ���Ľ���
	void OnHandleLeaveDaily(struct SQDailyLeaveMsg *pMsg);	//�ͻ��������뿪�ճ������
	void OnHandleGetFirstPayAward(struct SQFirstPayAwardMsg *pMsg);//����״γ�ֵ����
	void OnHandleGetEveryDayPayAward(struct SQEveryDayPayAwardMsg *pMsg); //�����ȡÿ�ճ�ֵ����
	void OnEveryDayPayAwardState(struct SQGetEveryDayAwardStateMsg* pMsg);//��ȡÿ�ճ�ֵ����״̬

	void OnQuestJuLongShanRisk(struct SQJuLongShanRiskMsg *pMsg);	//��������ھ���ɽ��̽��

	void OnTurnOnTarotCard(struct SQTurnOnOneTarotCardMsg* pMsg);	//������󷭿�һ��������
	void OnGetTarotAward(struct SQGetTarotAwardMsg* pMsg);	//������������ƽ�
	void OnUptPlayAnimateFlag(struct SQUptTarotPlayAnimateFlagMsg* pMsg);	//���������Ʋ��Ŷ�����־


	void OnGetCurDayOnlineLong(struct SQCurDayOnlineLongMsg* pMsg); 		//�������ÿ������ʱ�����ҵ�������ʱ��
	void OnCurDayOnlineGetAward(struct SQCurDayOnlineGetAwardMsg* pMsg);	//��ȡ��Ӧʱ��εĽ���

	void OnQuestAddUpLoginDay(struct SQAddUpLoginDayMsg* pMsg);			//���ҵ����ۼƵ�½����
	void OnAddUpLoginGetAward(struct SQAddUpLoginGetAwardMsg* pMsg);	//�����ȡ�ۼƵ�½��Ӧ����ID�Ľ���

	void OnGetRFBAward(struct SQRFBGetAwardMsg* pMsg); 	//��ȡ��ֵ������Ӧ�Ľ���

	void OnGetLRAward(struct SQLRGetAwardMsg* pMsg);	//��ȡ�ȼ���������

	void OnGetFWOpenResidueTime(struct SQFWOpenResidueTimeMsg* pMsg); 	//��ȡ������Ħ���ֵĿ���ʣ��ʱ������
	void OnStartFWGame(struct SQFWStartGameMsg* pMsg); 	//��ʼ����Ħ������Ϸ
	void OnGetFWAward(struct SQFWGetAwardMsg* pMsg);	//��ȡ����Ħ���ֽ���

	//add by ly 2014/4/28
	BOOL OnHandleGetSignInAward(struct SQGetSignInAwardMsg *pMsg);	//���������ȡÿ��ǩ��������Ϣ

	//add by ly 2014/5/7 �̳����
	void DispatchShopMessage(SShopMsg* pMsg);	//�̳������Ϣ����
	void OnHandleQuestShopListOpt(struct SQShopListMsg* pMsg);		//������������̳��б���Ϣ
	void OnHandleQuestGoodsListOpt(struct SQShopGoodsListMsg* pMsg);		//������������̳�����Ʒ�б���Ϣ
	void OnHandleBuyGoods(struct SQBuyGoodsMsg* pMsg);				//������ҹ�����Ʒ
	void OnHandleGetShopCountDown(struct SQGetShopCountDownMsg* pMsg);	//��ȡ�̳�ˢ�µ���ʱ
	void SendShopListInfo(struct SAShopListMsg* pData);	//�����̳�����
	void SendGoodsListInfo(struct SAShopGoodsListMsg* pData);	//����ָ���̳ǵ���Ʒ����
	void SendBuyGoodsResult(struct SABuyGoodsMsg* pData);		//���͹���������
	void SendNotityGoodsUpdated(struct SANotifySpecialGoodsUpdateMsg* pData);	//֪ͨ���̳��Ѿ�ˢ��
	void SendShopCountDown(struct SAGetShopCountDownMsg* pData);	//�����̳�ˢ�µ���ʱ
	void OnHandleFindGoodsByItemID(struct SQFindGoodsMsg* pMsg);	////�����ȡ��Ʒ��Ϣ����ͨ�����ߵ�ID
	void SendSigleGoodsInfo(struct SAFindGoodsMsg* pData);	//���͵�����Ʒ����Ϣ
	void OnHandlePlayerPay(struct SQPlayerPayMsg* pMsg);	//������ҳ�ֵ�����Ϣ

	//add by ly 2014/5/17 Vip���
	void DispatchVipMessage(SVIPMsg *pMsg);	//Vip�����Ϣ����
	void OnHandleInitVipDataOpt(struct SQInitVipInfMsg *pMsg);//��ʼ��VIP��Ϣ
	void OnHandleGetGiftInfOpt(struct SQGetVipGiftInfMsg *pMsg);//��ȡ�����Ϣ
	void OnHandleGetGiftOpt(struct SQGetVipGiftMsg *pMsg);//��ȡVIP���
	void SendVipStateInfOpt(struct SAInitVipInfMsg *pData);		//�������VIP��״̬��Ϣ
	void SendGiftInfoOpt(struct SAGetVipGiftInfMsg *pData);	//�������������Ϣ
	void SendGetOrBuyGiftResOpt(struct SAGetVipGiftMsg *pData);		//������ȡ���ȡVIP��������Ϣ

	//add by ly 2014 �������
	void DispatchPetMessage(struct SPetMsg *pMsg);		//���������Ϣ����
	void OnHandleCardExtract(struct SQExtractPetMsg *pMsg);		//������ҿ��Ƴ�ȡ����
	void OnHandleChipCompose(struct SQChipComposPetMsg *pMsg);	//���������Ƭ�ϳɳ������
	void OnHandleGetCardPetInfo(struct SQGetCardPetInf *pMsg);	//�����ȡ���Ƴ����б���Ϣ
	void OnHandlePetMerger(struct SQMergerPetMsg* pMsg);	//�����������
	void OnHandlePetBreach(struct SQPetBreachStarMsg* pMsg);	//�������ͻ��
	void OnHandlePetRename(struct SQRenamePetMsg* pMsg);	//��������������
	void SendExtractPetRes(struct SAExtractPetMsg* pData);		//���ͳ�ȡ���ĳ�������
	void SendClipComposePetRes(struct SAChipComposPetMsg* pData);	//������Ƭ�ϳɵĳ�������
	void SendCardPetInfo(struct SAGetCardPetInf* pData);	//���Ϳ��Ƴ����б���Ϣ
	BOOL UseItemGetPet(struct SPackageItem *pItem);		//ʹ�õ��߻�ó���
	void SendUseItemGetPet(struct SAUsePetItem *pData);	//����ʹ�ó�����߻�õĳ�������
	void OnHandleGetPetCardInfo(struct SQGetPetCardInf *pMsg);	//�����ȡ���￨����Ϣ
	void SendPetCardInfo(struct SAGetPetCardInf *pData);	//���ͳ��￨����Ϣ
	LPSNewPetData GetPetByID(DWORD PetID);	//��ȡ��������ͨ������ID
	LPSNewPetData GetPetByIndex(BYTE PetIndex);	//��ȡ��������ͨ����������
	BOOL AddPet(LPSNewPetData pPetData);	//��ӳ���
	BOOL DeletePet(BYTE PetIndex);		//ɾ��ָ��ID�ĳ���
	void SendPetMergerRes(struct SAMergerPetMsg* pData);	//���ͺϲ���ĳ�������
	void SendPetBreachRes(struct SAPetBreachStarMsg* pData);	//���ͳ���ͻ�ƽ������
	void SendPetRenameRes(struct SARenamePetMsg* pData);	//���ͳ����������Ƿ�ɹ�
	void PetSkillUpdate(struct SQPetSkillUpgradeMsg* pMsg);		//���＼������
	void OnHandleSkillConfig(struct SQPetSkillCfgInf* pMsg);	//���＼������
	void OnHandlePetFollowFight(struct SQSuiShenFightMsg *pMsg);	//����������ս��
	void OnHandlePetShapeshiftFight(struct SQShapeshiftFightMsg *pMsg);	//����������ս��
	BOOL PlayerTransferPet(BYTE PetIndex);	//��ұ���Ϊ����������������֮ǰ�����Բ��ѳ���ĵ����Ը�ֵ����ң������������󣬱�����֮ǰ��״̬
	BOOL TransferPlayer();		//�����ҵ���ͨ״̬������Ѿ�����ͨ��̬�Ͳ��ñ���
	void OnHandleGetPetExpInf(struct SQPetExpInfMsg *pMsg);	//��ȡ���ɳ�����Ի�ȡ�ľ���
	void OnHandleBuyPet(struct SQGlodDirectBuyPetMsg *pMsg);	//���ֱ�ӹ������
	void SendBuyPet(struct SAGlodDirectBuyPetMsg *pData);	//���ͽ�ҹ����õĳ�������
	void GetResumeDurableNeedGlod(struct SQResumeNdGoldMsg *pMsg);	//��ȡ�ָ����������Ҫ�Ľ����
	void ExcResumePetDurable(struct SQResumePetDurableMsg *pMsg);	//���ѽ�һָ��������
	void SynPetDurable(BYTE PetIndex);		//ͬ���������
	void CheckResumeFullPetHpAndMp();	//�˶Իظ�������Ѫ��������
	void CheckResumeFullPlayerHpAndMp();//�˶Իظ������Ѫ��������
	void OnHandlePetStudySkill(struct SQPetStudySkillMsg *pMsg);	//�������ѧϰ���ܲ���
	BOOL PetActiveSkill(BYTE PetIndex, WORD SkillID);		//���Ｄ���
	FightProperty m_PetFightPro;	//�������ս������
	time_t m_RecordPreTransferTime;	//��¼����ϴα���ʱ��
	time_t m_RecordPreDeadCTime;	//��¼�������������ȴʱ��
	BOOL m_SysAutoTransToPlayer;	//���������Ѫ��Ϊ0ʱ������������Ϊ��ҵ�ԭʼ��̬�������жϱ�����ȴʱ���Ƿ���������
	DWORD m_TranPrePlayerHp;	//����ǰ����Ѫ��
	DWORD m_TranPrePlayerMP;

	//add by ly 2014/7/22 ���а�
	void OnHandleRandListMsg(struct SQPlayerRankList *pMsg);	//�����������а��б���Ϣ
	void SendRandListData(struct SAPlayerRankList *pData);		//�������а��б�����
	void OnHandleGetRankAwardState(struct SQGetRankAwardState* pMsg);	//��ȡ������а��콱״̬
	void OnHandleRankAwardOpt(struct SQGetRankAward* pMsg);	//������ȡ����

	//add by ly 2014/7/24 ��Ծ��
	void OnGetActivenessInf(struct SQGetActivenessInfo *pMsg);	//��ȡ��һ�Ծ����Ϣ
	void OnGetActivenessAward(struct SQGetActivenessAward *pMsg);	//��ȡ��Ծ�ȵĶ�Ӧ����
	void SendActivenessInf(struct SAGetActivenessInfo *pData);	//������һ�Ծ����Ϣ


	void OnTreasureMsg(struct SQTreasureMsg *pMsg);//�۱�����Ϣ

	void OnInitPrayerMsg(struct SQInitPrayerMsg *pMsg); //�������ʼ��
	void OnStartPrayerMsg(struct SQPrayerMsg *pMsg); //��ʼ��
	void OnShopRefresh(struct SQShopRefresh *pMsg);//ˢ���̵�
	void OnShopBusiness(struct SQShopBusiness *pMsg);//�̵꽻��

	void OnRequestFactionScene(struct SQFactionSceneData *pMsg);//����ͬ���������Ÿ���
	void OnRequestFactionInfo(struct SQFactionSceneMapData *pMsg);//������Ÿ�����ϸ��Ϣ
	void OnChallengeScene(struct SQChallengeSceneMsg *pMsg);//������ս���Ÿ���
	void OnChallengeFinish(struct SQSceneFinishMsg *pMsg);//���Ÿ�����ս���
	void OnRequestFactionSceneRank(struct SQSceneHurtRank *pMsg);//������Ÿ������а�
	void OnInitFactionSalaryData(struct SQInitFactionSalary *pMsg);//����ٺ»����
	void OnGetFactionSalary(struct SQGetFactionSalary *pMsg);//��ȡٺ»����
	void OnShowNotice(struct SQFactionNotice *pMsg);//�鿴����
	void OnModifyNotice(struct SQFModifyNotice *pMsg);//�޸Ĺ���
	void OnShowoperateLog(struct SQFactionOperateLog *pMsg);//�鿴������־
	void OnSendFcEmailToAll(struct SQFcEmailToAll *pMsg);//���;����ʼ�

	void OnGetGiftCode(struct SQGiftcodeActivity  *pMsg);//������
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	////2014/11/12 �����佫���
	////////////////////////////////////////////////////////////////////////////////////////////////////
	///@brief ����������
	void DispatchSGPlayer(struct SSGPlayerMsg* pMsg);
	
	///@brief ����ֿ����
	void DsiapatchStorage(struct SSanguoItem * pMsg);

private:

	///@brief �����������
	void OnSetPlayerAttr(struct SQSetAttrMsg *pMsg);

	void CheckTimeWhenLogin();	//��鲢����ʱ�䣬����¼ʱ
	void CheckTimeWhenLogout();
public:
	BatchReadyEvent m_BatchReadyEvent; // ������ӵ������ϵ�ReadyEvent

	//add by ly 2014/4/8
	BYTE m_IsUseKillMonster;	//�Ƿ�ʹ������ƶ�ɱ�֣���Ϊ�������ˢ�������ԣ�ɱ��ʱҲ����ɱ��
	time_t m_EntryScenceTime;	//���볡�����ʱ�䳤��
	time_t pret;
	CRegion* m_pPreFightRegion;	//�����һ��ս������

	//add by ly 2014/5/16
	//VIP���Լӳ�
	WORD m_VIPFactor[VFT_MAX];


	//add by ly 2014/06/20 ���������Ϊ0ʱ���ڼ��ʱ��ﵽ��ָ�������Ļ���ֵ
	void CheckResumePetHunli();
};


inline void CPlayer::CloseGroundItem()
{
	//rfalse(2, 1, "����ȡ���鿴һ����%s���İ���", m_pCheckItem ? "��ע��" : "�Ѳ���ע");

	if (m_pCheckItem)
	{
		m_pCheckItem->RemoveChecker(this);
		m_pCheckItem = 0;
	}
}

inline bool CPlayer::IsInJump()
{
	return m_StartJumpTime ? true : false;
}

