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

const   int     MAX_MONSTER_ONGOAT  = 81;      // 81个怪剑气满
const   int     AUTO_ADDMC_ONGOAT   = 72;      // 1.2分钟自动增加 1%
const   int     MAX_SKILL_STATE     = 6;       // 武功最大境界

const DWORD DEFAULT_HP_RES_INTERVAL = 10 * 1000;
const DWORD DEFAULT_MP_RES_INTERVAL = 10 * 1000;
const DWORD DEFAULT_SP_RES_INTERVAL = 10 * 1000;
const BYTE	MAX_FIGHTPETACTIVED = 4 ;// 侠客格子最大激活次数
#define MOUNT_HUNGER_TIME (1000 * 10)
#define CHECKTIMELIMIT_TIME (1000 * 10)
#define PLAYER_SP_RESTIME 20
#define PLAYER_SP_RESVALUE 4		///真气恢复值
#define PLAYER_PKVALUERISE 100  //杀孽增长值

typedef multimap<DWORD, Rank4Client> RankMap;

struct SQAutoUseItemMsg;

#include "sfuncmanager.h"

struct ClickLimitChecker
{
	// 加速&连点的限制数据
	// 说明：如果每5秒内，服务器接收到的指令次数（目前限于移动，攻击两类消息）大于15次，
	// 则将其断网[或不断而采取忽略其指令]
	// 建立在假设1秒钟最多发3个指令的基础下
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
	DWORD		m_BaseMaxHp;		// 基础最大生命
	DWORD		m_BaseMaxMp;		// 基础最大内力
	DWORD		m_BaseMaxTp;		// 基础最大体力
	WORD		m_BaseAtk;			// 基础攻击
	WORD		m_BaseDefence;		// 基础防御
	WORD		m_BaseCtrAtk;		// 基础暴击
	WORD		m_BaseEscape;		// 基础躲避
	BYTE		m_nPkRule;			// PK模式
	DWORD		m_nChagePkRuleTime;	// 切换PK模式时间
protected:
	int OnCreate(_W64 long pParameter);

public:
	void SendData(BOOL isSave);					// 数据保存（发给登陆服务器）
	void Backup( SFixProperty &data );			// 备份数据到data中
	void Logout(bool SaveAndLogOut = false);	// 玩家退出
	bool CheckDisWait();						// 玩家断线等待重连时间检测
	void OnReconnect();							// 玩家重连
	void OnDisconnect();						// 玩家断线
	void ClearLogoutStatus();					// 退出清理状态
	void ClearDisConnectStatus();				// 掉线清理状态
	void SaveLogoutBuffList();					// 退出游戏时保存Buff列表

	void OnClickSomebody(LPIObject pObj);		// 玩家点击一个对象

	

	void ResetState();							// 转换场景时重设玩家状态

	void MyTitleChanged(char* pNewTitle);
	
	void ClearPlayerStatus(bool IsCommon);

	void OnDead( CFightObject *PKiller );		// 当死亡的时候的回调
	bool isDead(){ return m_Property.m_CurHp <= 0; }
	void OnRegionChanged(CRegion *newReigon, bool Before);
	void RecoverPlayer(CPlayer *pPlayer); //恢复数据
	void SaveData();							// 保存玩家数据
public:
	BOOL CheckAction(int eCA);					// 检测动作执行的条件
	LPCSTR GetName() { return m_FixData.m_Name; }
	LPCSTR GetAccount() { return m_szAccount.c_str(); }
	LPCSTR GetNotPrefixAccount() { return m_szAccount_without_prefix.c_str(); }
	LPCSTR GetAccountPrefix( int lv = 0 ) { return lv ? m_szAccount_lv2_prefix.c_str() : m_szAccount_lv1_prefix.c_str(); }
	DNID	GetMac();
	BYTE	GetSex() { return m_Property.m_Sex ? 1 : 2; }
	// -----------------------------------
	// ActiveObject & FightObject 虚函数
private:
	DWORD m_checkTick;

	void CheckOnRunTime();		// 侠义道三每帧检查
	void OnRun();               // 继承基本控制入口
	
	//void CheckRunOnTime(void);
	BOOL EndPrevAction();       // 上一个动作结束时的回调
	BOOL DoCurAction();         // 执行当前的动作
	BOOL CheckTrapTrigger();

private:
	void FillSelectMsg(struct SASelPlayerMsg *pMsg);	// 填充玩家信息
public :
	void OnEveryDayManagerRun(int nIndex);

	void AddValueByVenaDataEx( LPSVenapointDataEx pVenaData );
	bool IsPassVenationEx( BYTE byVenaNum, BYTE byPointNum );
	int CheckPlusPoints();
	// -----------------------------------
	// 消息处理函数
public:
	void OnRecvMoveMsg(struct SQSynPathMsg *pMsg);
	void OnRecvSynPosMsg(struct SQSynPosMsg *pMsg);
	void OnRecvSynZMsg(struct SQSynZMsg *pMsg);
	void OnRecvJumpMsg(struct SAQSynJumpMsg *pMsg);
	void OnRecvUnity3DJumpMsg(struct SAQUnity3DJumpMsg *pMsg);

	void OnJumpMsg(struct SQASynWayJumpMsg *pMsg);
	void OnMoveMsg(struct SQSynWayTrackMsg *pMsg);
	struct SASynPlayerMsg *GetStateMsg();

	void OnRecvLevelUp(struct SQLevelChangeMsg *msg);		// 接收到玩家升级消息
	void OnRecvSetPoint(struct SQSetPointMsg *pMsg);        // 接收到玩家分配点数
	void OnRecvResetPoint(struct SQResetPointMsg* pMsg);	// 接收到玩家洗点消息
	void OnRecvClearPoint(struct SQResetPointMsg* pMsg);	// 接收到玩家洗点消息

	void OnReceiveClickMsg(struct SQClickMsg);	            // 接收到玩家点击某对象的消息，这个可以被解释为多种行为
	void OnRecvEquipColor(struct SQAEquipColorMsg *pMsg);   // 接收到玩家改变装束颜色
	void OnRecvZazenMsg( struct SQSetZazenMsg *pMsg );      // 接收到玩家设置打坐状态
	void OnWifeRemitMsg();                                  // 夫妻传送消息处理
	void OnRevDoctorAddBuffMsg( struct SQDoctorAddBuffMsg* pMsg, CPlayer* pDoctor );  // 接收到医生加医德BUFF的消息

	void FightOnSomebody(struct SFightAction *pMsg, LPIObject DestObject);	// 该玩家攻击某对象

	void OnTaskMsg(struct SRoleTaskBaseMsg *pMsg);
	void StoreClickMsg(struct SQClickObjectMsg *pMsg);
	bool OnUpdateTask(struct SRoleTaskBaseMsg *pMsg);//更新新增任务状态

	void SetAccount(const char * szSetAccount);
	BOOL SetFixProperty(SFixProperty *pData);
	BOOL CPlayer::SetFixData(SFixData *pData); //设置三国玩家数据
	BOOL UpdateProperty();

	BOOL GetTempData(SPlayerTempData *pData);       // 保存临时数据
	BOOL SetTempData(SPlayerTempData *pData);       // 设置临时数据

	BOOL AddPKiller(CPlayer *pPKiller);
	BOOL IsPKiller(CPlayer *pPKiller);
	BOOL CheckPKiller(CPlayer *pDestPlayer,BOOL );

	void UpdatePkMap();

	BOOL AddPlayerPoint(WORD type, WORD point);
	BOOL ResetPlayerPoint(BYTE type, WORD point);

	// 属性取值接口
	BYTE	GetAmuck(void);
	short	GetXValue(void);
	BYTE    GetPKValue(void);

	// 获取随机装备上附加的属性
	WORD    GetEquipDAM(void);
	WORD    GetEquipPOW(void);
	WORD    GetEquipDEF(void);
	WORD    GetEquipAGI(void);

	// 获取血内体提升速度
	WORD    GetAddHPSpeed(void);
	WORD    GetAddMPSpeed(void);
	WORD    GetAddSPSpeed(void);

	bool    SetMutate( BYTE byMutateType, WORD	wMutateID );        // 设置变身状态
	void    SetScapegoat( WORD wScapegoatID, WORD wGoatEffID );     // 设置替身状态

	void UpdateEquipWear(int pos, int value, bool IfMaxWear = false);			// 改变装备的耐久度

	// 消耗一定量的心法
	bool ReduceTelergyVal( BYTE byTelergyID, DWORD dwUseVal, BOOL bIsReduce, LPCSTR info );   
	// 激活心法（学会心法）
	BOOL ActivaTelergy( WORD wTelergyID, LPCSTR info  = NULL);
	// 激活轻功（学会轻功）
	BOOL ActiveFly(WORD wFlyID, LPCSTR info, bool bForce = false);
	// 激活心法（学会护体）
	BOOL ActiveProtected(WORD wProtected, LPCSTR info , bool bForce = false );
	// 删除装备的心法(按装备的位置0-5)
	BOOL DeleteTeletgy( BYTE byTelergyNum );      
	// 获取当前修炼心法层数
	int GetTelergyLevel( void );   
	// 获取心法层数
	int GetTelergyLevel( WORD wTelergyID );   
	// 提升心法
	BOOL AddTelergy( int iAddVal, LPCSTR info = NULL);
	// 提升轻功 
	BOOL AddFly( int id, int iAddVal, LPCSTR info = NULL );   
	// 提升护体
	BOOL AddProtected( int id, int iAddVal, LPCSTR info=  NULL );   
	// 设置心法熟练度
	BOOL SetTelergyProfic( WORD wTelergyID, DWORD dwProfic );   
	// 设置心法熟练度
	BOOL SetTelergyProficM( WORD wTelergyID, DWORD dwProfic, LPCSTR info );   
	// 设置当前修炼的心法
	BOOL SetCurTelergyNum( BYTE byCurTelergyNum );       

	bool GetTelergyLevelEx(int index,int* level);
	// 更具ID获得当前心法的等级 --anhuajie
	WORD GetTelerLevelByIndex(WORD index);

	// 心法中恢复玩家的攻击力跟防御
	void ListenProperty();

	//填充需要保存的BUFF数据
	void SetBuffSaveData();
	
	//查找护体技能
//	SSkill* GetProtectSkillById( int id, EProtectedSkillType& type );
	//查找轻功技能
//	SSkill* GetFlySkillById( int id, EFlySKillType& type );

	// 设置当前修炼武功
	BOOL SetCurSelSkills( WORD cur );	
	// 获取心法攻击系数
	int GetTelergyCoef( BOOL bIsAttack );         
	// 根据心法类型计算当前装备心法的相应值
	int GetTelergyCalculate( E_TOTAL_TELERGY iDataType );    

	// 根据心法类型计算当前装备心法的相应值
	int GetTelergyEffect( E_TOTAL_TELERGY iDataType );          
	void CalculateTelergyEffects();

	// 经脉相关
	BOOL    DamageToVenation( CPlayer *pAttacker, int iDamage, BOOL bIsDead,
		BYTE &byWhoVenation, BYTE &byVenationState );// 根据伤害值计算经脉所受到的伤害
	BOOL    DamageToVenation( CPlayer *pAttacker, BYTE &byWhoVenation, BYTE &byVenationState, DWORD extraPos );

	BOOL    BeCureVenation( CPlayer *pCurer );                      // 被治疗经脉受伤
	BOOL    CureSomebodyVenation( DWORD dwDestGID );                // 治疗某人经脉受伤
	void    UpdateCureVenation( void );                             // 每秒20次更新治疗经脉的时间

	// 单个心法改变消息函数
	void	SendCurTelChange(SXYD3Telergy *pTempTel, BYTE pos, bool bAddNewTel);
	
	/******************************************* 
	侠义世界 
	*/
	void SendUpSpeed();

	// 经脉加成的最终数据
	int m_VenaAddFactor[SVenapointData::VENA_PROPERTY_MAX];
	bool InitVenaAddFactors();
	int GetVenaAddFactor(SVenapointData::VENA_PROP_DEF type);

	// 判断一条脉线是否已经打通
	bool IsPassVenation( BYTE byVenaNum );									
	// 根据穴道数据增加属性值
	void AddValueByVenaData( LPSVenapointData pVenaData );												
	// 取得已经打通的经脉数量
	int  GetPassVenationCount( BYTE byCount );
	int  m_iPassVenationCount;
	// 打开经脉
	bool SetPassVenapoint( BYTE byVenaNum, BYTE byPointNum );    // 设置一个穴道被打通
	bool OpenVena( LPSVenapointData pVenaData, BYTE byVenaNum, BYTE byPointNum );		
	bool GmOpenVena( BYTE byVenaNum, BYTE byPointNum );		
	// 心法加成的最终数据
	INT32 m_TelergyAddFactor[ TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE ];
	bool InitAddTelergyAddFactorTbl();
	INT32 GetTelergyFactorByIndex(TelergyDataXiaYiShiJie::TELE_PROP_DEF index);

	// 设置心法等级，如果设置的等级比当前的低，那么数值会减下去。
	bool SetTelergyLevel(int index,int level);

	// 处理客户端请求加点的消息
	void OnRecvQuestAddPoints(struct SQuestAddPoint_C2S_MsgBody* pMsg);

	// 装备属性加成（加值和加百分比）
	int m_EquipFactorValue[SEquipDataEx::EEA_MAX];

	// 骑乘属性加成
	WORD m_MountAddFactor[SPlayerMounts::SPM_MAX];

	// 经脉属性加成
	WORD m_JingMaiFactor[JMP_MAX];
	//信仰属性加成
	WORD m_XinYangFactor[XYP_MAX];

	//add by ly 2014/2/25 称号属性加成
	WORD m_TitleFactor[TitleAddType::TITLE_AMAX];

	// 心法附加属性加成（值与百分比）
	WORD m_TelergyFactor[TET_MAX];
	// 心法触发概率数据
	WORD m_TelergyActiveRate[TET_MAX - TET_SUCKHP_PER];

	// 1 装备 2 上马 3 卸装 4 下马
	void ChangeMountFactor(SPlayerMounts::Mounts *pMount, WORD type);
	WORD GetMountFactor(WORD index);

	void InitEquipJewelData(int EquipIndex);
	void InitEquipAttribute(const SItemFactorData* pItemFactorData);
	void InitEquipAttriData(int EquipIndex);
	void InitEquipmentData();	
	BOOL InitEquipSuitAttributes_New(); // 生成套装新属性
	BOOL InitEquipSuitAttributes();		// 装备套装加属性接口
	BOOL MinusAllSuitAttibutes();		// 减去所有套装属性，用在装备不全的情况下
	BOOL ModifySuitAttributes(WORD wMark);		// 修改套装额外属性

	// ------test----------
	BOOL ModifySuitAttriValue(WORD wMark);		// 修改属性值
	// ---------------------

// 	//侠客的装备属性变化
// 	void _fpInitEquipJewelData(int EquipIndex);
// 	void _fpInitEquipAttribute(const SItemFactorData* pItemFactorData);
// 	void _fpInitEquipRandAttriData(int EquipIndex);
// 	void _fpInitEquipmentData();
// 	///设置需要装备道具的侠客索引
// 	BOOL _fpSetEquipFightPet(BYTE index);
// 	///清除需要装备道具的侠客索引
// 	void _fpInitEquipFightPetIndex(){m_nEquipfpIndex = -1;}
// 	int	_fpgetEquipFightPetIndex(){return m_nEquipfpIndex;}
// 	///侠客属性更新
// 	void _fpUpdateAllProperties();

	BOOL RecvCloseGroundItem();
	BOOL RecvCheckGroundItem(SQCheckGroundItemMsg *pMsg);
	BOOL RecvStartTakeChance(SQGroundItemChanceBack *pMsg);

	//登陆时根据情况重新启动BUFF
	void InitBeginBuffData();	

	//检查清除上一个地图所添加的BUFF
	void DropRegionBuffList(WORD regionId);

	//设置心法状态
	bool SetTelergyState(SQSetTelergyStateMsg *pMsg);

	// 更新属性加成
	virtual void UpdateAllProperties();
	void UpdateBaseProperties();
	void UpdataPlayerCounterpartProperties();//更新玩家副本属性
	long FindpartData(short Index);
	// 按需发送更新后的属性
	virtual void SendPropertiesUpdate();
	virtual void SwitchFightState(bool IsIn);

	// 上一次回复HP,MP,SP的时间
	DWORD m_dwLastHPResTime;
	DWORD m_dwLastMPResTime;
	DWORD m_dwLastTPResTime;
	DWORD m_HpRSInterval;
	DWORD m_MpRSInterval;
	DWORD m_TpRSInterval;
	DWORD m_HpRecoverSpeed;
	DWORD m_MpRecoverSpeed;
	DWORD m_TpRecoverSpeed;

	///上一次回复真气的时间
	DWORD m_dwLastSPResTime;
	/// 根据在线好友的多少定时送经验,记录时间
	DWORD m_nOnlineFriendTime;
	/// 根据在线好友的多少定时送经验,记录经验
	DWORD m_nOnlineFriendExp;
	/// 根据在线好友的多少定时送经验,记录累计的经验次数
	WORD m_nOnlineExpCount;
	/// 根据在线好友的多少定时送经验,记录最后一次增加的经验用于验证
	DWORD m_nOnlineFriendLastExp;

	///正在装备道具的侠客索引
	int m_nEquipfpIndex;

	/// 根据在线好友的多少定时送经验
	void  OnCheckFriendNum(DWORD dwCurTick);

	// 处理自动恢复
	void __OnCheckAutoRestore(DWORD dwCurTick);
	bool __OnCheckHPAutoRestore(DWORD dwCurTick);
	bool __OnCheckMPAutoRestore(DWORD dwCurTick);
	bool __OnCheckSPAutoRestore(DWORD dwCurTick);///真气恢复
	bool __OnCheckTPAutoRestore(DWORD dwCurTick);

	bool AddSkillProficiency(WORD wSkillID, int iAddVal, LPCSTR info = NULL);    // 提升武功熟练度

// 	DWORD GetSkillProficiency( DWORD dSkillID );                  // 获取武功熟练度
// 	bool AddSkillProficiency(DWORD dSkillID,  int iAddVal);// 提升武功熟练度

	// 装备耐久度减少
	void OnWeaponWearReduce();
	void OnEquipWearReduce( WORD point);
	int __OnEquipWearReduce( EQUIP_POSITION position, WORD point);		// 返回1表示耐久度变化，返回2表示耐久度变化并且为0（失效）
	void __SendEquipWearChangeMsg(EQUIP_POSITION position);

	/*****************************************************/

	// 熟练度相关（没有护体神功了）
	double  GetSkillCoef( BYTE bySkillID, BOOL bIsAttack );         // 获取武功系数
	BOOL    CheckUpdataAttackLevel( BYTE bySkillID, BYTE byLevel ); // 检查武功升级条件

	void    SendTelergyChange( BOOL extraOnly = FALSE );            // 发送心法数据的改变
	void    SendUpdataSkill( BYTE bySkillPosID );                   // 发送招式更新消息
	void    SendTemplateSkill(WORD index,WORD Flag);
	void    SendVenationState( BYTE byWhichvena, BYTE byState, BYTE byIsUpdate=1 );    // 发送心法状态，包括受伤、治疗
	void    SendOnCureing(DWORD dwGID);                             // 发送心法治疗的确认消息，可能包括治疗动作，所以是广播
	void    SendExtraState(void);                                   // 发送附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04

	void    SendPassVenapoint( BYTE byVenaNum, BYTE bypointNum );   // 发送打通穴道确认消息
	void    SendChangeMutate( BYTE byMutateType, WORD	wMutateID );// 发送变身消息
	void    SendScapegoat( WORD	wScapegoatID, WORD wGoatEffID );    // 发送替身消息
	void    SendOnGoatMonsterCount( WORD wCount, WORD wMutateTime );	// 发送替身状态打怪数量
	void    SendRefreshOnUp(void);										// 离线后上线数据更新

	void    OnRecvPassVenapointAsk( BYTE byVenaNum, BYTE bypointNum );	// 接受打通穴道请求消息

	void	OnRecvUpdateTelegry(SQTelergyUpdate *TelMsg);				// 玩家要升级心法
	void	OnRecvUpdateSkill(SQSkillUpdate *SkillMsg);					// 玩家要升级技能
	void UpdateSingleSkill(SQSkillUpdate *SkillMsg);	//处理升级单个技能
	void UpdateGroupSkill(SQSkillUpdate *SkillMsg);		//处理升级一组技能

public:
	BOOL	IsMoneyEnough(DWORD type, DWORD value);
	DWORD	AddPlayerMoney(DWORD type, DWORD value);
	BOOL	CheckPlayerMoney(DWORD type, DWORD value, bool OnlyCheck);
	WORD	GetPlayerReginID();
public:
	DWORD	AddPlayerSp(DWORD value);
	BOOL	CheckPlayerSp(DWORD value, bool OnlyCheck);
	bool	CheckPkRuleAttk(DWORD gid,BYTE AtkType=0);	// 检测PK模式能否攻击，判断是否在仇恨列表中
	
	void	SendPlayerPkValue();						// 发送玩家的杀孽值
	void	SendPlayerNameColor();						// 发送改变玩家的名字颜色消息
	BYTE	GetPlayerNameColor();						// 杀孽值得到玩家名字颜色
	void	OnCheckNameColor();							// 根据时间减少玩家杀孽值
	bool    OnCheckNameColorByVen();                    //根据经脉打通数目来判断名字颜色
	DWORD	CalculatePkValueDecrease();					// 每1点杀孽值减少的时间
	void	CheckPkPlayerName(CFightObject *fightobj);	// 检测是否应该黄名
	void	OnDropItem();								// 死亡掉落物品
	int		GetRandomEquipmentIndex();					// 得到装备在身上的一件随机装备
	int		GetEquipmentNum();							// 得到装备在身上的装备数量
	int		RandomDropItemToGroundBag(std::vector<SRawItemBuffer> &itembag,BYTE dropType);//随机掉落身上装备或者物品,0装备1物品
	void	OnChangePKValue(int pvalue);				// 改变玩家的杀孽值
	// 一个特殊操作，关于触发器的设定
public:
	ITEMDROPIDTABLE m_vTaskDropIdTable;					// 任务需要掉落物品ID表
public:
	BOOL UpdatePlayerProperty(SFixBaseData * pFixBaseData);     // GM 指令更新玩家资料

	/************************************************************************/
	/*              OnRun函数的功能子函数                            */
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
		DNID    dnidClient;     // 连接编号
		DWORD   dwVersion;      // 玩家数据
	};

public:
	struct SFluxProperty : public SFixProperty
	{
		SFluxProperty() {}
		
		QWORD puid;
	};

	SFluxProperty	m_Property;			// 玩家基本属性
	BYTE			m_aClickParam[12];	// 点击的参数
	SFixData m_FixData; //三国玩家成员数据
	std::map<DWORD , DWORD> m_StroageIndex; //仓库索引容器
public:
	// 侠义道3玩家属性更新
	const SBaseAttribute *m_PlayerAttri;				// 快速引用
	DWORD m_PlayerFightPower;	//玩家战斗力
	void	*m_PlayerAttriRefence[XA_MAX-XA_MAX_EXP];	// 引用优化
	bool	m_PlayerPropertyStatus[XA_MAX-XA_MAX_EXP];
	bool	m_IsPlayerUpdated;

	QWORD	m_MaxExp;		// 当前等级最大经验值
	DWORD	m_MaxJp;		// 当前等级最大精力值
	DWORD	m_MaxSp;		// 当前等级最大真气值

	DWORD		m_dwTeamID;			// 队伍ID
	WORD		m_wTeamMemberCount;	// 队伍当前个数
	bool		m_bIsTeamLeader;	// 是否为队长

	WORD		m_wPKValue;				// PK值,杀孽值
	BYTE		m_bNameColor;			// 名字颜色
	DWORD		m_dChangeColorTime;		// 改变名字颜色计时
	BOOL IsTeamLeader();


	void ChangeTeamSkill(BYTE byNewTeamSkill,WORD wMemberCount);
	BOOL CheckCanTalk(DWORD type, DWORD time);

	LPCSTR GetTeamLeaderName(BYTE index); ///获取队伍中玩家名字
	DWORD GetTeamLeaderRegionGID();
	DWORD GetTeamCurPartRegionGID();
// ********************************排行榜相关**************************************
public:
	BYTE		m_RankNum[RT_MAX];				// 排行榜当前排名
	Rank4Client m_RankList[RT_MAX][MAX_RANKLIST_NUM];

    BOOL QuestDB2GetRankList(struct SQClientQuestRankFromDB *pMsg);		// 请求DB，拉数据
	BOOL GetRankListInfo(struct SQRankListMsg *pMsg);					// XYD3查看排行榜信息接口

// ==================================================================================

private:
	long	m_DeadWaitTime;	// 死亡等待时间
	DWORD	m_DeadTime;		// 死亡时间，用于验证

	WORD px, py;

	DWORD   m_dwOneSecondTick;		// 1秒的间隔
	DWORD   m_dwSecondTick;			// 10秒的间隔
	DWORD   m_dwMountTimer;			// 1分的间隔
	DWORD   m_dwMinuteTick;			// 1分一次的动作
	DWORD   m_dwHellionBuffTick;	// 3秒的间隔
	DWORD   m_dwDecDurTime;			// 5分钟间隔

	BOOL    m_bCountFlag;			// 统计的标记
	WORD    m_wScriptState;			// 脚本状态值
	WORD    m_wScriptIcon;			// 脚本图标编号

	struct MultipleTime
	{
		BYTE m_byMultipleTime;		// 当前双倍时间
		BYTE m_byLeaveTime;			// 剩余时间
		DWORD m_dwLastTime;			// 最后一次领取时间 
		BYTE m_byCurWeekUseTime;	// 当前使用时间
		BYTE m_fMultiple;           // 倍数
		BYTE m_reserve1;		
	};

	MultipleTime multipletime;

	DWORD   m_dwUseTelergyVal;  // 本次冲穴所消耗的心法点数

	WORD    m_SportsState;      // 竞技状态
	DWORD   m_SportsRoomID;     // 房间

public:
	DWORD   m_ProRefTime;      // 重生保护开始时间
	bool    m_bProtect;       // 保护状态
	BYTE	m_DynamicRegionState;		// 0,1是否在动态区域
public:
	void EnableProtect(void);
	bool GetProtect(void) const { return m_bProtect; }

	_PLAYERDATA m_CountData;   //需要的统计的数据

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

	// 多倍掉经验、金钱相关
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

	// 游戏大厅
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

	void UpdateDoctorBuffTime( bool loginCheck = false ); // 更新医德BUFF剩余时间

protected:
	void UpdateMultipleTime(void);

	WORD	m_wMutateID;		    // 变身的图档编号
	WORD    m_wScapegoatID;         // 替身的图档编号 =0 无替身
	WORD    m_wGoatEffectID;        // 替身的效果编号

	WORD    m_DropItemBaseRand; // 掉落特殊物品的变化概率

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

	BOOL	IsBetting();			//是否正在游戏大厅

	void    AddCheckID(void)        {   m_wCheckID++;                                           }

	// 掉落特殊物品变化概率的控制
	WORD    GetDropItemBaseRand(void)   {   return  m_DropItemBaseRand;                         }
	void    InitDropItemBaseRand(void);
	void    RdcDropItemBaseRand(void);

	void    DoAddDoctorBuff( CPlayer* pDoctor, bool useItem );
	void    UpdateWhineBuffTime( bool loginCheck = false);
	BOOL    AddWhineBuff(WORD buffType);

	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);


	DWORD   GetSpouseSID(void);
private:
	// 陷阱触发器相关数据
	WORD    m_PrevTrapID;

public:
	// !!!!!!!!!!
	std::map<DWORD, DWORD> m_PkMap;      // PK对象映射表
	std::list< DWORD > m_LookMeList;
	DWORD              m_dwLookGID;
	DWORD              m_dwLookStartTime;
	DWORD              m_dwLookDisable;

	// ==== 临时道具==== 
	struct tempAddItem
	{
		DWORD dwItemCoolingTime; // 冷却时间
		BYTE  itemBuf[68];
	};
	std::list<tempAddItem> m_tempItem;
	void OnClearTempItem();
	void UpdateTempItemToClient( BOOL nullUpdate = false );
	// =================

	bool m_bInit;
	long m_DisWaitTime;		// 断线等待时间，frame
	
//	bool m_isLockedTarget;  // 是否处于攻击锁定状态

	DWORD m_dwSaveTime;     // 保存数据的时间
	DWORD m_dwHalfHourTime; // 半小时一次的时间
	DWORD m_dwOneHourTime;

	// ################防沉迷系统################

	/*1、根据2010年8月1日实施的《网络游戏管理暂行办法》，网络游戏用户需使用有效身份证件进行实名注册。

		2、保护未成年人身心健康，未满18岁的用户将受到防沉迷系统的限制：
		游戏过程，会提示您的累计在线时间。
		累计游戏时间超过3小时，游戏收益（经验，金钱）减半。
		累计游戏时间超过5小时，游戏收益为0。

	*/
	void NotifyTimeLimit( DWORD m_OnlineTime );
	void InitTimeLimit( DWORD limitedState, DWORD online, DWORD offline );
	void OnCheckTimeLimit(DWORD dwCurTick);
	void ShowLimitTips();//对纳入防沉迷的玩家进行提示

	DWORD getShowStep      ()         { return m_bStrShowStep; } 
	void  putShowStep      ( DWORD v ) { m_bStrShowStep = (BYTE)v; }
	__declspec( property( get = getShowStep, put = putShowStep ) ) DWORD showStep;         // 表示提示已经进行到哪个阶段了!

	DWORD m_OnlineTime;       // 沉迷时间!!!
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
		LIMITED_NULL,			//未纳入防沉迷
		LIMITED_ALREADY,	//已加入防沉迷
		LIMITED_HALF,			//收益减半
		LIMITED_ZERO,			//收益为0
	};
	// ##########################################

	// 账号相关数据
	std::string m_szAccount;
	std::string m_szAccount_without_prefix;
	std::string m_szAccount_lv1_prefix;
	std::string m_szAccount_lv2_prefix;

	enum ChatType { CT_SLOW, CT_NORMAL, CT_FAST ,CT_GMVEERYSLOW};

	DWORD m_dwChatSkipTime[sizeof(ChatType)+1];   // 刷屏检测时间

	DWORD   m_dwExtraState;     // 玩家的一些附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04

	unsigned    int m_iCurerGID;
	DWORD   m_dwCureDAMTime;    // 治疗手太阳时间 初始化=0
	DWORD   m_dwCurePOWTime;    // 治疗手少阳时间 初始化=0
	DWORD   m_dwCureDEFTime;    // 治疗足太阴时间 初始化=0
	DWORD   m_dwCureAGITime;    // 治疗足少阴时间 初始化=0

	WORD    m_wMonsterCountOnGoat;  // 套装替身杀死怪物的计数
	WORD    m_wGoatMutateID;        // 当前替身变身的套装编号(1-11)
	DWORD   m_dwAutoAddMCountTime;  // 自动增涨（每1.2分钟涨1%的剑气条）累计时间
	WORD    m_wDoctorBuffTime;     // 医德加经脉的持续时间

	WORD    m_wDoctorDefBuff;   // 医者防御提升BUFF
	WORD    m_wDoctorDamBuff;   // 医者外功提升BUFF
	WORD    m_wDoctorPowBuff;   // 医者内功提升BUFF
	WORD    m_wDoctorAgiBuff;   // 医者身法提升BUFF

	BYTE    m_byDoctorDefBUFF;  // 未启用的def BUFF
	BYTE    m_byDoctorDamBUFF;  // 未启用的dam BUFF
	BYTE    m_byDoctorPowBUFF;  // 未启用的pow BUFF
	BYTE    m_byDoctorAgiBUFF;  // 未启用的agi BUFF

	DWORD   m_dwDoctorGID;      // 医德BUFF医生的GID

	WORD    m_wCheckID;

	WORD    m_wWhineBUFFType;		// 桃酒BUFF类型
	WORD    m_wWhineBUFFData[6];    // 桃酒所加BUFF数据 0:外功 1：内功 2：防御 3：身法 4：血量 5：杀气

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

	// （怪物<----->任务旗标）
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKKILL;
	TASKKILL m_KillTask;

	// （道具<----->任务旗标）
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKITEM;
	TASKITEM m_ItemTask;

	// （道具<----->任务旗标）
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKUSEITEM;
	TASKUSEITEM m_UseItemTask;

	// （道具<----->任务旗标）
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKGIVEITEM;
	TASKGIVEITEM m_GiveItemTask;

	// （道具<----->任务旗标）
	typedef std::hash_map<DWORD, std::list<DWORD> > TASKGETITEM;
	TASKGETITEM	m_GetItemTask;

	// （地图<----->任务旗标）
	typedef std::hash_map<WORD, std::list<DWORD> > TASKMAP;
	TASKMAP m_MapTask;

	// （关卡<----->任务旗标）
	typedef std::hash_map<WORD, std::list<DWORD> > TASKSCENE;
	TASKSCENE m_SceneTask;


	// （新增<----->任务旗标）
	typedef std::hash_map<WORD, std::list<DWORD> > TASKADD;
	TASKADD m_AddTask;

public:
	void DeleteTaskItemMap(WORD taskID);									// 删除一个任务的所有旗标和以上表的映射
	void DeleteTaskFlagMap(const STaskFlag& flag);							// 删除一个旗标和以上表的映射
	void SendUpdateTaskFlagMsg(STaskFlag &flag, bool updateStatus = true);	// 发送更新旗标消息
	int  GetTaskStatus(WORD TaskID);										// 获取任务状态
	void OnTimeLimitTask();
	void DeleteCycData(WORD taskID,BYTE flag = 0);										// 清空2个数组的数据而且是只有一个任务的情况
	void UpDataCycData();													// 更新循环数据
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

		timestruct.wYear      = year;				//年
		timestruct.wMonth     = month;				//月
		timestruct.wDay       = day;				//日
		timestruct.wHour      = hour;				//小时
		timestruct.wMinute    = minute;			//分
		timestruct.wSecond    = second;			//秒
		timestruct.wMilliseconds = 0;
		timestruct.wDayOfWeek    = week;
	}
	inline bool IsCompleteDayTime(const std::string & OverTime)
	{
		SYSTEMTIME curtimeTick;//当前时间
		GetLocalTime(&curtimeTick);

		SYSTEMTIME ctimestruct;//这个任务完成的时间
		GetSysTimeStruct(OverTime,ctimestruct);
		if (curtimeTick.wYear > ctimestruct.wYear)//如果大于年可以满足日常
		{
			return true;
		}

		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth > ctimestruct.wMonth)//同一年 并且大于月
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
		SYSTEMTIME ctimestruct;//这个任务完成的时间
		GetSysTimeStruct(OverTime,ctimestruct);

		SYSTEMTIME curtimeTick;//当前时间
		GetLocalTime(&curtimeTick);
		long tk = GetDiffDays(ctimestruct,curtimeTick);
		WORD dayofweek = curtimeTick.wDayOfWeek;//星期几

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
		SYSTEMTIME ctimestruct;//这个任务完成的时间
		GetSysTimeStruct(OverTime,ctimestruct);

		SYSTEMTIME curtimeTick;//当前时间
		GetLocalTime(&curtimeTick);
		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth == ctimestruct.wMonth)//同年同月
		{
			return curtimeTick.wDay - ctimestruct.wDay;
		}
		if (curtimeTick.wYear == ctimestruct.wYear && curtimeTick.wMonth > ctimestruct.wMonth)//同年不同月
		{
			return GetDiffDays(ctimestruct,curtimeTick) + 1;
		}
		if (curtimeTick.wYear > ctimestruct.wYear)//不同年
		{
			return GetDiffDays(ctimestruct,curtimeTick) + 1;
		}

		return 0;
	}
	void UpDataPlayerRdTaskInfo();//更新随机日常任务的情况
	void InsertPlayerRdTaskInfo(WORD NpcID,WORD taskID);
	WORD GetRdTasksNum(WORD NpcID);
	WORD GetCurrentNpcTaskID(WORD NpcID);
	void RecvRdData(WORD taskID);
	WORD GetRdNpcNum();
	void UpDataPlayerXKL();//更新侠客令
	long OnXKLProcess();
	void AddXKLUseNum(short num);
	short GetXKLUseNum();
	void OnHandleCycTask(WORD npcID);
	WORD GetHeadIDByStr(std::string  headHail);
	void PrintXKLLOG(WORD num,long ItemID);
	void OnPlayerBlessTime();
	long GetPlayerMoney(long type) const;
	void ChangeMoney(long type,long value);	
	long GetPlayerAllMoney() const; // 获取该玩家身上所有的钱
	bool ReducedCommonMoney( long lMoneyValue, TPlayerMoneyType ePriorityReduced = EMoneyType_Bind); // [扣钱-绑定和非绑定钱都可以扣取。注意不能加钱]
	void UpdataBless();//更新祈福
	void SendPlayerBlessOpen();
	//日常副本记录玩家数据
	long OnPartEnterNum(DWORD RegionID,BYTE numLimit);//进入副本成功的时候更新数据
	long OnTeamPartEnterNum(DWORD RegionID,BYTE numLimit);//进入副本成功的时候更新数据
	long IsPersonalEnterPart(DWORD RegionID);
	long IsTeamEnterPart(DWORD RegionID);
	void UpdataPartEnterNum();
	DWORD m_PartDyRegionID;		//进入副本的上一次副本ID
	DWORD m_PartDyRegionGID;
	long IsNonTeamPartEnter(DWORD RegionGid);
	//
	std::string m_ServerRelationName;
	long m_CountTA;
	std::vector<TemplateSkillInfo> m_TemplateSkillInfo;
	long m_ClickTaskFlag; //0是直接点击NPC 1是交接任务
public:
	// 帮派相关
	SimFactionData::SimMemberInfo m_stFacRight;
	//BOOL  m_bisFaction;	// 是否是帮主
	DWORD m_dwFactionTitleID;
	void  UpdateFaction();
	void  UpdateMemberData( SimFactionData::SimMemberInfo *member, LPCSTR factionName, WORD factionId, BOOL doSynchro );
	// 记录当前位置 以便重新回来
	WORD m_wBackRegionID;
	WORD m_wBackPosX;
	WORD m_wBackPosY;

	//20150126 wk 三国属性改了位置
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

	// 第一次进入游戏时必要的初始化动作！
	BOOL FirstLoginInit();

	// 挂机时进入游戏时必要的初始化动作！
	BOOL HangupLoginInit();

	// 客户端还没有任何数据的时候，发送所有必要的消息！
	BOOL SendInitMsgs();

	void RecoverTask();

public:
	BOOL isWaitingLockedResult( BOOL useAutoUnlock, BOOL showDialog );

private:
	SQAutoUseItemMsg m_stAutoUseItemSet;								// 自动吃药设置
	WORD             m_dUpXMValTimes;                                   // 更新心魔值次数 20次一秒
	bool             m_bHaveXMBuff;                                     // 是否有心魔BUFF

public:
	WORD selected_item_x;
	WORD selected_item_y;

public:
	DWORD	temporaryVerifier;		// 效验变量

	// 和ATM机相关的脚本，用于在操作结束执行脚本回调
	// 如果存在脚本回调，则该栈底的变量必定是脚本函数名（字符串），其余的变量则为该函数的参数
	DWORD                       ATM_temporaryVerifier;      // 效验变量
	std::stack< lite::Variant > ATM_temporaryScriptVariant;

	// 发布的远程交易信息
	std::map< DWORD, time_t >    excInfoMap; // 扩展交易信息映射表，里边记录了发布的道具和最后发布的时间

	// 和建筑系统相关校验数据, 避免玩家进行数据修改!
	DWORD                       buildVerify;      // 效验变量

	// 杀怪任务关联的怪物映射表( key=怪物名字,value=list<任务ID> )
	// 这个不保存数据库,在玩家加载数据的时候,需要赋值一下
	std::map< std::string, std::list<DWORD> > taskKillMonsters;

public:
	// 时间检测器，用于检测连点次数（加速器检测在客户端）
	ClickLimitChecker           clickLimitChecker;

	//扩展心法相关
	void    UpdateExtraTelerge();
	void    OnExtraTelergyModify( struct SQSetExtraTelergyMsg *pMsg );
	void    OnOpenExtraTelergy( struct SQOpenExtraTelergyMsg *pMsg );
	// 背包整理
	void    ArrangePackage(WORD type);

	// 动态仓库和背包
	//WORD    packageId[3];         // 
	//WORD    goodNumber[3];        // 0:背包容量 1 & 2：仓库容量

	// 设置转盘的结束点
	BYTE            dialEndIndex;

	STaskExBuffer   taskEx;             // 为新脚本系统准备的数据空间
	BOOL            isErrPlayer;        // 是否为外挂玩家
	BOOL            isAutoFight;        // 是否正在使用自动战斗功能
	DWORD           timeOutChk;         // 用于收取外挂信息的超时处理逻辑判断。。。
	BOOL            isUseRose;

	SPackageItem    *m_pItemXQD;        // 用于快速查找血气丹
	SPackageItem    *m_pItemXTBL;       // 用于快速查找玄天宝录
	SPackageItem    *m_pItemLZ[6];      // 用于快速查找火水木金土混灵珠
	int             m_nLZEffectVal[6];  // 灵珠的属性值，暂时记录在这里

	void    UseItemXTBL();              // 使用玄天宝录

	// GM权限验证相关
	void    SendGetGMIDInf();           // 发送获取GM身份信息请求
	struct  GMIDCheckInf
	{
		DWORD   dwRand;                 // 随机数
		SQGameMngMsg SQGMMsg;           // GM消息
	};

	GMIDCheckInf m_GMIDCheckInf;

	// 更新医德点
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

	// 心魔相关
	void    UpdateXinMoVaule();
	void    SendUpdateXMVauleMsg();
	void    AddXinMoBuff();

	// 由于每天更新的玩家数据的类型不同，将此函数更改为模板函数，保证数据安全，防止覆盖数据的情况出现
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
	

	DWORD GetSkillProficiency(DWORD dSkillID);                  // 获取武功熟练度
	bool AddSkillProficiency(DWORD dSkillID, int iAddVal);// 提升武功熟练度

	// 以前的状态是技能攻击状态，在技能攻击状态被打断切换到另外的状态的时候，
	virtual void OnChangeState(EActionState newActionID);
	virtual void OnRunEnd();

	void Move(INT32 moveTileX,INT32 moveTileY);

	void OnDamage(INT32 *nDamage, CFightObject *pFighter);

	void UpdateSKillBox(SUpdateSkillBox_C2S_MsgBody* msg);
	void UpdatePracticeBox(SUpdatePracticeBox_C2S_MsgBody * msg);

	void ProcessForceQusetEnemyInfo();

	// 消耗计算
	virtual bool ConsumeHP(int consume);
	virtual bool ConsumeMP(int consume);
	virtual	bool ConsumeTP(int consume);
	virtual SCC_RESULT CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP);
	WORD GetMpConsume(BYTE type, WORD baseMPConsume,WORD telergyLevel);
	// 内部测试接口，不要使用
	void _SetPropertyTemp(int type, int data);

	//增加一个列表：保存目前选中自己的玩家编号
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

public://锁定
	// 保存目标全局ID
	void StoreTarget(CFightObject* pObj);
	void CALLBACK OnCloseListener(CFightListener* p);

public:
	CFightListener *m_pAttribListener;	

protected:
	virtual BOOL _UseItem(SPackageItem *pItem,DWORD useType=0);
public://坐骑,坐骑表在基类中定义
	int 	GetEquipMount( );									//得到当前装备的坐骑索引
	int 	GetRidingMount( );									//等到当前骑乘的坐骑索引
	void	InitMounts( SPlayerMounts* pFixMounts );			//初始化一个坐骑,并添加到角色坐骑表中,
	bool	CreateMounts( int mountId, BYTE quality );			//创建一个指定ID,指定品质的坐骑
	bool	CreatePets(int petId, BYTE quality);				//创建一个指定ID,指定品质的宠物
	bool	CreateFightPet(WORD fpID);							//创建一个指定ID的侠客
	void	EquipMount(BYTE mountIndex);						//装备坐骑,指定坐骑索引
	void	UnEquipMount(BYTE mountIndex);						//取消装备坐骑
	void	DeleteMounts(BYTE mountIndex );						//删除坐骑
	void	DeletePets(BYTE petIndex);							//删除宠物
	void	DeleteFightPet(BYTE fpIndex);						//删除侠客
	SPlayerMounts::Mounts*	GetMounts( int index );				//通过索引得到坐骑属性
	SPlayerPets::Pets* GetPets(int index);						//获取宠物属性

	//上马
	void	PreUpMounts(BYTE mountIndex);						//准备上马
	void	UpMounts( int mountIndex );							//上马
	void	DownMounts( );										//下马
	
	void	OnMountsMsg(SMountsMsg* pMsg);						//处理坐骑消息
	void	UseMountSkill(short mountIndex,short skillIdx);		//使用坐骑技能
	void	FeedMounts( struct SQFeedMounts* pMsg);				//给坐骑喂食
	void	AddMountPoint(SQMountAddPoint *msg);
	
	void	SkillNumUpdate();									//宠物技能熟练度更新 暂时留接口
	void	InitMountSkillColdTime();							//初始化坐骑技能冷却时间
	void	ReleaseMountSkillCD();								//释放坐骑技能冷却时间对象				

	DWORD	m_RidingStatus;										// 骑马的状态，如果为0，表示没有上马
	BYTE			m_InRidingevens;										//在骑马读条的状态	
	BYTE			m_InUseItemevens;										//在道具读条的状态

	std::map<int,EventMgr::EventBase*> m_MountSkillColdTimeTbl; //坐骑技能冷却时间表.

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

	//add by 2014/3/25 初始化称号附加属性
	void InitTitleData();
	
	void OnShareExpWhenJingMaiOpenVenSuccess(WORD wXueID); // /// 当经脉充穴成功的经验分享

public:
	inline BOOL IsInStall()		// 是否处于摆摊状态
	{
	//	return m_Property.m_ShowState & PSE_STALL_SALE;
		return FALSE;
	}

public:	//挂机
	bool	m_bIsStarPractice;	//表示挂机是否启动
	DWORD	m_lStarTime;		//开始时间
	DWORD	m_lEndTime;			//结束时间
	WORD	m_wCurSkillLevel;
	
	DWORD	m_dwSkillProNum;	//记录普通武功的熟练度增长
	void	OnGetPracticeRequest(SAPracRequesttMsg * pMsg);//得到请求
	void	SendPracticeResult(bool bSuccess,WORD wType);	//发送请求结果
	bool	CheckPlayPracticeCondition(DWORD dwGID,WORD wType,WORD wID); //判断是否满足修炼条件：目前只有护体
	void	AccountPracticeResult(WORD wType,WORD wSKillID);	//计算修炼结果
	//void	UpDatePracticeBox(SUpdatePracticeBox_C2S_MsgBody *msg);
	WORD	GetCurPracticeSkillLevel(WORD wType,WORD wID);
	DWORD	GetSkillProForMP(WORD wType,WORD wSkillID,DWORD dwPro);	//通过玩家身上实际的蓝计算最终的熟练度
	SSkill*	GetSkillInfoforID(WORD wskillID);
	bool	CheckBoxInfo(WORD wID,DWORD wstOldBox[]);
	void	LogOutUnPractice();			//离线挂机处理
	void	SendSkillPro(WORD wType,WORD wSkillID,DWORD dwPro);			// 同步技能熟练度
	void	SendPlayerPracticeResult();
	void	UnPractice();
	void	SendSynPlayMount(WORD index, BYTE level, BYTE bAction);		// 骑马补充

public:
	enum COPYSCENE_STATE
	{
		COPYSCENS_NONE,		// 没有副本任务
		COPYSCENS_TASK,		// 接受副本任务
		COPYSCENE_PREPARE,	// 准备进入副本
		COPYSCENE_IN,		// 副本中
		COPYSCENE_LOCK,		// 副本中锁定
	};
	void SetCopySceneState( COPYSCENE_STATE eState){ m_nCopySceneStata = eState;}
	COPYSCENE_STATE GetCopySceneState(){ return m_nCopySceneStata;}

private:
	COPYSCENE_STATE m_nCopySceneStata;

//-------------------------------------------------侠客------------------------------------//
public:
	BOOL CanUpMount(BYTE mountIndex);
	void CallOutFightPet(BYTE index);
	void CallOutFightPet(SQCallOutFightPet *pmsg);
	void CallBackFightPet(bool IsTemp = true);//是否从场景中移除
	void ShowPet(BYTE index);	// 显示索引为index的宠物
	void CallBackPet();			// 回收当前的宠物
	void CloseGroundItem();		// 接触查看包裹的映射
	void MoveFightPet(SQFightPetMoveto *pmsg); //瞬移侠客

	void _fpUpdateExtraProperties(BYTE changetype,BYTE index);//更新侠客额外的属性变化
	void _fpChangeExtraProperties(BYTE changetype,int changevalue,BYTE index);//更改侠客额外的属性变化

	void _fpMakeItem(BYTE index);//侠客转换为道具
	BOOL _fpItemTofp(SPackageItem *pItem);//道具变成侠客
	void _fpChangeAttType(SQfpchangeatttypeMsg *msg);//切换当前出战的侠客的攻击模式
	//侠客的装备属性变化
	void _fpInitEquipJewelData(int EquipIndex);
	void _fpInitEquipAttribute(const SItemFactorData* pItemFactorData);
	void _fpInitEquipRandAttriData(int EquipIndex);
	void _fpInitEquipmentData();
	///设置需要装备道具的侠客索引
	BOOL _fpSetEquipFightPet(BYTE index);
	///清除需要装备道具的侠客索引
	void _fpInitEquipFightPetIndex(){m_nEquipfpIndex = -1;}
	int	_fpgetEquipFightPetIndex(){return m_nEquipfpIndex;}
	///侠客属性更新
	void _fpUpdateAllProperties(int index = -1);
	SFightPetExt *_fpGetFightPetByIndex(BYTE index);//根据索引获取侠客
	void _fpUpdateProperties(BYTE type,WORD propertyvalue,BYTE index);
	void _fpUpdateProperties();
	//侠客装备技能书
	BOOL _fpRecvEquipSkillBookMsg(BYTE index,SPackageItem *pItem);

	bool _fpIsEquipSkillItem(DWORD itemid);//是否为可以装备的侠客技能书

	int   lua_fpGetItemToSkillID(DWORD itemid);//根据当前所装备的技能书返回相对的技能ID
	int   lua_fpGetItemToSkillLevel(DWORD itemid);//根据当前所装备的技能书返回相对的技能等级

	int  _fpGetSkillSlot(WORD wSkillID,BYTE index);//获取侠客技能位置，如果为-1是没有学会这个技能,-2没有找到这个侠客
	int _fpLearnSkillByfp(SQfpSkillUpdate *pMsg);//从其他侠客身上学习技能

	int _fpGetSkillNum(BYTE index);//获取侠客已经学习的技能数量

	bool _fpIsInfightPet(BYTE index);//是否是出战中的侠客

	bool IsInJump();
	
	///获取当前动态场景ID，为0是普通场景
	//DWORD GetDynamicRegionID();
	//激励侠客
	void ImpelFightPet(SQFightPetImpel *PetImpelmsg);
	void GetImpelValueOntime(DWORD dwCurTick);//根据在线时间获取激励值,包含侠客的一些根据时间改变的东西
	void ClearMetrialRelation();
	DWORD	m_nImpelFightPetTime;	// 侠客激励值获取时间
	WORD	m_ImpelValue;				// 侠客激励值
	DWORD	m_nRestoreTiredTime[MAX_FIGHTPET_NUM];		//侠客回复疲劳时间
	DWORD	m_nfpLearnSkillTime;				//侠客开始学习技能的时间
	DWORD	m_nchecklivetime;						//用于检测玩家是否在线的记录
	//BYTE			m_fpAtkType;						//侠客攻击模式
public:	
	WORD  m_PetStatus;			// 当前的宠物状态(宠物编号// [2012-7-27 16-35 gw: +])，0表示没有 
	DWORD m_PetGID;				// 宠物的唯一ID

	// 侠义道3道具升级系统	
	SPackageItem	*m_UpdateItem;	
	SPackageItem	*m_LJS_Metrial;		// 炼制石	
	SPackageItem	*m_XMTJ_Metrial;// 玄梦天精	
	BYTE			m_UpdateItemType;	

	BYTE			m_MaxUpdateTimes;	// 回神丹最大升级次数
	BYTE			m_MaxUseTimes;		// 回神丹最大使用次数

	// 侠义道3装备锻造系统
	//SPackageItem	*m_forgeItem;			// 当前被锻造的装备
	SPackageItem	*m_forgeMetrial;		// 原材料
	BYTE			m_forgeType;			// 拖宝石
	DWORD			m_MakeHoleTime;			// 开始打孔的时间

	// 装备强化系统，包括升级跟精炼
	// 精炼（升星、升阶），装备、原材料、概率材料，其中升阶还有保护材料
	// 升级（升品质、升级）	
	SPackageItem				*m_strengthenMetrial;			// 强化原材料
	std::vector<WORD>			m_vecStrengthenEquip;			// 强化装备位置列表，记录位置更方便一些	
	std::vector<WORD>			m_vecStrengthenEquipAssist;		// 强化辅助格子
	std::vector<WORD>			m_vecStrengthenMetrial;			// 强化原材料列表
	
	BYTE						m_refineType;					// 强化类型
	DWORD						m_MetrialItemID;	// 材料道具ID
	// 洗点材料
	SEquipment					*m_pIdentifyEquipBackUpNeedDel;		// 洗练生成的新装备临时备份，如果不为0需要手动删除

	/***********************************
	 * XYD3套装属性加成相关变量
	 ***********************************/
	WORD						m_ModifyTypeMark;				// 需要修改的套装属性标记

	// 临时保存套装属性的值，使更换装备时候的属性值保持一致性
	WORD						m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAX];	

	// 侠义道3交易支撑机制
	typedef std::map<DWORD, DWORD> CHXQuestList;
	CHXQuestList	m_ChxSentOut;				// 送出的请求
	CHXQuestList	m_ChxGetIn;					// 收到的请求

	// 侠义道3组队支撑机制
	typedef std::map<DWORD, std::pair<DWORD, BYTE> > TeamQuestList;
	typedef std::hash_map<DWORD, DWORD> SyncFindTeamList;
	typedef std::set<DWORD> SyncTeamQuestList;

	TeamQuestList		m_TeamSentOut;					// 送出的请求（用于取消）
	TeamQuestList		m_TeamGetIn;					// 收到的请求（用于拒绝/同意）

	DWORD				m_SendRequestID;				// 发出的组队请求编号
	SyncTeamQuestList	m_LastSynTeamRequestIDList;		// 同步给客户端的组队请求列表
	SyncFindTeamList	m_LastSynFindTeamIDList;		// 同步给客户端的队伍列表

	// 侠义道3包裹拾取摇号支撑机制
	typedef std::map<DWORD, WORD> OwnedChanceItems;
	OwnedChanceItems	m_OwnedChangeItems;				// 玩家需要摇号的列表

	SaledItemType	m_buyBackItem[CONST_BUYBACK_COUNT];	// 道具回购系统
	WORD			m_curSaledItemCount;				// 当前回购列表的个数
	CItem			*m_pCheckItem;						// 查看地上的包裹
	CSingleItem		*m_pSingleItem;						// 查看地上的包裹
	DWORD			m_dwFactionLimitTime;				// 限制聊天时间间隔
	DWORD			m_dwPublicLimitTime;
	DWORD			m_dwSchoolLimitTime;
	DWORD			m_dwTeamLimitTime;
	DWORD			m_dwRumorLimitTime;
	DWORD			m_dwWhisperLimitTime;
	DWORD			m_dwGlobalLimitTime;

	class CFightPet	*m_pFightPet;						// 当前正在出战的侠客
	DWORD			m_FightPetTimeLimit;				// 侠客出战时间限制

	BYTE			m_PreFPIndex;						// 用于解决侠客切换地图的问题

	// 侠义道3跳跃缓冲数据
	DWORD			m_StartJumpTime;					// 开始跳跃时间
	DWORD			m_LastingTime;						// 跳跃持续时间

	// 玩家的九字杀阵数量判断
	int				m_NineWordsKill;
	// 判断是否发送等级、心法升级消息
	BOOL			m_updateSkill;
	BOOL			m_updateTelergy;

	// 是否是麒麟臂伤害
	BOOL			m_bKrlinArmDamge;

	//在线时间，用于送新生礼包
	INT64 m_dwOnlineBeginTime;	//累计在线开始时间
	INT64 m_dwLoginTime; //当前的登陆时间。用于记录
	BOOL m_AddOnceTpFlag;	//每隔16分钟恢复玩家体力，当离线后登陆时，只计算一次离线时间增加玩家的TP值
	INT64 m_SecondOnceAddTp;	//每秒只能增加玩家的体力一次，因为在循环式，会出现时间相同的情况，造成多次执行
	//服务器属性模板属性,更改规则
	typedef std::map<short,long> TemplateMap;
	TemplateMap m_TemplateMap;

	BYTE		m_bFashionMode; //时装模式还是普通模式
	bool		m_bFirstLogIn;	// 第一次登陆全部计算心法属性
	BYTE		m_bTempLevel;	// 登陆标记心法等级

	WORD	m_bCurDeadreginID;		//当前死亡地图ID
	WORD	m_wCurDeadCount;		//在当前地图死亡次数
	WORD wReliveTime[4];
	WORD wReliveMoney[4];
	DWORD m_dUpdateState;			//用于检测客户端的更新状态，活动信息更新
	BYTE	m_HoleType;
	BYTE m_PlayerOpenClose;
	DWORD m_dLoginCheckTime;	//登录检测时间，用于异常时间检测
public://闭关奖励
	void OnRecvBiGuanMsg(SBiguanMsg* pMsg);
	void OnRecvBiGuanLingquMsg(SQBiguanLingquMsg* pMsg);
	void calculateSpAndExpByTime(INT64 sec,DWORD &sp,DWORD &exp);
public://体质成长
	bool tizhiUpgrade(DWORD itemId);		//升级体质
	void SendTiZhiUpgradeResult();
	bool tizhiGrow(DWORD itemId);			//增长体质，增加属性
	void SendTiZhiGrowResult();								
	void SendTiZhiData();					//向客户端发送体质数据
	void OnRecvTiZhiUpdateMsg(SQTiZhiUpdateMsg *pMsg);			
	void OnRecvTizhiTupoMsg(SQTiZhiTupoMsg *pMsg);
	void OnRecvKylinArmUpgradeMsg(SQKylinArmUpgradeMsg* pMsg);
public://麒麟臂
	bool activeKylinArm();
	bool kylinArmUpgrade();
	void SendKylinArmData();
	virtual void OnAttack(CFightObject *pFighter);
	void addPlayerYuanqi(WORD value);
	
public:
	void ShowChangeProperty(BYTE type, WORD value);
	void UpdatePlayerXwzMsg();//发送更新玩家修为值
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
	void _L_UpdateCountDownGiftState();			// 更新倒计时礼包领取的阶段
	int _L_GetCountDownGiftState();				// 得到倒计时奖励给与阶段
	BYTE _L_GetOnlineGiftState(int index);	// 得到当前应该领取的礼包
	void _L_UpdateOnlineGiftState(int index, BYTE state);
	void _L_ResetCountDownGiftState();			// 重置倒计时礼包领取的阶段
	void _L_ResetOnlineGiftState();
	void _L_ResetCountDownGiftTime();	//重置倒计时礼包时间
	INT64 _L_GetCountDownGiftBeginTime();//得到倒计时开始时间
	INT64 _L_GetOnlineBeginTime();			// 得到当前在线时间
	INT64 _L_GetNowTime();				// 得到当前时间
	void OnRecvQuestChangeFashion(SQFashionChangeMsg *pmsg);
	void OnClinetReady();				// 在客户端进度条读取完以后开始发送一些消息，现在为初始化调用一次脚本中消息
	void SentClinetGiftSucceed(BYTE bType,BYTE bResult, BYTE index = 0);
	void OnProcessDead();//处理玩家死亡
	void OnRecvQuestActivityNotice(SQBossDeadUpdatemsg* pMsg);
	float GetWalkSpeed();


	INT64 _L_GetLastLoginTime(){return m_Property.m_dLoginTime64;} //获取上次的登录游戏时间	
	INT64 _L_GetLastLeaveTime(){return m_Property.m_dLeaveTime64;}//获取上次的离线时间	
	INT64 _L_GetCurrentLoginTime(){return m_dwLoginTime;} //获取当前的登录游戏时间	

	void _L_SetPlayerVipLevel(BYTE level){m_Property.m_bVipLevel = level <= 10 ?  level : m_Property.m_bVipLevel;}
	void OnSetPlayerLevel(BYTE Level);

	bool _L_calculateSpAndExpByTime(INT64 sec,DWORD &sp,DWORD &exp);//新归元气计算，从lua中返回

	DWORD	m_PartprevRegID;			// 玩家之前的场景ID
	WORD	m_PartprevX;				// 坐标
	WORD	m_PartprevY;				
	WORD  m_XwzValue;			//玩家修为值
	WORD    m_CurFollowIndex;
public:
	void InitTelergyAndSkill();			// 心法初始化，这里不用存入数据库，直接在登陆的时候重新计算一次就OK了
	void OnRecvTaskMovemsg(SQtaskmove *pmsg);//请求任务传送到NPC

	////add by ly 2014/3/17  玩家升星处理
	void OnHandlePlayerRiseStar(struct SQXinYangRiseStarmsg *pMsg);

	//add by ly 2014/3/25
	void SendSAGloryMsg(struct SAGloryMsg *pMsg);		//发送荣耀的请求应答消息
	void GetPlayerGloryInfo(struct SQGloryMsg *pMsg);	//玩家荣耀请求处理
	BOOL JudgeCanGetGloryAward(const SGloryBaseData *lpGloryBaseData);		//判断是否满足获取荣耀奖励
	void GetGloryAward(struct SQGetGloryMsg *pMsg);		//获取荣耀奖励
	void OnHandleTitleMsg(struct SQTitleMsg *pMsg);		//处理称号消息

	//add by ly 2014/4/16
	void SendDailyMsgToPlayer(struct SAInitDailyMsg *pMsg);	//发送日常活动列表信息
	BOOL DispatchDailyMessage(struct SDailyMsg *pMsg);	//日常活动相关消息
	void OnHandleInitDailyMsg(struct SQInitDailyMsg *pMsg);	//初始化日常活动消息
	void OnHandleEntryDaily(struct SQEntryDailyMsg *pMsg);	//进入日常活动消息
	void OnHandleAwardDaily(struct SQDailyAwardMsg *pMsg);	//请求获取日常活动的奖励
	void OnHandleLeaveDaily(struct SQDailyLeaveMsg *pMsg);	//客户端请求离开日常活动场景
	void OnHandleGetFirstPayAward(struct SQFirstPayAwardMsg *pMsg);//玩家首次充值奖励
	void OnHandleGetEveryDayPayAward(struct SQEveryDayPayAwardMsg *pMsg); //玩家领取每日充值奖励
	void OnEveryDayPayAwardState(struct SQGetEveryDayAwardStateMsg* pMsg);//获取每日充值奖励状态

	void OnQuestJuLongShanRisk(struct SQJuLongShanRiskMsg *pMsg);	//玩家请求在巨龙山中探险

	void OnTurnOnTarotCard(struct SQTurnOnOneTarotCardMsg* pMsg);	//玩家请求翻开一张塔罗牌
	void OnGetTarotAward(struct SQGetTarotAwardMsg* pMsg);	//玩家请求塔罗牌奖
	void OnUptPlayAnimateFlag(struct SQUptTarotPlayAnimateFlagMsg* pMsg);	//更该塔罗牌播放动画标志


	void OnGetCurDayOnlineLong(struct SQCurDayOnlineLongMsg* pMsg); 		//玩家请求每日在线时长活动玩家当天在线时长
	void OnCurDayOnlineGetAward(struct SQCurDayOnlineGetAwardMsg* pMsg);	//获取对应时间段的奖励

	void OnQuestAddUpLoginDay(struct SQAddUpLoginDayMsg* pMsg);			//活动玩家当天累计登陆天数
	void OnAddUpLoginGetAward(struct SQAddUpLoginGetAwardMsg* pMsg);	//玩家领取累计登陆相应级别ID的奖励

	void OnGetRFBAward(struct SQRFBGetAwardMsg* pMsg); 	//领取充值回馈对应的奖励

	void OnGetLRAward(struct SQLRGetAwardMsg* pMsg);	//获取等级竞赛奖励

	void OnGetFWOpenResidueTime(struct SQFWOpenResidueTimeMsg* pMsg); 	//获取打开幸运摩天轮的开放剩余时间数据
	void OnStartFWGame(struct SQFWStartGameMsg* pMsg); 	//开始幸运摩天轮游戏
	void OnGetFWAward(struct SQFWGetAwardMsg* pMsg);	//获取幸运摩天轮奖励

	//add by ly 2014/4/28
	BOOL OnHandleGetSignInAward(struct SQGetSignInAwardMsg *pMsg);	//处理请求获取每日签到奖励消息

	//add by ly 2014/5/7 商城相关
	void DispatchShopMessage(SShopMsg* pMsg);	//商城相关信息处理
	void OnHandleQuestShopListOpt(struct SQShopListMsg* pMsg);		//处理玩家请求商城列表信息
	void OnHandleQuestGoodsListOpt(struct SQShopGoodsListMsg* pMsg);		//处理玩家请求商城中商品列表信息
	void OnHandleBuyGoods(struct SQBuyGoodsMsg* pMsg);				//处理玩家购买商品
	void OnHandleGetShopCountDown(struct SQGetShopCountDownMsg* pMsg);	//获取商城刷新倒计时
	void SendShopListInfo(struct SAShopListMsg* pData);	//发送商城数据
	void SendGoodsListInfo(struct SAShopGoodsListMsg* pData);	//发送指定商城的商品数据
	void SendBuyGoodsResult(struct SABuyGoodsMsg* pData);		//发送购买操作结果
	void SendNotityGoodsUpdated(struct SANotifySpecialGoodsUpdateMsg* pData);	//通知有商城已经刷新
	void SendShopCountDown(struct SAGetShopCountDownMsg* pData);	//发送商城刷新倒计时
	void OnHandleFindGoodsByItemID(struct SQFindGoodsMsg* pMsg);	////请求获取商品信息索引通过道具的ID
	void SendSigleGoodsInfo(struct SAFindGoodsMsg* pData);	//发送单个物品的信息
	void OnHandlePlayerPay(struct SQPlayerPayMsg* pMsg);	//处理玩家充值金币消息

	//add by ly 2014/5/17 Vip相关
	void DispatchVipMessage(SVIPMsg *pMsg);	//Vip相关消息处理
	void OnHandleInitVipDataOpt(struct SQInitVipInfMsg *pMsg);//初始化VIP信息
	void OnHandleGetGiftInfOpt(struct SQGetVipGiftInfMsg *pMsg);//获取礼包信息
	void OnHandleGetGiftOpt(struct SQGetVipGiftMsg *pMsg);//获取VIP礼包
	void SendVipStateInfOpt(struct SAInitVipInfMsg *pData);		//发送玩家VIP的状态信息
	void SendGiftInfoOpt(struct SAGetVipGiftInfMsg *pData);	//发送礼包数据信息
	void SendGetOrBuyGiftResOpt(struct SAGetVipGiftMsg *pData);		//发送领取或获取VIP礼包结果信息

	//add by ly 2014 宠物相关
	void DispatchPetMessage(struct SPetMsg *pMsg);		//宠物相关消息处理
	void OnHandleCardExtract(struct SQExtractPetMsg *pMsg);		//处理玩家卡牌抽取操作
	void OnHandleChipCompose(struct SQChipComposPetMsg *pMsg);	//处理玩家碎片合成宠物操作
	void OnHandleGetCardPetInfo(struct SQGetCardPetInf *pMsg);	//处理获取卡牌宠物列表信息
	void OnHandlePetMerger(struct SQMergerPetMsg* pMsg);	//处理宠物吞噬
	void OnHandlePetBreach(struct SQPetBreachStarMsg* pMsg);	//处理宠物突破
	void OnHandlePetRename(struct SQRenamePetMsg* pMsg);	//处理宠物更改名字
	void SendExtractPetRes(struct SAExtractPetMsg* pData);		//发送抽取到的宠物数据
	void SendClipComposePetRes(struct SAChipComposPetMsg* pData);	//发送碎片合成的宠物数据
	void SendCardPetInfo(struct SAGetCardPetInf* pData);	//发送卡牌宠物列表信息
	BOOL UseItemGetPet(struct SPackageItem *pItem);		//使用道具获得宠物
	void SendUseItemGetPet(struct SAUsePetItem *pData);	//发送使用宠物道具获得的宠物数据
	void OnHandleGetPetCardInfo(struct SQGetPetCardInf *pMsg);	//处理获取宠物卡牌信息
	void SendPetCardInfo(struct SAGetPetCardInf *pData);	//发送宠物卡牌信息
	LPSNewPetData GetPetByID(DWORD PetID);	//获取宠物数据通过宠物ID
	LPSNewPetData GetPetByIndex(BYTE PetIndex);	//获取宠物数据通过宠物索引
	BOOL AddPet(LPSNewPetData pPetData);	//添加宠物
	BOOL DeletePet(BYTE PetIndex);		//删除指定ID的宠物
	void SendPetMergerRes(struct SAMergerPetMsg* pData);	//发送合并后的宠物数据
	void SendPetBreachRes(struct SAPetBreachStarMsg* pData);	//发送宠物突破结果数据
	void SendPetRenameRes(struct SARenamePetMsg* pData);	//发送宠物重命名是否成功
	void PetSkillUpdate(struct SQPetSkillUpgradeMsg* pMsg);		//宠物技能升级
	void OnHandleSkillConfig(struct SQPetSkillCfgInf* pMsg);	//宠物技能配置
	void OnHandlePetFollowFight(struct SQSuiShenFightMsg *pMsg);	//处理宠物跟随战斗
	void OnHandlePetShapeshiftFight(struct SQShapeshiftFightMsg *pMsg);	//处理变身宠物战斗
	BOOL PlayerTransferPet(BYTE PetIndex);	//玩家变身为宠物操作，保存玩家之前的属性并把宠物的的属性赋值给玩家，当宠物死亡后，变回玩家之前的状态
	BOOL TransferPlayer();		//变回玩家的普通状态，如果已经是普通形态就不用变身
	void OnHandleGetPetExpInf(struct SQPetExpInfMsg *pMsg);	//获取吞噬宠物可以获取的经验
	void OnHandleBuyPet(struct SQGlodDirectBuyPetMsg *pMsg);	//金币直接购买宠物
	void SendBuyPet(struct SAGlodDirectBuyPetMsg *pData);	//发送金币购买获得的宠物数据
	void GetResumeDurableNeedGlod(struct SQResumeNdGoldMsg *pMsg);	//获取恢复宠物魂力需要的金币数
	void ExcResumePetDurable(struct SQResumePetDurableMsg *pMsg);	//花费金币恢复宠物魂力
	void SynPetDurable(BYTE PetIndex);		//同步宠物魂力
	void CheckResumeFullPetHpAndMp();	//核对回复满宠物血量和内力
	void CheckResumeFullPlayerHpAndMp();//核对回复满玩家血量和内力
	void OnHandlePetStudySkill(struct SQPetStudySkillMsg *pMsg);	//处理宠物学习技能操作
	BOOL PetActiveSkill(BYTE PetIndex, WORD SkillID);		//宠物激活技能
	FightProperty m_PetFightPro;	//保存宠物战斗数据
	time_t m_RecordPreTransferTime;	//记录玩家上次变身时间
	time_t m_RecordPreDeadCTime;	//记录玩家死亡变身冷却时间
	BOOL m_SysAutoTransToPlayer;	//宠物魂力和血量为0时，会主动变身为玩家的原始形态，不用判断变身冷却时间是否满足条件
	DWORD m_TranPrePlayerHp;	//变身前宠物血量
	DWORD m_TranPrePlayerMP;

	//add by ly 2014/7/22 排行榜
	void OnHandleRandListMsg(struct SQPlayerRankList *pMsg);	//处理请求排行榜列表消息
	void SendRandListData(struct SAPlayerRankList *pData);		//发送排行榜列表数据
	void OnHandleGetRankAwardState(struct SQGetRankAwardState* pMsg);	//获取玩家排行榜领奖状态
	void OnHandleRankAwardOpt(struct SQGetRankAward* pMsg);	//请求领取奖励

	//add by ly 2014/7/24 活跃度
	void OnGetActivenessInf(struct SQGetActivenessInfo *pMsg);	//获取玩家活跃度信息
	void OnGetActivenessAward(struct SQGetActivenessAward *pMsg);	//领取活跃度的对应奖励
	void SendActivenessInf(struct SAGetActivenessInfo *pData);	//发送玩家活跃度信息


	void OnTreasureMsg(struct SQTreasureMsg *pMsg);//聚宝盆消息

	void OnInitPrayerMsg(struct SQInitPrayerMsg *pMsg); //祈福界面初始化
	void OnStartPrayerMsg(struct SQPrayerMsg *pMsg); //开始祈福
	void OnShopRefresh(struct SQShopRefresh *pMsg);//刷新商店
	void OnShopBusiness(struct SQShopBusiness *pMsg);//商店交易

	void OnRequestFactionScene(struct SQFactionSceneData *pMsg);//请求同步或开启军团副本
	void OnRequestFactionInfo(struct SQFactionSceneMapData *pMsg);//请求军团副本详细信息
	void OnChallengeScene(struct SQChallengeSceneMsg *pMsg);//请求挑战军团副本
	void OnChallengeFinish(struct SQSceneFinishMsg *pMsg);//军团副本挑战完成
	void OnRequestFactionSceneRank(struct SQSceneHurtRank *pMsg);//请求军团副本排行榜
	void OnInitFactionSalaryData(struct SQInitFactionSalary *pMsg);//请求俸禄数据
	void OnGetFactionSalary(struct SQGetFactionSalary *pMsg);//领取俸禄数据
	void OnShowNotice(struct SQFactionNotice *pMsg);//查看公告
	void OnModifyNotice(struct SQFModifyNotice *pMsg);//修改公告
	void OnShowoperateLog(struct SQFactionOperateLog *pMsg);//查看军团日志
	void OnSendFcEmailToAll(struct SQFcEmailToAll *pMsg);//发送军团邮件

	void OnGetGiftCode(struct SQGiftcodeActivity  *pMsg);//激活码
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	////2014/11/12 三国武将相关
	////////////////////////////////////////////////////////////////////////////////////////////////////
	///@brief 分配玩家相关
	void DispatchSGPlayer(struct SSGPlayerMsg* pMsg);
	
	///@brief 处理仓库相关
	void DsiapatchStorage(struct SSanguoItem * pMsg);

private:

	///@brief 设置玩家属性
	void OnSetPlayerAttr(struct SQSetAttrMsg *pMsg);

	void CheckTimeWhenLogin();	//检查并计算时间，当登录时
	void CheckTimeWhenLogout();
public:
	BatchReadyEvent m_BatchReadyEvent; // 处理添加到人身上的ReadyEvent

	//add by ly 2014/4/8
	BYTE m_IsUseKillMonster;	//是否使用玩家移动杀怪（因为怪物分批刷出，所以，杀怪时也分批杀死
	time_t m_EntryScenceTime;	//进入场景后的时间长度
	time_t pret;
	CRegion* m_pPreFightRegion;	//玩家上一个战斗场景

	//add by ly 2014/5/16
	//VIP属性加成
	WORD m_VIPFactor[VFT_MAX];


	//add by ly 2014/06/20 当宠物魂力为0时，在间隔时间达到后恢复满宠物的魂力值
	void CheckResumePetHunli();
};


inline void CPlayer::CloseGroundItem()
{
	//rfalse(2, 1, "正在取消查看一个【%s】的包裹", m_pCheckItem ? "关注中" : "已不关注");

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

