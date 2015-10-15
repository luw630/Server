#pragma once
#include <d3dx9math.h>
#include "fightobject.h"
#include "CMonsterDefine.h"
#include "NetWorkModule/TeamMsgs.h"

// 怪物技能数据文件
#define FILENAME_MSKILL "Data\\MonSkillData.def"     

// 怪物的IID
const __int32 IID_MONSTER = 0x112100a4;

// 怪物武功数量(必须<255)
const int MAX_MSKILLCOUNT  = 20;      

// 侠义世界怪物武功数量
#define  MONSTERSKILLMAX 6

// 侠义世界怪物抵抗BUFF最大个数
#define  MONSTER_REMOVE_BUFF_NUM 8

/************************************************************************/
/*      侠义世界  装备掉落随机属性相关									*/
/************************************************************************/

//目前设计的最大同时出现属性个数
#define Max_Att_Num			11	

//每个属性最多可能出现的值个数	
#define Max_Att_Value_Num	100		

//装备分为8阶，用来限制每阶武器可能随机属性的最大条数
#define Max_Att_Num_Rule	8

struct SEquipAttTypeOddsData
{
	char name[20];		//属性名
	WORD	OddsValue;	//属性值几率
};
struct SEquipAttValueOddsData
{
	char     name[20];		//属性名
	DWORD	OddsValue[Max_Att_Value_Num];	//属性值几率
};
struct SEquipAttNumRuleData
{
	WORD	levle;	//装备等级
	WORD	number; //属性条数
};

// 掉落ITEM结构，最终掉落ITEM号和几率
struct SItemOddsTabledata  
{
	SItemOddsTabledata()
	{
		itemId = 0;
		wOdds  = 0;
	}
	WORD    itemId;  //Item id
	WORD    wOdds;	 //Item 掉落几率
};

//掉落ITEM 表（一个掉落文件）
typedef std::vector< SItemOddsTabledata > ITEMDROPTABLE;	

//查询表结构，保存了掉落表文件名ID，以及掉落ITEM 表
struct SItemDropTableData
{
	ITEMDROPTABLE itemIdTable;	
	WORD	itemId;
};

typedef std::vector< SItemDropTableData > ITEMDROPIDTABLE;
extern ITEMDROPIDTABLE				g_vItemDropIdTable;		// 掉落物品id表,所有掉落表数据结构
extern ITEMDROPIDTABLE				g_vTaskDropIdTable;		// 任务掉落物品ID表
extern WORD							*g_wAttNumOdds;			// 装备随机属性的个数（万分率，除 10000）		
extern SEquipAttTypeOddsData		*g_sAttTypeOdds;		// 装备随机每种属性出现几率（万分率，除 10000）
extern SEquipAttValueOddsData  		*g_sAttValueOdds;		// 装备每种属性取每个值的几率(万分率，除 10000)
extern SEquipAttNumRuleData			*g_sAttNumRule;			// 装备属性条数规则数据
// end--ab_yue

/*
编号	
名称	
图像档名	
头像图档名	
等级	
是否是BOSS	
生命值	
外功基础	
内功基础	
外防基础	
内防基础	
使用外功基础概率
使用内功基础概率
使用平均值概率
特殊攻击力（无视防御）	
破空手白刃几率	
特殊防御力(无视攻击)	
破脱袍卸甲几率	
武功倍数	
武功使用几率	
身法	
攻击间隔[攻击速度]	
命中时间	
攻击范围	
变身外功色怪几率	
变身内功色怪几率	
变身外防色怪比率	
变身内防色怪几率	
变身“金钱色怪”几率	
携带经验值	
携带金钱	
携带侠义值	
技能1	
技能1几率	
技能2	
技能2几率	
技能3	
技能3几率	
技能4	
技能4几率	
技能5	
技能5几率	
破BUFF1编号	
破BUFF1几率
破BUFF2编号
破BUFF2几率
破BUFF3编号
破BUFF3几率	
破BUFF4编号
破BUFF4几率
破BUFF5编号
破BUFF5几率	
破BUFF6编号
破BUFF6几率	
破BUFF7编号	
破BUFF7几率	
破BUFF8编号
破BUFF8几率
脚本编号	
最大掉落个数	
携带物品列表1	
物品掉落比率1	
携带物品列表n	
物品掉落比率n
*/
// struct SMonsterData
// {
//     // 新的怪物表
//     // 编号 名称 图像档名 头像图档名 等级 是否是BOSS 
// 	char name[20];
//     int id, image, iHeadImage,level, boss;
//     // 生命值 外功基础 内功基础 外防基础 内防基础 
//     int hp, outAtkBase, innerAtkBase, outDefBase, innerDefBase;
// 	// 使用外功基础概率、使用内功基础概率、使用平均值概率
// 	int outAtkChance, innerAtkChance, avgAtkChance;
//     // 特殊攻击力 破空手白刃几率 特殊防御力 破脱袍卸甲几率 武功倍数 武功使用几率 身法 攻击间隔[攻击速度] 命中时间 攻击范围 
//     int specAtk, bWeaponOdds, specPro, bArmourOdds,skillScale, skillRate, agility, hitSpeed, hitTick, hitArea;
//     // 变身几率
//     int ChangeRate;
//     // 携带经验值 携带金钱 携带侠义值 技能编号，技能几率，消BUFF编号，消BUFF几率 ，触发脚本编号
//     int exp, money, xydvalue,skillInfo[5][2],moveBuffInfo[8][2],scriptId;
// 	// 最大掉落个数 任务物品掉落表（携带物品列表n 物品掉落比率n）
// 	int dropMaxNum, dropTaskTable,dropTable[30][2];
// 	int extraScriptId;
// };

struct SMonsterSkill
{
	WORD type;			// 技能类型1.点穴(时间/2/20每秒) 2.晕眩(时间/10/20每秒) 3.中毒(时间/1/20每秒) 4.封招(时间/100/20每秒) 5.减体力并不能吃体药(时间/1/20每秒) 6.尖蓝并不能吃蓝药(时间/1/20每秒) 7.吸星
                        //         8.破甲(时间/10/20每秒) 9.虚弱(时间/10/20每秒) ...(未定义)10.吸血 11.灵魂锁链  12.厄运降临 13.群攻伤害平分
    WORD  rating;		// 以1/10000为小数单位,计算出现概率
    WORD  vitalsodds;	// 层数，这个数值会被玩家心法抵抗
    WORD  effectTime;	// 持续时间(这个值也跟层数有关)
    DWORD effectId;		// 客户端播放的命中效果图档编号
    DWORD waiting;		// 从发动到命中的等待时间

    // 扩展    
    char  szName[16];         // 名称[未用]
    int   dwSkillTimes;       // 使用的次数
    WORD  wRedDefRate;        // 降低防御力比率[脚本可选]                      
    WORD  wRedAgiRate;        // 降低灵敏度比率[脚本可选]                           
    WORD  wToxicRedHPRate;    // 中毒损生命比率[脚本可选]              
    WORD  wDemToHPRate;       // 吸收伤害转生命比率[脚本可选]
    DWORD dwDamageArea;       // 群伤的攻击范围
    DWORD dwDamageTotal;      // 总伤害
};

class CMonsterScriptEvent
{
public:
	struct MEventBase
	{
		MEventBase(const string& f): func(f) {}
		string	func;
	};

	//血量变化事件表
	struct HPEvent:public MEventBase
	{
		float	hpRate;

		HPEvent(float rate, const string& func) : hpRate(rate), MEventBase(func) {}
		bool operator<(const HPEvent& e) { return hpRate<e.hpRate; }
	};
	typedef std::vector<HPEvent> HPEventTable;

    //攻击持续时间事件表
	struct BeAttackEvent:public MEventBase
	{
		DWORD	dwAttackTime;

		BeAttackEvent(DWORD time, const string& f): dwAttackTime(time),MEventBase(f){}
		bool operator<(const BeAttackEvent& e) { return dwAttackTime<e.dwAttackTime; }
	};	
	typedef std::vector<BeAttackEvent> BeAttackTable;


	//具有Buff事件表
	struct BuffEvent:public MEventBase
	{
		DWORD	dwBuffId;

		BuffEvent(DWORD id, const string& f) : dwBuffId(id),MEventBase(f){}
	};
	typedef std::vector<BuffEvent> BuffTable;
    
	void Sort()
	{
		std::sort( hpTable.begin(), hpTable.end() );
		std::sort( baTable.begin(), baTable.end() );
	}

	HPEventTable	hpTable;
	BeAttackTable	baTable;
	BuffTable		buffTable;
};

class CMonster : public CFightObject
{
public:
	typedef CFightObject Parent;

	static int LoadMonsterData();                           // 载入怪物的数据
	static void ClearMonsterData();                         // 载入怪物的数据
//    static SMonsterData *GetMonsterData(int index);         // 载入怪物的数据

    // 技能
    static void LoadMonsterSkillData();                             // 载入怪物的技能数据
    static SMonsterSkill *GetMonsterSkillData(int SkillType);      // 获取怪物的技能数据 SkillType [1, MAX_MSKILLCOUNT]

	//侠义世界全局道具、装备属性表
	static int LoadEquipDropAttributeTable();

	////载入随机属性个数几率表
	static int LoadEquipDropAttNum();	

	//载入随机属性类型几率表
	static int LoadEquipDropAttType();	

	//载入装备每种属性取每个值的几率
	static int LoadEquipDropAttVale();	

	//载入装备属性条数规则数据
	static int LoadEquipDropAttRule();	

	//载入侠义世界道具掉落表
	static int LoadItemDropTable();				

	//载入侠义世界怪物掉落任务物品表
	static int LoadTaskDropTable();
	//end

    // -----------------------------------
    // 继承虚函数 
public:
    IObject *VDC(const __int32 IID) { if ((IID == IID_ACTIVEOBJECT) || (IID == IID_FIGHTOBJECT) ) return this; return NULL; }

public:
	void OnClose();
	void OnDisppear();
protected:
	int OnCreate(_W64 long pParameter);

public:
	struct SASynMonsterMsg *GetStateMsg();

  //扫描一定范围内的地图，取得攻击目标的列表
    int Attack();
    int WatchArea();

    BOOL CheckAction(int eCA);        // 检测动作执行的条件

    void SetRenascence(DWORD dwLast, DWORD dwCur, DWORD dwHP, WORD wTime); 
    // 怪物特殊相关处理
    BOOL SendDropItemOnDead( BOOL bDoubleDrop, class CPlayer *pActPlayer );

	//侠义世界
	BOOL SendDropItemOnDeadXYSJ( BOOL bDoubleDrop, class CPlayer *pActPlayer );
	//end

    bool IsBoss(void);              // 判断是否是Boss

	// 怪物的死亡函数
    void OnDead( CFightObject *PKiller );
    void _OnDead( CFightObject *PKiller );

    BOOL AttackTarget(void);
    BOOL CheckTarget(void);

	void ChangeFightSpeed(bool inFight);

	//BUFF添加时做的逻辑处理
	virtual bool CheckAddBuff(const SBuffBaseData *pBuff);

	//更新怪物的属性
	virtual void UpdateAllProperties();

	bool StopTracing();
	bool SetStopTracing(EActionState action=EA_RUN);///强制停止追击目标
	bool MoveToBirth();///在返回出生点被阻挡的情况下强制瞬移
public:

	int		m_ActionFrame;		// 动作的时间，用于怪物的AI
	DWORD	m_BaseSpeed;		// 怪物的基本速度（在战斗/非战斗下不同）
	void Thinking();				// 思考，用于选择下一步的行动

	struct SParameter
	{
		WORD	wListID;			// 怪物ID
		DWORD	dwRefreshTime;		// 刷新间隔
		WORD	wX;					// 坐标（格子坐标）
		WORD	wY;					
		WORD	wAIID;				// 怪物AI的ID, =0表示不使用AI
		WORD	wGroupID;			// 阵营ID
		WORD	reginID;			//地图ID
		WORD	PartolRadius;
		SPath	m_Path;				//为怪物设定的路径点，=0表示没有设置
		DWORD wCreatePlayerID;	//创建怪物的玩家GID，不一定有，只有当特殊脚本中才有
		DWORD dRankIndex;//创建竞技场怪物复制的玩家sid
		SParameter()
		{
			wListID  = 0;			// 怪物ID
			dwRefreshTime = 0;		// 刷新间隔
			wX = 0;					// 坐标（格子坐标）
			wY = 0;					
			wAIID = 0;				// 怪物AI的ID, =0表示不使用AI
			wGroupID = 0;			// 阵营ID
			reginID = 0;
			wCreatePlayerID = 0;
			PartolRadius = 0;
			dRankIndex = 0;
		}
	};

	/*
	struct SParameter
	{
		WORD	wRefreshStyle;		// 刷新方式 扩展 1表示不批量 2表示批量
		WORD	wListID;			// 该怪物的编号（在总列表中的编号）
		DWORD	dwRefreshTime;		// 刷新时间
		WORD	wRefreshScriptID;	// 重生脚本编号（当该怪物被刷新（死亡之后）时触发）

		WORD	wX;					// 当前的坐标
		WORD	wY;					// 。。。
		WORD	wMoveArea;			// 如果自由行走的话，可能限制的范围
		WORD	wAIType;			// 怪物的AI类型
		WORD	wAIScriptID;		// AI脚本编号（这个脚本的触发要看AI里边是如何设定的了）
		WORD	wDeadScriptID;		// 死亡脚本编号（当被玩家杀死的时候触发）
		WORD	wExtraScriptId;		// 扩展脚本ID
        SMonsterData *extraData;

		char	szName[CONST_USERNAME];

        DWORD   dwBRTeamNumber;      // 批量刷新批次
        WORD	wCenterX;			 // 中心的坐标(不用上面的wX是要和下面m_wCenterX对应，后面重刷的时候要用到)
        WORD	wCenterY;			 // 。。。
        DWORD   dwBRNumber;          // 批量刷新数量
        WORD    wBRArea;             // 批量刷新范围 
	}; //*/

public:
	/*
	struct SRefreshData				// 刷新相关的数据
	{
		WORD m_wRefreshStyle;		// 出现方式 
		WORD m_wListID;				// 怪物编号
		DWORD m_dwRefreshTime;		// 重生时间 
		WORD m_wSrcX;				// 出现坐标 
		WORD m_wSrcY;				//	...
		WORD m_wRefreshScriptID;	// 重生脚本编号（当该怪物被刷新（死亡之后）时触发）

        DWORD m_dwBRTeamNumber;     // 批量刷新批次
		float m_BirthPosX;			// 出生点坐标
		float m_BirthPosY;			

        WORD  m_wCenterX;			// 中心的坐标
        WORD  m_wCenterY;			// 
        DWORD m_dwBRNumber;         // 批量刷新数量
        WORD  m_wBRArea;            // 批量刷新范围  
	}; //*/

	struct SRefreshData
	{
		WORD m_BirthPosX;
		WORD m_BirthPosY;
	};

	struct SMonsterProperty : public SRefreshData, public SMonsterBaseData
	{
		DWORD controlId;        // 控制编号,用于脚本控制
	};

	struct SMonsterAIProperty
	{
		DWORD   startUpdateTime;	// 第一次更新时间（怪物生成的时间）
		DWORD	preUpdateTime;		// 上一次更新时间
	};

	/*
	struct SMonsterProperty : public SRefreshData, public SMonsterBaseData
	{
		WORD m_wMoveArea;		// 移动范围
		WORD m_wAIType;			// AI类型
		WORD m_wAIScriptID;		// AI脚本编号
		WORD m_wDeadScriptID;	// 死亡脚本编号

		DWORD m_dwCurHP;		// 当前的HP
		DWORD m_dwExtraColor;	// 怪物颜色
		
		//BYTE m_byGoodState;		// 当前良性状态
		//BYTE m_byBadState;		// 当前恶性状态
        //WORD m_wDamageType;       // 伤害类型(根据指定类型对指定目标造成额外伤害,如:对少林额外伤害 )
        struct SDamageType
        {
            BYTE sex: 2;            // 性别1女2男
            BYTE school:4;          // 门派
            BYTE reserve : 2;
        };

        SDamageType mageType;

        WORD m_wDamageValue;       // (1-65535%)百分比

        char szMaskName[22];    // 限定名
        DWORD sideId;           // 限定阵营ID，目前计划通过szMaskName来直接找到阵营编号！

        static const IS_NAME_LIMITED    = 0x00000008;       // 存在限定名限制[受到攻击时]
        static const IS_TONG_VALIDATE   = 0x00000004;       // 帮派限定/角色限定
        static const IS_MASK_ATTACK     = 0x00000002;       // 不能攻击/只能攻击
        static const IS_ACT_MASK_ATTACK = 0x00000010;       // 不能攻击/只能攻击
        static const IS_TEAM_VALIDATE   = 0x00000001;       // 组队不能攻击
        static const IS_SCHOOL_VALIDATE = 0x00002000;       // 指定门派不能攻击

        static const IS_RE_LIMIT_SKILL  = 0x00000020;       // 抵抗封招
        static const IS_LOS_ATTACK      = 0x00000040;       // 无视N级以下攻击
        static const IS_RE_DAM_DAM      = 0x00000080;       // 抵抗X%外功伤害
        static const IS_RE_POW_DAM      = 0x00000100;       // 抵抗X%内功伤害
        static const IS_EXC_DAM_DAM     = 0x00000200;       // 受到外功伤害有X%加成       
        static const IS_EXC_POW_DAM     = 0x00000400;       // 受到内功伤害有X%加成
        static const IS_TOXIC_VALIDATE  = 0x00000800;       // 五毒奇经能攻击/不能攻击
        static const IS_RE_DIZZY        = 0x00001000;       // 抵抗忘情天书

        DWORD controlId;        // 控制编号,用于脚本控制

        // 多目标攻击设定
        struct {
            DWORD rating;   // 以1/10000为小数单位,计算出现概率
            DWORD damage;   // 以1/10000为小数单位,计算浮动概率(在原基本伤害上进行浮动)
            DWORD effect;   // 客户端播放的命中效果图档编号
            DWORD area;     // 群伤的攻击范围
            DWORD hit;      // 群伤的命中概率
            DWORD waiting;  // 从发动到命中的等待时间
        } multiAttack;

        // 用于增加怪物AI多样性的脚本触发设置
        struct {
			std::string stateChanged;
            std::string hpLimited;
        } scriptAi;

        struct {
            int dodge;     // 怪物的绝对躲避率
            int hit;       // 怪物的绝对命中率
            int damage;    // 怪物的绝对伤害率 ( 该值比较特殊,为负数时为固定伤害值,为正数时为固定伤害白分比 )
            int defence;   // 怪物的绝对防御率
        } absoluteSetting;

        DWORD curSegment;
    }; //*/

    // 技能相关 
	std::list<SMonsterSkill> skillList;			// 怪物可以使用的技能
	SMonsterSkill skill;						// 怪物准备使用的技能

	SMonsterProperty	m_Property;		
	
	SParameter			m_AIParameter;
	SMonsterAIProperty	m_AIProperty;

	DWORD	m_nlastdropTime;		// 上一次掉落时间
	// 用于移动的数据
	struct		SWayTrack { BYTE dir:3; BYTE len:5; };

	BYTE		m_bySpeedFrame;					// 该玩家的移动速度，一般是以主循环Frame为单位	
    BYTE        m_byMutateState;                // 当前是否出现了突变（ 0表示未突变，1~6表示突变的类型）

    DWORD m_dwEndAttackTime; // 没有受到攻击一段时间后，停止追击

    DWORD m_dwTelesportTime;                    // 控制瞬移时间
    static const int TELESPORT_TICK = 2000;         // 2秒

public:
    // 分配相关
    struct SHitData
    {
        SHitData();

        DWORD dwGID;            // 该对象的ID
        DWORD dwTeamID;         // 该对象的TeamID
        DWORD dwHitTimes;       // 该对象的攻击次数
        DWORD dwDamage;         // 该对象的造成的伤害
        DWORD dwTimeMargin;     // 该对象攻击过程中的最大时间间隔
        DWORD dwLastHitTime;    // 该对象最后一次攻击的时间（这个不参与结果计算，用于判断这个玩家是否放弃了攻击怪物）
        DWORD dwFristHitTime;   // 第一次攻击时间

        class CPlayer *pPlayer;       // 最后计算时用
    };

    std::map<DWORD, SHitData> m_HitMap;

    void SetHit(DWORD GID, DWORD TeamID, DWORD Damage);
    void UpdateHitMap();
	void KillMonsterTask(CPlayer *pKiller);
	void KillMonsterTaskWithSingle(CPlayer *pCurrPlayer);
	void HandleExpWithTeam(CPlayer *pKiller);
	void HandleGetSP(CPlayer *pKiller);
	void HandleExpWithObj(CFightObject *pKiller);

	// 当被攻击的时候被调用
	void OnDamage(INT32 *nDamage, CFightObject *pFighter);
	void OnRebound(int rebound, CFightObject* pFighter);
	void OnRun();

	BOOL EndPrevAction();   // 上一个动作结束时的回调
	BOOL DoCurAction();

	bool ProcessDeadWaitTime();
	bool EndPrevActionByStandState();
	bool EndPrevActionByDeadState();
	bool EndPrevActionByWoundState();
	bool EndPrevActionByStandState_DoAIByID3Or4();
	bool EndPrevActionByStandState_DoAIByID2();
	bool EndPrevActionByStandState_DoAIByID1();
	bool DoRenascenceScript();
	bool DeDeadEvent();
	bool ProcessEndAttack();

	// 按需发送更新后的属性
	virtual void SendPropertiesUpdate();

	virtual void SwitchFightState(bool IsIn);	// 进入/离开战斗状态回调

	// 侠义道三物品掉落
	BOOL GetDropItem(CPlayer *Killer);
	// 侠义道三物品掉落
	BOOL GetDropItem(DWORD itemID,INT32 itemNum=1);
	/************************************************************************/
	/* 新的技能攻击流程--yuntao.liu											*/
	/************************************************************************/

	// 当怪物使用技能攻击的时候，距离过远，那么发送移动消息
	virtual bool SendMove2TargetForAttackMsg(INT32 skillDistance,INT32 skillIndex,CFightObject* pLife);

	// 创建一条路径
	bool CreatePath(SQSynPathMsg& WayMsg, CFightObject *pLife);

	// 创建一条路径
	int CreatePath(SQSynWayTrackMsg& WayMsg,CFightObject* pLife);

	// 创建随即点追击目标，在目标周围区域
	int CreateRadomPoint(CFightObject* pLife, D3DXVECTOR2 &vtargetPos, WORD mindistance, WORD maxdistance);

	virtual INT32 GetCurrentSkillLevel(DWORD dwSkillIndex);

	// 根据输入的技能索引得到技能ID
	virtual INT32 GetSkillIDBySkillIndex(INT32 index);

	// 状态切换的时候的回调函数
	virtual void OnChangeState(EActionState newActionID);

	// 当移动完成后回调函数
	virtual void OnRunEnd();

	// 目前怪物最多学习5个技能，加一个普通技能，总共6个技能
	SSkill m_pSkills[ MONSTERSKILLMAX ];   //modify by ab_yue
	// end--yuntao.liu

	int OnCreateEnd();//创建完成，在把怪物放入场景后的一次回调

	void OnRecvPath(struct SQSynPathMonsterMsg *pMsg);
	float GetWalkSpeed();
private :
    BOOL  m_bStopToStand;
    WORD  m_wIdleTime;
	WORD m_wCuruseskill;//当前使用的技能
	//------------------------------------------------
	// 副本相关
private:
	DWORD m_CopySceneGID;

public:
	void SetCopySceneGID(DWORD dwGID) { m_CopySceneGID = dwGID; }

public:
    // 4 monster dead event
    BOOL m_bDeadEvent;

    DWORD m_dwDeadDelayScript;
    DWORD m_dwKillerPlayerGID;

    WORD  m_wDeadTimes;

    // 怪物ai相关，是否需要重新选择一个攻击目标？
    DWORD targetSegTime;
    DWORD targetDamage;

    // 统计数据
    static DWORD monsterDeadMoney;

    BYTE  bySelectType;				// 表示优先或者排除，1.优先 2.排除  
    BYTE  byPriorityExceptType;		// 优先或排除选择目标类型， 1,角色  2,门派  3,性别  4,帮派  5,队伍
    DWORD dwPriorityTarget;			// 优先选择目标

	DWORD m_SearchTargetTime;		// 搜索目标时间
	bool  m_BackProtection;			// 处于返回保护
	///用于怪物AI时间控制
	DWORD m_nAttackTime;			///被攻击开始时间记录
	DWORD GetRandomEnemyID();
	DWORD	m_nRandomSkillIndex;//随即技能ID
	ULONGLONG	m_nBirthTime;
	DWORD  m_nCollisionTime;
	ULONGLONG m_nMoveStartTime;//开始移动时间
	DWORD  m_dCreatePlayerID;	//怪物的创建者
	DWORD  m_dRankIndex;	//竞技场怪物复制玩家排名
	DWORD  m_ShowState;  //对应玩家m_ShowState
	char   m_Name[CONST_USERNAME];//名字
	WORD m_dModID;//模型ID
	D3DXVECTOR2 *m_vdesPos;
	
	long   m_StaticpreUpdateTime;//静态场景预前时间
	BYTE m_gmakeparm1;
	BYTE m_gmakeparm2;
	BYTE m_gmakeparm3;
	BYTE m_gmakeparm4;
public:
	CMonsterScriptEvent scriptEvent;
	//更新怪物状态
	void UpdateMyData(WORD reginID,WORD wHour,WORD wMinute,char *killmeName,WORD mystate);
	//void ChangeProperty();
private:
	void InitScriptEvent();
	void TriggerHpScriptEvent(int nDamage, CFightObject *pFighter);

private:
	CPlayer*	m_PlayerInDis[MAX_TEAM_MEMBER];	
	DWORD		m_dwPlayerInDisCount;			//有效范围内玩家的总数
	void HandlePlayerInDis(CPlayer *pKiller);   //计算队伍里的玩家，那些在经验分配/任务共享的有效范围内

	void SetNineWordsCondition(CPlayer* player);

	BossDeadData m_BossDeadData;
///////////////////////////////////////////////////////////////////////////////////////////////////
//AI怪物相关
///////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void MakeSynPathMsg(SQSynPathMsg &msg);

	void AIThinking();
	void ProcessScript();
	void SetMonsterPath(CMonster::SParameter param);

///////////////////////////////////////////////////////////////////////////////////////////////////

};