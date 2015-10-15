#pragma once

#include "NetModule.h"
#include "PlayerTypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 数值变化（升级）消息类
//=============================================================================================
DECLARE_MSG_MAP(SUpgradeMsg, SMessage, SMessage::EPRO_UPGRADE_MESSAGE)
//{{AFX
EPRO_RES_CHANGE,				// 资源（当前生命、内力、体力）属性（包括定时变化）
EPRO_EXP_CHANGE,				// 经验值变化
EPRO_MONEY_CHANGE,				// 金钱变化
EPRO_SET_POINT,					// 分配点数,客户端向服务器端
EPRO_BASEPOINT_CHANGE,			// 基本属性的变化（点数分配以后）
EPRO_ABILITY_CHANGE,			// 能力属性的变化
EPRO_XVALUE_CHANGE,				// 狭义值的变化，因为这个数据是整个场景的玩家都会看到的，所以单独处理
EPRO_MAXPROFIC_CHANGE,			// 武功熟练度最大值变化
EPRO_TETERGY_CHANGE,			// 心法数据的变化
EPRO_ACTSPEED_CHANGE,			// 动作速度的改变
EPRO_PASS_VENAPOINTASK,			// 请求打通穴道
EPRO_PASS_VENAPOINTOK,			// 打通穴道确认

EPRO_SET_EXTRATELERGY,			// 设置扩展心法
EPRO_UPDATE_NPCLIST,			// 更新NPC消息[当角色进入一个场景时，发送当前场景的所有NPC到客户端][用于自动寻人处理]
EPRO_OPEN_EXTRATELERGY,			// 打开34扩展心法
EPRO_LOCKRES_CHANGE,			// 发红蓝体经脉给锁定我的玩家
EPRO_LOCKRES_NAME,				// 目标玩家名字

EPRO_UPDATE_USE_POINTS,			// 更新客户端目前可以使用的点数
EPRO_UPDATE_STATE_PANEL_DATA,	// 更新客户端的状态面板的数据 EPRO_UPDATE_ATTACK_DEFENSE_VALUE,		
EPRO_QUEST_ADD_POINTS,			// 客户端请求加点
EPRO_BUFF_UPDATE,				// 更新BUFF信息（新建、更新、销毁）

EPRO_BUFF_PRO_SYN_TO_SELF,		// BUFF属性同步消息给自己
EPRO_BUFF_PRO_SYN_TO_SELECT,	// BUFF属性同步消息给选择了自己的
EPRO_BUFF_PRO_SYN_TO_ALL,		// BUFF属性同步消息给所有人
EPRO_BUFF_EFFECT_SYN_TO_ALL,	// BUFF特效同步消息给所有人

EPRO_FLY_CHANGE,
EPRO_PROTECTED_CHANGE,

EPRO_FP_BUFF,					// buff的启动和结束 通知客户端
EPRO_TELERGY_CHANGE,			// 心法变化

#if 0
EPRO_BUFF_USE_SUCCESS,
#endif

EPRO_FP_ATTRIB_CHANGE,
EPRO_DRUG_INFO_SHOW,

EPRO_BUFFFIGHTOUT_POS,			// BUFF影响的击退特效消息
EPRO_BUFF_SPLITMOVE,			// 瞬移效果消息
EPRO_LEVEL_CHANGE,				// 等级变化（资源属性最大值变化），包括分配点数、经验值
EPRO_TELERGY_UPDATE,			// 升级心法
//add 2014.3.1
EPRO_OPEN_SETSKILL,				//请求打开玩家的技能配置面板
EPRO_UPDATE_PLAYERSKILL,		//更新玩家当前所使用的技能

EPRO_SKILL_UPDATE,				// 更新技能信息
EPRO_SPEED_CHANGED,				// 速度更新消息
EPRO_ATKSPEED_CHANGED,			// 攻击速度更新消息
EPRO_PROPERTY_CHANGED,			// 属性更改统一通知
EPRO_PLAYER_DEAD,				// 玩家死亡
EPRO_PLAYER_RELIVERESULT,				// 玩家复活的结果
EPRO_REGION_SYN_TIME,			// 场景时间更新
EPRO_OPEN_VEN,					// 开始冲穴


EPRO_FIGHTPET_EXPCHANGE,				// 侠客经验值变化
EPRO_FIGHTPET_LEVELCHANGE,				// 侠客等级变化
EPRO_FIGHTPET_PROPERTYCHANGE,			// 侠客属性变化
EPRO_FIGHTPET_FIGHTPROPERTYCHANGE,		// 侠客战斗属性变化
EPRO_FIGHTPET_EXTRAPROPERTYCHANGE,		// 侠客额外属性变化
EPRO_FIGHTPET_SKILLUPDATE,				// 侠客更新技能信息
EPRO_ALL_PROPERTYCHANGED,				// 属性更改通知

EPRO_TIZHI_UPDATE,						//体质数据更新
EPRO_TIZHI_TUPO,						//体质突破
EPRO_KYLINARM_UPDATE,					//麒麟臂数据更新

EPRO_KYLINARM_UPGRADE,					//麒麟臂升级
EPRO_RESET_POINT,						// 洗点
EPRO_XWZ_UPDATE,						// 修为值更新

EPRO_PLAYERDEAD_ADD,				// 玩家死亡时的附近消息，仅发送给死亡的玩家自身

//add by ly 2014/3/25
//荣耀信息通过请求获得，客户端有这个数据。但是数据是lua序列化的数据，客户端没有解析
EPRO_PLAYERGETGLORY,	//获取玩家荣耀信息
EPRO_GETPLAYERGLORY,	//领取荣耀奖励
//称号信息不用请求，客户端已经有这个数据
EPRO_TITLE,	//称号信息

//add by ly 2014/5/16
EPRO_NOTITY_SKILLUPDATED,	//通知玩家，当前玩家的技能配置已经更新

//add by ly 2014/7/22
EPRO_QUEST_RANDLISTINF,		//请求排行榜数据
EPRO_RANKLIST_GETWARD,	//获取排行榜对应的奖励
EPRO_RANKLIST_GEAWARDSTATE,		//获取玩家排行榜奖励领取状态

//add by ly 2014/7/23
EPRO_ACTIVENESS_GETINFO,	//获取活跃度相关信息
EPRO_ACTIVENESS_GETAWARD,	//领取活跃度的对应奖励

//}}AFX
END_MSG_MAP()

//新加枚举	标识玩家吃药回复，吃的什么药，回复的什么状态   ——————xj
typedef enum
{
	TYPE_NULLDRUG = 0,
	TYPE_LIFE,
	TYPE_MAGIC,
	TYPE_STRENGTH,
}enDrug_Type;
//武功类型 ------xj	//最开始设定只是为了mainpanel中m_Box显示。现在修炼武功也用了这里的类型标示
enum BOX_TYPE
{
	TYPE_ITEM,
	TYPE_COMMONSKILL,
	TYPE_FLYSKILL,
	TYPE_PROSKILL,
	TYPE_TELSKILL,
	TYPE_MOUNTSKILL,
	//新类型加在前面
	TYPE_ALL,
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SKylinArmUpdateMsg,SUpgradeMsg,SUpgradeMsg::EPRO_KYLINARM_UPDATE)
struct SAKylinArmUpdateMsg : public SKylinArmUpdateMsg
{
	SPlayerKylinArm sData;
};
struct SQKylinArmUpdateMsg : public SKylinArmUpdateMsg
{

};
DECLARE_MSG(SKylinArmUpgradeMsg,SUpgradeMsg,SUpgradeMsg::EPRO_KYLINARM_UPGRADE)
struct SQKylinArmUpgradeMsg : public SKylinArmUpgradeMsg
{

};
struct SAKylinArmUpgradeMsg : public SKylinArmUpgradeMsg
{
	enum
	{
		SUCCESS,
		FAILED,
		NOTYUANQI,
		NOTLEVEL,
		NOTMONEY,
		NOTSP,
		NOTITEM,
		NOTICE,
	};
	BYTE byState;
};
DECLARE_MSG(STiZhiUpdateMsg,SUpgradeMsg,SUpgradeMsg::EPRO_TIZHI_UPDATE)
struct SATiZhiUpdateMsg : public STiZhiUpdateMsg
{
	SPlayerTiZhi tizhiData;
};
struct SQTiZhiUpdateMsg : public STiZhiUpdateMsg
{
	
};
DECLARE_MSG(STiZhiTupoMsg,SUpgradeMsg,SUpgradeMsg::EPRO_TIZHI_TUPO)
struct SQTiZhiTupoMsg : public STiZhiTupoMsg
{

};
struct SATiZhiTupoMsg : public STiZhiTupoMsg
{
	enum
	{
		SUCCESS,
		NOITEM,
		FAILED,
		NOTLEVEL,
		NOTSP,
		MAXLEVEL
	};
	BYTE byState;
};
DECLARE_MSG(SOpenVenMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_VEN)
struct SQOpenVenMsg : public SOpenVenMsg
{
	WORD wItemPos;		// 材料的坐标
	WORD wVenID;		// 穴位的ID
	byte bNum;          //材料的数目
};

struct SAOpenVenMsg : public SOpenVenMsg
{
	enum
	{
		SOV_SUCCESS,
		SOV_FAILED,
	};

	BYTE bResult;		// 结果
	WORD wVenID;		// 穴位ID
};

DECLARE_MSG(SPlayerDeadMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYER_DEAD)
struct SAPlayerDeadMsg : public SPlayerDeadMsg
{
	DWORD dwGID;		// 死者的ID
	DWORD killGID;		//攻击者GID
};

struct SQPlayerDeadMsg : public SPlayerDeadMsg
{
	enum
	{
		PDM_BACK_CITY,					// 回城复活
		PDM_HERE_FREE,					// 免费原地复活
		PDM_HERE_PAY,					//收费原地复活
		PDM_HERE_FULLLIFE,			// 健康原地复活
	};
	
	BYTE bType;				// 复活选择
};
     
DECLARE_MSG(SPlayerDeadResultMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYER_RELIVERESULT)
struct SAPlayerDeadResultMsg :  public SPlayerDeadResultMsg
{
	enum
	{
		RELIVE_SUCCESS, //复活成功
		RELIVE_FAIL_TIME, //复活失败。没有达到需要等待的时间
		RELIVE_FAIL_MONEY, //复活失败。金钱不足
	};
	BYTE bResult;
};

DECLARE_MSG(SSpeedChanged, SUpgradeMsg, SUpgradeMsg::EPRO_SPEED_CHANGED)
struct SASpeedChanged : public SSpeedChanged
{
	DWORD dwGlobal;		// ID
	float fSpeed;		// 当前速度
};

DECLARE_MSG(SAtkSpeedChanged, SUpgradeMsg, SUpgradeMsg::EPRO_ATKSPEED_CHANGED)
struct SAAtkSpeedChanged : public SAtkSpeedChanged
{
	DWORD dwGlobal;		// ID
	float wAtkSpeed;	// 当前攻击速度
};

DECLARE_MSG(SPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_PROPERTY_CHANGED)
struct SAPropertyChange : public SPropertyChange
{
	enum { PROPERTY_BUFFSIZE = 512 };

	WORD	buffSize;	// 缓冲大小
	BYTE	PropertyData[PROPERTY_BUFFSIZE];
};

DECLARE_MSG(_SUpdateBuff, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_UPDATE)
struct SUpdateBuff : public _SUpdateBuff 
{
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
	DWORD	dwBuffID;			// ID
	DWORD	dwMargin;			// 间隔
	DWORD	dwWillActTimes;		// 要激活的次数
	int		m_curStep;			// 当前阶段（0，-1，-2，正常值）
	float		wReginBuffHeight;//区域长度（如果为圆形就配置为区域半径）
	float		wReginBuffWidth;//区域宽度（如果为圆形将不使用这个参数）
	float    fCenterPosX;  //圆心坐标
	float    fCenterPosY;
	BYTE	bType;				// 增益/损益/区域Buff
	BYTE bReginBuffShape;// 区域形状 0 圆形 1矩形
};

//=============================================================================================
// NPC列表
DECLARE_MSG(SUpdateNpcListMsg, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_NPCLIST)
struct  SAUpdateNpcListMsg : public SUpdateNpcListMsg
{
    enum
    {
        OPTION_REGION_BLOCK = 0x1,      // 表示场景存在阻挡
        OPTION_REGION_GJLIMIT
    };

	struct NpcInfo {
		int npcid;
		DWORD gid;
		WORD x, y;
		BYTE type;
		char name[CONST_USERNAME];
	};
	WORD num;
	WORD option;
	NpcInfo npcArray[1];
};

// （定时变化）资源属性
DECLARE_MSG(SResChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_RES_CHANGE)
struct SAResChangeMsg : public SResChangeMsg
{
    DWORD	dwGlobalID;         // 本次运行全局唯一标识符
    DWORD	dwCurHP;            // 当前新的生命值
    WORD	wCurMP;             // 当前新的内力值
    WORD	wCurSP;             // 当前新的体力值
};

DECLARE_MSG(SLockResNameMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LOCKRES_NAME )
struct SALockResNameMsg :  public SLockResNameMsg // 玩家点击目标的名字，用于跨服同步给观众
{
	char name[CONST_USERNAME];		// 锁定目标的名字
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// （定时变化）锁定我的玩家获取信息
DECLARE_MSG(SLockResChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LOCKRES_CHANGE )
struct SALockResChangeMsg :  public SLockResChangeMsg
{
    DWORD dwGlobalID;      // 本次运行全局唯一标识符
    BYTE byHP;            // 当前新的生命值百分比
    BYTE byMP;            // 当前新的内力值百分比
    BYTE bySP;            // 当前新的体力值百分比
    BYTE byVA;            // 四条经脉的排序序列
};

// 金钱变化
DECLARE_MSG(SMoneyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_MONEY_CHANGE)
struct SAMoneyChangeMsg : public SMoneyChangeMsg
{
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
	DWORD   dwBindMoney;		// 绑定货币
	DWORD   dwMoney;			// 非绑定货币
};

// 经验值变化
DECLARE_MSG(SExpChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_EXP_CHANGE)
struct SAExpChangeMsg : public SExpChangeMsg
{
    enum
    {
        EMPTY,
		GM,
		TASKFINISHED,			// 完成任务
        MONSTERDEAD,			// 怪物死亡
        TEAMSHARED,				// 组队加成
		FRIENDUPDATE,			//好友在线奖励
		BIGUANJIANGLI,			//闭关奖励
		JINGMAIOPENVENSHARE,	//经脉充穴经验分享
    };
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
	UINT64  qwCurGain;			// 本次获得的经验
    BYTE	byExpChangeType;	// 引起经验改变的类型
};


// 侠客经验值变化
DECLARE_MSG(SFightPetExpChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_EXPCHANGE)
struct SAFightPetExpChange : public SFightPetExpChange
{
	BYTE		m_index;					// 侠客索引
	enum
	{
		EMPTY,
		GM,
		TASKFINISHED,			// 完成任务
		MONSTERDEAD,			// 怪物死亡
		TEAMSHARED,				// 组队加成
	};
	UINT64  qwCurGain;			// 本次获得的经验
	BYTE	byExpChangeType;	// 引起经验改变的类型
};

// 侠客等级变化
DECLARE_MSG(SFightPetLevelChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_LEVELCHANGE)
struct SAFightPetLevelChange : public SFightPetLevelChange
{
	BYTE		m_index;					// 侠客索引
	BYTE		bLevel;	
	UINT64 m_curExp;					//当前经验
	UINT64 m_MaxExp;				//最大经验
};

// 侠客属性变化
DECLARE_MSG(SFightPetPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_PROPERTYCHANGE)
struct SAFightPetPropertyChange : public SFightPetPropertyChange
{
	BYTE			m_index;					// 侠客索引
	WORD			m_MaxHp;					// 最大生命
	WORD			m_MaxMp;					// 最大内力
	WORD			m_CurHp;					// 当前生命
	WORD			m_CurMp;					// 当前内力
	WORD			m_GongJi;					// 攻击
	WORD			m_FangYu;					// 防御
	BYTE			m_BaoJi;					// 暴击
	BYTE			m_ShanBi;					// 闪避
};	

// 侠客属性变化
DECLARE_MSG(SfpfightPropertyChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_FIGHTPROPERTYCHANGE)
struct SAfpfightPropertyChange : public SfpfightPropertyChange
{
	enum	FightProperty
	{
		FP_GONGJI	= 0x01,			// 攻击
		FP_FANGYU,		// 防御
		FP_BAOJI,		// 暴击
		FP_SHANBI,			// 闪避

		FP_MAX_HP	,		// 最大生命
		FP_MAX_MP	,			// 最大内力
		FP_MAX_TP		,		// 最大体力

		FP_CUR_HP		,		// 当前生命
		FP_CUR_MP		,		// 当前内力
		FP_CUR_TP		,		// 当前体力
		FP_MAXPROPERTY,
	};
	
	BYTE				m_index;						// 侠客索引
	BYTE			m_type;							//类型
	WORD			value;			//改变的值
};	
//// 侠义世界，由EPRO_UPDATE_STATE_PANEL_DATA同步
////=============================================================================================
//// 等级变化
//DECLARE_MSG(SLevelChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LEVEL_CHANGE)
//struct  SALevelChangeMsg    :   public SLevelChangeMsg
//{
//    DWORD   dwGlobalID;         // 本次运行全局唯一标识符
//    WORD    byLevel;            // 当前新的等级
//    DWORD   dwMaxHP;             // 当前最大生命值
//    WORD    wMaxMP;             // 当前最大内力值
//    WORD    wMaxSP;             // 当前最大体力值
//    WORD    wSparePoint;        // 当前剩下的分配点数
//    UINT64   dwCurExp;           // 当前新的经验值
//    UINT64   dwMaxExp;           // 当前新的经验值上限
//    BYTE    byAmuck;            // 杀气
//};
////---------------------------------------------------------------------------------------------

// 侠义道三 等级变化（手动升级）
DECLARE_MSG(SLevelChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_LEVEL_CHANGE)
struct  SQLevelChangeMsg    :   public SLevelChangeMsg
{
	DWORD   dwGlobalID;         // 本次运行全局唯一标识符
};

// 设置分配点数
DECLARE_MSG(SSetPointMsg, SUpgradeMsg, SUpgradeMsg::EPRO_SET_POINT)
struct SQSetPointMsg : public SSetPointMsg
{
	enum
	{
		SSP_GONGJI,
		SSP_FANGYU,
		SSP_QINGSHEN,
		SSP_JIANSHEN,
	};
	WORD wPoints[4];	// 分配天赋
};

//add by yuntao.liu
DECLARE_MSG(SUpdateUsePoints_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_USE_POINTS)
struct SUpdateUsePoints_S2C_MsgBody : public SUpdateUsePoints_S2C
{
	WORD mAttackLeftPoints;
	WORD mDefenseLeftPoints;
};

DECLARE_MSG(SUpdateStatePanelData_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_STATE_PANEL_DATA)
struct SUpdateStatePanelData_S2C_MsgBody : public SUpdateStatePanelData_S2C
{
	DWORD		dwGlobalID;			// 本次运行全局唯一标识符
	WORD		m_WaiGong;			// 外功
	WORD		m_NeiGong;			// 内功
	WORD		m_WuXing;			// 悟性
	WORD		m_TiZhi;			// 体质
	WORD		m_ShenFa;			// 身法
	WORD		m_PhyAtk;			// 物理攻击
	WORD		m_FpAtk;			// 内功攻击
	WORD		m_PhyDefence;		// 物理防御
	WORD		m_FpDefence;		// 内功防御
	WORD		m_PhyHit;			// 物理命中
	WORD		m_FpHit;			// 内功命中
	WORD		m_PhyEscape;		// 物理躲避
	WORD		m_FpEscape;			// 内功躲避
	WORD		m_OutCriticalAtt;	// 外功暴击
	WORD		m_InCriticalAtt;	// 内功暴击
	INT32		m_PhyEscapeFix;		// 物理躲避修正
	INT32		m_PhyHitFix;		// 物理命中修正
	INT32		m_PhyCriHitFix;		// 物理重击修正
	INT32		m_PhyDamageFix;		// 物理伤害修正
	INT32		m_FpEscapeFix;		// 内功躲避修正
	INT32		m_FpHitFix;			// 内功命中修正
	INT32		m_FpCriHitFix;		// 内功重击修正
	INT32		m_FpDamageFix;		// 内功伤害修正
	WORD		byLevel;			// 当前新的等级
	DWORD		dwMaxHp;			// 最大生命力
	WORD		dwMaxMp;			// 最大内力
	WORD		dwMaxSp;			// 最大真气
	QWORD		maxExp;				// 最大经验
	QWORD		iExp;				// 当前经验
};

DECLARE_MSG(SBuffSynPropertyToSelf_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_SELF)
struct SABuffSynPropertyToSelf_S2C : public SBuffSynPropertyToSelf_S2C
{
    DWORD		dwGlobalID;			// 本次运行全局唯一标识符
    DWORD		dwCurHP;			// 当前新的生命值
    WORD		wCurMP;				// 当前新的内力值
    WORD		wCurSP;				// 当前新的体力值
	float		fSpeed;				// 当前新的速度
    bool		bDead;              // 是否死亡
};

DECLARE_MSG(SBuffSynPropertyToSelect_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_SELECT)
struct SABuffSynPropertyToSelect_S2C : public SBuffSynPropertyToSelect_S2C
{
	DWORD       dwGlobalID;         // 本次运行全局唯一标识符
	bool        bDead;              // 是否死亡
	DWORD       dwCurHP;            // 当前新的生命值
	WORD        wCurMP;             // 当前新的内力值
	WORD        wCurSP;             // 当前新的体力值
	DWORD       dwMaxHP;	        // 当前最大生命值
	WORD        wMaxMP;	            // 当前最大内力值
	WORD        wMaxSP;	            // 当前最大真气值
	
	// buff 列表以及BUFF的时间信息后续

};

DECLARE_MSG(SBuffSynPropertyToAll_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_PRO_SYN_TO_ALL)
struct SABuffSynPropertyToAll_S2C : public SBuffSynPropertyToAll_S2C
{
	DWORD   dwGlobalID;             // 本次运行全局唯一标识符
	float	fSpeed;					// 当前的新的速度
	bool    bAction[12];		    // 动作限制,12个限制类型
	bool    bDead;                  // 是否死亡
};

DECLARE_MSG(SBuffSynEffectToAll_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_EFFECT_SYN_TO_ALL)
struct  SABuffSynEffectToAll_S2C : public SBuffSynEffectToAll_S2C
{
	DWORD	dwGlobalID;	            // 本次运行全局唯一标识符
	WORD	wBuffId;	            // buff id
	bool	bIsBegin;	            // 是否结束
	bool	bIsOnce;                // 是否是一次性BUFF
};

//end buff syn property

DECLARE_MSG(SQuestAddPoint_C2S, SUpgradeMsg, SUpgradeMsg::EPRO_QUEST_ADD_POINTS)
struct SQuestAddPoint_C2S_MsgBody : public SQuestAddPoint_C2S
{
//	WORD mQuestAddPoints[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 基本属性的变化（点数分配以后）
DECLARE_MSG(SBasePointChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BASEPOINT_CHANGE)
struct  SABasePointChangeMsg    :   public SBasePointChangeMsg
{
    DWORD dwGlobalID;        // 本次运行全局唯一标识符
    WORD wCurEN;             // 活力 （体质）
    WORD wCurST;             // 体魄 （强壮）
    WORD wCurIN;             // 悟性 （智慧）
    WORD wCurAG;             // 身法 （灵敏）
    WORD wCurLU;             // 气运 （运气）
    BYTE byPoint;            // 剩余分配点数
    short sXvalue;           // 侠义值 
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 能力属性的变化
DECLARE_MSG(SAbilityChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_ABILITY_CHANGE)
struct  SAAbilityChangeMsg : public SAbilityChangeMsg
{
    DWORD dwGlobalID;        // 本次运行全局唯一标识符
    WORD wCurDAM;            // 武功攻击力初始值：4位数内，和门派，装备，基础能力有关
    WORD wCurPOW;            // 内功攻击力初始值：4位数内
    WORD wCurDEF;            // 防御力初始值：4位数内
    WORD wCurAGI;            // 灵敏度初始值：4位数内
    BYTE byDocBuff[4];       // 医德BUFF点数 用于显示： 防御 外功 内功 身法
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 狭义值的变化，因为这个数据是整个场景的玩家都会看到的，所以单独处理
DECLARE_MSG(SXVauleChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_XVALUE_CHANGE)
struct  SAXVauleChangeMsg : public SXVauleChangeMsg
{
    DWORD dwGlobalID;         // 本次运行全局唯一标识符
    short sXvalue;            // 侠义值 
    BYTE byPKValue;          // PK值 
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 武功熟练度最大值变化
DECLARE_MSG(SMaxProficChange, SUpgradeMsg, SUpgradeMsg::EPRO_MAXPROFIC_CHANGE)
struct  SAMaxProficChange : public SMaxProficChange
{
    DWORD dwGlobalID;         // 本次运行全局唯一标识符
    DWORD dwMaxProfic[MAX_SKILLCOUNT];    // 武功熟练度最大值 
	DWORD dwCurPorfic[MAX_SKILLCOUNT];	// 武功熟练度
};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SSkillUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_SKILL_UPDATE)
struct SQSkillUpdate : public SSkillUpdate
{
	DWORD	dwGlobalID;		// 玩家标识
	//技能升级组
	WORD	wID;			// 要升技能的ID
	WORD    wID1;		//要升级技能ID1
	WORD	wID2;		//要升级技能ID2
	BYTE    UpdateSkillType;	//升级技能类型；1为升级单个技能；2为升级一组技能
};

struct SASkillUpdate : public SSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,			// 新学习
		SSU_UPDATE_SHULIANDU,		// 熟练度改变
		SSU_LEVELUP,					// 升级
		SSU_LoadSkill,				// 加载技能
		SSU_UpLoadSkill,			// 卸载技能
	};

	BYTE	byWhat;					// 更新原因
	WORD	wPos;					// 技能位置
	SSkill	stSkill;				// 技能数据
	BYTE byIsRefreshPain;	//是否刷新面板，默认为0时要刷新，为1时不刷新
};

// 升级心法等级
DECLARE_MSG(STelergyUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_TELERGY_UPDATE)
struct SQTelergyUpdate : public STelergyUpdate
{
	DWORD	dwGlobalID;		// 玩家标识
	WORD	wID;			// 要升级心法的ID
};

struct SATelergyUpdate : public STelergyUpdate
{
	enum
	{
		SAT_SUCCESS = 1,	
		SAT_INOTHERSTATUS,	// 处于别的互斥操作中（摆摊etc..）
		SAT_FULLLEVEL,		// 等级已满
		SAT_LACK_MONEY_SP,	// 金钱/真气不足

		SAT_LEARNED,		// 学习
		SAT_LEVELUP,		// 升级
	};

	WORD			pos;			// 心法位置
	BYTE			bResult;		// 结果
	SXYD3Telergy	m_Telergy;
};

DECLARE_MSG(SOpenSetSkillBaseMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_SETSKILL)
struct SQOpenSetSkillBaseMsg : public SOpenSetSkillBaseMsg	//请求打开玩家的技能配置面板
{
};

struct SAOpenSetSkillBaseMsg : public SOpenSetSkillBaseMsg
{
	BYTE      m_UpdatePlayerPattern;		//一共3中形态（普通形态、形态1和形态2）
	DWORD		m_UpdateCurUsedSkill[10];		//当前玩家使用的技能。0-2为普通形态下所使用的技能；3-5为形态1下；6-8为形态2下的；9为付费使用的技能
};

DECLARE_MSG(SUpdateSkillBaseMsg, SUpgradeMsg, SUpgradeMsg::EPRO_UPDATE_PLAYERSKILL)
struct SQUpdateSkillBaseMsg : public SUpdateSkillBaseMsg	//请求更新玩家的技能配置信息
{
	BYTE      m_UpdatePlayerPattern;		//一共3中形态（普通形态、形态1和形态2）
	DWORD		m_UpdateCurUsedSkill[10];		//当前玩家使用的技能。0-2为普通形态下所使用的技能；3-5为形态1下；6-8为形态2下的；9为付费使用的技能
};

struct SAUpdateSkillBaseMsg : public SUpdateSkillBaseMsg
{
	BYTE m_byRet;	//响应操作结果。1代表成功，0表示失败
};

//add by ly 2014/5/16
DECLARE_MSG(SNotityUpdateSkillMsg, SUpgradeMsg, SUpgradeMsg::EPRO_NOTITY_SKILLUPDATED)
struct SANotityUpdateSkillMsg : public SNotityUpdateSkillMsg	//通知玩家，当前玩家的技能配置已经更新
{
	BYTE      m_UpdatePlayerPattern;		//一共3中形态（普通形态、形态1和形态2）
	DWORD		m_UpdateCurUsedSkill[10];		//当前玩家使用的技能。0-2为普通形态下所使用的技能；3-5为形态1下；6-8为形态2下的；9为付费使用的技能
};

//=============================================================================================
// 心法数据的变化
DECLARE_MSG(STetergyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TETERGY_CHANGE)
struct  SATetergyChangeMsg : public STetergyChangeMsg
{
    DWORD dwGlobalID;         // 本次运行全局唯一标识符
    // STelergy Tetergy[MAX_EQUIPTELERGY];      // 心法数据
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 动作速度的改变
DECLARE_MSG(SActSpeedChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_ACTSPEED_CHANGE)
struct  SAActSpeedChangeMsg : public SActSpeedChangeMsg
{
    DWORD dwGlobalID;         // 本次运行全局唯一标识符
    float fActSpeedDec;      // 由敏捷决定的速度
};
// buff击退特效通知
DECLARE_MSG(SBuffFightOutMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BUFFFIGHTOUT_POS)
struct  SABuffFightOutMsg : public SBuffFightOutMsg
{
	DWORD dwGlobalID;         // 本次运行全局唯一标识符
	WORD wEndX;				// 结束点X
	WORD wEndY;				// 结束点Y
};

// 请求打通穴道
DECLARE_MSG(SPassVenapointAskMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PASS_VENAPOINTASK)
struct SQPassVenapointAskMsg : public SPassVenapointAskMsg
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE	byVenationNum;		// 脉线序号
    BYTE	byVenapointNum;		// 穴位序号
};

// 打通穴道确认
DECLARE_MSG(SPassVenapointOKMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PASS_VENAPOINTOK)
struct SAPassVenapointOKMsg : public SPassVenapointOKMsg
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE	byVenationCount;	// 所剩穴道点数
    BYTE	byVenationNum;		// 脉线序号
    BYTE	byVenapointNum;		// 穴位序号
    DWORD	dwUseTelergyVal;	// 所消耗的心法值
};

//改变当前的扩展心法EPRO_SET_EXTRATELERGY
DECLARE_MSG(SSetExtraTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_SET_EXTRATELERGY)
struct SQSetExtraTelergyMsg : public SSetExtraTelergyMsg
{
    BYTE state[4];  // 修改后的心法格状态
//	WORD wTelergyID[ MAX_EQUIPTELERGY + MAX_EXTRATELERGY ];  // 填充修改后扩展栏里放置的心法ID前 最后4个表示扩展的
};

//服务器回应给客户端的新的心法数据，做一次全拷贝。
struct SASetExtrTelergyMsg: public SSetExtraTelergyMsg
{
//	SExtraTelergy m_ExtraTelergy;                       // 附加装备心法
    WORD nTelergy;                             // 指明装备心法部分有多少有效（为0xffff时表示m_Telergy没有数据）
	// STelergy m_Telergy[ TELERGY_NUM ];        // 可以学习的心法
};

// 心法的改变
DECLARE_MSG(SSetChangeTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TELERGY_CHANGE)
struct SASetChangeTelergyMsg : public SSetChangeTelergyMsg
{
	bool			m_bIsAddTel;	// 是否增加新的心法，True标示增加心法
	BYTE			m_Pos;			// 位置
	SXYD3Telergy	m_Telergy;		// 心法

	SASetChangeTelergyMsg()
	{
		m_bIsAddTel = false;		// false表示改变心法值
	}
};

//---------------------------------------------------------------------------------------------

// 扩展心法格在面板上的位置 number 1-4 心法总编号8-11
/****************
* 3(10)   4(11) *
*               *
*               *
* 2(9)    1(8)  *
*****************/
// 打开当前的扩展心法格EPRO_OPEN_EXTRATELERGY
DECLARE_MSG(SOpenExtraTelergyMsg, SUpgradeMsg, SUpgradeMsg::EPRO_OPEN_EXTRATELERGY)
struct SQOpenExtraTelergyMsg : public SOpenExtraTelergyMsg
{
    BYTE number;       // 打开的第几个格子
    BYTE state;        // 修改后的心法格状态 0关闭 1开启
};

//服务器回应给客户端的
struct SAOpenExtrTelergyMsg: public SOpenExtraTelergyMsg
{
     BYTE number;      // 打开的第几个格子
     BYTE state;       // 修改后的心法格状态
};


// 轻功数据的变化
DECLARE_MSG(SFlyChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_FLY_CHANGE)
struct  SAFlyChangeMsg : public SFlyChangeMsg
{
	int	   type;		//来源于 EFlySKillType枚举
	SSkill FlySkill;
};

// 护体数据的变化
DECLARE_MSG(SProtectedChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PROTECTED_CHANGE)
struct  SAProtectedChangeMsg : public SProtectedChangeMsg
{
	int	   type;		//来源于 EProtectedSkillType枚举
	SSkill ProtectSkill;
};
//轻功、护体影响的属性
DECLARE_MSG(SFPAttribChangeMsg, SUpgradeMsg, SUpgradeMsg::EPRO_FP_ATTRIB_CHANGE)
struct SAFPAttribChangeMsg : public SFPAttribChangeMsg
{
	DWORD	dwHP;
	DWORD	dwMaxHP;
	WORD	wSP;
	WORD	wMaxSP;
	BYTE	byMaxJumpTile;

	float	iCurSpeed;

	// 身法
	WORD wAgile;

	// 攻防基础值
//	WORD mAttackAndDefenseMin[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX];				
//	WORD mAttackAndDefenseMax[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX];
};
//轻功，护体的变化提醒
DECLARE_MSG(SFPBuffChange, SUpgradeMsg, SUpgradeMsg::EPRO_FP_BUFF)
struct  SAFPBuffChange : public SFPBuffChange
{
	int	   type;				//类型（包括轻功和护体）		
	int    iID;					//（ID）
	bool   bIsStar;			//true标示启动,false表示启动完毕
	INT32 coldeTime;
};

#if 0
//轻功，护体的使用成功的时候通知客户端播放特效
DECLARE_MSG(SBuffUseSuccess_S2C, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_USE_SUCCESS)
struct  SBuffUseSuccess_S2C_MsgBody : public SBuffUseSuccess_S2C
{
	int	   type;			//类型（包括轻功和护体）		
	int    iID;				//（ID）
};
#endif

DECLARE_MSG(SDrugInfoShowMsg, SUpgradeMsg, SUpgradeMsg::EPRO_DRUG_INFO_SHOW)
struct  SADrugInfoShowMsg  :   public SDrugInfoShowMsg
{
	DWORD dwGlobalID;         // 本次运行全局唯一标识符
	DWORD dwDeltaValue;
	enDrug_Type  enDrug;				//是否吃药回复
};
//buff效果通知：瞬移
DECLARE_MSG(SSBuffSplitMoveMsg, SUpgradeMsg, SUpgradeMsg::EPRO_BUFF_SPLITMOVE)
struct  SABuffSplitMoveMsg : public SSBuffSplitMoveMsg
{
	DWORD dwGlobalID;         // 本次运行全局唯一标识符
	WORD wEndX;				// 结束点X
	WORD wEndY;				// 结束点Y
};

//一些额外的属性变化
DECLARE_MSG(SFightPetExtraChange, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_EXTRAPROPERTYCHANGE)
struct  SAFightPetExtraChange : public SFightPetExtraChange
{
	enum
	{
		CHANGE_TIRED,			//疲劳度改变
		CHANGE_RELATION,		//亲密度改变
		CHANGE_MOOD,			//心情改变
	};
	BYTE index;						//侠客索引
	BYTE	 changeType;			// 改变类型
	WORD wchangeValue;		//改变的值
};

//侠客更新技能消息
DECLARE_MSG(SfpSkillUpdate, SUpgradeMsg, SUpgradeMsg::EPRO_FIGHTPET_SKILLUPDATE)
struct SQfpSkillUpdate : public SfpSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,								// 新学习/装备技能
		SSU_UPDATE_SHULIANDU,					// 熟练度改变
		SSU_LEVELUP,										// 升级
		SSU_LEARNEDBYANOTHER,					// 从其他侠客身上学习
	};
	BYTE			byWhat;			// 更新原因其他
	WORD		wID;			// 要升级或者装备学习技能的ID
	BYTE			index;				//要学习技能的侠客索引
	BYTE			anotherindex;	//副侠客，从这个身上学习所有技能
};

struct SAfpSkillUpdate : public SfpSkillUpdate
{
	enum
	{
		SSU_LEARNED = 1,			// 新学习
		SSU_UPDATE_SHULIANDU,		// 熟练度改变
		SSU_LEVELUP,					// 升级
		SSU_EQUIPSKILL,					// 装备在技能学习栏的技能书
		SSU_UNEQUIPSKILL,					// 删除装备在技能学习栏的技能书
	};

	BYTE	byWhat;					// 更新原因
	WORD	wPos;					// 技能位置
	BYTE		index;					//侠客索引
	DWORD itemID;				//装备在技能学习栏的技能书ID
	SPetSkill spSkill;				// 侠客技能数据
};

//属性变化消息
DECLARE_MSG(Sallpropertychange, SUpgradeMsg, SUpgradeMsg::EPRO_ALL_PROPERTYCHANGED)
struct SAallpropertychange : public Sallpropertychange
{
	enum	FightProperty
	{
		PY_GONGJI	= 0x01,			// 攻击
		PY_FANGYU= 0x02,	// 防御
		PY_BAOJI= 0x04,	// 暴击
		PY_SHANBI= 0x08,			// 闪避

		PY_MAX_HP= 0x10,		// 最大生命
		PY_MAX_MP= 0x20,			// 最大内力
		PY_MAX_TP		= 0x40,		// 最大体力

		PY_CUR_HP		= 0x80,		// 当前生命
		PY_CUR_MP		= 0x100,		// 当前内力
		PY_CUR_TP		= 0x200,		// 当前体力
		PY_MAXPROPERTY= 0x400,
	};
	enum {BUFFSIZE = 256,};
	enum{PROPERTYNUM = 10,};
	WORD			m_type;							//类型
	WORD			messagesize;					//长度
	BYTE			valueBuff[BUFFSIZE];			//改变的值
};

// 玩家洗点
DECLARE_MSG(SSResetPointMsg, SUpgradeMsg, SUpgradeMsg::EPRO_RESET_POINT)
struct SQResetPointMsg : public SSResetPointMsg
{
	enum PointType
	{
		PT_JINGONG,
		PT_FANGYU,
		PT_QINGSHEN,
		PT_JIANSHEN,

		PT_ALL,
	};

// 	bool	bBuy;				// 道具是否不足
// 	BYTE	type;				// 洗点类型，PT_ALL表示全部洗点，此时，point = 0
	WORD	point[PT_ALL];				// 洗点数
	//WORD	itemCount;			// 需要道具数，当道具不足时，标示需要的道具数。其他情况，此变量为0；
};

struct SAResetPointMsg : public SSResetPointMsg
{
	enum
	{
		RPM_FAIL,
		RPM_SUCCESS,
	};

	BYTE	result;
};

//修为值更新
DECLARE_MSG(SXwzUpdateMsg, SUpgradeMsg, SUpgradeMsg::EPRO_XWZ_UPDATE)
struct  SAXwzUpdateMsg : public SXwzUpdateMsg
{
	DWORD xwzValue;         // 修为值
};

// 玩家死亡时的附近消息，仅发送给死亡的玩家自身
DECLARE_MSG(SPlayerDeadAddMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYERDEAD_ADD)
struct  SAPlayerDeadAddMsg : public SPlayerDeadAddMsg
{
	WORD wFreeReliveTime;  //免费入口复活时间
	WORD wReliveCost; //原地复活花费钱
	WORD activityAttkCost;//激发攻击花费
	WORD activityDefenceCost;//激发防御花费
// 	char killname[CONST_USERNAME];
// 	WORD skillid;
// 	struct relivecon
// 	{
// 		WORD wReliveTime;
// 		WORD wReliveMoney;
// 		BYTE		bMoneytype;
// 	};
// 	relivecon wreliveconType[4];
};

//add by ly 2014/3/25

//获取玩家荣耀信息
DECLARE_MSG(SGloryMsg, SUpgradeMsg, SUpgradeMsg::EPRO_PLAYERGETGLORY)
struct  SQGloryMsg : public SGloryMsg
{
};

struct  SAGloryMsg : public SGloryMsg
{
	DWORD m_PlayerGloryDot;		//玩家荣耀点
	WORD m_GloryArrSize;	//荣耀数组的大小
	WORD* m_pGloryInfArr;	//荣耀数据和完成状态，第1位为完成状态（1或0），后面15位为荣耀ID
	SAGloryMsg(DWORD PlayerGloryDot, WORD GloryArrSize) :m_PlayerGloryDot(PlayerGloryDot), m_GloryArrSize(GloryArrSize)
	{
		if (m_GloryArrSize == 0)
			m_pGloryInfArr = NULL;
		else
			m_pGloryInfArr = new WORD[m_GloryArrSize];
	}

	//设置荣耀对应索引的值
	void SetGloryData(BYTE FinishFlag, BYTE GloryID, WORD GloryArrIndex)
	{
		WORD GloryData = ((WORD)FinishFlag << 15) + GloryID;
		m_pGloryInfArr[GloryArrIndex] = GloryData;
	}

	~SAGloryMsg()
	{
		if (m_pGloryInfArr != NULL)
		{
			delete[] m_pGloryInfArr;
			m_pGloryInfArr = NULL;
		}
	}
};

//领取荣耀奖励
DECLARE_MSG(SGetGloryMsg, SUpgradeMsg, SUpgradeMsg::EPRO_GETPLAYERGLORY)
struct  SQGetGloryMsg : public SGetGloryMsg
{
	WORD m_GloryID;		//荣耀ID
};

struct SAGetGloryMsg : public SGetGloryMsg
{
	BYTE m_Ressult;		//领取操作结果（1成功， 0失败）
};

//称号信息
DECLARE_MSG(STitleMsg, SUpgradeMsg, SUpgradeMsg::EPRO_TITLE)
struct  SQTitleMsg : public STitleMsg
{
	enum MsgType
	{
		EXCHANGE,	//交换
		USE,	//使用
		UNUSE,	//取消使用
	};
	BYTE m_MsgType;	//消息类型
	WORD m_TitleID;		//称号ID
};

struct SATitleMsg : public STitleMsg
{
	BYTE m_Ressult;		//操作结果（1成功， 0失败）
	BYTE m_TitleSize;	//已经拥有的称号大小
	DWORD m_GloryDot;	//最新荣耀点
	BYTE *m_TitleArr;	//称号数组
	SATitleMsg() 
	{ 
		m_TitleArr = NULL; 
		m_TitleSize = 0; 
		m_Ressult = 0; 
	}
	~SATitleMsg()
	{ 
		if (m_TitleArr != NULL)
			delete[] m_TitleArr; 
		m_TitleArr = NULL; 
	}
};

//add by ly 2014/7/22
//玩家排行榜数据结构
struct PlayerRankList
{
	char m_PlayerName[CONST_USERNAME];		//玩家名字
	BYTE m_PlayerLevel;		//玩家等级
	BYTE m_PlayerSchool;	//玩家种族
	WORD m_PlayerCurRank;	//玩家当前排名
	DWORD m_PlayerPower;	//玩家战斗力
	DWORD m_PlayerGlod;		//玩家金币数
};

//请求排行榜数据
DECLARE_MSG(SPlayerRankList, SUpgradeMsg, SUpgradeMsg::EPRO_QUEST_RANDLISTINF)
struct SQPlayerRankList : public SPlayerRankList
{
	enum{ PLAYYERPOWER, PLAYERLEVEL, PLAYERGLOD };
	BYTE m_Type;	//请求类型（战斗力、等级、金币数）
};
struct SAPlayerRankList : public SPlayerRankList
{
	BYTE m_Type;	//应答类型（战斗力、等级、金币数）
	WORD m_TopNSize;	//在排行榜中前N个玩家数据大小
	WORD m_PlayerTopNSize;	//排在玩家前的N个玩家数据大小（包括玩家自己）
	PlayerRankList *m_pPlayerRankList;
};


//获取排行榜对应的奖励
DECLARE_MSG(SGetRankAward, SUpgradeMsg, SUpgradeMsg::EPRO_RANKLIST_GETWARD)
struct SQGetRankAward : public SGetRankAward
{
	enum{ PLAYYERPOWER = 1, PLAYERLEVEL, PLAYERGLOD };
	BYTE m_Type;	//请求类型（战斗力、等级、金币数）
};
struct SAGetRankAward : public SGetRankAward
{
	BYTE m_Result;	//操作结果
};

//获取玩家排行榜奖励领取状态
DECLARE_MSG(SGetRankAwardState, SUpgradeMsg, SUpgradeMsg::EPRO_RANKLIST_GEAWARDSTATE)
struct SQGetRankAwardState : public SGetRankAwardState
{
};
struct SAGetRankAwardState : public SGetRankAwardState
{
	BYTE m_RankState[3];	//依次为：战斗力排行奖励领取状态、玩家等级排行榜奖励领取状态、玩家金币排行榜奖励领取状态
};

//add by ly 2014/7/23
struct ActivenessItem
{
	BYTE m_ItmeNum;		//道具数量
	DWORD m_ItmeID;		//道具ID
};

#define ACTIVENESSITEMMAXNUM 4

struct ActivenessAward
{
	BYTE m_AwardLevel;	//奖励等级
	BYTE m_NeedActivenessNum;	//需要活跃度数量
	BYTE m_IsGetAwardFlag;	//每个阶段的奖励是否领取标识0表示不能领取，1表示可以领取，2表示已领取
	ActivenessItem m_ItemInfo[ACTIVENESSITEMMAXNUM];	//奖励的道具组
};

struct ActivenessTaskInf
{
	BYTE m_TaskCompleteTimes;	//任务完成次数
	BYTE m_TaskTotalTimes;	//任务总次数
	BYTE m_SingleTaskGetNum;	//单次获得的活跃度数量
	BYTE m_TaskIndex;	//任务索引
};

//获取活跃度相关信息
DECLARE_MSG(SGetActivenessInfo, SUpgradeMsg, SUpgradeMsg::EPRO_ACTIVENESS_GETINFO)
struct SQGetActivenessInfo : public SGetActivenessInfo
{
};
struct SAGetActivenessInfo : public SGetActivenessInfo
{
	BYTE m_ActivenessValue;	//活跃度
	BYTE m_ActivenessAwardNum;	//活跃度奖励数量
	BYTE m_ActivenessTaskNum;	//任务数量
	ActivenessTaskInf *m_pTaskInfo;	//任务信息
	ActivenessAward *m_pAwardInfo;	//奖励信息
};

//领取活跃度的对应奖励
DECLARE_MSG(SGetActivenessAward, SUpgradeMsg, SUpgradeMsg::EPRO_ACTIVENESS_GETAWARD)
struct SQGetActivenessAward : public SGetActivenessAward
{
	BYTE m_AwardLevel;	//领取该等级的奖励
};
struct SAGetActivenessAward : public SGetActivenessAward
{
	BYTE m_Result;	//领取结果
};
