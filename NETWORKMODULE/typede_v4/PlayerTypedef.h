#pragma once
#include "../pub/ConstValue.h"
#include "../NETWORKMODULE/NewPetTypedef.h"
#include "../NETWORKMODULE/ConstValueSG.h"

#define GMMODULEON
//wk 20150205 优化不要的数据
//
//const int MAX_FRIEND_NUMBER			= 200;			//好友
//const int MAX_BLOCK_NUMBER			= 100;			//黑名单
//const int MAX_MATE_NUMBER			= 100;			//结义
//const int MAX_ENEMIE_NUMBER			= 100;			//仇人
//
////const int MAXPLAYERLEVEL			= 75;			// 玩家的最高等级
//
//const int MAX_WAREHOUSE_ITEMNUMBER	= 72;			// 仓库的道具最大数量
//const int MAX_TASK_NUMBER			= 20;			// 玩家最大的任务数量
//const int MAX_WHOLE_TASK			= 8192 * 8;		// 最大任务数
//
//const int MAX_TELEGRYNUM			= 50;			// 心法数量
//const int MAX_SKILLCOUNT			= 50;			// 技能数量
//
//const int MAX_JM_XUE				= 195;			// 当前系统穴的数量
//const int MAX_JM_XUE_MEMORY			= 32;			// 所需存储量
//const int MAX_MAI_NUMBER			= 9;			// 最大的脉线
//
//const int MAX_FIGHTPET_NUM			= 6;			// 携带最大侠客数
//const int MAX_NEWPET_NUM = 99;		//玩家携带的最大宠物数
//
//const int MAX_ROSERECOD_NUM			= 100;			// 最大记录的送花记录数
//const int ALL_RANK_NUM				= 3000;			// 从数据库里面取出的最大排行榜条数
//const int SizeTask					= 100;
//#define MAX_TITLE (16+1)							// 玩家所拥有的称号的最大数（16个系统+1个玩家）
//const int MAX_ONLINEGIFT_NUM		= 50;			// 在线奖励最大数组
//#define TASK_MAX	8
//#define TASK_MAX_TYPE	8


//const int MAX_FRIEND_NUMBER = 100;			//好友
const int MAX_BLOCK_NUMBER = 1;			//黑名单
const int MAX_MATE_NUMBER = 1;			//结义
const int MAX_ADD_NUMBER = 50;			//可以添加的好友列表
const int MAX_ENEMIE_NUMBER = 1;			//仇人

//const int MAXPLAYERLEVEL			= 75;			// 玩家的最高等级

const int MAX_WAREHOUSE_ITEMNUMBER = 1;			// 仓库的道具最大数量
const int MAX_TASK_NUMBER = 1;			// 玩家最大的任务数量
const int MAX_WHOLE_TASK = 1;		// 最大任务数

const int MAX_TELEGRYNUM = 1;			// 心法数量
const int MAX_SKILLCOUNT = 1;			// 技能数量

const int MAX_JM_XUE = 1;			// 当前系统穴的数量
const int MAX_JM_XUE_MEMORY = 1;			// 所需存储量
const int MAX_MAI_NUMBER = 1;			// 最大的脉线

const int MAX_FIGHTPET_NUM = 1;			// 携带最大侠客数
const int MAX_NEWPET_NUM = 1;		//玩家携带的最大宠物数

const int MAX_ROSERECOD_NUM = 1;			// 最大记录的送花记录数
const int ALL_RANK_NUM = 1;			// 从数据库里面取出的最大排行榜条数
const int SizeTask = 1;
#define MAX_TITLE (1)							// 玩家所拥有的称号的最大数（16个系统+1个玩家）
const int MAX_ONLINEGIFT_NUM = 1;			// 在线奖励最大数组
#define TASK_MAX	1
#define TASK_MAX_TYPE	1
// 侠义道3门派定义职业
enum XYD3_SCHOOL
{
	XS_NONE = 0,
	XS_WUYUE = 1,		// 人类
	XS_SHUSHAN,			// 狼人
	XS_SHAOLIN,			// 吸血鬼
	XS_CIHANG,			// 天使
	XS_RIYUE,			// 日月

	XS_MAX,				// Max
};

// 金钱类型
enum TPlayerMoneyType
{
	EMoneyType_NonBind = 1, // 非绑定金币
	EMoneyType_Bind = 2, // 绑定金币	
};

// 侠义道3玩家状态标记
enum PLAYER_SHOW_EXTRASTATE
{
	PSE_NONE		= 0x00000000,	// 空状态
	PSE_STALL_SALE	= 0x00000001,	// 摆摊卖
};

// 侠义道3心法数据结构
struct SXYD3Telergy
{
	WORD	m_TelergyID;		// 心法ID
	WORD	m_TelergyLevel;		// 心法等级
};

struct SFixBaseData
{
    char	m_szTongName[11];	// 帮会名称，同时为用于查找该帮派的关键字
	DWORD	m_dwPowerVal;		// 战斗力

	SFixBaseData() { memset(m_szTongName, 0, sizeof(m_szTongName)); }
};

struct SFixPackage				// 基本包裹（基本+任务+扩展1+扩展2共96个格子）
{
	SPackageItem	m_BaseGoods[PackageAllCells];
};

struct SFixStorage1
{
    SPackageItem    m_pStorageGoods[MAX_WAREHOUSE_ITEMNUMBER];		// 道具的仓库
	DWORD			m_whMoney;										// 仓库非绑定货币
	DWORD			m_whBindMoney;									// 仓库的绑定货币
	// ------------- 如果要存入到仓库，用到得时候在打开---------------------------
// 	DWORD			m_whZengBao;									// 仓库中的赠宝
// 	DWORD			m_whYuanBao;									// 仓库中的元宝
	BYTE			m_ActiveTimes;									// 激活的次数
};

// *************************** 侠义道3任务系统 ***************************
enum TaskStatus
{
	TS_FAIL,								//任务失败
	TS_NOTACCEPT = 1,				// 未接
	TS_ACCEPTED,					// 已接（完成中）
	TS_FINISHED,					// 已完成（未交）
	TS_COMPLETE,					// 已交
	TS_TALKIMME,					// 对话（中介）
	TS_GIVE,                        // 给NPC道具（中介）
	TS_GET,                         // 从NPC处获得道具（中介）
	TS_FUTUREACCEPT,				// 未来可接（主线任务断开时会存在）
};

enum TaskFlagType
{
	TT_ITEM  = 1,					// 物品
	TT_KILL,						// 杀怪
	TT_TALK,						// 对话
	TT_GIVE,						// 送物品
	TT_GET,                         // 获取道具
	TT_USEITEM,						// 使用道具
	TT_REGION,						// 跑地图
	TT_SCENE,						// 完成关卡
	TT_ADD,						// 新增任务
	TT_EMPTY = 100,					// 空旗标
};

// 用于传送任务信息（点击NPC后）
struct TaskInfo
{
	WORD	m_TaskID;
	BYTE	m_TaskStatus;			// 任务状态
};

#pragma pack (push)
#pragma pack (1)

// 任务旗标的信息解析器
typedef struct STaskFlagImpl
{
	union
	{
		// 对于对话旗标
		struct
		{
			DWORD	NpcID;
			WORD	Times;
			WORD	CurTimes;
		}Talk;

		// 对于道具旗标
		struct
		{
			DWORD	ItemID;
			WORD	ItemNum;
			WORD	CurItemNum;
		}Item;

		// 对于给/获取旗标
		struct  
		{
			DWORD	NpcID;
			DWORD	ItemID;
			WORD	ItemNum;
			WORD	CurItemNum;
		}GiveGet;

		// 对于杀怪旗标
		struct
		{
			DWORD	MonsterID;
			WORD	KillNum;
			WORD	CurKillNum;
		}Kill;

		// 对于使用道具旗标
		struct  
		{
			DWORD	ItemID;
			WORD	UseTimes;
			WORD	CurUseTimes;
		}UseItem;

		// 对于跑地图旗标
		struct
		{
			WORD	DestMapID;
			WORD	DestX;
			WORD	DestY;
		}Region;

		// 对于场景关卡旗标
		struct
		{
			WORD	DestSceneID;
			WORD	DestX;
			WORD	DestY;
		}Scene;

		//新增任务
		struct 
		{
			enum 
			{
				TASK_EINTENSIFY,  //装备强化
				TASK_EJDING,  //装备鉴定
				TASK_ERISE,  //装备升星
				TASK_EKEYIN,  //装备刻印
				TASK_EQUALITY,  //装备升阶
				TASK_XINYANG, //信仰任务
				TASK_TRANSFORMERS, //变身任务
				TASK_SKILL, //技能任务 技能升级
				TASK_SCENECOMPLETE, //关卡完成
			};
			BYTE btype;//任务类型
			WORD wCount; //当前次数
			WORD wAllCount;//总次数
			DWORD dIndex;	//对应装备任务是道具ID，对于技能任务为技能ID
		}TaskADD;

	};
}STFImpl;

// 任务旗标的数据结构
struct STaskFlag
{
	STaskFlag() 
	{
		memset(this, 0, sizeof(STaskFlag));
	}
	BYTE		m_Type;				// 旗标类型
	BYTE		m_Complete;			// 是否完成
	DWORD		m_Index;			// 高16位：主任务，低16位：旗标索引
	STFImpl		m_TaskDetail;		// 旗标的解析器
};

// 玩家身上所接的任务
struct SPlayerTask
{
	WORD		m_TaskID;			// 任务ID
	BYTE		m_FlagCount;		// 任务旗标数目
	QWORD		m_CreateTime;		// 任务创建时间
	STaskFlag	m_Flags[8];			// 任务旗标数据
};

struct SPlayerCycTask
{
	WORD	m_TaskID;				//任务ID，最后一个完成的日常任务索引ID
	char	m_CompleteTime[30];		//交最后一个任务完成时间
	BYTE	m_type;					//taskType = 1(单个任务)  taskType = 2(多个任务)(必须填写)
	BYTE    m_cyctype;				//cyctype  = 1(日常任务),	2(跑环(即时)任务),	3(周长任务)
	SPlayerCycTask() : m_TaskID(0),m_type(0),m_cyctype(0)
	{
		memset(m_CompleteTime, 0, 30);
	}
};
struct SPlayerRdTaskInfo //随机任务 这个结构体只是记录当天的任务情况
{
	WORD m_NpcID;									  //NpcID
	WORD m_RdTaskNum;								  //完成任务的数量 今天完成的任务数量
	WORD m_TaskID;									  //右键点击这个   今天必须接这个任务ID
	SPlayerRdTaskInfo():m_NpcID(0),m_RdTaskNum(0){}
};
struct SPlayerLimtTime
{
	char m_CompleteTime[30];		                  //上一次的上线时间
	SPlayerRdTaskInfo m_SPlayerRdTaskInfo[80];
	BYTE m_RdNum;
	BYTE flag;
	SPlayerLimtTime():flag(0),m_RdNum(0)
	{
		memset(m_CompleteTime,0,30);
	}
	void UpdataLimtTimeData()
	{
		m_RdNum = 0;
		memset(m_CompleteTime,0,30);
		memset(m_SPlayerRdTaskInfo,0,sizeof(SPlayerRdTaskInfo));
	}
	bool IsExitNpcIDWithIndex(WORD NpcID,long &index)
	{
		bool flag = false;
		for (long i = 0; i < m_RdNum; ++i)
		{
			 if ( m_SPlayerRdTaskInfo[i].m_NpcID == NpcID)
			 {
				index = i;
				flag = true;
				break;
			 }
		}
		return flag;
	}
};
#pragma pack (pop)

// 玩家的任务信息，最多支持记录MAX_WHOLE_TASK个任务
struct SPlayerTasks
{
	SPlayerTask		m_PlayerTask[MAX_TASK_NUMBER];		// 任务数据
	BYTE			m_flags[MAX_WHOLE_TASK / TASK_MAX_TYPE];			// 任务完成标记
	SPlayerCycTask  m_PlayerCyc[20];
	SPlayerLimtTime m_SPlayerLimtTime;
	int MarkComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;
		
		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;

		m_flags[group] |= (1 << idx); 
		return 1;
	}
	
	int MarkUnComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;

		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;
		
		m_flags[group] &= ~(1 << idx); 
		return 1;
	}

	int IsComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;

		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;

		return (m_flags[group] & (1 << idx)) ? 1 : 0;
	}

	void ClearCompleteMark(WORD taskID)
	{
		if (taskID == 0)
		{
			return;
		}
		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		m_flags[group] = 0;
	}
};
// *************************** 侠义道3任务系统 ***************************

// 玩家的经脉信息，目前最多支持256个穴位
struct SPlayerJingMai
{
	BYTE m_JingMai[MAX_JM_XUE_MEMORY];
	BYTE m_byShareExpTimes; //gw+ 还有分享经验次数
	//INT64 m_countShareExpLoginTime; //gw+分享经验登录时间（限制每天的次数）

	//gw+init
	SPlayerJingMai()
	{
		memset(this, 0, sizeof(*this));
	}
	// 设置经脉充穴分享经验次数
	void SetJingMaiShareExpTimes(BYTE byShareTimes)
	{
		m_byShareExpTimes = byShareTimes;
	}
	BYTE GetJingMaiShareExpTimes()
	{
		return m_byShareExpTimes;
	}

	int MarkVenPoint(WORD VenPointID)
	{
		if (0 == VenPointID || VenPointID > MAX_JM_XUE)
			return -1;

		DWORD group = (VenPointID-1) / 8;
		DWORD idx	= 7 - (VenPointID-1) % 8;

		m_JingMai[group] |= (1 << idx);
		return 1;
	}

	int IsVenMarked(WORD VenPointID)
	{
		if (0 == VenPointID || VenPointID > MAX_JM_XUE)
			return -1;

		DWORD group = (VenPointID-1) / 8;
		DWORD idx	= 7 - (VenPointID-1) % 8;

		return (m_JingMai[group] & (1 << idx)) ? 1 : 0;
	}
};
// *************************** 侠义道3经脉系统 ***************************

struct SPlayerGM
{
    DWORD  m_GMLevel;
};

// 角色的坐骑数据
struct SPlayerMounts
{
	//wk 20150205 优化不要的数据
	//enum { MAX_MOUNTS = 8, };
	enum { MAX_MOUNTS = 1, };
	enum { State_None, State_Equip, State_Riding, };
	enum 
	{
		SPM_MHP,			// 生命上限
		SPM_MMP,			// 内力上限
		SPM_MTP,			// 体力上限
		SPM_GONGJI,			// 攻击
		SPM_FANGYU,			// 防御
		SPM_BAOJI,			// 暴击
		SPM_SHANBI,			// 闪避
		SPM_SPEED,			// 速度（不保存）

		SPM_MAX,
	};

	struct Mounts
	{
		Mounts() { ID = 0; }

		WORD	ID;						// 编号
		DWORD	GrowPoint;				// 经验
		BYTE	Level;					// 等级
		BYTE	State;					// 状态
		WORD	Points[SPM_SPEED];		// 属性
	} mount[MAX_MOUNTS];
};

// 角色的非战斗宠物数据
struct SPlayerPets
{
	//wk 20150205 优化不要的数据
	//enum { MAX_PET = 5, };							// 最大宠物数
	enum { MAX_PET = 1, };							// 最大宠物数
	enum { Pet_State_None, Pet_State_Equip, };		// 状态
	enum PetAttriType 
	{
		PAT_WAIGONG,
		PAT_NEIGONG,
		PAT_TIZHI,
		PAT_WUXING,
		PAT_SHENFA,

		PAT_MAX,
	};

	struct PetAttri
	{
		WORD	type	: 4;						// 支持15种属性
		WORD	value	: 8;						// 最高值255
		WORD	reserve	: 4;						// 保留
	};

	struct Pets
	{
		BYTE		ID;					// 编号
		BYTE		Sex;				// 性别	
		BYTE		Level;				// 等级
		BYTE		State;				// 状态
		BYTE		LeftPoint;			// 剩余点数
		char		Name[11];			// 名称
		WORD		Attri[PAT_MAX];		// 五种属性
		WORD		Quality[PAT_MAX];	// 五种资质
		WORD		Happyness;			// 欢乐度
		WORD		FuseDegree;			// 融合度
		DWORD		UpdateTimer;		// 更新计时器
	}pets[MAX_PET];
};

// 侠义道3侠客系统
struct SFightPetExt : public SFightPetBase
{
	SEquipment	m_Equip[EQUIP_P_MAX];	// 侠客目前所装备的道具
};

// 侠义道3的玩家基本数据
struct SXYD3FixData
{
	DWORD		m_version;				// 版本号，用于二进制数据扩展
	DWORD		m_LeaveTime;			// 玩家离线时的时间, 当前用于保存玩家不在线的总时间（通过m_OnlineTime和m_LeaveTime可以计算出玩家的注册时间）
	DWORD		m_LeaveIP;				// 玩家离线时的IP
	DWORD		m_OnlineTime;			// 在线时间， 当前用于保存玩家在线的总时间
	BYTE		m_Sex:1;				// 性别
	BYTE		m_BRON:3;				// 玩家所显示图片级别，1表示出师
	BYTE		m_School:4;				// 门派
	BYTE		m_CurTitle;				// 当前显示的头衔
	BYTE		m_FaceID;				// 脸模型
	BYTE		m_HairID;				// 头发模型
	WORD		m_Level;				// 等级
	WORD		m_TurnLife;				// 转生次数
	DWORD		m_TongID;				// 所属帮派ID
	DWORD		m_ShowState;			// 玩家的变身状态0 原始 1，2对应变身状态
	
	DWORD		m_BindMoney;			// 绑定货币（不可交易，不收税）//银两
	DWORD		m_Money;				// 非绑定货币（可交易，不收税）//银币
	DWORD		m_ZengBao;				// 赠宝（不可交易）XYD3--	   //礼券
	QWORD		m_ZBTotalReplenish;		// 玩家获得的赠宝总数
	DWORD		m_YuanBao;				// 元宝（现实货币，可交易，收税）
	DWORD		m_YBTranOut;			// 元宝交易总出帐数
	DWORD		m_YBTranIn;				// 元宝交易总进账数
	DWORD		m_YBTranTax;			// 元宝交易总税收数
	QWORD		m_YBTotalReplenish;		// 玩家充值的总数额
	
	DWORD		m_CurHp;				// 当前生命
	DWORD		m_CurMp;				// 当前内力
	DWORD		m_CurSp;				// 当前真气
	DWORD		m_CurTp;				// 当前体力
	DWORD		m_CurJp;				// 当前精力
	
	WORD		m_JingGong;				// 进攻
	WORD		m_FangYu;				// 防御
	WORD		m_QingShen;				// 轻身
	WORD		m_JianShen;				// 健身

	QWORD		m_Exp;					// 玩家当前经验
	WORD		m_RemainPoint;			// 玩家剩余点数
	WORD		m_MingWang;				// 名望
	WORD		m_PKValue;				// PK值
	WORD		m_XYValue;				// 侠义值
	WORD		m_EngValue;				// 侠客激励值
	
	WORD		m_CurRegionID;			// 当前地图ID
	float		m_X;					// 当前世界坐标
	float		m_Y;
	float		m_Z;
	float		m_dir;					// 方向

	WORD		m_wExtGoodsActivedNum;	// 扩展背包被激活格子数(总被激活的数量=初始激活+扩展激活数)

	bool		m_bWarehouseLocked;		// 是否锁定了仓库
	BYTE		m_FightPetActived;		// 当前用于保存玩家是否处于变身状态，如果是，保存宠物的索引（0-99），如果不是，则为0xff

	char		m_Name[CONST_USERNAME];					// m_Name[CONST_USERNAME-1] == 0
	char		m_Title[MAX_TITLE][CONST_USERNAME];		// 玩家的称号，0为玩家自定义的。2014/3/27 ly默认第一个元素为当前使用中的称号
	SEquipment	m_Equip[EQUIP_P_MAX];					// 玩家目前所装备的道具
	SSkill		m_pSkills[MAX_SKILLCOUNT];				// 目前已学习的武功
	SXYD3Telergy	m_Xyd3Telergy[MAX_TELEGRYNUM];		// 目前已学习的心法
	SFightPetExt	m_FightPets[MAX_FIGHTPET_NUM];		// 玩家身上的侠客

	INT64		m_lastBiguanTime;
	INT64		m_dayOnlineTime;	// 每天在线累计时间
	BYTE		m_bStopTime;		// 是否停止计时,现已经改为是否为初次创建玩家
	BYTE		m_onlineGiftStates[MAX_ONLINEGIFT_NUM];	// 在线奖励领取状态
	BYTE		m_FashionMode;		// 当前的服装模式，是时装还是普通
	BYTE		m_bVipLevel;		// 玩家VIP等级
	INT64		m_dLoginTime64;		// 仅在玩家登陆游戏时记录他的登陆时间
	INT64		m_dLeaveTime64;		// 仅在玩家离开游戏时记录他的离开时间

	INT64		m_CountDownBeginTime;		//倒计时礼包-开始时间
	BYTE     	m_dTimeCountDownGiftState;	//倒计时礼包-当前应领取的倒计时礼包索引


	DWORD		m_KillBossNum;		// 击杀BOSS数目

	////玩家技能相关的数据
	BYTE      m_PlayerPattern;		//当前用于保存玩家是否为首次充值标识(0为玩家一次也没有充值过；1为玩家当前充值了一次，还没有领取奖励；2为玩家领取首次充值奖励后的状态)
	DWORD		m_CurUsedSkill[10];		//当前玩家使用的技能。0-2为普通形态下所使用的技能；3-5为形态1下；6-8为形态2下的；9为付费使用的技能
	////玩家祈祷相关的数据，玩家每天获得的祈祷相关数据，24点后数据重置
	BYTE m_bySilerCoinUsedNum;	//当天使用的银币数
	BYTE m_byAnimaUsedNum;		//当天使用的灵力数
	BYTE m_byExpUsedNum;		//当天使用的经验数
	time_t m_dwPreReqTime;		//当天使用的时间。0表示当天没有使用，具体时间表示已经使用

	////玩家元宝礼包相关数据
	BYTE m_IsBuyed;	//是否已经购买
	DWORD m_YuanBaoGiftPos;		//点击元宝礼包的位置

	//add by ly 2014/5/24 新宠物系统的宠物数据
	BYTE m_CurPetNum;	//当前玩家拥有的宠物数
	SNewPetData m_NewPetData[MAX_NEWPET_NUM];
	DWORD m_TransPetIndex;		//变身出战宠物索引
	DWORD m_FollowPetIndex;	//跟随出战宠物索引
};
//麒麟臂数据
struct SPlayerKylinArm
{
	BYTE	byKylinArmLevel;
	DWORD	wYuanqi;

	const static BYTE MaxLevel = 9;
};
//体质数据
struct SPlayerTiZhi
{
	enum
	{
		TZ_MAX_HP = 0,				//生命
		TZ_MAX_MP,					//内力
		TZ_MAX_TL,					//体力
		TZ_MAX_AT,					//攻击
		TZ_MAX_DF,					//防御
		TZ_MAX_BJ,					//爆击
		TZ_MAX_SB,					//闪避
		TZ_MAX_SH,					//伤害
		TZ_MAX_MS,					//免伤
		TZ_MAX_RS,					//移动速度
		TZ_MAX_AS,					//攻速
		//TZ_MAX_HA,				//副手攻击率		
		TZ_MAX_QL,					//麒麟臂机率		
		TZ_MAX,
	};
	BYTE byTiZhiLevel;
	DWORD dwTiZhiAttrs[TZ_MAX];
	const static BYTE MaxLevel = 10;
};
// 好友，黑名单
// struct SPlayerRelates
// {
// 	char		szName[CONST_USERNAME];
// 	BYTE		byRelation	:4;		// 关系
// 	BYTE		wLevle;					// 等级                                                              
// };

// struct SPlayerRelation
// {
// 	SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];	// 好友
// };

struct sRoseRecod
{
	char		szName[CONST_USERNAME];
	WORD		wYear;				// 记录年月日
	BYTE		wMonth		:4;
	BYTE		wDay		:5;            
	DWORD		wIndex;				// 花的道具ID
	WORD		wRosenum;			// 数量
};

struct SPlayerRoseRecod //记录好友送花的记录
{
	sRoseRecod m_PlayerRoseRecod[MAX_ROSERECOD_NUM];
	DWORD	m_PlayerRoseNum;			//玩家当前的花总数
	UINT64		m_uPlayerRoseUpdata;		//玩家当前送花状态值，这个值用于判断是否更新客户端
};

// 用于保存
struct SPlayerUnionBase
{
};

// 用于存放在游戏内存中
struct SPlayerUnionData : public SPlayerUnionBase
{
};
// 
struct GKLUpLevelLimt //鬼谷令使用限制
{
	char  GKLtime[30];  //记录时间
	short m_GKLUseNum;	//次数
	BYTE  m_GKLFlag;
	GKLUpLevelLimt():m_GKLUseNum(0),m_GKLFlag(0)
	{
		memset(GKLtime,0,30);
	}
	void AddUseNum(short Num)
	{
		m_GKLUseNum += Num;
		if (m_GKLUseNum < 0)
		{
			m_GKLUseNum = 0;
		}
		if (m_GKLUseNum > 2)
		{
			m_GKLUseNum = 2;
		}
	}
};
struct BlessLevelLimt
{
	char  Blesstime[30];  //记录时间
	short m_BlessUseNum;	//次数
	BYTE  m_BlessFlag;
	BYTE  m_BlessOnce;
	BYTE  m_HightiTEM;
	SPackageItem m_BlessSPackageitem;
	BlessLevelLimt():m_BlessUseNum(0),m_BlessFlag(0),m_BlessOnce(0),m_HightiTEM(0)
	{
		memset(Blesstime,0,30);
	}

};

struct DayPartDataRecord//日常副本记录
{
	WORD m_RegionID;	//EventRegion[...]事件区域ID[...]
	BYTE m_EnterNum;	//进入几次
	BYTE m_NumLimit;	//进入副本上限
	DayPartDataRecord():m_RegionID(0),m_EnterNum(0),m_NumLimit(0){}
};
struct DayRecordArray//一个玩家的数据
{
	char  m_DayRecordtime[30];  //记录时间
	BYTE  m_DayRecordFlag;	  //0没有记录1已经记录
	DayPartDataRecord m_DayRecordArray[30];//设计副本上限30个
	DayRecordArray():m_DayRecordFlag(0)
	{
		memset(m_DayRecordtime,0,30);
	}
	void InitRecordArray()
	{
		memset(m_DayRecordArray,0,sizeof(m_DayRecordArray));
		memset(m_DayRecordtime,0,30);
		m_DayRecordFlag = 0;
	}
};

// 保存BUFF数据
struct TSaveBuffData
{
	DWORD	m_dwBuffID; // Buff ID
	int		m_curStep;	// 当前的阶段
	WORD	m_ActedTimes;// Buff已经激活的次数
	bool	m_CanDropBuff;// 是否删除
};
struct SPlayerSaveBuff
{
	SPlayerSaveBuff()
	{
		memset(this, 0, sizeof(*this));
	}
	const static BYTE msc_byMaxSaveBuffCount = 5; //保存的最大Buff数量
	TSaveBuffData m_tSaveBuffData[msc_byMaxSaveBuffCount];
};

//20150121 wk 以前数据,萌将三国数据单独分配5K
//#define LUABUFFERSIZE 32768			//lua的数据存储空间 




#define MAX_BUFFERSIZE 67848        //预留的最大空间
// 固定的玩家属性，会保存下来的数据	
struct SFixProperty : 
    public SFixBaseData,
    public SFixPackage,
    public SFixStorage1,
    public SPlayerTasks,
	public SPlayerJingMai,
    public SPlayerGM,
//    public SPlayerRelation,
	public SPlayerMounts,
	public SPlayerPets,
	public SXYD3FixData,
	public SPlayerTiZhi,
	public SPlayerKylinArm,
	public GKLUpLevelLimt,
	public BlessLevelLimt,
	public SPlayerRoseRecod,
	public DayRecordArray,
	public SPlayerSaveBuff
{
	enum SAVESTATE		// 角色服务器上的状态
	{
		ST_LOGIN =1,	// 刚刚登入
		ST_LOGOUT,		// 退出
		ST_HANGUP,		// 离线挂机
	};

    DWORD   m_dwStaticID;					// 玩家对服务器集群唯一静态的ID，由数据库服务器生成
    char    m_UPassword[CONST_USERPASS];	// 玩家二级密码
	BYTE    m_byStoreFlag;					// 玩家的当前状态，用于区分保存时的信息
	//wk 20150205 优化不要的数据
//	BYTE	m_bcacheBuffer[MAX_BUFFERSIZE]; // 预留的玩家数据
	//BYTE	m_bluaBuffer[LUABUFFERSIZE];	// 用于在lua 中数据的存储
	BYTE	m_bluaBuffer[10];	// 用于在lua 中数据的存储
	// 侠义道3的版本号
    static const int VERSIONID = 5;
    static DWORD GetVersion() { /*rfalse("sizeof(SfixProperty) %d", sizeof(SFixProperty)); */return (sizeof(SFixProperty) << 16 | VERSIONID); };
};


struct SPlayerTempData				// 玩家的临时数据，不保存，但是使用于跨场景服务器时候的数据传递
{
    DWORD	m_dwTeamID;				// 组队的数据
    bool	m_IsRegionChange;		// 是初始化还是场景转换

    struct 
	{
        DWORD GID;                  // pk对象
        DWORD lasttime;             // 剩下的pk时间
    } m_PkList[30];                 // PK对象列表

    DWORD	m_dwTimeLeaving;		// 定时数据的更新
    DWORD	m_dwOneHourLeaving;		// 定时数据的更新（小时）
    DWORD   m_dwExtraState;         // 玩家的一些附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04
	WORD	m_wMutateID;		    // 变身的图档编号
    WORD    m_wScapegoatID;         // 替身的图档编号 =0 无替身
    WORD    m_wGoatEffectID;        // 替身的效果编号
    WORD    m_wMonsterCountOnGoat;  // 套装替身杀死怪物的计数
    WORD    m_wGoatMutateID;        // 当前替身变身的套装编号(1-11)
    DWORD   m_TVOnGoatMutate_dwTime;// 变身替身时间
    DWORD   m_dwSaveTime;           // 保存时间！
    WORD    m_wCheckID;             // 刷新用编号！
    BYTE    m_bQuestFullData;       // 是否需要向客户端发送初始化数据
	WORD	m_wTeamSkillID;			// 队形技ID
	WORD	m_wTeamMemberCount;		// 队员数量
    DWORD   checkPoints[4];			// 0-无用[或用于后续扩展]， 1-元宝， 2-赠宝， 3-通宝
};

// 场景相关数据
struct SRegion
{
    char    szName[17];             // 场景的名称
    WORD    ID;                     // 该场景的编号
    WORD    MapID;                  // 该场景的地图编号
    WORD    NeedLevel;              // 该场景需求的等级
};

static const int MAX_SCORE_NUMBER		= 10;
static const int MAX_SCHOOL_NUMBER		= 5;
static const int MAX_FIVESCORE_NUMBER	= 5;

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

    SScoreRecodeEx RoseNum[MAX_FIVESCORE_NUMBER];                // 公子佳人
    SScoreRecodeEx BaoTu[MAX_FIVESCORE_NUMBER];                  // 宝图大师
    SScoreRecodeEx ZhenFa[MAX_FIVESCORE_NUMBER];                 // 阵法宗师
    SScoreRecodeEx HuntScore[MAX_FIVESCORE_NUMBER];              // 狩猎游侠
    SScoreRecodeEx MazeScore[MAX_FIVESCORE_NUMBER];              // 迷宫校尉
};

struct SSimplePlayer
{
    DWORD   dwStaticID;         // 该玩家在数据库中的唯一编号
    DWORD   dwGlobalID;         // 该玩家本次登录的全局关键字

    WORD    wServerID;          // 该玩家区域服务器ID
    QWORD	dnidClient;			// 区域服务器上该玩家的DNID

    WORD    wCurRegionID;       // 所在场景的ID
    DWORD   dwGMLevel;          // GM等级

	BYTE	bOnlineState;		// 0:离线  1:在线  2:挂机 
};

// 公告信息列表的数据结构
const int MAXPUBLICINFO		= 128;
const int MAXPUBLICINFONUM	= 10;

struct SPublicInfo
{
	SPublicInfo() { nTimeAdd=0; }

	char	szSay[MAXPUBLICINFO];	// 内容
	long	lClr;					// 颜色
	int		nTime;					// 间隔时间
	int     nTimeAdd;				// 时间累计
	WORD	wType;

	enum
	{
		TP_CHANNEL,					// 聊天频道显示
		TP_ROLL,					// 滚动显示
	};
};

class CPublicInfoTab
{
public:
	CPublicInfoTab() { m_listInfo.clear(); }

	std::list<SPublicInfo> m_listInfo;

	void AddPInfo(SPublicInfo pInfo)
	{
		if (m_listInfo.size() >= MAXPUBLICINFONUM)
			m_listInfo.pop_front();
		m_listInfo.push_back(pInfo);
	}
};

// 固定的玩家属性，会保存下来的数据
typedef SFixProperty SFixPlayerDataBuf;

// 跨服保存的数据
struct SSpanPlayerDataBuf : public SFixBaseData, public SXYD3FixData
{
    static const int VERSIONID = 0;

    SSpanPlayerDataBuf()
    {
        m_version = GetVersion(); 
    }

    static DWORD GetVersion() { return (sizeof( SSpanPlayerDataBuf ) << 16 | VERSIONID); };
};

// 新版本中无账号公用数据
struct SWareHouses
{
};

// 
// struct SRankList
// {
// 	struct SRankContent
// 	{
// 		char	name[11];	// 名称
// 		BYTE	school;		// 门派
// 		DWORD	value;		// 当前值
// 	};
// 
// 	SRankContent Level[MAX_RANDLIST_NUM];		// 等级排行
// 	SRankContent Money[MAX_RANDLIST_NUM];		// 金钱排行
// };

// ============================XYD3排行榜====================
//20150427 wk 100 改为1
//static const int MAX_RANKLIST_NUM	= 100;		// 发送给客户端最大排行榜数目
static const int MAX_RANKLIST_NUM = 1;		// 发送给客户端最大排行榜数目

struct NewRankList
{
	DWORD Level;				// 等级
	DWORD Money;				// 非绑定货币
	DWORD BossNum;				// 击杀BOSS数目
	char name[CONST_USERNAME];	// 昵称
	BYTE  School;				// 门派

	NewRankList():Level(0),Money(0),BossNum(0)
	{
		memset(name,0,CONST_USERNAME);
	}
};

// 用于发送给客户端的排行榜结构
enum RANK_TYPE
{
	RT_LEVEL,
	RT_MONEY,
	RT_BOSSKILL,

	RT_MAX,
};

struct Rank4Client
{
	Rank4Client() {}
	Rank4Client(char* pName, BYTE rt, DWORD val, BYTE sc) : RankType(rt), RankValue(val), School(sc)
	{
		memcpy(name, pName, sizeof(char) * CONST_USERNAME);
	}


	BYTE	RankType;				// 排行榜类型
	DWORD	RankValue;				// 排行榜数值	
	char	name[CONST_USERNAME];	// 玩家姓名
	BYTE	School;					// 门派
};

//20150821 三国排行榜,普通排行
//排行数值,sid,等级,头像,vipLV,名字,帮会名,heroID,heroStar,heroStep,heroLv  --50个,1为名将榜,多5个参数,武将id 武将星级 武将品阶 武将等级,其他的没有
struct RankList_SG
{
	DWORD Num;				// 排行数值
	DWORD sid;				// sid
	DWORD lv;				// 等级
	DWORD icon;				// 头像
	char name[CONST_USERNAME];	// 昵称
	char fname[CONST_USERNAME];	//帮会名

	RankList_SG() :Num(0), sid(0), lv(0), icon(0)
	{
		memset(name, 0, CONST_USERNAME);
		memset(fname, 0, CONST_USERNAME);
	}
};
//20150821 三国排行榜,名将排行
struct RankList_hero_SG
{
	DWORD Num;				// 排行数值
	DWORD sid;				// sid
	DWORD lv;				// 等级
	DWORD icon;				// 头像
	char name[CONST_USERNAME];	// 昵称
	char fname[CONST_USERNAME];	//帮会名

	DWORD heroID;				// 武将id
	DWORD heroStar;				// 武将星级
	DWORD heroStep;				// 武将品阶
	DWORD heroLv;				// 武将等级

	RankList_hero_SG() :Num(0), sid(0), lv(0), icon(0),  heroID(0), heroStar(0), heroStep(0), heroLv(0)
	{
		memset(name, 0, CONST_USERNAME);
		memset(fname, 0, CONST_USERNAME);
	}
};

//三国排行榜枚举
enum RankType_SG
{
	MaxHero=1,// 名将榜, 
	BattleTeam=2,// 阵容榜, 
	HeroAllStar=3,// 总星榜, 
	PlayerLevel=4,// 等级榜, 
	Overcome=5 ,//过关斩将榜, 
	HeroNum=6,// 武将总数, 
	PlayerMoney = 7,// 金币富豪榜
};
////////////////////////////////////////////////////////


