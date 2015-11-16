#pragma once

#include "networkmodule/netmodule.h"
#include "..\区域服务器\GameObjects\SanguoCode\Common\ConfigManager.h"
#include "SanguoPlayerMsg.h"


#define MAX_FACTION_NUMBER 50 //globalConfig.FactionPlayerMaxNum 
//#define MAX_MEMBER_NUMBER 200	// 不管帮派级别多大，最多这么多人		2015-5-28 彭海洋改
#define MAX_MEMBER_NUMBER 50//globalConfig.FactionPlayerMaxNum 	// 不管帮派级别多大，最多这么多人
#define MAX_MEMO_NUMBER 50//globalConfig.FactionMemoMaxNum //军团宣言长度上限
#define MAX_NOTICE_NUMBER 100
#define MAX_JoinListCount 20
#define MAX_SENDFACTIONS 10   //发送的最大数量
#define FACTIONDATA_LENTH		25 * 1024 *2    //军团数据大小
#define MAX_SENDFACTIONDATA 1   //发送的最大数量
#define CONSTMAILTITLE  256
#define CONSTMAILCONTENT  512
#define MAX_EQUIPTLISTNUM  20	//查看装备请求队列的最大数量
#define MAX_QUESTEQUIPTBUFFER  1024   //请求查看装备的Buff最大大小
//CGlobalConfig globalConfig; globalConfig = CGlobalConfig globalConfig;
struct SimFactionData
{
	

	struct RequestJionPlayer
	{

		DWORD headIconID;
		char name[CONST_USERNAME];
		int level;
		RequestJionPlayer()
		{
			headIconID = 0;
			memset(name, 0, sizeof(name));
			int level = 0;
		}

	};

	struct SimFactionInfo
	{
		char szFactionName[CONST_FACTNAME];			// 帮派名字
		char szCreatorName[CONST_USERNAME];			// 创始人名字
		char szFactionMemo[100];			// 帮派宣言暂定150个字符
		//char factionIcon[MAX_PlayerIcon];			// 帮派图标
		int factionIcon;
		BYTE  byMemberNum;				// 成员数量
		BYTE  byFactionLevel;			// 当前帮派等级
		WORD  factionId;                // 帮派Id（该ID为帮派的唯一性标志！而帮派场景也可以用他来作为场景ID！）
		DWORD dwFactionProsperity;	    // 帮派繁荣度，以前是帮派活跃度
		DWORD iMoney;					// 帮派资金(以千为单位)


		DWORD dwWood;					// 木材
		DWORD dwStone;					// 石头
		DWORD dwMine;					// 矿石
		DWORD dwPaper;					// 纸浆
		DWORD dwJade;					// 玉石		


        struct {
            QWORD parentId  : 24;       // 帮派大门的父场景ID
            QWORD uniqueId  : 40;       // 帮派大门的唯一ID
        } buildingStub;                 // 通过该数据来找到帮派大门！
		BYTE byNpcNum;					// NPC个数

        BYTE byOpenRApplyJoin;          // 开启远程入帮 1开启 0关闭

		//玩家加入请求表
		//std::map < std::string, RequestJionPlayer > RJlist;
		RequestJionPlayer RjoinList[MAX_JoinListCount];
		int Rhlistcount ;
		SimFactionInfo()
		{
			Rhlistcount = 0;
			memset(RjoinList, 0, sizeof(RjoinList));
		}
		//BYTE  mapTemplateId;            // 帮派场景的地图模版编号
		//WORD  factionRegionId;          // 帮派场景Id（该Id是世界唯一的，且只有16位，主要做为在登陆服务器上注册用）
		//WORD  parentRegionId;           // 帮派场景的父场景Id
		//WORD  xEntry, yEntry;           // 帮派场景在父场景上的入口点坐标
	};

	struct SimMemberRight
	{
		//----------------------------------------
		DWORD Level:9;					// 成员在帮派中的等级
		DWORD Title:4;					// 职务 帮主、副帮主、左护法、右护法、坛主、香主、大师兄[帮医]、大师姐[帮医]、帮众
		DWORD School:3;					// 玩家所属门派及性别 （少林男、武当男、峨嵋女、唐门男、魔教男、武当女、唐门女、魔教女）　
		DWORD Camp:2;					// 玩家的阵营 正、邪、中立。
		DWORD IsOnline:1;				// 是否在线

		DWORD Appoint:1;				// 任命权
		DWORD Accept:1;					// 接纳权
		DWORD Authorize:1;				// 授权
		DWORD Dismiss:1;				// 解骋权
		DWORD Editioner:1;				// 留主版权力
		DWORD CastOut:1;				// 驱逐权

		//DWORD EspecialTitle:1;		// 给帮内成员定义特别称号的权力 ( *只有帮主能使用 )
		//DWORD Resignation:1;			// 辞职权 (*辞职不需要受权)
		//DWORD QuitFaction:1;			// 退帮
		//DWORD IsMsg:1;				// 是否有留言
		//-----------------------------------------用了28位，还剩下的13位用于以后扩展
	};


	

	struct SimMemberInfo : public SimMemberRight
	{
		DWORD dwJoinTime;				// (个人加入帮派/改变职位)的时间
		DWORD iDonation;				// 个人捐献金钱(以千为单位)
		DWORD dwAccomplishment;			// 个人成就
		DWORD dwWeeklySalary;			// 周薪
		DWORD  PlayerIconID;			//成员头像
		char  szName[CONST_USERNAME];				// 成员的名字
		char  szCustomTitle[CONST_USERNAME];		// 帮派内部由帮主指定的特殊称号
		
	};

	SimFactionInfo stFaction;
	SimMemberInfo  stMember[MAX_MEMBER_NUMBER];
};

// 这个版本用于服务器共享内存使用的,数据是最全面
struct SFactionData
{
	struct SFaction : public SimFactionData::SimFactionInfo
	{
		DWORD   dwTotalMoney;			// 每个周消耗的资金
		BYTE	byDelete;
	};

	struct SMember : public SimFactionData::SimMemberInfo
	{
		DNID  dnidClient;
		DWORD wServerID;				// 所在区域服务器的ID值
		DWORD LastLineTime;	
	};

	SFaction stFaction;
	SMember stMember[MAX_MEMBER_NUMBER];
};

// 这个版本用于DB和登陆保存用的
struct SaveFactionData
{
	struct SaveFactionInfo : public SimFactionData::SimFactionInfo
	{
	};

	struct SaveMemberInfo : public SimFactionData::SimMemberInfo
	{
	};

	SaveFactionInfo stFaction;
	SaveMemberInfo  stMember[MAX_MEMBER_NUMBER];
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 帮会相关消息类
//=============================================================================================
DECLARE_MSG_MAP(STongBaseMsg,SMessage,SMessage::EPRO_TONG_MESSAGE)
//{{AFX
EPRO_TONG_SYSMSG,
EPRO_TONG_REQUARE_FACTION_NAME,     // 通知客户端输入帮派名字！用以创建帮派！
EPRO_TONG_QUEST_JOIN,               // 邀请其他玩家加入自己帮派！
EPRO_TONG_MEMBER_UPDATE,            // 帮派成员的单个更新！
EPRO_TONG_DELETE,				    // 删除帮派
EPRO_TONG_INITFACTION_INFO,			// 发送帮派里的基础数据给客服端
EPRO_TONG_SEND_SERVER,				// 帮派广播消息用的
EPRO_INPUT_MEMO,					// 要求输入帮派宣言
EPRO_TONG_UPDATA_FACTION_HEAD,		// 客服端请求更新帮派数据
EPRO_TONG_MEMBER_MANAGER,			// 帮派成员管理
EPRO_FACTIONBBS_MESSAGE,			// 帮派的留言管理
EPRO_TONG_ENTERREGION_NOTIFY,		// 玩家进入帮派场景时的提示消息！
EPRO_TONG_UPDATA_RECRUIT,           // 更改招募状态
EPRO_TONG_REMOTE_APPLY_JOIN,        // 远程申请进帮
EPRO_TONG_RECV_RAPPLY_JOIN,         // 处理远程申请进帮

EPRO_TONG_CREATE_FACTION,			//创建军团
EPRO_TONG_FACTION_INFO,               // 军团数据！
EPRO_TONG_FACTION_LIST,               // 所有军团列表
EPRO_TONG_QUEST_JOINFACTION,           // 请求加入军团
EPRO_TONG_QUEST_LEAVEFACTION,           // 请求离开军团
//EPRO_TONG_SHOW_JOINLIST,           // 军团长查看当前请求加入军团的人员列表
EPRO_TONG_MANAGER_MEMBER,          // 军团长对军团的管理
EPRO_TONG_FACTION_DELETE,				    // 删除军团
EPRO_TONG_SYNFACTION,				    // 同步军团信息
EPRO_TONG_SCENEDATA,				    // 军团副本信息
EPRO_TONG_SCENEMAPDATA,				    // 军团副本关卡信息
EPRO_TONG_SCENECHALLENGE,				    // 军团关卡挑战
EPRO_TONG_SCENEFINISH,				    // 军团副本挑战完成
EPRO_TONG_SCENEHURTRANK,				    // 军团副本伤害排行
EPRO_TONG_INITFACTIONSALARY,				    // 军团俸禄初始化
EPRO_TONG_GETFACTIONSALARY,				    // 领取军团俸禄
EPRO_TONG_NOTICE,									//军团公告
EPRO_TONG_MODIFYNOTICE,									//军团修改公告
EPRO_TONG_OPERATELOG,									//军团操作日志
EPRO_TONG_SENDEMAILTOALL,							//军团群发邮件
EPRO_TONG_SHOWEQUIPT,							//军团查看可以申请到的装备列表
EPRO_TONG_QUESTEQUIPT,							//军团申请装备
EPRO_TONG_SHOWQUESTSTATUS,							//查看申请状态
EPRO_TONG_CANCELEDQUEST,							//取消申请装备

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

/************************************/
/* 帮派数据部分的消息管理				*/
/************************************/
//=============================================================================================
// 邀请其他玩家加入自己帮派！
DECLARE_MSG(SQuestJoinMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_JOIN)
// 服务器通知目标玩家打开确定加入帮派面板
struct SAQuestJoinMsg :
    public SQuestJoinMsg
{
    DWORD dwSrcGID;     // 帮派邀请发起人的GID
    DWORD dwCheckID;    // 用于校验的ID
};

// 客户端提交确认结果到服务器
struct SQQuestJoinMsg :
    public SQuestJoinMsg
{
    DWORD dwSrcGID;     // 帮派邀请发起人的GID（回传）
    DWORD dwCheckID;    // 用于校验的ID（回传）
    BOOL bAccept;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================

//---------------------------------------------------------------------------------------------

//=============================================================================================
// 删除一个帮派
DECLARE_MSG(SDeleteFactionrMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_DELETE)
struct SQDeleteFactionrMsg:
    public SDeleteFactionrMsg
{
	// 这个是发给登陆用，登陆在转回给区域，区域清除其成员数据,最后删除掉
	char szFactionName[CONST_USERNAME];// 所要操作的帮派名
};

struct SADeleteFactionrMsg:
    public SDeleteFactionrMsg
{
	// 这个是发给客户端用的
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 帮派成员一组的更新！
DECLARE_MSG(SInitFactionInfoMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_INITFACTION_INFO)
// 服务器通知客户端更新帮派数据
struct SAInitFactionInfoMsg:
	public SInitFactionInfoMsg
{
	BYTE byMemberNum;				// 成员数量
	char szFactionName[CONST_USERNAME];			// 帮派名字
	char szCreatorName[CONST_USERNAME];			// 创始人名字
	SimFactionData::SimMemberInfo stMemberInfo[MAX_MEMBER_NUMBER];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 请求更新帮派数据
DECLARE_MSG(SUpdataFactionHeadMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_UPDATA_FACTION_HEAD)
// 客户端请求更新帮派数据
struct SQUpdataFactionHeadMsg:
	public SUpdataFactionHeadMsg
{
};

struct SAUpdataFactionHeadMsg:
	public SUpdataFactionHeadMsg
{
	char szFactionMemo[50];			// 帮派宣言暂定50个字符
	BYTE  byFactionLevel;			// 当前帮派等级
	DWORD dwFactionProsperity;		// 帮派繁荣度，以前是帮派活跃度
	DWORD dwTotalMoney;				// 周消耗的资金

	__int64 iMoney;					// 帮派资金
	DWORD dwWood;					// 木材
	DWORD dwStone;					// 石头
	DWORD dwMine;					// 矿石
	DWORD dwPaper;					// 纸浆
	DWORD dwJade;					// 玉石	

    BYTE  byOpenRApplyJoin;         // 开启远程入帮 1开启 0关闭
};
//---------------------------------------------------------------------------------------------


//=============================================================================================
// 帮派成员单个更新！
DECLARE_MSG(SUpdateMemberMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_MEMBER_UPDATE)
// 服务器通知客户端更新一个成员
struct SAUpdateMemberMsg:
    public SUpdateMemberMsg
{
    enum 
	{
		E_UPDATE_MEMBER,		// 添加或者更新
        E_DELETE_MEMBER,		// 删除
        E_DELETE_MEMBER_LEAVE	// 离开, 非帮主踢人
    };

	BYTE byType;            // 本次操作的类型
	SimFactionData::SimMemberInfo stInfo;
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 邀请其他玩家加入自己帮派！
DECLARE_MSG(SSendServerMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SEND_SERVER)
struct SQSendServerMsg :
    public SSendServerMsg
{
	enum { TALK, DELFACTION, UPDATEMEMBER, CITYOWENTALK, MSGCOPY }; //CITYOWENTALK要城主并且带了虎符才是
	BYTE byMsgType;
	WORD wRegion;			// 0=全服务器
	char szFactionName[CONST_USERNAME];
	char szBuffer[1024*5];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// 修改帮派宣言
DECLARE_MSG(SInputMemoMsg, STongBaseMsg, STongBaseMsg::EPRO_INPUT_MEMO)
struct SQInputMemoMsg :
    public SInputMemoMsg
{
	char szFactionMemo[50];
};

struct SAInputMemoMsg :
    public SInputMemoMsg
{
	char szFactionMemo[50];
};
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SMemberManager, STongBaseMsg, STongBaseMsg::EPRO_TONG_MEMBER_MANAGER)
struct SQMemberManager 
	: public SMemberManager
{
	enum { AUTHORIZE, EMPOWER, SHELVE, TITLENAME ,ABDICATE, BANISH, QUIT, KCAUTHORIZE, KCQUIT };
	WORD tpye;
	char streamData[1024];
};

struct SAMemberManager
	: public SMemberManager
{

};
//---------------------------------------------------------------------------------------------

DECLARE_MSG(_SEntryRegionNotify, STongBaseMsg, STongBaseMsg::EPRO_TONG_ENTERREGION_NOTIFY)
struct SEntryRegionNotify
	: public _SEntryRegionNotify
{
	WORD type; // 用于控制 streamData 解码方式的标志，主要用于后期扩展，现在暂时没用（置0！）
	char streamData[256];
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SRemoteApplyJoin, STongBaseMsg, STongBaseMsg::EPRO_TONG_REMOTE_APPLY_JOIN)
struct SQRemoteApplyJoin : public SRemoteApplyJoin
{
   DWORD dwApplyGID;             // 申请人的ID
   //DWORD dwTongID;               // 要申请进5大帮的ID
   char szTongName[CONST_USERNAME];
};

struct SARemoteApplyJoin: public SRemoteApplyJoin
{
    enum // 用于通知
    { 
        SA_AJ_MSGBOX,        // 弹出申请框
        SA_AJ_ADOPT,         // 通过申请
       
        // 失败信息
        SA_AJ_ER_BUSY,       // 帮主忙
        SA_AJ_ER_REFUSE,     // 拒绝申请
        SA_AJ_ER_CLOSE,      // 关闭申请
        SA_AJ_ER_OUTLINE,    // 帮主不在线
        SA_AJ_ER_HAVE,       // 申请者已经有帮
        SA_AJ_ER_NOTONG,     // 帮派不存在
        SA_AJ_ER_AJNULL,     // 申请人不在了
        
        SA_AJ_ER_UNKONWN     // 未知错误信息
    };
    WORD type;
    DWORD dwApplyGID;             // 申请人的ID
};

//---------------------------------------------------------------------------------------------

DECLARE_MSG(SRecvOperApplyJoin, STongBaseMsg, STongBaseMsg::EPRO_TONG_RECV_RAPPLY_JOIN)
struct SQRecvOperApplyJoin: public SRecvOperApplyJoin
{
    enum // 用于操作
    { 
        SA_RRAJ_ADOPT,         // 通过申请
        SA_RRAJ_REFUSE,        // 拒绝申请 
    };
    WORD type;
    DWORD dwSrcGID; 
    DWORD dwApplyGID;         // 申请人的ID
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SUpdateRecruit, STongBaseMsg, STongBaseMsg::EPRO_TONG_UPDATA_RECRUIT)
struct SQSUpdateRecruit : public SUpdateRecruit
{
    BYTE tpye;                // 0关闭 1开启
    DWORD dwApplyGID;         // 申请人的ID
};

struct SASUpdateRecruit: public SUpdateRecruit
{
    BYTE tpye;
};



//--2015/07/31  新加修改的军团

struct FactionMember   //用于网络发送的成员数据
{
	char strMemberName[CONST_USERNAME];
	WORD wLevel;						//等级
	WORD wFightPower;			//战斗力
	WORD wPlayerIcon;//玩家头像
	WORD  wWeekActiveValue;//7日活跃度贡献
	BYTE  bisonline;				//是否在线
	BYTE  Title;	//自己在帮派中的职位
};

struct FactionInfoList  //用以军团列表中列出军团的简单信息
{
	char szFactionName[CONST_USERNAME];			// 帮派名字
	char szCreatorName[CONST_USERNAME];			// 创始人名字 
	WORD  byFactionLevel;			// 当前帮派等级
	WORD FactionIcon;				//军团图标
	BYTE  CurMemberNum;				// 成员数量
	DWORD  FactionID;//军团ID
};

// 通知客户端输入帮派名字！用以创建帮派！
DECLARE_MSG(SCreateFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_CREATE_FACTION)
// 客户端提交输入的帮派名字到服务器
struct SQCreateFaction :public SCreateFaction
{
	char szFactionName[CONST_USERNAME];
	WORD	wFactionIcon;		//军团图标
};
// 服务器返回结果
struct SACreateFaction :public SCreateFaction
{
	BYTE		bresult;//  1 成功 2 元宝不足 3 名字重复 4 名字长度不足 5 名字不符合要求
};


//请求自身军团数据
DECLARE_MSG(SFactioninfo, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_INFO)
struct SQFactioninfo : public SFactioninfo
{
};

struct SAFactioninfo : public SFactioninfo
{
	char szFactionName[CONST_USERNAME];			// 帮派名字
	char szCreatorName[CONST_USERNAME];			// 创始人名字 

	WORD FactionIcon;
	WORD  byFactionLevel;			// 当前帮派等级
	BYTE  CurMemberNum;				// 成员数量
	BYTE	  QuestJoinNum;			//申请军团人员数量
	DWORD  FactionID;//军团ID
	DWORD  dActiveValue;		//军团活跃度
	DWORD   dfactionexp;  //军团的经验值
	BYTE  Title;	//自己在帮派中的职位
	FactionMember  m_factionmember[MAX_FACTION_NUMBER];
};
//军团所使用的数据结构
typedef struct ITEMDATA   //道具结构体
{
	BYTE  bitemtype;				//道具类型
	DWORD  ditemIndex;    //道具ID
	DWORD  ditemnum;     //道具数量
}itemdata;

typedef struct FACTIONSCENE   //军团关卡
{
	BYTE   bscenestatus;		//副本状态  0 副本关闭未通关 1 副本关闭已通关  2 副本开启未通关 3 副本开启已通关 
	WORD   wTurnOnNum;  //副本关卡进度
	DWORD dsceneID;     //副本ID
}factionscene;

typedef struct FACTIONSCENEMAP   //军团关卡怪物
{
	BYTE  bscenemapIndex;//关卡索引
	BYTE   bscenestatus;		//副本状态  0 副本关闭未通关 1 副本关闭已通关  2 副本开启未通关 3 副本开启已通关 
	double   dmonsterHP;		//当前关卡中的怪物总血量
	double   dmonstercurrentHP;	//关卡怪物当前血量 
}factionscenemap;

typedef struct FACTIONSCENERANK  //军团伤害排名
{
	char playername[CONST_USERNAME];  //玩家名字
	double  dHurt;		//玩家伤害
	DWORD  drewardnum; //可得到的军需令数量
}factionscenerank;

typedef struct FACTIONSALARYDATA   //军团俸禄配置
{
	//DWORD ditemlist[10];  //道具ID + 数量  （每四个字节）
	itemdata  sitemdata[MAX_SENDFACTIONS];
	DWORD  dActivityValue;//增加的活跃度
	DWORD  dExp; //增加的经验
	WORD  wcostgold;	//需要花费的元宝数量
}factionsalarydata;

typedef struct OPERATELOG    //操作日志
{
	BYTE   btype;			//类型
	char  strName[CONST_USERNAME];  //操作玩家名
	DWORD  dParamID;			//在军团副本中表示军团副本ID
}operatelog;


typedef struct REQUESTELIST     //申请装备
{
	WORD  wRank;
	WORD wIconIndex; //头像
	WORD  wLevle;// 等级
	char   strname[CONST_USERNAME]; //玩家名字
}requestlist;

//请求军团列表
DECLARE_MSG(SFactionList, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_LIST)
struct SQFactionList : public SFactionList
{
	BYTE		btype;			//查找类型 0  全部类别  1 ID查找 2 名字查找
	DWORD  FactionID;//军团ID
	char szFactionName[CONST_USERNAME]; //军团名字
};
struct SAFactionList : public SFactionList
{
	FactionInfoList m_factionlist[MAX_FACTION_NUMBER];

};

//请求加入某个军团
DECLARE_MSG(SJoinFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_JOINFACTION)
struct SQJoinFaction : public SJoinFaction
{
	DWORD dFactionID;			//军团ID
};
struct SAJoinFaction : public SJoinFaction
{
	BYTE bresult;	//1 成功  2 已经达到最大申请请求  3  军团成员已满
};

//请求离开军团
DECLARE_MSG(SLeaveFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUEST_LEAVEFACTION)
struct SQLeaveFaction : public SLeaveFaction
{
};
struct SALeaveFaction : public SLeaveFaction
{
	BYTE bresult;	//1 成功返回    
};

//军团长管理军团
DECLARE_MSG(SManagerFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_MANAGER_MEMBER)
struct SQManagerFaction : public SManagerFaction
{
	BYTE  bManagerType;		//   0  查看申请加入军团玩家列表     1   同意或拒绝玩家加入  2  踢玩家出军团 
	BYTE		bAgree;					//   同意或拒绝玩家加入
	char szPlayerName[CONST_USERNAME];			// 需要操作的玩家名字 
};

struct SAManagerFaction : public SManagerFaction
{
	BYTE bresult;	//1 成功返回    0  权限不足
	BYTE  bMembernum;	//实际数量
	BYTE		Buffer[MAX_JoinListCount * sizeof(FactionMember)];//当前请求的玩家列表 如果是其他类型就不会发送数据
};

//解散军团disband
DECLARE_MSG(SDisbandFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_FACTION_DELETE)
struct SQDisbandFaction : public SDisbandFaction
{
};

struct SADisbandFaction : public SDisbandFaction
{
	BYTE bresult;	//1 成功返回    0  权限不足
};



//同步军团
DECLARE_MSG(SSynFaction, STongBaseMsg, STongBaseMsg::EPRO_TONG_SYNFACTION)
struct SQSynFaction : public SSynFaction
{
};
struct SASynFaction : public SSynFaction
{
	DWORD  dFactionID;    //所属军团ID
	DWORD  dJoinID;        //当前请求的ID
	DWORD  dFactionMoney;		//自己的军需令
	BYTE			bJoinState;	 //请求的状态  0 无状态   1 已经同意加入  2 拒绝
};



//军团副本信息
DECLARE_MSG(SFactionSceneData, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEDATA)
struct SQFactionSceneData : public SFactionSceneData
{
	BYTE   btype;	//   0  请求同步副本数据   1   开启副本
	DWORD   dsceneID;        //开启某个副本
};
struct SAFactionSceneData : public SFactionSceneData
{
	WORD   wturnonnum;			//当前打开的数量
	DWORD    wactivevalue;			//军团当前活跃度
	factionscene m_factionscene[MAX_SENDFACTIONS];  //副本列表
};

//同步军团副本关卡信息
DECLARE_MSG(SFactionSceneMapData, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEMAPDATA)
struct SQFactionSceneMapData : public SFactionSceneMapData
{
	DWORD   dsceneID;        //副本ID
};
struct SAFactionSceneMapData : public SFactionSceneMapData
{
	BYTE   bChallengeCount; //挑战次数
	DWORD   dsceneID;        //副本ID
	factionscenemap  m_factionscenemap[MAX_SENDFACTIONS];   //关卡信息列表
};


//挑战关卡
DECLARE_MSG(SChallengeSceneMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENECHALLENGE)
struct SQChallengeSceneMsg : public SChallengeSceneMsg
{
	DWORD dsceneID; //副本ID
	BYTE		bscenemapIndex;	//挑战的关卡中副本中的索引
};
struct SAChallengeSceneMsg : public SChallengeSceneMsg
{
	BYTE  bresult;  //  结果
	MonsterData  monsterdata;	//当前的怪物数据
};

//挑战关卡完成
DECLARE_MSG(SSceneFinishMsg, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEFINISH)
struct SQSceneFinishMsg : public SSceneFinishMsg
{
	double   dHurtValue;		//对怪物造成的伤害
};

DECLARE_MSG(SSceneHurtRank, STongBaseMsg, STongBaseMsg::EPRO_TONG_SCENEHURTRANK)
struct SQSceneHurtRank : public SSceneHurtRank
{
	DWORD dsceneID; //副本ID
};
struct SASceneHurtRank : public SSceneHurtRank
{
	factionscenerank m_factionrank[MAX_FACTION_NUMBER];
	double  dselfhurt;   //自身伤害
	WORD  wRankNum;//排行榜数量
	WORD  wPlayerRank;//玩家自身的排名
	DWORD dRewardNum;// 可得到的军需令数量
	BYTE    bNextRange;//距离下一阶段的奖励相差数量
};

//初始化获取俸禄配置
DECLARE_MSG(SInitFactionSalary, STongBaseMsg, STongBaseMsg::EPRO_TONG_INITFACTIONSALARY)
struct SQInitFactionSalary : public SInitFactionSalary
{
};
struct SAInitFactionSalary : public SInitFactionSalary
{
	BYTE   bIsGot;  // 是否已经领取   
	factionsalarydata  m_fsalarydata[2];
};

//玩家领取
DECLARE_MSG(SGetFactionSalary, STongBaseMsg, STongBaseMsg::EPRO_TONG_GETFACTIONSALARY)
struct SQGetFactionSalary : public SGetFactionSalary
{
	BYTE  bType; //领取在配置中的索引
};
struct SAGetFactionSalary : public SGetFactionSalary
{
	BYTE  bresult;		//  1 成功  2 元宝不足  3 已经领取  4  无军团或者配置出错等
};

//军团公告
DECLARE_MSG(SFactionNotice, STongBaseMsg, STongBaseMsg::EPRO_TONG_NOTICE)
struct SQFactionNotice : public SFactionNotice
{ 
};
struct SAFactionNotice : public SFactionNotice
{
	char strnotice[MAX_NOTICE_NUMBER];
};


//军团修改公告
DECLARE_MSG(SFModifyNotice, STongBaseMsg, STongBaseMsg::EPRO_TONG_MODIFYNOTICE)
struct SQFModifyNotice : public SFModifyNotice
{
	char modifynotice[MAX_NOTICE_NUMBER];
};
struct SAFModifyNotice : public SFModifyNotice
{
	BYTE  bresult;		// 0修改失败 1 成功 
};

//军团操作日志
DECLARE_MSG(SFactionOperateLog, STongBaseMsg, STongBaseMsg::EPRO_TONG_OPERATELOG)
struct SQFactionOperateLog : public SFactionOperateLog
{
};
struct SAFactionOperateLog : public SFactionOperateLog
{
	BYTE			blognum;
	operatelog   factionlog[MAX_NOTICE_NUMBER];
};


//军团群发邮件
DECLARE_MSG(SFcEmailToAll, STongBaseMsg, STongBaseMsg::EPRO_TONG_SENDEMAILTOALL)
struct SQFcEmailToAll : public SFcEmailToAll
{
	char title[CONSTMAILTITLE];	//邮件标题,要填
	char content[CONSTMAILCONTENT];//邮件内容,要填
};
struct SAFcEmailToAll : public SFcEmailToAll
{
	BYTE			bresult; //0  发送失败  1成功
};

//军团查看可以申请到的装备列表
DECLARE_MSG(SShowEquipt, STongBaseMsg, STongBaseMsg::EPRO_TONG_SHOWEQUIPT)
struct SQShowEquipt : public SShowEquipt
{
};
struct SAShowEquipt : public SShowEquipt
{
	BYTE  blevelnum;   //装备等级数量
	DWORD  dSelectedEquipt;    //玩家已选的装备
	BYTE  Buffer[MAX_QUESTEQUIPTBUFFER];
};

//军团申请到的装备
DECLARE_MSG(SRequestEquipt, STongBaseMsg, STongBaseMsg::EPRO_TONG_QUESTEQUIPT)
struct SQRequestEquipt : public SRequestEquipt
{
	BYTE  blevle;    //等级
	BYTE   bIndex;  //索引
};
struct SARequestEquipt : public SRequestEquipt
{
	BYTE  bresult;   //申请结果  1成功 2 申请等级或者索引出错  3 申请人数超出限制 4 玩家没有加入军团 5 在申请冷却时间内  6 已经申请过装备 
	DWORD dParam;//扩展参数 

};

//军团申请装备状态
DECLARE_MSG(SRequestStatus, STongBaseMsg, STongBaseMsg::EPRO_TONG_SHOWQUESTSTATUS)
struct SQRequestStatus : public SRequestStatus
{
	BYTE    bCanceled;	//是否取消申请        1 取消申请      0 查看当前申请状态
};
struct SARequestStatus : public SRequestStatus
{
	WORD   wEquiptNum;	//当前数量
	requestlist mquestlist[MAX_EQUIPTLISTNUM];
};

//取消当前的申请
DECLARE_MSG(SCanceledQuest, STongBaseMsg, STongBaseMsg::EPRO_TONG_CANCELEDQUEST)
struct SQCanceledQuest : public SCanceledQuest
{
};
struct SACanceledQuest : public SCanceledQuest
{
	BYTE  bresult;   //1 成功  2 取消申请的装备没有找到 3 没有加入军团
};


struct SaveFactionData_New
{
	FactionMember sjoinlist[MAX_JoinListCount];  //申请本军团的玩家列表
	SAFactioninfo sfactioninfo;								//军团数据
};


struct SaveFactionData_Lua
{
	char szFactionName[CONST_USERNAME];			// 帮派名字
	DWORD  FactionID;//军团ID
	DWORD   dfactionlenth;
	BYTE   bFactionData[FACTIONDATA_LENTH];  //军团数据
};





//----------------------------------------------------------三国军团----------------------------------------你告诉我--------------------------------------------------------


struct SAFactionListHeadData
{
	char szFactionName[CONST_USERNAME];			// 帮派名字
	char szCreatorName[CONST_USERNAME];			// 创始人名字 
	//char FactionIcon[MAX_PlayerIcon];//军团图标
	int FactionIcon;
	int  byFactionLevel;			// 当前帮派等级
	int  CurMemberNum;				// 成员数量
	int FactionID;//军团ID

	int Title;	//自己在帮派中的职位
	int FactionExp;					//军团经验
	int FactionMoney;				//军团金钱
	char FactionMemo[100];				//军团宣言
	SAFactionListHeadData()
	{
		memset(szFactionName, 0, sizeof(szFactionName));
		memset(szCreatorName, 0, sizeof(szCreatorName));
		//memset(FactionIcon, 0, sizeof(FactionIcon));
		FactionIcon = 0;
		byFactionLevel=0;			// 当前帮派等级
		CurMemberNum=0;				// 成员数量
		FactionID=0;//军团ID
		FactionExp = 0;
		FactionMoney = 0;
		Title = 0;
		memset(FactionMemo, 0, sizeof(FactionMemo));
	}

};

enum Enum_TongMsg
{
	Tong_CreateFaction = 80,		//创建帮会
	Tong_DletePalyer,		//玩家离开帮会
	Tong_JoinFation,		//玩家请求加入帮会
	Tong_JoinFationToList,		//玩家请求加入帮会到申请表 搜索帮会用的同一个
	Tong_DisposeFaction,	//解散帮会
	Tong_RequestFactionList,	//请求帮会列表
	Tong_RequestMySelfFactionMsg,	//请求自己的帮会信息
	Tong_ChangeFactionMemo,	//变更帮会日志
	Tong_RequestGetApplicant,//获取申请人队列
	Tong_RequestMembersData, //获取成员数据
	Tong_RequestAffirmJioned, //确认加入情况
	Tong_MemberLeave //玩家离开玩家
};

//错误枚举
enum Enum_TongMsg_ErrorCode
{
	Tong_OK,
	Tong_ReName ,    //军团名已存在
	Tong_MaxCount ,  //达到军团创建极限
	Tong_VaildName , //无效军团名
	Tong_IntoIng, 	//已有军团
	Tong_MemBerToMax,	//军团人数已满
	Tong_JoinED,		//玩家已被其他公会收录
	Tong_RequestJionListToMax, //请求列表达到上限
	Tong_RequestJionCurDayToMax, //申请数量达到上限
	Tong_FacationDsiposeed, //帮会已经解散
	Tong_FacationJoinNoOk //入帮申请被拒绝
};

//创建军团请求
struct SQCreatetchTong 
{
	 byte _messageprotocol;
	 byte _baseProtocol;

	 char factionName[CONST_USERNAME];
	 //char factionIcon[MAX_PlayerIcon];
	 int factionIcon;
	 SQCreatetchTong()
	 {
		 memset(factionName, 0, sizeof(factionName));
	 }
};

struct SACreatetchTong
{
	byte _messageprotocol;
	byte _baseProtocol;
	byte _result;

	SACreatetchTong()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_baseProtocol = Enum_TongMsg::Tong_CreateFaction;
		_result = 0;
	}
};


//请求帮派列表
struct SQRequestFactionList
{
	byte _messageprotocol;
	byte _protocol;
	bool allOrsingle;
	int factionID;
	char factionName[32];
	SQRequestFactionList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestFactionList;
		memset(factionName, 0, sizeof(factionName));
		factionID = 0;
	}

};

struct SARequestFactionList
{
	byte _messageprotocol;
	byte _protocol;
	byte Fcount;
	SAFactionListHeadData factionlist[MAX_FACTION_NUMBER];
	SARequestFactionList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestFactionList;
		Fcount = 0;
	}
};


//请求加入帮派
struct SQRequestJoinFaction
{
	byte _messageprotocol;
	byte _protocol;
	bool agree;
	char RequsetName[CONST_USERNAME];

	SQRequestJoinFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFation;
		memset(RequsetName, 0, sizeof(RequsetName));
	}

};

struct SARequestJoinFaction
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SARequestJoinFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFation;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};

//加入军团到申请表
struct SQRequestJoinFactionToList
{
	byte _messageprotocol;
	byte _protocol;
	char factionName[CONST_USERNAME];

	SQRequestJoinFactionToList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFationToList;
		memset(factionName, 0, sizeof(factionName));
	}

};

struct SARequestJoinFactionToList
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SARequestJoinFactionToList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_JoinFationToList;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};





//解散军团
struct SQDisposeFaction
{
	byte _messageprotocol;
	byte _protocol;
	char factionName[CONST_USERNAME];
	SQDisposeFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DisposeFaction;
		memset(factionName, 0, sizeof(factionName));
	}

};

struct SADisposeFaction
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SADisposeFaction()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DisposeFaction;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}

};



//提出军团
struct SQDletePalyer
{
	byte _messageprotocol;
	byte _protocol;
	char pname[CONST_USERNAME];
	SQDletePalyer()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DletePalyer;
		memset(pname, 0, sizeof(pname));
	}

};

struct SADletePalyer
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SADletePalyer()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_DletePalyer;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}

};

//更改帮派宣言
struct SQChangeFactionMemo
{
	byte _messageprotocol;
	byte _protocol;
	char memomsg[100];
	SQChangeFactionMemo()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_ChangeFactionMemo;
		memset(memomsg, 0, sizeof(memomsg));
	}
};

struct SAChangeFactionMemo
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SAChangeFactionMemo()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_ChangeFactionMemo;
		result = Enum_TongMsg_ErrorCode::Tong_OK;
	}
};



//请求自身帮数据
struct SQMySelfFactionMsg
{
	byte _messageprotocol;
	byte _protocol;
	byte factionName[CONST_USERNAME];
	byte pname[CONST_USERNAME];
	
	SQMySelfFactionMsg()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMySelfFactionMsg;
		memset(factionName, 0, sizeof(factionName));
		memset(pname, 0, sizeof(pname));
	};
};

struct SAMySelfFactionMsg
{
	byte _messageprotocol;
	byte _protocol;
	byte result;
	SAFactionListHeadData headdata;
	SAMySelfFactionMsg()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMySelfFactionMsg;
		result = Enum_TongMsg_ErrorCode::Tong_OK;

	};
};

// 获取申请人队列
struct SQArmyGroupApplicantList
{
	byte _messageprotocol;
	byte _protocol;
	SQArmyGroupApplicantList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestGetApplicant;
	}
};

struct SAArmyGroupApplicantList
{
	byte _messageprotocol;
	byte _protocol;
	byte _count;
	SimFactionData::RequestJionPlayer RjoinList[20];
	
	SAArmyGroupApplicantList()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestGetApplicant;
		_count = 0;
		memset(RjoinList, 0, sizeof(RjoinList));
	};
};

//获取成员数据
struct SQArmyGroupGetMembers
{
	byte _messageprotocol;
	byte _protocol;
	SQArmyGroupGetMembers()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMembersData;
	};
};

struct ArmyGroupMembersData
{
	char name[CONST_USERNAME];
	int IconID;
	int level;
	int Position;
	int LineTime;
	int LastLineTime;

	ArmyGroupMembersData()
	{
		IconID = 0;
		memset(name, 0, sizeof(name));
		level = 0;
		Position = 0;
		LineTime = 0;
		LastLineTime = 0;
	};
};

struct SAArmyGroupGetMembers
{
	byte _messageprotocol;
	byte _protocol;
	byte _count;
	ArmyGroupMembersData memberlist[MAX_FACTION_NUMBER];
	SAArmyGroupGetMembers()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestMembersData;
		_count = 0;
		//memset(members, 0, sizeof(members));
	};
};




//确认进团消息
struct SQAffirmJoinFacttion
{
	byte _messageprotocol;
	byte _protocol;

	SQAffirmJoinFacttion()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestAffirmJioned;
		
	};
};

struct SAAffirmJoinFacttion
{
	byte _messageprotocol;
	byte _protocol;
	byte factionName[CONST_USERNAME];
	bool result;
	SAAffirmJoinFacttion()
	{
		_messageprotocol = SMessage::EPRO_TONG_MESSAGE;
		_protocol = Enum_TongMsg::Tong_RequestAffirmJioned;
		result = true;
		memset(factionName, 0, sizeof(factionName));
	};
};
