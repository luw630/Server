#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
#define MAX_STAR_SIZE 256				//评分数据大小
#define MAX_SCENE_SIZE		1024		//关卡数据大小
#define MAX_IREWARD_SIZE	256		//奖励道具数据大小
#define MAX_OBJ	3
#define MAX_LIST_SIZE		128
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 关卡相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SSceneMsg, SMessage, SMessage::EPRO_SCENE_MESSAGE)
EPRO_SCENE_INIT,				//进入场景地图关卡同步一次数据
EPRO_SCENE_SELECT,				//选择其中某一个关卡
EPRO_SCENE_ENTER,				//选择进入某一个关卡
EPRO_SCENE_WINREWARD,		//胜利结算
EPRO_SCENE_PICKCARDS,		//胜利以后抽牌
EPRO_SCENE_FAIL,					//闯关失败
EPRO_SCENE_SDINFO,				//扫荡消息
EPRO_SCENE_SDREWARD,		//请求在扫荡后的奖励
EPRO_SCENE_LEAVE,					//请求离开关卡
EPRO_SCENE_LIST,					//章节的关卡列表
EPRO_SCENE_FINISH,					//发送关卡完成
EPRO_SCENE_QUESTREWARD,		//请求关卡奖励
EPRO_SCENE_DEBRISINFO,		//碎片收集信息
EPRO_SCENE_DEBRISAWARD,		//碎片奖励相关
EPRO_SCENE_BATCHESMONSTER, //分批刷怪相关通知
EPRO_SCENE_SDREWARDLIST,				//扫荡奖励列表
EPRO_SCENE_GROUPMONSTERDIE,			//当前分组的怪物已经死亡(用于单机模式客户端通知服务器)
END_MSG_MAP()


// 请求当前的关卡数据
DECLARE_MSG( SSceneInit, SSceneMsg, SSceneMsg::EPRO_SCENE_INIT )
struct SQSceneInit : public SSceneInit
{
	BYTE		bSceneMapIndex;			//请求的章节索引
};

struct SASceneInit : public SSceneInit
{
	WORD	wCurrentSceneMap;	//当前章节地图
	WORD	wCurrentScene;	//当前关卡
	BYTE		bStarData[MAX_STAR_SIZE]; //当前关卡所包含的评分数据
};


// 请求选择的关卡数据， 当客户端选择某一关卡的时候
DECLARE_MSG( SSceneSelect, SSceneMsg, SSceneMsg::EPRO_SCENE_SELECT )
struct SQSceneSelect : public SSceneSelect
{
	WORD	wSceneMap;		//选择的章节地图
	WORD wsceneIndex;		//选择的关卡
};
struct SASceneSelect : public SSceneSelect
{
	WORD	wscenemap;		//选择的章节
	WORD wsceneIndex;		//选择的关卡
	BYTE		bsdtime;		//当前剩余扫荡次数
	BYTE		bmaxsdtime;		//最大扫荡次数

	BYTE		bcommontime;	//当前剩余普通次数
	BYTE		bmaxcommontime;	//最大普通次数

	BYTE		bsplimit;		//体力限制，进入副本所需消耗的体力
	WORD SDTime;		//通关关卡所需时间秒
	WORD  wbesttime;//最佳时间秒

	DWORD	dexpreward;	//  经验奖励
	DWORD	dmoneyreward;//金钱奖励
	BYTE			bitemreward[MAX_IREWARD_SIZE];//装备奖励 UINT 类型
	BYTE		bequip[MAX_IREWARD_SIZE];//装备推荐 UINT 类型
	
	char   bestplayername[CONST_USERNAME];//最佳完成玩家姓名
	
};


//  当客户端选择进入某一关卡的时候
DECLARE_MSG( SSceneEnter, SSceneMsg, SSceneMsg::EPRO_SCENE_ENTER )
struct SQSceneEnter : public SSceneEnter
{
	WORD	wSceneMap;		//选择的章节地图
	WORD wsceneIndex;		//选择的关卡
	BYTE		bentertype;		//进入方式，0 进入 1 扫荡
	BYTE		bcount;				//扫荡次数
};
struct SASceneEnter : public SSceneEnter
{
	WORD  wSDcost;			//扫荡立即冷却所需金币
	DWORD	dSDTime;		//扫荡通关冷却时间秒
	BYTE		bresult;			// 0 普通通关开始 1 扫荡通关开始 2 进入关卡或者扫荡关卡失败 体力不足 3 次数限制 4 金钱不足 
}; 

//通关关卡以后的奖励结算数据
DECLARE_MSG( SWinreward, SSceneMsg, SSceneMsg::EPRO_SCENE_WINREWARD )
struct SAWinrewardInfo : public SWinreward				
{
	BYTE	bwintype;		//通关类型 0 普通通关 1扫荡 
	BYTE bwinstar;		//评星星级
	BYTE	 bspecialscene;			//奇遇关卡 1 普通关卡 2奇遇关卡
	WORD	wfinishtime;	//通关时间
	WORD	wkillcount;	//击杀数量
	WORD	wmaxhit; //最大连击
	DWORD	dexp[MAX_OBJ];//经验值 0  玩家 1 宠物 2 坐骑
	DWORD	dmoney;//玩家得到的银币数量
	DWORD	dmoneyextract[3];		//游戏币抽取
	DWORD	dgoldextract[3];			//金币抽取
	BYTE			bitemreward[MAX_IREWARD_SIZE];//装备奖励 UINT 类型		
};

//客户端请求抽牌胜利奖励
DECLARE_MSG( SPickCards, SSceneMsg, SSceneMsg::EPRO_SCENE_PICKCARDS )
struct SQPickCards: public SPickCards		
{
	BYTE		bextracttype; //抽牌类型 0 游戏币 1 金币
	BYTE		bextractindex;//抽牌索引
};

//服务器发送抽牌胜利奖励
struct SAPickCards: public SPickCards		
{
	BYTE		bextracttype; //抽牌类型 0 游戏币 1 金币
	BYTE		bextractindex;//抽牌索引
	BYTE		bitemreward[MAX_IREWARD_SIZE];//装备奖励 UINT 类型		
};

//闯关失败消息
DECLARE_MSG( SPassFail, SSceneMsg, SSceneMsg::EPRO_SCENE_FAIL )
struct SAPassFail:public SPassFail
{
	WORD	wfinishtime;	//通关时间
	WORD	wmaxhit; //最大连击
};

//客户端请求扫荡终止
DECLARE_MSG( SStopSD, SSceneMsg, SSceneMsg::EPRO_SCENE_SDINFO )
struct SQStopSD: public SStopSD		
{
	BYTE bStopType;// 扫荡停止类型 0 扫荡停止 1 使用金币直接完成扫荡
};

struct SAStopSD: public SStopSD		
{
	WORD	wSceneMap;		//选择的章节地图
	WORD wsceneIndex;		//选择的关卡 
	BYTE		bresult;				// 0 扫荡关卡完成  1 已经停止扫荡的关卡  2 停止失败 3 上线开始扫荡后发送扫荡的关卡ID
};


//客户端请求扫荡的奖励
DECLARE_MSG( SSDReward, SSceneMsg, SSceneMsg::EPRO_SCENE_SDREWARD )
struct SQSDReward: public SSDReward		
{
	BYTE bisdouble;//是否双倍购买 0 1
};

struct SASDReward: public SSDReward		
{
	BYTE		bresult;				// 0 无法领取错误 背包已满   1 奖励领取完成（关闭界面） 
};


//客户端请求离开关卡
DECLARE_MSG( SSceneLeave, SSceneMsg, SSceneMsg::EPRO_SCENE_LEAVE )
struct SQSceneLeave: public SSceneLeave		
{
};

//打开章节列表面板
DECLARE_MSG( SSceneList, SSceneMsg, SSceneMsg::EPRO_SCENE_LIST )
struct SASceneList: public SSceneList		
{
	WORD	wSceneMap;		//选择的章节地图
	WORD	wCurrentScene;	//已经解锁的最大关卡
	BYTE		bhighlight[MAX_LIST_SIZE];//需要高亮的关卡索引 ，以1为开始索引
};

//发送当前关卡已经完成的消息 
DECLARE_MSG(SSceneFinish, SSceneMsg, SSceneMsg::EPRO_SCENE_FINISH)
struct SASceneFinish : public SSceneFinish
{
};

//请求关卡奖励
DECLARE_MSG(SSceneQuestReward, SSceneMsg, SSceneMsg::EPRO_SCENE_QUESTREWARD)
struct SQSceneQuestReward : public SSceneQuestReward
{
};

//请求碎片收集信息
DECLARE_MSG(SDebrisinfo, SSceneMsg, SSceneMsg::EPRO_SCENE_DEBRISINFO)
struct SQDebrisinfo : public SDebrisinfo
{
};

struct SADebrisinfo : public SDebrisinfo
{
	BYTE bbadgeindex;//当前收集的徽章索引
	BYTE bDebrisnum;//当前碎片的数量
	BYTE bDebrisAllnum;//当前徽章所有碎片的数量
};

//碎片奖励
DECLARE_MSG(SDebrisAward, SSceneMsg, SSceneMsg::EPRO_SCENE_DEBRISAWARD)
struct SADebrisAward : public SDebrisAward   //通知客户端可以领取当前徽章奖励
{
};

struct SQDebrisAward : public SDebrisAward   //客户端请求领取当前徽章奖励
{
};

//分批刷怪通知
DECLARE_MSG(SBatchesMonster, SSceneMsg, SSceneMsg::EPRO_SCENE_BATCHESMONSTER)
struct SABatchesMonster : public SBatchesMonster   //分批刷怪通知
{
	BYTE bIndex; //当前第几批
	BYTE bstate;//状态，0 死亡 1刷新
};

// 关卡扫荡完成时发送当前扫荡奖励列表
DECLARE_MSG(SSceneRewardList, SSceneMsg, SSceneMsg::EPRO_SCENE_SDREWARDLIST)
struct SASceneRewardList : public SSceneRewardList   //扫荡奖励列表
{
	WORD	wSceneMap;		//选择的章节地图
	WORD wsceneIndex;		//选择的关卡 
	DWORD	dexp;//经验值奖励 
	DWORD	dmoney;//玩家银币奖励
	WORD  wbuymoey; //购买双倍花费金币
	BYTE		bitemreward[MAX_IREWARD_SIZE];//装备奖励 UINT 类型	
};

//当前分组的怪物已经死亡(用于单机模式客户端通知服务器)
DECLARE_MSG(SGroupMonsterDie, SSceneMsg, SSceneMsg::EPRO_SCENE_GROUPMONSTERDIE)
struct SQGroupMonsterDie : public SGroupMonsterDie
{
};