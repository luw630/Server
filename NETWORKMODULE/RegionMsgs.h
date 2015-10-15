#pragma once

#include "NetModule.h"
#include "movetypedef.h"
#include "StallDef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 场景管理消息类
//=============================================================================================
DECLARE_MSG_MAP(SRegionBaseMsg, SMessage, SMessage::EPRO_REGION_MESSAGE)
//{{AFX
EPRO_DEL_OBJECT,		// 删除场景上的对象
EPRO_DEL_OBJECTLIST,	// 删除场景上的多个对象
EPRO_SYN_PLAYER_INFO,	// 同步玩家信息
EPRO_SYN_NPC_INFO,		// 同步NPC信息
EPRO_SYN_MONSTER_INFO,	// 同步怪物信息
EPRO_SET_EFFECT,	    // 场景上某个对象需要出现的效果(声光效果)
EPRO_OBJECT_INFO,	    // 请求获取场景上某个对象的数据
EPRO_SET_REGION,	    // 通知客户端设置场景
EPRO_SYN_OBJECT,	    // 通知场景上出现了某个对象
EPRO_SYN_SALENAME,	    // 摆摊信息 
EPRO_SETEXTRASTATE,     // 设置附加状态
EPRO_SETPKRULE,			// 设置PK规则
EPRO_ONMUTATE,          // 设置变身状态
EPRO_ONSCAPEGOAT,       // 设置变身状态

EPRO_QUERY_SIMPLAYER,   // 查询角色信息 added by yg
EPRO_SET_REGION_TASK,   // 跑地图 任务更新
EPRO_SYN_PET_INFO,
EPRO_SYN_EFFECTS,  //同步玩家特效
//}}AFX
END_MSG_MAP()

// 通知场景上出现了某个对象
DECLARE_MSG(SSynObjectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_OBJECT)
struct SASynObjectMsg : public SSynObjectMsg
{
	DWORD	dwGlobalID;		// 对象的全局ID编号
	DWORD	dwExtra;		// 扩展
};

//=============================================================================================
// 客户端请求获取场景上某个对象的数据
DECLARE_MSG(SObjectInfoMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_OBJECT_INFO)
struct SQObjectInfoMsg : public SObjectInfoMsg 
{
    enum GETINFO_TYPE {
        GIT_ALL,
        GIT_SALENAME
    };

    WORD  wInfoType;    // 请求的信息类型
	DWORD dwGlobalID;	// 对象的全局ID编号
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// 服务器要求客户端删除场景上的对象
DECLARE_MSG(SDelObjectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_DEL_OBJECT)
struct SADelObjectMsg : public SDelObjectMsg 
{
	DWORD dwGlobalID;	// 对象的全局ID编号
};

DECLARE_MSG(SDelObjectListMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_DEL_OBJECTLIST)
struct SADelObjectListMsg : public SDelObjectListMsg 
{
	WORD  num;
	DWORD dwGlobalIDs[1024];
};

DECLARE_MSG(SSaleNameMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_SALENAME)
struct SASendSaleNameMsg : public SSaleNameMsg
{
    DWORD	dwGlobalID;						// 本次运行全局唯一标识符
    char    szStallName[MAX_STALLNAME];		// 玩家的摊位名称
};

// 服务器要求客户端同步角色信息
DECLARE_MSG(SSynPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PLAYER_INFO)
struct SASynPlayerMsg : public SSynPlayerMsg
{
	enum
	{
		SPT_WEAPON = 0,
		SPT_WRIST,
		SPT_SHOSE,
		SPT_CLOTH,
		SPT_MANTLE,
		SPT_PENDANTS1,
		SPT_PENDANTS2,
		SPT_PRECIOUS,
		SPT_HAT,
		SPT_SHIZHUANG_HAT1,			// 时装头饰1
		SPT_SHIZHUANG_CLOTH1,		// 时装衣服1
		SPT_PAT,				// 裤子
		SPT_WAIST,				// 肩膀
		SPT_PROTECTION,			// 项链
	};

	enum
	{
		COLOR_NORMAL,			//正常颜色,
		COLOR_YELLOW,			//黄色
		COLOR_LIGHTRED,		//浅红色
		COLOR_RED,					//红色
	};

	enum
	{
		NORMALMODE,		//普通模式
		FASHIONMODE,		//时装模式
	};

	BYTE		byCurTitle;		//当前玩家称号
	DWORD		dwGlobalID;				// 玩家GID
	DWORD		dwShowState;			// 玩家变身状态
	char		sName[CONST_USERNAME];	// 玩家名
	char		sTranPetName[PETNAMEMAXLEN];	// 变身宠物名
	BYTE		bySex:1;				// 性别
	BYTE		byBRON:3;				// 保留
	BYTE		bySchool:4;				// 门派
	BYTE		byFace;					// 脸型
	BYTE		byHair;					// 头发
	DWORD		MountID;				// 同步角色坐骑（高16位骑乘ID，低16位骑乘等级）
	WORD		PetID;					// 同步非战斗宠物
	DWORD		PetGlobalID;			// 宠物的唯一ID
	DWORD		PetTransferID;	//变身的宠物ID
	float		mMoveSpeed;				// 速度
	float		direction;				// 方向
	WORD		wAtkSpeed;				// 攻击速度
	DWORD		wEquipIndex[SPT_PROTECTION ];			// 13种可显示的装备,包含2种时装
	SPath		ssp;					// 同步路径
	BYTE			bNameColor;	//名字颜色
	BYTE			bFashionMode;		//时装模式还是普通模式

	int MySize()
	{
		return sizeof(SASynPlayerMsg) - (MAX_TRACK_LENGTH - ssp.m_wayPoints)*sizeof(SWayPoint);
	}
};

/*
//=============================================================================================
// 服务器要求客户端同步角色信息
DECLARE_MSG(SSynPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PLAYER_INFO)
struct SASynPlayerMsg : public SSynPlayerMsg 
{
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE    bySex:1;            // 玩家角色性别
    BYTE    byBRON:3;           // 玩家所显示图片级别，==1表示出师
    BYTE    bySchool:4;         // 玩家所属门派
	BYTE	byPKValue;			// PK值，红名决定
	short	sXValue;			// 侠义值
	BYTE    bCittaLevel;        // 玩家心法等级
	char	sName[MAX_NAMELEN];			// 非0结尾的名字字符串
	char	sTongName[CONST_USERNAME];		// 帮会名称，同时为用于查找该帮派的关键字
	char	sTitle[CONST_USERNAME];			// 该玩家当前显示的称号

	BYTE	isSaleState;	    // 是否是摆摊状态
    BYTE	isFactionMaster:1;	// 是否是帮派头子
    BYTE	byOnlineState:7;    // 玩家在线状态

    DWORD   dwExtraState;       // 玩家的一些附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04

	WORD	wMutateID;			// 玩家当前变身的编号
    WORD    m_wEquipCol[EEC_MAX];       // 玩家衣服的颜色值 

    BYTE    m_byEquipColCount[EEC_MAX]; // 当前换色次数

	float	mMoveSpeed;

    BYTE    dwVenationState[4];   // 经脉受伤状态，每个BYTE表示一个受伤状态，依次：手太阳（轻伤、中伤、重伤、断裂）、手少阳、足太阴、足少阴

    float   fActFrameDec;       // 由敏捷决定的速度

    WORD    wScriptIcon;        // 脚本小图标
    WORD    wScapegoatID;       // 玩家当前替身的编号 （0=没有替身）
    WORD    wGoatEffectID;      // 玩家当前替身的效果编号

    DWORD   dwSysTitleID;       // 系统称号图标ID m_dwSysTitle[1]
    DWORD   dwFactionTitleID;   // 帮派状态图标ID 

    char    sSpouseName[MAX_NAMELEN];    // 配偶的名称

	WORD	wCheckID;           // 用于刷新的版本号
	DWORD   dwConsumePoint;     //消费积分
    DWORD   fsicons;            // fight state icons setting
	WORD	wGemEff;			// 宝石特效 0xff | 0xff ( 高位类型 低位特效等级 )
	WORD    wTempItem[10];      // 玩家身上的临时数据

    char masterName[CONST_USERNAME];     // 师傅名字
    char prenticeName[CONST_USERNAME];   // 徒弟名字
    char szUnionName[13];    // 结义名称

	enum EQUIP_TYPE_SYNC
	{
		ETS_HAT = 0,
		ETS_CLOTH,
		ETS_SHOSE,
		ETS_WRIST,
		ETS_WAIST,
		ETS_MANTLE,
		ETS_WEAPON,

		ETS_MAX,
	};
	WORD wEquipIndex[ETS_MAX]; // 全身需要换装的所有装备的INDEX
	BYTE bySkillState;		 // 玩家武功境界
    WORD wMedicalEthics;     // 医德值
    WORD wMaxPerDayME;       // 当天累计量
	WORD wLevel;			 // 玩家等级
	char sIconName[CONST_USERNAME];		// 挂牌
	//add by xj
	int  iMaxHP;			//同步血量
	int  iMaxMP;
	int  iMaxSP;
	int  iCurHP;
	int  iCurMP;
	int  iCurSP;
	WORD wPracticeType;		//同步修炼状态
	bool bIsStar;			//是否开始修炼
	//add by dc
	int	 MountID;			// 同步角色坐骑, this is mount id

	SPath ssp;
};// */
//---------------------------------------------------------------------------------------------

// 同步侠客的数据
DECLARE_MSG(SSynPetMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_PET_INFO)
struct SASynFightPetMsg : public SSynPetMsg
{
	DWORD	dwGlobalID;	// ID;
	DWORD	dwOwnerID;	// 主人ID;
	WORD	ID;			// 宠物ID
	char	sName[PETNAMEMAXLEN];
	float	mMoveSpeed;	// 速度
	float	direction;	// 方向
	SPath	ssp;		// 同步路径
};

// 同步NPC的数据
DECLARE_MSG(SSynNpcMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_NPC_INFO)
struct SASynNpcMsg : public SSynNpcMsg 
{
	DWORD	wImageID;			// 该NPC的图片类型
	DWORD	wClickScriptID;		// 脚本ID
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
	DWORD	dwControlId;		// 控制ID
	WORD	wChose;				// 可否选中
	float	byDir;			    // 该NPC的方向
	float	mMoveSpeed;			// 速度
	SPath	ssp;				// 校正趋向数据（必须为最后一个）
};

// 同步怪物的数据
DECLARE_MSG(SSynMonsterMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_MONSTER_INFO)
struct SASynMonsterMsg : public SSynMonsterMsg 
{
	DWORD	dwGlobalID;			// 怪物ID
	char	sName[CONST_USERNAME];			// 非0结尾的名字字符串
	BYTE	bChangeType;		// 色怪类型
	BYTE bMonsterType;//怪物类型  0 1 2 普通，精英，BOSS
	int		ilevel;				// 等级
	WORD	mImageID;			// 模型ID
	WORD	monsterID; //在配置表中ID
	float	mMoveSpeed;			// 速度
	float	direction;			// 方向
	float	fScale;				// 模型大小
	DWORD dfightState;//战斗状态
	SPath	ssp;				// 同步路径

	/*
	WORD	wTypeID;			// 该怪物的图片类型（应该是图片编号吧）
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符  
	DWORD   dwExtraColor;		// 当前该怪物的颜色数据

	char	sName[CONST_USERNAME];			// 非0结尾的名字字符串
	WORD	wDeadTimes;			// 该怪物的图片类型（应该是图片编号吧？）
	
    DWORD   fsicons;            // fight state icons setting
    DWORD   dwHalo;             // 光环 
	//////////////////////////////////////////////////////////////////////////
	int		iHeadImageNum;		//怪物头像编号
	int     ilevel;				//怪物等级

	DWORD	dwControlId;
	//*/
};
//---------------------------------------------------------------------------------------------

  // 作场景转换的消息
DECLARE_MSG(SSetRegionMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SET_REGION)
struct SQSetRegionMsg : public SSetRegionMsg 
{
	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
	WORD	wConfigSerialID;	// 配置序列号，用于服务器验证
};

struct SASetRegionMsg : public SSetRegionMsg 
{
	WORD	wMapID;				// 目标场景的编号
	WORD	wCurRegionID;		// 目标场景的编号
	float	x;
	float	y;
	float	z;
	WORD	wPkType;		// PK类型  0 不能PK 1允许PK
};
//---------------------------------------------------------------------------------------------

// 场景上某个对象需要出现的效果
DECLARE_MSG(SSetEffectMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SET_EFFECT)
struct SASetEffectMsg : public SSetEffectMsg
{
    enum
    {
        EEFF_LEVELUP,           // 该玩家升级了
		EEFF_UPDATETELERGY,		// 心法升级了
		EEFF_UPDATESKILL,		// 技能升级了
    };

    BYTE effect;

    union
    {
        struct SLevelUp
        {
            DWORD gid;			// 升级的对象
        } LevelUp;

		struct UpdateTelergy
		{
			DWORD	dwGID;		// 升级的对象
			WORD	wID;		// 升级的心法ID
			BYTE	bLevel;		// 等级
		} Telergy;

		struct UpdateSkill
		{
			DWORD	dwGID;		// 升级的对象
			WORD	wID;		// 技能的ID
			BYTE	bLevel;		// 技能的等级
		} Skill;
    } data;
};

// 设置附加状态
DECLARE_MSG(SSetExtraStateMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SETEXTRASTATE)
struct SASetExtraStateMsg : public SSetExtraStateMsg 
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    DWORD   dwExtraState;       // 玩家的附加状态
};

//=============================================================================================
// 设置PK规则
DECLARE_MSG(SPKRule, SRegionBaseMsg, SRegionBaseMsg::EPRO_SETPKRULE)
struct SQPKRule : public SPKRule 
{
	BYTE	byPKRule;			// PK规则 0:和平模式  1:帮派模式   2:队伍模式   3:门派模式  4：全体模式
};
struct SAPKRule : public SPKRule 
{
	BYTE	byPKRule;			// PK规则 0:和平模式  1:帮派模式   2:队伍模式   3:门派模式  4：全体模式
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 跨区域通过ORB查询玩家信息
DECLARE_MSG(SQueryPlayerMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_QUERY_SIMPLAYER)
struct SQQueryPlayerMsg :
    public SQueryPlayerMsg
{
	char szName[CONST_USERNAME];			// Player Name to query
	BYTE mode;                  // 查询模式，0=好友 1=仇人普通查询 2=仇人打探
};

///考虑跨区域查询玩家信息的情况
struct SAQueryPlayerMsg :
    public SQueryPlayerMsg
{
    WORD    wServerID;          // 该玩家区域服务器ID
    DNID	  dnidClient;         // 区域服务器上该玩家的DNID
	char		  szName[CONST_USERNAME];				//名称
	BYTE		  bySchool;					//门派
	bool		  bSex;						//性别
	WORD	  wLevle;				//等级
	WORD	wRegion;				//所在场景ID
// 	char	szTitle[CONST_USERNAME];					
// 	char	szFactionName[CONST_USERNAME];
// 	WORD	wPKValue;
// 	int		nXValue;
 	
// 	WORD	wXPos;
// 	WORD	wYPos;
// 	BYTE    mode;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 设置附加状态 - 变身
DECLARE_MSG(SChangeMutateMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_ONMUTATE)
struct SQChangeMutateMsg : public SChangeMutateMsg 
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE    byMutateType;       // 变身类型（0、不变身；1、变怪物；2、变NPC）
    WORD	wMutateID;          // 变身后的图片ID
};
struct SAChangeMutateMsg : public SChangeMutateMsg 
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    BYTE    byMutateType;       // 变身类型（0、不变身；1、变怪物；2、变NPC）
    WORD	wMutateID;          // 变身后的图片ID
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 设置附加状态 - 替身
DECLARE_MSG(SSetScapegoatMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_ONSCAPEGOAT)
struct SASetScapegoatMsg : public SSetScapegoatMsg 
{
    DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    WORD	wScapegoatID;       // 替身的图片ID
    WORD	wGoatEffectID;      // 替身的效果ID
};


//同步玩家特效，以后所有特效都可以使用
DECLARE_MSG(SSynEffectsMsg, SRegionBaseMsg, SRegionBaseMsg::EPRO_SYN_EFFECTS)
struct  SASynEffectsMsg : public SSynEffectsMsg
{
	enum
	{
		EFFECT_ZHENQI,   //真气特效
	};
	WORD  weffectsindex;//特效ID
	DWORD dwgid;//特效被释放者  全局GID,
	DWORD dwgidatter;//特效释放者 
	float feffectPosX;//释放者 坐标
	float feffectPosY;
};
//---------------------------------------------------------------------------------------------
