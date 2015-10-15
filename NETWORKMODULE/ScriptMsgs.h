#pragma once

#include "NetModule.h"
#include "playertypedef.h"

#define STRING_LENTH 366
#define RANK_LIST_MAX 50
#define MAX_PRAYER_SHOPITEM 8
#define MAX_GIFTCODE_LENTH 16
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 脚本相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SScriptBaseMsg, SMessage, SMessage::EPRO_SCRIPT_MESSAGE)
//{{AFX
EPRO_CLICK_OBJECT,		    // 点击场景上的对象
EPRO_CLICK_MENU,            // 选择选项
EPRO_FORGE_GOODS,           // 炼制物品
EPRO_BRON_MSG,              // 出师
EPRO_BACK_MSG,          
EPRO_SHOW_TASK,             // 任务显示
EPRO_SEND_SCORE,            // 排行榜
EPRO_SEND_KILLINFO,         // 追杀令
EPRO_SEND_ITEMINFO,         // 物品
EPRO_HELP_PINFO,            // 新手任务信息
EPRO_ROLETASK_INFO,         // 任务相关
EPRO_MULTIME_INFO,			// 多倍经验时间
EPRO_SCORE_LIST,			// （新）江湖排行榜！
EPRO_SCORE_LIST_EX,			// （侠外）江湖排行榜！
EPRO_CUSTOM_WND,			// 特殊面版定制消息！
EPRO_SCRIPT_TRIGGER,		// 脚本逻辑触发器！
EPRO_LUACUSTOM_MSG,         // 脚本定制消息
EPRO_LUATIPS_MSG,			// 脚本提示消息
EPRO_DOCTORTIME_INFO,		// 医德剩余时间
EPRO_OTHEREQUIPMENTNAME,    // 通过名字查看装备
EPRO_SYNCTASKDATA,			// 同步任务数据到客户端
EPRO_SYNNPCTASKDATA,		// 同步NPC的任务信息到客户端（用于显示NPC头顶的提示标记）
EPRO_NOTIFY_STATUS,			// 提示用户信息
EPRO_PLAY_CG,				// 播放CG的消息
EPRO_CLICK_MENUNAME,        // 得到NPC买卖物品分页名字
EPRO_RANK_LIST,				// XYD3排行榜
EPRO_RANK_GETOTHEREQUIPMENT,// XYD3排行榜查看其他玩家装备
EPRO_CLIENT_REQUEST_DB,		// 客户端请求服务器从DB拉取数据
EPRO_QUEST_TREASURE,//请求抽取聚宝盆
EPRO_INIT_PRAYER,//祈福初始化消息
EPRO_START_PRAYER,//祈福消息
EPRO_SHOP_REFRESH,//refreshshop 商店刷新
EPRO_SHOP_BUSINESS,//refreshshop 商店交易
EPRO_GIFTCODE_ACTIVITY,//礼包激活码
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------
//EPRO_SEND_CLIENT        // 要求客户端执行的信息


DECLARE_MSG(SPlayCG, SScriptBaseMsg, SScriptBaseMsg::EPRO_PLAY_CG)
struct SAPlayCG : public SPlayCG
{
	WORD wCgID;		 // CG的ID
};

struct SQPlayCG : public SPlayCG
{
	WORD wCgID;		 // 已经播放完成的CG ID
};



DECLARE_MSG(SNotifyStatus, SScriptBaseMsg, SScriptBaseMsg::EPRO_NOTIFY_STATUS)
struct SANotifyStatus : public SNotifyStatus
{
	enum
	{
		// 交易相关
		NS_EXG_SELFINEXGING = 1,		// 自己处于交易中，无法与别人交易
		NS_EXT_DESTEXGING,				// 对方处于交易中，无法与你交易
		NS_EXG_CANTEXGSELF,				// 不能和自己交易
		NS_EXT_CANTFIND_SRC,			// 交易源不存在
		NS_EXG_CANTFIND_DEST,			// 交易目标不存在
		NS_EXT_WAITDEST_RESPOND,		// 交易请求已发出，等待回应
		NS_EXT_REQUEST_REJECT,			// 交易被拒绝
		NS_EXT_ALREADY_IN_LOCK,			// 已处于锁定状态中
		NS_EXT_ALREADY_IN_COMMIT,		// 已处于交易状态中
		NS_EXT_MONEY_INVALID,			// 输入的金钱数目不合法
		NS_EXT_MONEY_LACK,				// 金钱输入不足
		NS_EXT_ITEM_CANT_FIND,			// 想拖入的道具不存在
		NS_EXT_ITEM_LOCKORBIND,			// 想拖入的道具被锁定/绑定
		NS_EXT_ITEM_CANTEXG,			// 想拖入的道具无法被交易		
		NS_EXT_EXTBOX_FULL,				// 交易栏已满
		NS_EXT_NOTHING,					// 没有交易任何东西
		NS_EXT_DEST_PACKAGE_LACK,		// 对方背包不足
		NS_EXT_SELF_PACKAGE_LACK,		// 自己背包不足
		NS_EXT_ITEM_BIND,				// 被绑定无法被交易

		// 拆分道具相关
		NS_SPLITITEM_BIND = 201,		// 道具被绑定，无法拆分
		NS_SPLITITEM_LOCKED,			// 道具被锁定，无法拆分
		NS_SPLITITEM_TASKNOTALLOWED,	// 任务道具无法被拆分
		NS_SPLITITEM_PACKAGEFULL,		// 背包已满，无法拆分
		NS_SPLITITEM_COOLING,			// 还在拆分冷却

		// 仓库相关
		NS_WH_ALREADY_OPEN = 401,		// 当前已经打开了一个仓库
		NS_WH_MONEY_FULL,				// 仓库金钱已满
		NS_WH_ITEMCANT_STORAGABLE,		// 道具不允许拖入仓库
		NS_WH_ITEM_LOCKED,				// 道具被锁定，无法拖入仓库
		NS_WH_PLAYER_MONEY_FULL,		// 玩家身上的金钱已满

		// 整理道具相关
		NS_ZL_COOLING,					// 还在整理冷却
	};

	WORD wIndex;					// 要显示消息的索引
};

// added by xhy
DECLARE_MSG(SSyncTaskData, SScriptBaseMsg, SScriptBaseMsg::EPRO_SYNCTASKDATA)
struct SQASyncTaskData : public SSyncTaskData 
{
    char streamData[8*1024];
};

DECLARE_MSG(SSyncNpcTaskData, SScriptBaseMsg, SScriptBaseMsg::EPRO_SYNNPCTASKDATA)
struct SASyncNpcTaskData : public SSyncNpcTaskData
{
	WORD	NpcCount;			// Npc个数
	DWORD	TaskData[128];		// NpcID + TaskCount + TaskStatus
};

//=============================================================================================
// 脚本自定义消息
// 注意，由于该消息可能会直接由客户端发起，所以服务器代码是无法直接判断其有效性的，需要在脚本里做出正确的处理才行！！！
DECLARE_MSG(SLuaCustomMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_LUACUSTOM_MSG)
struct SQALuaCustomMsg : public SLuaCustomMsg 
{
    BYTE flags;
    char streamData[2048];
};
DECLARE_MSG(SLuaTipsMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_LUATIPS_MSG)
struct SQALuaTipsMsg : public SLuaTipsMsg 
{
    BYTE flags;	//错误类型
	char streamData[2048];	//消息
};

// 客户端点击了NPC请求服务器触发点击响应
DECLARE_MSG(SClickObjectMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_OBJECT)
struct SQClickObjectMsg : public SClickObjectMsg
{
	DWORD	dwGlobalID;			// 点击者的ID
	DWORD	dwDestGlobalID;		// 被点击者的ID
	BYTE	dymmy[12];			// 点击的参数
};

struct SAClickNpcMsg : public SClickObjectMsg
{
	DWORD		verifier;			// 验证信息
	DWORD		wScriptID;			// 脚本执行代码
	WORD		wTaskNum;			// 任务信息个数
	WORD		wMenuIndex[20];		// 要显示的菜单索引
	WORD		wTaskData[64];		// 任务的信息（按需发送）
};

struct SAClickObjectMsg : public SClickObjectMsg
{
	DWORD	verifier;			// 验证信息
	DWORD	wScriptID;			// 脚本执行代码
	WORD	wMenuCount;			// 菜单个数
	WORD	wTaskGroupCount;	// 任务群个数
	DWORD	wParam[5];			// 服务器返回给客户端的参数
};

// 客户端做出对话框选择后请求服务器端处理
DECLARE_MSG(SChoseMenuMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_MENU)
struct SQChoseMenuMsg : public SChoseMenuMsg
{
    DWORD	verifier;			//	调用效验码
	DWORD	byChoseID;			//  玩家的选择请求
};

struct SAChoseMenuMsg : public SChoseMenuMsg
{
    enum 
	{
        CM_BUYPANEL = 1,            // 弹出买卖面板
        CM_FORGEPANEL,              // 弹出炼制面板 
        CM_FUSEPANEL,               // 弹出融合面板
        CM_STORAGEPANEL,            // 弹出仓库面板
        CM_FAMEPANEL,               // 弹出买名望面板

        CM_SHOWPICTURE,             // 弹出图片
        CM_KILLINFOPANEL,           // 弹出追杀令面板

		CM_COLOURPANEL,				// 弹出染坊面板
		CM_INPUTPANEL,				// 弹出输入面板
        CM_CHECKITEMPANEL,			// 弹出鉴定面板
		CM_BACKITEMPANEL,			// 弹出回收面板
		CM_INPUTMCPANEL,			// 弹出请求侠义币换点卡面板
		CM_MCINFOPANEL,				// 弹出侠义币换点卡信息面板
		CM_OWNMCINFOPANEL,			// 弹出自己的侠义币换点卡信息面板

		CM_UPDATEITEM,				// 弹出升级面板
		CM_XIDIAN,					// 弹出洗点界面
		CM_DAKONG,					// 弹出打孔界面
		CM_XIANGQIAN,				// 弹出镶嵌面板
		CM_ZHAICHU,					// 弹出摘除面板
    };

	WORD	m_Index;				// 选择后要求客户端做的处理
	BYTE	m_count;				// 物品个数			
	DWORD	m_Goods[128];			// NPC填充的物品（64个，物品ID+数量）
};
//---------------------------------------------------------------------------------------------


enum SItemType { ST_CHECKONE, ST_BACKITEMS };
//=============================================================================================
// 客户端请求炼制物品消息  -- 融合也用这一个消息
DECLARE_MSG(SForgeGoodsMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_FORGE_GOODS)
struct SQForgeGoodsMsg : public SForgeGoodsMsg
{
    enum {
        SP_FORGE = 1,
        SP_FUSE,
        SP_FAME,
        SP_MONEY,
        SP_SITEMS
    };

	BYTE byWhat;      // 炼制还是融合? 1:炼制  2:融合  3: 名望   4:钱输入框 5: 脚本道具
    DWORD wSuccess;
};

struct SAForgeGoodsMsg : public SForgeGoodsMsg
{
	BYTE  byWhat;
    WORD  wSuccess;        // 成功率
   	WORD  wEquipment;      // 装备有否
	WORD  wDrinkNum;       // 酒的数量
	DWORD wMoney;           
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 一些脚本执行的信息返回
DECLARE_MSG(SBackMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_BACK_MSG)
struct SABackMsg : public SBackMsg
{
	enum EBType
	{
        B_FAIL = 0,
		B_FORGE = 1,             
		B_NOFORGE,
		B_FUSE,
		B_NOFUSE,
		B_NOBIJOU,
		B_FULLBAG,
        B_FARFIGHT,
		B_NOLEARNGEST,    
        B_POSABLE,              // 所占的位置不能攻击
        B_SKILLABLE,            // 没有学会当前招式
        B_MONSTERABLE,          // 不能对怪物辅助攻击
        B_NOTTARGET,            // 没有目标或目标已经死了
        B_ATTACKTEAM,           // 不能攻击队友
        B_LEVELABLE,            // 等级不够不能PK
        B_LIMITATTACK,          // 你被封招了！
        B_LIMITHP,              // 血量不够
        B_LIMITMP,              // 内里不足
        B_LIMITSP,              // 体力不够
		B_HADLEARNED,           // 已经学习了这门武功
        B_LIMITUSE,             // 当前限制使用的武功
        B_BEVITALS,             // 被点穴
        B_BEDIZZY,              // 被眩晕

        B_NOTSCHOOL,            // 门派不同
        B_LV_NOTENOUGH,         // 等级不足
        B_EN_NOTENOUGH,         // 活力不足
        B_IN_NOTENOUGH,         // 悟性不足
        B_ST_NOTENOUGH,         // 体魄不足
        B_AG_NOTENOUGH,         // 身法不足

        B_LIMIT_TALK,           // 禁止刷屏

        SYS_LEVEL_NOT_ENOUGH,   // 进入场景的等级不足
        SYS_CAN_NOT_DO_PK,      // 场景限制不能进行PK

        EXM_FAIL,                       // 在交易中出现失败
        EXM_CANT_FIND_OTHERPLAYER,      // 找不到交易对象
        EXM_OTHERPLAYER_BUSY,           // 对方忙
        EXM_WAIT_OTHERPLAYER_RESPOND,   // 请等待对方的响应
        EXM_MONEY_NOT_ENOUGH,           // 钱不足够
        EXM_CANT_FIND_ITEM,             // 无法找到对应的道具
        EXM_CANT_PUT_ITEM,              // 无法放下该物品
        EXM_REJECT,                     // 交易被拒绝
        EXM_CANT_ADDSELF_ITEM,          // 无法为自己添加道具
        EXM_CANT_ADDDEST_ITEM,          // 无法为对方添加道具
        EXM_CANT_DELSELF_ITEM,          // 无法删除自己的道具
        EXM_CANT_DELDEST_ITEM,          // 无法删除对方的道具
		EXM_ITEMEX_DISABLED,			// 道具无法被交易

        ERR_GMLIMIT,                    // 没有GM权限，或不够

        B_ITEM_PROTECTED,               // 物品保护时间
        B_TALKMASK,                     // 被禁言
        // ...
        B_NOTFORGELEVEL,                // 炼制必须在3级以上
        B_NOTFORGEITEM,                 // 不是炼制物品

        B_ATTERR_NOTTELERGY,            // 不能使用当前心法
        B_FORCE_LOGOUT,                 // 你被管理员强制断开连接！
        B_ACCOUNT_KICK,                 // 有人使用你的账号上线了！

        // 心法相关
        B_FULLTELERGY,                  // 你已经装备了六种心法

        // 经脉相关
        B_BECUREING,                    // 你正在接受治疗
        B_CUREING,                      // 你正在为别人治疗
        B_CUREROVER,                    // 为你治病的人已经消失，治疗中止
        B_CURERGOOUT,                   // 在治病期间，任何人都不能动，你看嘛！治疗中止了！
        B_CURE_NOZAZEN,                 // 要先两个人都打坐才能疗伤
        B_CURE_FARAWAY,                 // 距离太远不能能疗伤
        B_CURE_END,                     // 疗伤完毕
        B_CURER_END,                    // 疗伤者疗伤完毕

        // 穴道相关
        B_VENA_PASSED,                  // "你的这个穴道已经打通了的"
        B_VENA_SETPPASS,                // "这条经脉上的穴道可是要一个一个来的"
        B_VENA_PASSNOT6,                // "等你打通前面的六个经脉再来吧！"
        B_VENA_COUNTNOT,                // "你的充穴点数不够"
        B_VENA_ITEMNOT,                 // "你没有穴道相应的指南书"
        B_VENA_LOWTELERGY,              // 你的心法不够用于打通这个穴道
        B_VENA_PASSVENASO,              // 由于受到冲穴影响
		B_VENA_NOTITEM,					// 没有对应铜人

        B_LONEMANSELF,                  // 独行侠状态(myself)
        B_LONEMANOTHER,                  // 独行侠状态(other)

        B_MUTATE_CANOTRUN,              // 变身状态就不能跑和跳了
        B_MUTATE_CANOTZAZEN,            // 想要变身状态就别想打坐
        B_MUTATE_CANOTONZAZEN,          // 先站起来才变身
        B_MUTATE_CANOTSKILL,            // 你还没有学会变身之后使用武功

        //B_UPASSWORD_ERROR,              // 二级密码出错

        B_TESTMSG,                      // 返回测试信息

        B_ASK_DOCTORBUFFADD,            // 询问是否同意附加医德BUFF
        B_AGREE_ADDDOCTORBUFF,          // 同意附加医德BUFF
        B_BECURED,                      // 对方正在被治疗
        B_CUREOTHER,                    // 想给多个人治疗
        B_REDUCEWERA,                   // 用来通知客户端减少耐久值
        B_CANCELLOCK,                   // 通知客户端取消玩家锁定
        B_STOPLOCK,                     // 这个消息表示服务器不会同步状态了，客户端可以变灰

		B_NOT_ENOUGH_SPACE,				// 背包空间不足
        B_SUCCESS = 255,
	};

	BYTE byType;
};

struct SAAddDoctorBUFFMsg : public SABackMsg
{
    DWORD dwGID;
};

struct SAReduceEquipWearMsg : public SABackMsg
{
    enum 
    {
        ATTACK_EUIP,    // 攻击装备（ 武器和饰品 ）
        DEF_EQUIP       // 防具
    };

    BYTE byEquipType;
};
//=============================================================================================
// 出师？…… out school & out teacher -_______-bb
DECLARE_MSG(SOutSchoolMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_BRON_MSG)
struct SAOutSchoolMsg : public SOutSchoolMsg
{
	DWORD  dwGlobalID;
	BYTE   byBRON;
	BYTE   bySchool;
	BYTE   bySex;
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SShowTaskInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOW_TASK)
struct SAShowTaskInfoMsg : public SShowTaskInfoMsg
{
    enum 
    {
        EO_ADD_TASK,  // 以前旧的任务系统
		//EO_ADD_TASK2,  // 现在特殊处理的任务
        EO_DEL_TASK,
        EO_CLEAN_TASK
    };

    BYTE byOperate;
	BYTE byTaskType;
    WORD wTaskID;
    DWORD wTaskState;
};
/*
struct SAShowTaskInfoMsgEx : public SShowTaskInfoMsg
{
	WORD wTaskID;               // 任务号   这个号可跟NPC所触发的脚本号一致  也可是专门指定的任务号 
	DWORD wTaskState;           // 完成情况\状态值
	BYTE byOperate;
	BYTE byTaskType;            // 任务类型
};
*/
// --------------------------------------------------------------------------------------------
// 排行榜
DECLARE_MSG(SSendScoreInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_SCORE)
struct SASendScoreInfoMsg : public SSendScoreInfoMsg
{
    SScoreTable::ScoreType type;
    SScoreTable::SScoreRecode Score[MAX_SCORE_NUMBER];
};

// --------------------------------------------------------------------------------------------
// EPRO_SEND_KILLINFO
DECLARE_MSG(SSendKillInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_KILLINFO)
struct SQSendKillInfoMsg : public SSendKillInfoMsg
{
    DWORD dwMoney;
	char szName[CONST_USERNAME];
};

struct SASendKillInfoMsg : public SSendKillInfoMsg
{
//    KILLINFO stKillInfo[MAX_KILLINFO];
};

//--------------
DECLARE_MSG(SSendSItemInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SEND_ITEMINFO)
struct SQSendSItemInfoMsg : public SSendSItemInfoMsg
{
//     enum STYPE {
//         ST_CHECKITEM,
//         ST_BACKITEMS
//     };
// 
//     BYTE type;
//     SITEMINFO info;
};

struct SQSendSItemDelMsg : public SQSendSItemInfoMsg
{   
};

///------------------------------
DECLARE_MSG(SHelpPInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_HELP_PINFO)
struct SQHelpPInfoMsg : public SHelpPInfoMsg
{
    enum PTYPE {
        PT_MASTER,
        PT_PRENTIS 
    };

    WORD wPPType;
	char szName[CONST_USERNAME];
    DWORD dwStaticID;
};

// 新任务相关
DECLARE_MSG_MAP(SRoleTaskBaseMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_ROLETASK_INFO)
EPRO_SENDROLETASKINFO,				// 任务情况
EPRO_ADDROLETASK,					// 接受任务
EPRO_HANDINTASK,					// 交任务
EPRO_FINISHTALK,					// 完成对话
EPRO_FINISHGIVE,					// 完成给予
EPRO_FINISHGET, 					// 完成获取
EPRO_UPDATETASKFLAG,				// 更新旗标状态
EPRO_SENDROLETASKELEMENT,			// 任务旗标情况
EPRO_SET_SPEF,						// 设置任务
EPRO_CLEAR_ALL,						// 清空任务
EPRO_GIVEUPTASK,					// 放弃任务
EPRO_FINISHTADDTASK,				//完成新增的某个任务
END_MSG_MAP()

DECLARE_MSG(SSendRoleTaskInfoMsg, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SENDROLETASKINFO)
struct SASendRoleTaskInfoMsg : public SSendRoleTaskInfoMsg
{
	WORD	wTaskNum;			// 任务信息个数
	WORD	wTaskData[64];		// 任务的信息（按需发送）
};

DECLARE_MSG(SAddRoleTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_ADDROLETASK)
struct SQAddRoleTask : public SAddRoleTask
{
	DWORD	dwGlobalID;				// 玩家ID
	WORD	wTaskID;				// 任务ID
};

// 添加了一个任务
struct SAAddRoleTask : public SAddRoleTask
{
	DWORD		dwGlobalID;			// 玩家ID
	WORD		wTaskID;			// 任务ID（增加的任务ID）
	WORD		wFlagCount;			// 旗标数目
	QWORD		m_CreateTime;		// 任务的创建时间
	STaskFlag	w_flags[TASK_MAX];			// 任务旗标（按需发送）
};

DECLARE_MSG(SHandInRoleTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_HANDINTASK)
struct SQHandInRoleTask : public SHandInRoleTask
{
	DWORD	dwGlobalID;				// 玩家ID
	WORD	wTaskID;				// 任务ID
	WORD	wAwardCount;			// 奖励物品的编号，如果没有可选奖励，则为0
};

// 交任务回馈
struct SAHandInRoleTask : public SHandInRoleTask
{
	enum
	{
		HTR_SUCCESS = 1,
		HTR_FULLBAG,				// 背包满了，无法容纳奖励
	};

	DWORD	dwGlobalID;				// 玩家ID
	WORD	wTaskID;				// 任务ID
	WORD	wResult;
};

// 放弃任务
DECLARE_MSG(SGiveUpTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_GIVEUPTASK)
struct SQGiveUpTask : public SGiveUpTask
{
	DWORD dwGlobalID;
	WORD  wTaskID;
};

// 放弃任务回馈
struct SAGiveUpTask : public SGiveUpTask
{
	enum
	{
		QUT_SUCCESS = 1,
		QUT_DONT_EXIST,
		QUT_ALREADY_HANDIN,
	};

	DWORD	dwGlobalID;				// 玩家ID
	WORD	wTaskID;				// 任务ID
	WORD	wResult;
};

DECLARE_MSG(SFinishTalk, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHTALK)
struct SQFinishTalk : public SFinishTalk
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wTalkDes;
};

DECLARE_MSG(SFinishGive, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHGIVE)
struct SQFinishGive : public SFinishGive
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wGiveDes;
};

DECLARE_MSG(SFinishGet, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHGET)
struct SQFinishGet : public SFinishGet
{
	DWORD	dwGolbalID;
	WORD	wTaskID;
	WORD	wGetDes;
};

// 更新了旗标信息
DECLARE_MSG(SUpdateTaskFlag, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_UPDATETASKFLAG)
struct SAUpdateTaskFlag : public SUpdateTaskFlag
{
	STaskFlag flag;
};

DECLARE_MSG(SSendRoleTaskElementMsg, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SENDROLETASKELEMENT)
struct SASendRoleTaskElementMsg : public SSendRoleTaskElementMsg
{
	DWORD		m_Index;			// 旗标的索引（高16位：主任务，低16位：旗标索引）
	BYTE		m_Type;				// 旗标的类型
	BYTE		m_Complete;			// 是否完成
	DWORD		m_Value;			// 描述当前状态的一个值，不同的任务有不同的含义
};

DECLARE_MSG(SSetSpefTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_SET_SPEF)
struct SQSetSpefTask : public SSetSpefTask
{
	DWORD	dwPlrID;				// 玩家ID
	WORD	wTaskID;				// 任务ID
};

DECLARE_MSG(SClearAllTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_CLEAR_ALL)
struct SAClearAllTask : public SClearAllTask
{
	DWORD	dwPlrID;				// 玩家ID
};

//完成新增某类型任务
DECLARE_MSG(SFinishAddTask, SRoleTaskBaseMsg, SRoleTaskBaseMsg::EPRO_FINISHTADDTASK)
struct SQFinishAddTask : public SFinishAddTask
{
	BYTE btype;
	WORD wCount;
	WORD	wTaskID;
	DWORD dIndex;
};

//=============================================================================================
// 多倍经验的时间
DECLARE_MSG(SMulTimeInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_MULTIME_INFO)
struct SAMulTimeInfoMsg : public SMulTimeInfoMsg
{
	double	dTime;
	WORD	wMulTime;    // 高位存储多倍类型
};
// --------------------------------------------------------------------------------------------

// 医德BUFF剩余时间
DECLARE_MSG(SDoctorTimeInfoMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_DOCTORTIME_INFO )
struct SADoctorTimeInfoMsg : public SDoctorTimeInfoMsg
{
    QWORD	dLeaveTime; // 里面存放的四种BUFF的分钟数，从高位到低位：AGI,POW,DAM,DEF
};
// --------------------------------------------------------------------------------------------
// ############################################################################################
// --------------------------------------------------------------------------------------------
// （新）江湖排行榜
DECLARE_MSG( SUpdateScoreListMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCORE_LIST )
struct SQUpdateScoreListMsg : public SUpdateScoreListMsg
{
    enum CMD_TYPE 
    { 
        QUERY_TITLE, 
        QUERY_SCORE, 
        SCORE_BEGIN, 
        SCORE_PREV, 
        SCORE_NEXT, 
        SCORE_END,
        UPDATE_APPRAISE,
        SCORE_AWARD,

    }   commandType;

	int param;
	int page;
};

struct SAUpdateScoreListMsg : public SUpdateScoreListMsg
{
    // 因该只有2类数据：获取分类列表，更新排行数据
    // serialized RPC data
    char streamData[1024];
};

// --------------------------------------------------------------------------------------------
// ############################################################################################
// --------------------------------------------------------------------------------------------
// （侠外）江湖随身排行榜 
DECLARE_MSG( SUpdateScoreListExMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCORE_LIST_EX )
struct SQUpdateScoreListExMsg : public SUpdateScoreListExMsg
{
    WORD wScoreType;
};

struct SAUpdateScoreListExMsg : public SUpdateScoreListExMsg
{
    WORD wScoreType;
    char streamData[1024];
};
// --------------------------------------------------------------------------------------------


// ############################################################################################
// 特殊面版定制消息！
// --------------------------------------------------------------------------------------------
DECLARE_MSG( SCustomWndMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_CUSTOM_WND )
struct SQCustomWndMsg : public SCustomWndMsg
{
    // 用于效验客户端返回值的操作
    // 同时，如果verifier和上一次的相同，则表明本次操作属于窗口更新，而不是重新创建窗口
    DWORD verifier;

	enum OperateType
	{
		CustomWndShow = 0,
		CustomWndClose = 1,
		CustomWndUpdate = 2,
		CustomWndJUSuccess = 3,
		CustomWndJUFailed = 4
	};

	BYTE operate;

    // MAX10K长度的面版定制消息！
    char streamData[10240];
};

struct SACustomWndMsg : public SCustomWndMsg
{
    DWORD verifier;
    DWORD endFlag;          // 客户端返回消息时，通知服务器本次操作是属于完全关闭还是只是操作请求（非完全关闭）

    // 客户端返回数据应该不多，1K因该已经足够
    char streamData[1024];
};
// --------------------------------------------------------------------------------------------

DECLARE_MSG( SScriptTriggerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_SCRIPT_TRIGGER )
struct SQScriptTriggerMsg : public SScriptTriggerMsg
{
    // 脚本触发器，所有逻辑都直接进入脚本内部进行处理！
    char streamData[1024];
};

// --------------------------------------------------------------------------------------------
DECLARE_MSG( SOthereQuipmentNameMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_OTHEREQUIPMENTNAME )
struct SQOthereQuipmentNameMsg : public SOthereQuipmentNameMsg
{
	char szDestName[CONST_USERNAME];
};//EPRO_CLICK_MENUNAME


DECLARE_MSG( SGetMenuName, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLICK_MENUNAME)
struct SAGetMenuName : public SGetMenuName
{
	enum MENUINDEX
	{
		MI_INDEX = 6,
	};

	DWORD		verifier;					// 验证信息
	DWORD		wScriptID;					// 脚本执行代码
	DWORD		wIndex;						// 个数
	DWORD		wMenuIndex[MI_INDEX];		// 要显示的菜单索引
};

// XYD3查看其他玩家装备消息
DECLARE_MSG(SRankListEquipMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_RANK_GETOTHEREQUIPMENT)
struct SQRankListEquipMsg : public SRankListEquipMsg
{
	char szDestName[CONST_USERNAME];
};

/************************************************************************/
// 新排行榜消息
// 一个消息是客户端在点开[排行榜界面]的时候发送的，这个消息通知服务器，从DB里面
// 拉取排行榜数据
// 另一个消息主要是根据不同的请求发送排行榜数据，不再像以前那样全部发送，现在是分开
// 发送
/************************************************************************/

// (1)客户端打开界面发送的消息，通知服务器需要向DB拉取排行榜数据
DECLARE_MSG(SClientQuestRankFromDB, SScriptBaseMsg, SScriptBaseMsg::EPRO_CLIENT_REQUEST_DB)
struct SQClientQuestRankFromDB : public SClientQuestRankFromDB
{
};

struct SAClientQuestRankFromDB : public SClientQuestRankFromDB
{
	enum IF_UPDATE
	{
		IU_UPDATE,
		IU_UNUPDATE,
	};

	BYTE		type;									// 是否更新，更新后才发数据给客户端
	BYTE		RankNum[RT_MAX];						// 排行榜当前排名
	Rank4Client RankList[RT_MAX][MAX_RANKLIST_NUM];		// 当前所有排行榜数据
};

// (2)客户端请求具体的排行榜信息，这个时候是经过服务器处理以后的。按需发送，不全部发送
DECLARE_MSG(SRankListMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_RANK_LIST)
struct SQRankListMsg : public SRankListMsg
{
	BYTE	bType;		// 排行榜类型
};

struct SARankListMsg : public SRankListMsg
{
	WORD		bMyRank;		// 我的排名
	Rank4Client	ranklist[MAX_RANKLIST_NUM];
};

DECLARE_MSG(STreasureMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_QUEST_TREASURE)
struct SQTreasureMsg : public STreasureMsg
{
	BYTE  bTreasureType;		//  0   请求当前索引    1  请求购买当前索引的聚宝盆
};

struct SATreasureMsg : public STreasureMsg
{
	BYTE bTreasureIndex;	//当前显示的聚宝盆索引
	BYTE  bResule;	//是否成功  1 成功  2 购买元宝不足 3 已经达到最大购买次数 4 聚宝盆活动已经关闭
	WORD wTreasureNum;//购买成功后得到的元宝
};

typedef struct PRAYERPOINT    //玩家祈福积分
{
	char playername[CONST_USERNAME];  //玩家名字
	WORD  wPoint;		//玩家分数
}PrayerPoint;

typedef struct PRAYERSHOPITEM    //玩家祈福商城道具
{
	DWORD  ditemIndex;	//道具ID
	DWORD  ditemprice;	//价格
	BYTE			bnum;	//数量
}PrayerShopItem;

//祈福初始化消息
DECLARE_MSG(SInitPrayerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_INIT_PRAYER)
struct SQInitPrayerMsg : public SInitPrayerMsg
{
};
struct SAInitPrayerMsg : public SInitPrayerMsg
{
	PrayerPoint  pPlayerPoint[RANK_LIST_MAX];//积分排行榜			
	WORD  wCurrentPoint;					//玩家当前剩余分数
	WORD  wHistoryPoint;					//玩家历史分数
	WORD   wRemainDay;					//活动剩余开启时间
	DWORD  dfreetime;						//当前距离免费抽取时间秒
	DWORD  dclosetime;						//活动结束后，商城的关闭时间
	BYTE		 bShopOpened;					//商城是否开启
};

DECLARE_MSG(SPrayerMsg, SScriptBaseMsg, SScriptBaseMsg::EPRO_START_PRAYER)
struct SQPrayerMsg : public SPrayerMsg
{
	BYTE  bcount;			//次数，10连抽 则发10
};
const WORD bufflen = (sizeof(BYTE)  + sizeof(DWORD)* 2 ) * 10;
struct SAPrayerMsg : public SPrayerMsg
{
	BYTE  bresult;   //结果 。成功或者失败的定义	
	WORD wItemNum;	//道具数量
	BYTE   bBuffer[bufflen];
};

DECLARE_MSG(SShopRefresh, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOP_REFRESH)
struct SQShopRefresh : public SShopRefresh  //刷新商店道具
{
	BYTE  brefreshtype; //  0 打开商店,获取道具列表   1  使用 积分刷新 
};
struct SAShopRefresh : public SShopRefresh  //刷新商店道具
{
	BYTE  bmoneyType; //货币类型
	WORD  bRefreshPoint; //刷新花费
	DWORD  dRefreshTime; //免费刷新时间秒
	PrayerShopItem  m_PrayerItem[MAX_PRAYER_SHOPITEM];
};

DECLARE_MSG(SShopBusiness, SScriptBaseMsg, SScriptBaseMsg::EPRO_SHOP_BUSINESS)
struct SQShopBusiness : public SShopBusiness  //刷新商店道具
{
	BYTE  btype;	//0购买或者1出售 现阶段只有购买   
	BYTE  bIndex;	//在道具商店中的索引，不是道具ID
};
struct SAShopBusiness : public SShopBusiness  //刷新商店道具
{
	BYTE  bresult;	//  1成功  0 道具数量不足 2 积分不足 3 扣除积分失败 4 没有刷新商店中的道具
};


DECLARE_MSG(SGiftcodeActivity, SScriptBaseMsg, SScriptBaseMsg::EPRO_GIFTCODE_ACTIVITY)
struct SQGiftcodeActivity : public SGiftcodeActivity  //激活码领取道具
{
	char  strgiftcode[MAX_GIFTCODE_LENTH];
};
struct SAGiftcodeActivity : public SGiftcodeActivity  //激活码领取道具
{
	BYTE  bresult;	//  1成功  其他激活码错误
	WORD wItemNum;	//道具数量
	BYTE   bBuffer[bufflen];
};