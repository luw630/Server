#pragma once
#define GMMODULEON
///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////
static const int MAX_FRIEND_NUMBER = 50;
static const int MAX_BLOCK_NUMBER = 30;
static const int MAX_MATE_NUMBER = 10;

static const int MAX_EQUIPTELERGY = 8;        // 可以装备的心法数量
static const int MAX_EXTRATELERGY = 4;		// 附加心法

// 多倍
const WORD N_MULTIPLES  = 2;  
//static WORD s_wMulHours = 16; //移到环境变量单元（Environment）中来处理


//------------------ 与客户端公用数据结构 ----------------------

enum    ESHOW_EXTRASTATE                        // 玩家附加状态标记
{
    SHOWEXTRASTATE_NONE     = 0x00000000,		// 空状态
    SHOWEXTRASTATE_SALE     = 0x00000001,		// 摆摊卖
    SHOWEXTRASTATE_CURE     = 0x00000002,		// 治疗
    SHOWEXTRASTATE_BECURE   = 0x00000004,		// 被治疗
	SHOWEXTRASTATE_STARTZAZEN= 0x00000007,		// 打坐需要清除的状态
    SHOWEXTRASTATE_SCRIPT   = 0x00000008,       // 脚本状态
    SHOWEXTRASTATE_SCORES_LEVEL = 0x00000010,   // 排行10大高手
    SHOWEXTRASTATE_SCORES_MONEY = 0x00000020,   // 排行10大富豪
    SHOWEXTRASTATE_SCORES_FAMEG = 0x00000040,   // 排行10大侠客
    SHOWEXTRASTATE_SCORES_FAMEX = 0x00000080,   // 排行10大恶人
	SHOWEXTRASTATE_ONMOSMUTATE	= 0x00000100,   // MOG变身状态
    SHOWEXTRASTATE_ONNPCMUTATE	= 0x00000200,   // NPC变身状态
    SHOWEXTRASTATE_ONGOATMUTATE	= 0x00000400,   // 替身变身状态
    SHOWEXTRASTATE_ONESPMUTATE	= 0x00000800,   // 特殊变身状态（结婚等）
    //SHOWEXTRASTATE_ONGOAT		= 0x00000400    // 替身状态(暂时不用)
    SHOWEXTRASTATE_SPORT   = 0x0001f000,        // 竞技状态
    SHOWEXTRASTATE_D3D_EQUIP    = 0x00100000,   // 显示3D装备状态
    SHOWEXTRASTATE_NEWMODEL     = 0x00020000,   // 新造型
    SHOWEXTRASTATE_BUY          = 0x00040000,   // 摆摊收
    SHOWEXTRASTATE_SCORES_MEDICALETHICS = 0x00080000, // 排行10大医者
    SHOWEXTRASTATE_ADDDOCTORBUFF     = 0x00200000,		// 加医德BUFF者
    SHOWEXTRASTATE_BEADDDOCTORBUFF   = 0x00400000,		// 被加医德BUFF者
	SHOWEXTRASTATE_INTEAM			 = 0x00800000,      // 在队内状态
	SHOWEXTRASTATE_UNIONRED			= 0x01000000,		// 兄弟名红色字体
	SHOWEXTRASTATE_UNIONGOLD		= 0x02000000,		// 兄弟名暗金色字体	
	SHOWEXTRASTATE_UNIONPURPLE 		= 0x04000000,		// 兄弟名紫色字体	
};

// ---  任务 ---------------------------------------------
// -------------------------------------------------------
struct SRoleTask
{
	WORD wTaskID;               // 任务号   这个号可跟NPC所触发的脚本号一致  也可是专门指定的任务号 
	WORD byGoodsID;             // 完成任务需要的物品ID号 （保留）
    WORD wNextGoods;            // 预设下一个任务要的物品ID号
	WORD byNeedNum;             // 现在可以作为需要完成的任务数
    WORD byFinished;            // 已完成的任务数
	WORD wForgeGoods[15];       // 炼制任务需要
    DWORD wComplete;              // 完成情况\状态值
    bool bSave;                 // 是否保存
    char m_szName[11];	        // 需要特定名称
	BYTE byTaskType;            // 任务类型
};

// --- 点击NPC，挂起等待的标志 其实没有挂起，只有标志而已 
//     当byGoodsID = 0 时 默认为不保存的任务 
typedef struct _RECVITEMINFO {
    WORD wPosX;
    WORD wPosY;
    WORD wItemID;
}SITEMINFO, *LPSITEMINFO;
// -------------------------------------------------------

struct  STelergy
{
    DWORD   dwTelergyVal;       // 心法值
    WORD    wTelergyID;         // 心法ID
    BYTE    byTelergyLevel;     // 心法等级
};

struct SExtraTelergyOld     // [废弃的数据]
{
    STelergy telergy[2];
	BYTE  state[2];         // 两个格子的激活状态, 
    BYTE  margin;           // 由于是按整分钟进行数据判断，所以需要增加分钟间的秒间隔！
	DWORD charge;           // 充值时间（以点数计）
	DWORD segment;          // 激活启动时间( 如果其中一个格子已经激活, 再激活第二个时就不算启动时间了)
};

struct SExtraTelergy
{
    STelergy telergy[4];
    BYTE  state[4];         // 4个格子的激活状态, 
    BYTE  margin;           // 由于是按整分钟进行数据判断，所以需要增加分钟间的秒间隔！
    DWORD charge;           // 充值时间（以点数计）
    DWORD segment;          // 激活启动时间( 如果其中一个格子已经激活, 再激活第二个时就不算启动时间了)
};

struct SFixBaseData // 存放玩家的基本数值
{
	DWORD	m_version;				// 版本号，用于二进制数据扩展！
    DWORD	  m_dwConsumePoint; //玩家消费积分
    // 名称：因为在游戏中玩家的名称不会（绝对不能）重复，将名称定为该角色的关键字，所以不在属性数据中出现
    BYTE    m_bySex:1;              // 玩家角色性别
    BYTE    m_byBRON:3;             // 玩家所显示图片级别，==1表示出师
    BYTE    m_bySchool:4;           // 玩家所属门派
    //BYTE	m_byBron;			    // 该玩家出身，包括是否出师，从这里可以决定该玩家使用何种角色图片

    WORD	m_byLevel;			    // 玩家的等级，255级上限 -->现改为WORD, 65535上限

    //DWORD   m_dwMaxExp;           // 玩家的当前经验值上限
    __int64	m_iExp;			        // 玩家的当前经验值
    DWORD	m_dwMoney;			    // 玩家当前所携带的金钱
	DWORD	m_dwYuanBao;			// 玩家当前所携带的元宝
	DWORD	m_dwZenBao;			    // 玩家当前所携带的赠宝

    DWORD	m_dwBaseHP;			    // 基本生命力
    WORD	m_wBaseMP;			    // 基本内力
    WORD	m_wBaseSP;			    // 基本体力

    WORD    m_wMaxHPOther;          // 其他固定增加生命最大值
    WORD    m_wMaxMPOther;          // 其他固定增加内力最大值
    WORD    m_wMaxSPOther;          // 其他固定增加体力最大值

    DWORD	m_dwCurHP;			    // 当前生命力
    WORD	m_wCurMP;			    // 当前内力
    WORD	m_wCurSP;			    // 当前体力

    WORD	m_wEN;				    // 活力 （体质）（手太阳）
    WORD	m_wST;				    // 体魄 （强壮）（足太阴）
    WORD	m_wIN;				    // 悟性 （智慧）（手少阳）
    WORD	m_wAG;				    // 身法 （灵敏）（足少阴）
    WORD	m_wLU;				    // 气运 （运气）

    BYTE    m_byENState;            // 手太阳经脉状态
    BYTE    m_bySTState;            // 足太阴经脉状态
    BYTE    m_byINState;            // 手少阳经脉状态
    BYTE    m_byAGState;            // 足少阴经脉状态

    BYTE	m_byPoint;			    // 玩家当前还没有分配的能力点数
    BYTE	m_byAmuck;			    // 杀气值
    short	m_sXValue;		        // 侠义值
    BYTE    m_byPKValue;            // pk值

    WORD	m_wCurRegionID;		    // 当前所在地图的ID
    WORD	m_wSegX;			    // 当前所在地图的X坐标（Tile）
    WORD	m_wSegY;			    // ...

    char	m_szName[11];		    // m_szName[10] == 0！！！
    char	m_szTongName[11];	    // 帮会名称，同时为用于查找该帮派的关键字
    char	m_szTitle[11];		    // 该玩家自定的称号
    DWORD	m_dwSysTitle[2];	    // 系统定义的称号，64种以内

    SSkill	m_pSkills[12];		    // 目前已学习的武功
    SSkill	m_pTroopsSkills[4];	    // 目前已学习的阵型技

    // 心法相关的
    STelergy    m_Telergy[TELERGY_NUM];    // 可以学习的心法
	SExtraTelergyOld m_ExtraTelergyOld;   // 附加装备心法	[废弃的数据 m_ExtraTelergy 同上 ]

    BYTE    m_CurTelergy;           // 当前所用的心法
    BYTE    m_byTelergyStep;        // 心法领悟力
    //BYTE    m_CurUseAmulet;         // 当前使用的护体神功

    SEquipment	m_Equip[16];	        // 玩家目前所装备的道具

    WORD    m_wEquipCol[4];         // 玩家衣服的颜色值EEC_MAX
    BYTE    m_byEquipColCount[4];   // 当前换色次数
    DWORD   m_dwStoreMoney;         // 玩家仓库里的存钱

    // 穴位相关数据
    BYTE    m_byVenapointCount;     // 穴位点数
    DWORD   m_dwVenapointState[9];  // 穴位状态

    WORD	m_wAddHPSpeed;			    // 生命力增加速度附加值
    WORD	m_wAddMPSpeed;			    // 内力增加速度附加值
    WORD	m_wAddSPSpeed;			    // 内力增加速度附加值

    // 多倍相关
    BYTE    m_byNTime; 
    BYTE    m_byLeaveTime; 
    //__int32 m_nStartTime[2];
    DWORD   m_dwLastTime;              // 32

    BYTE    m_nMonsterCountOnGoat;       // 替身杀死怪的数量
    
    BYTE    m_reserve1;
    BYTE    m_fMultiple;
    BYTE    m_nCurWeekUseTime;         // 下面4个 32

	WORD    m_wScriptID;
	WORD    m_wScriptIcon;
	bool	m_bWarehouseLocked;

    char	m_szMateName[11];		    // 夫（妻）的名字
    union {
        __int64 m_qwMarryDate;          // 结婚的日期
        struct {
            __int32 m_dwMarryDate;      // 结婚的日期
            WORD factionId;             // 帮派ID
            WORD reserve;              
        };
    };
	DWORD	m_dwPowerVal;//战斗力

	SItemBase addion_packages[4];	// 玩家背包、仓库当前装备的扩展包裹数据！
};

struct SFixPackage // 道具
{
    SPackageItem    m_pGoods[MAX_ITEM_NUMBER];  // 玩家的道具
};

struct SFixStorage1 // 仓库
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // 仓库的道具
};

struct SFixStorage2 // 仓库
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // 仓库的道具
};

struct SFixStorage3 // 仓库
{
    SPackageItem    m_pStorageGoods[MAX_ITEM_NUMBER];  // 仓库的道具
};

struct SPlayerTask
{
    WORD PlayerTaskID;
    DWORD PlayerTaskStatus;
};

struct SPlayerTasks
{
    SPlayerTask PlayerTask[MAX_TASK_NUMBER];
    BYTE flags[128]; // 1024bit位的标志数据，主要用于记录那些不可撤销的标志信息！
    BOOL SetMark( DWORD group, DWORD idx ) { if ( group >= 128 || idx >= 8 ) return -1; flags[group] |= ( 1 << idx ); return 1; }
    BOOL ClrMark( DWORD group, DWORD idx ) { if ( group >= 128 || idx >= 8 ) return -1; flags[group] &= ~( 1 << idx ); return 1; }
    BOOL ChkMark( DWORD group, DWORD idx ) { if ( group >= 128 || idx >= 8 ) return -1; return ( flags[group] &= ( 1 << idx ) ) == 1; }
};

struct SPlayerGM
{
    DWORD  m_wGMLevel;
};
// 好友,黑名单,
struct SPlayerRelates
{
    DWORD dwStaticID;
    WORD  wDearValue; // 亲密度
    char  szName[11];
    BYTE  byRelation; // 0 好友, 1 黑名单, 2 结拜 ,3 夫妻
};

struct SPlayerRelation
{
    SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];
    SPlayerRelates m_PlayerBlocks[MAX_BLOCK_NUMBER];
    SPlayerRelates m_PlayerMates[MAX_MATE_NUMBER];
};

// 大周天
struct SPlayerVenapointEx
{
    enum
    {
        VENAPOINTEX_SY = 1,
        VENAPOINTEX_SJ,
        VENAPOINTEX_ST,
        VENAPOINTEX_SS,
        VENAPOINTEX_ZY,
        VENAPOINTEX_ZT,
        VENAPOINTEX_ZS,
        VENAPOINTEX_ZJ,
    };
    // 穴位相关数据
    DWORD   m_dwVenapointStateEx[8];  // 穴位状态
};


// 大转盘奖品需要保存，因为在意外退出的时候，可能要作恢复使用，
struct SPlayerDialUnit
{
    union
    {
        struct SDialBase
        {
            BYTE type;
            BYTE data[64];
        }base;

        // 道具
        struct SDialUnitItem
        {
            BYTE type;
            SRawItemBuffer itemBuffer;
        }unitItem;

        // 其他杂项如经验，等
        struct SDialUnitOther
        {
            BYTE type;
            int value;
        }unitOther;
    };
};

struct SPlayerDataExpand
{
    char masterName[11];    // 师傅名字
    char prenticeName[11];  // 徒弟名字
    char __dummy1[2];       // 和上面做4字节对齐
    DWORD killMonster;
    DWORD killPlayer;
    DWORD deadTimes;
    DWORD onlineTime;
    DWORD openBoxTimes;
    DWORD useXYDPoint;
    BYTE  bySkillState;     // 武功境界
    BYTE __dummy3;			// 对齐
    WORD wMedicalEthics;    // 医德值
    WORD wMaxPerDayME;      // 当天累计量
	WORD   wWhineBuffType;		   // 桃酒BUFF类型
    
	DWORD dwDoctorBuffLastTime;    // 医者提升BUFF开始时间
    BYTE  byDoctorDefBuffer;             
    BYTE  byDoctorDamBuffer;
    BYTE  byDoctorPowBuffer;
    BYTE  byDoctorAgiBuffer;

    DWORD dwLastAddMETime;         // 最后一次保存医德值时间
    DWORD dwLastSaveExpTime;       // 最后一次保存经验值的时间
    DWORD dwTodayAchieveMonsterExpCount;  // 今天打怪所获得的总EXP值
    DWORD dwTodayAchieveOtherExpCount;    // 今天任务以及其他获得的经验值
    
    SExtraTelergy m_ExtraTelergy;  // 附加装备心法   大小 48
    
    DWORD  dwTotalCharmValue;      // 获得送花后的总魅力值
    DWORD  dwLastSaveCharmValTime; // 上次保存今日魅力值的时间
    DWORD  dwTodayCharmValue;      // 获得送花后的今日魅力值
    DWORD  dwBaoTuScore;           // 挖藏宝图获得分数
    DWORD  dwZhenFaPassScore;      // 闯过阵法分数
    DWORD  dwHuntScore;            // 狩猎积分
    DWORD  dwMazePassScore;        // 迷宫通关积分

	DWORD  dwWhineBuffStartTime;   // 桃酒BUFF开始时间
	WORD   wWhineBUFFData[6];	   // 桃酒BUFF数据，用于还原
    WORD   wXinMoVaule;            // 心魔值  最大65535
    WORD   revs1;
	
	DWORD  dwLastLeaveFactionTime; // 退帮时间
	DWORD  dwLastSaveContributionTime; // 最后一次保存帮贡值的时间
	WORD   wTodayContributionVal; // 今日帮贡值
	WORD   wContribution;		  // 帮贡值

    DWORD dwDecDurTimeEnd;        // 掉持久结束时间


    char __dummy2[704];     // 做4k的数据填充
    BYTE taskExBuffer[3200];
};



template < bool > struct SizeGuard { private: SizeGuard() {} };
template <> struct SizeGuard < true > { SizeGuard( LPCSTR ) {} };

static SizeGuard< sizeof( SPlayerDataExpand ) == 4096 > 
    ck1( "正在检测SPlayerDataExpand的数据结构大小" );

static SizeGuard< offsetof( SPlayerDataExpand, taskExBuffer ) == ( 4096 - 3200 ) > 
    ck2( "正在检测SPlayerDataExpand的数据结构大小" );

// 用于保存
struct SPlayerUnionBase
{
    enum
    {
        NUMBER_MAX = 8,
    };
    char title[9];      // 标题
    char suffx[3];      // 后缀
    DWORD roleid[8];    // 8个玩家ID
    DWORD exp[8];       // 8个玩家的经验
};

// 用于存放在 游戏内存中
struct SPlayerUnionData : 
    public SPlayerUnionBase
{
    char szName[8][11]; //  在DB载入数据的时候附加名字
};

// 客户端显示使用
struct SPlayerUnionInfo : public SPlayerUnionData
{
	char unionName[13];
	BYTE school[8];              // 门派
	BYTE camp[8];                // 阵营
	BYTE job[8];                 // 职位
	char szFactionName[8][11];   // 所在帮派名
	bool online[8];              // 在线情况
};

struct SFixProperty : // 固定的玩家属性，会保存下来的数据
    public SFixBaseData,
    public SFixPackage,
    public SFixStorage1,
    public SFixStorage2,
    public SFixStorage3,
    public SPlayerTasks,
    public SPlayerGM,
    public SPlayerRelation,
    public SPlayerVenapointEx,
    public SPlayerDialUnit,
    public SPlayerDataExpand
{
    enum SAVESTATE //角色服务器上的状态
	{
	ST_LOGIN =1,//刚刚登入
	ST_LOGOUT,//退出
	ST_HANGUP //离线挂机
	};
    DWORD   m_dwStaticID;                       // 玩家对服务器集群唯一静态的ID，由数据库服务器生成。
    char    m_szUPassword[CONST_USERPASS];        // 玩家二级密码
	BYTE    m_byStoreFlag; //玩家的当前状态 用于区分保存时的信息

    //SFixProperty() { m_version = GetVersion(); }
    static const int VERSIONID = 3;
    static DWORD GetVersion() { return ( sizeof( SFixProperty ) << 16 | VERSIONID ); };
};

static const int QUEST_MONEYPOINT_TIME = 30;       // 元宝请求时间的期限时间（30秒）
struct SPlayerTempData // 玩家的临时数据，不保存，但是使用于跨场景服务器时候的数据传递
{
    DWORD m_dwTeamID;               // 组队的数据

    SRoleTask PlayerTask[100];      // 临时脚本数据

    bool m_IsRegionChange;          // 是初始化还是场景转换

    struct {
        DWORD GID;                  // pk对象
        DWORD lasttime;             // 剩下的pk时间
    } m_PkList[30];                 // PK对象列表

    DWORD m_dwTimeLeaving;          // 定时数据的更新
    DWORD m_dwOneHourLeaving;       // 定时数据的更新（小时）

    DWORD   m_dwExtraState;         // 玩家的一些附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04
	WORD	m_wMutateID;		    // 变身的图档编号
    WORD    m_wScapegoatID;         // 替身的图档编号 =0 无替身
    WORD    m_wGoatEffectID;        // 替身的效果编号

    WORD    m_wMonsterCountOnGoat;  // 套装替身杀死怪物的计数
    WORD    m_wGoatMutateID;        // 当前替身变身的套装编号(1-11)
    DWORD   m_TVOnGoatMutate_dwTime;// 变身替身时间

    DWORD   m_dwSaveTime;           // 保存时间！

    SAutoUseItemSet m_stAutoUseItemSet[3];    // 自动吃药状态数据

    WORD    m_wCheckID;              // 刷新用编号！

    BYTE    m_bQuestFullData;        // 是否需要向客户端发送初始化数据

	WORD	m_wTeamSkillID;			// 队形技ID
	WORD	m_wTeamMemberCount;		// 队员数量

    DWORD   checkPoints[4];         // 0-无用[或用于后续扩展]， 1-元宝， 2-赠宝， 3-通宝

	//__int64	m_qwMoneyPointTime;		// 元宝请求时间标志
};

///////////////////////////////////////////////////////////////////////////////////////////////
// 场景相关数据
struct SRegion
{
    char    szName[17];             // 场景的名称
    WORD    ID;                     // 该场景的编号
    WORD    MapID;                  // 该场景的地图编号
    WORD    NeedLevel;              // 该场景需求的等级
};

static const int MAX_SCORE_NUMBER = 10;
static const int MAX_SCHOOL_NUMBER = 5;
static const int MAX_FIVESCORE_NUMBER = 5;
// 排行榜相关数据
struct SScoreTable
{
    enum ScoreType {
                                     //  【名称】              【判断条件】
        ST_ALEVEL,                   // 本服10大            [等级]
        ST_AMONEY,                   // ....10大富豪        [侠义币]
        ST_RFAME,                    // ....10大侠客        [侠义值]
        ST_LFAME,                    // ....10大魔头        ...
        ST_MEDICALETHICS,            // ....10大医者        [医德值]
        ST_KILLVAL,                  // ....10大恶人        [PK值]      [等级]

        ST_SLEVEL,                   // 门派10大            [门派]      [等级]
        ST_SXVALUE,                  // [暂时没用]          ...         [侠义值]
        ST_SMONEY,                   // 门派10大富豪        ...         [侠义币]	

        //new
        ST_ROSENUM,                  // 公子佳人            [玫瑰数]
        ST_BAOTU,                    // 宝图大师            [挖宝数]
        ST_ZHENFA,                   // 阵法宗师            [闯关积分]
        ST_HUNTER,                   // 狩猎游侠            [狩猎积分]
        ST_MIGONG,                   // 迷宫校尉            [迷宫积分]   

		ST_FACTION,					 // 五强帮派

        ST_MAX,
    };

    struct SScoreRecode
    {
        DWORD dwValue;  // 当前值
        char Name[11];  // 角色名称
    };

    struct SScoreRecodeEx
    {
        DWORD dwValue;   // 当前值
        char  Name[11];  // 角色名称
        DWORD sid;       // 玩家SID
        WORD  level;     // 玩家等级
    };

    // 门派排行
    SScoreRecode Level[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // 等级排行榜
    SScoreRecode XValue[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];    // 名望排行榜
    SScoreRecode Money[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // 金钱排行榜

    // 江湖排行
    SScoreRecode ALevel[MAX_SCORE_NUMBER];
    SScoreRecode AMoney[MAX_SCORE_NUMBER];
    SScoreRecode FAME[2][MAX_SCORE_NUMBER];                      // 正、恶排行

	// 战斗力排行
    SScoreRecode PowerValue[2][MAX_SCORE_NUMBER];

	// 医德排行
	SScoreRecode MedicalEthics[MAX_SCORE_NUMBER];		         // 医德排行

    // 新增
    SScoreRecode PKValue[MAX_SCORE_NUMBER];                      // 恶人

    SScoreRecodeEx RoseNum[MAX_FIVESCORE_NUMBER];                      // 公子佳人
    SScoreRecodeEx BaoTu[MAX_FIVESCORE_NUMBER];                        // 宝图大师
    SScoreRecodeEx ZhenFa[MAX_FIVESCORE_NUMBER];                       // 阵法宗师
    SScoreRecodeEx HuntScore[MAX_FIVESCORE_NUMBER];                    // 狩猎游侠
    SScoreRecodeEx MazeScore[MAX_FIVESCORE_NUMBER];                    // 迷宫校尉
};

// 追杀令
static const int MAX_KILLINFO = 10;
static const int KILLINFO_BASEMONEY = 10000;

struct SSimplePlayer
{
    // 基本数据
    DWORD   dwStaticID;         // 该玩家在数据库中的唯一编号
    DWORD   dwGlobalID;         // 该玩家本次登录的全局关键字

    WORD    wServerID;          // 该玩家区域服务器ID
    unsigned __int64 dnidClient;// 区域服务器上该玩家的DNID

    WORD    wCurRegionID;       // 所在场景的ID
    DWORD   dwGMLevel;          // GM等级

	BYTE	bOnlineState;		// 0:离线  1:在线  2:挂机 
};

struct KILLINFO {
	KILLINFO() 
	{
		dwValue = 0;
		szName[0] = 0;

	}

	DWORD dwValue;
	char szName[11];
};



//--------------------------------------------------------------------
//公告信息列表的数据结构 Add By Lovelonely
//--------------------------------------------------------------------

const int MAXPUBLICINFO=128;
const int MAXPUBLICINFONUM=10;

struct SPublicInfo
{
	SPublicInfo(){nTimeAdd=0;}

	char	szSay[MAXPUBLICINFO];	//内容
	long	lClr;					//颜色
	int		nTime;					//间隔时间
	int     nTimeAdd;				//时间累计

	WORD	wType;

	enum
	{
		TP_CHANNEL,					//聊天频道显示
		TP_ROLL,					//滚动显示
	};
};

class CPublicInfoTab
{
public:

	CPublicInfoTab() {m_listInfo.clear();}

	std::list<SPublicInfo> m_listInfo;
	
	void	AddPInfo(SPublicInfo pInfo){if (m_listInfo.size() >= MAXPUBLICINFONUM)
										m_listInfo.pop_front();
										m_listInfo.push_back(pInfo);}

};

/*
//固定的玩家属性，会保存下来的数据
struct SFixPlayerDataBuf:
    public SFixBaseData,
    public SFixPackage,   
    public SPlayerTasks,
    public SPlayerGM,
    public SPlayerRelation,
    public SPlayerVenapointEx,
    public SPlayerDialUnit
{
    DWORD   m_dwStaticID;  // 玩家对服务器集群唯一静态的ID，由数据库服务器生成。
};

//仓库数据
struct SWareHouses
{
	SFixStorage1 m_WareHouse[3];
	//bool m_blocked;//仓库是否锁定
};
*/

//固定的玩家属性，会保存下来的数据
typedef SFixProperty SFixPlayerDataBuf;

//--------------------------------------------------------------------
// 跨服保存的数据
struct SSpanPlayerDataBuf :
    public SFixBaseData,
    public SPlayerVenapointEx,
    public SPlayerDataExpand
{
    static const int VERSIONID = 0;
    SSpanPlayerDataBuf()
    {
        m_version = GetVersion(); 
    }

    static DWORD GetVersion() { return (  sizeof( SSpanPlayerDataBuf ) << 16 | VERSIONID ); };
};


//新版本中无账号公用数据
struct SWareHouses
{
	//SFixStorage1 m_WareHouse[3];
	//bool m_blocked;//仓库是否锁定
};

//--------------------------------------------------------------------

//extern BOOL GenerateNewItem( SRawItemBuffer &itemBuffer, const GenItemParams &params, const LogInfo &log );
//extern BOOL GenerateNewItem( class CPlayer *player, const GenItemParams &params, const LogInfo &log );
