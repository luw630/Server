//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   NetModule.h
//FileAuthor		:	luou
//FileCreateDate	:	15:09 2003-4-3
//FileDescription	:	客户端及服务器端内部网络对象的抽象基类

//模块说明：
//基于 KNetServer的一个模块，负责处理服务器的数据
//Initialize 作为初始化函数

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once 

//#ifndef _NETMODULE_H_
//#define _NETMODULE_H_

// 这里所创建的具体消息类型都是以SQ或者SA来开头的
// S是struct的标识
// Q&A本表示为同一种类型的请求与应答
// 现表示该消息的网络流向
// 例如：
// SQLoginMsg	在客户端生成，发往服务器，由服务器处理的消息
// SALoginMsg	在服务器生成，发往客户端，由客户端处理的消息

typedef unsigned __int64 QWORD;
typedef QWORD DNID;

#define DECLARE_MSG(name, BaseMessage, MessageType) struct name : public BaseMessage { name() {BaseMessage::_protocol = MessageType; } };
#define DECLARE_MSG_MAP(name, BaseMessage, MessageType) struct name : public BaseMessage { name() { BaseMessage::_protocol = MessageType; /*ProtocolIndex=32; ClientIndex=0xffff;*/ } enum {

#define END_MSG_MAP() }; BYTE _protocol; };
#define END_MSG_MAP_WITH_ROTOC() }; BYTE _protocol; DNID dnidClient; QWORD qwSerialNo; }; // R.O.T.O.C mean is respon operation time out check
#define END_MSG_MAP_WITH_ROTOC_GMLEVEL() }; BYTE _protocol; DNID dnidClient; QWORD qwSerialNo; WORD wGMLevel;}; // R.O.T.O.C mean is respon operation time out check


struct SMessage
{
    enum 
    {
        EPRO_MOVE_MESSAGE = 32,		// 移动消息
        EPRO_CHAT_MESSAGE,			// 对话消息
        EPRO_FIGHT_MESSAGE,			// 战斗消息
        EPRO_SCRIPT_MESSAGE,		// 脚本消息
        EPRO_REGION_MESSAGE,        // 场景消息
        EPRO_ITEM_MESSAGE,          // 道具消息
        EPRO_SYSTEM_MESSAGE,		// 系统管理消息
        EPRO_UPGRADE_MESSAGE,       // 升级消息（玩家属性变化）
        EPRO_TEAM_MESSAGE,          // 组队相关的消息  （包括聊天的消息） 
        EPRO_TONG_MESSAGE,          // 帮会相关的消息  （包括聊天的消息）
        EPRO_MENU_MESSAGE,          // 菜单选择操作
        EPRO_NAMECARD_BASE,         // 名片
        EPRO_RELATION_MESSAGE,      // 好友,黑名单等等
        EPRO_SPORT_MESSAGE,         // 运动、竞技
		EPRO_BUILDING_MESSAGE,		// 动态建筑相关消息
        EPRO_PLUGINGAME_MESSAGE,	// 插入式小游戏
		ERPO_MOUNTS_MESSAGE,		// 坐骑相关消息
		EPRO_Center_MESSAGE,		//中心管理服相关消息
		EPRO_TRANSFORMERS_MESSAGE,	// 变身消息
		EPRO_DYARENA_MESSAGE,	// 竞技场消息

        EPRO_COLLECT_MESSAGE = 0xD0,// 数据采集功能类消息

        EPRO_GAMEMNG_MESSAGE = 0xe0,// GM指令（现在用来做验证了）
        EPRO_GMM_MESSAGE,           // GM模块指令（增补）
        EPRO_GMIDCHICK_MSG,         // 处理GM身份验证消息
		EPRO_GMTOOL_MSG,         // GM工具相关

        // 底层消息段
        EPRO_REFRESH_MESSAGE = 0xf0,// 数据刷新消息
        EPRO_SERVER_CONTROL,        // 服务器控制消息
        EPRO_REBIND_MESSAGE,        // 连接重定向消息
        EPRO_DATABASE_MESSAGE,      // 数据库相关消息
        EPRO_ACCOUNT_MESSAGE,       // 帐号检测相关消息
        EPRO_ORB_MESSAGE,           // 跨区域服务器相关消息
        EPRO_DATATRANS_MESSAGE,     // 数据传送相关消息
        EPRO_DBOP_MESSAGE,          // 数据库操作用消息
        EPRO_POINTMODIFY_MESSAGE,   // 点数交易操作相关消息
		EPRO_MAIL_MESSAGE,			// 留言系统相关消息
		EPRO_PHONE_MESSAGE,			// 电话系统相关消息
	
        EPRO_UNION_MESSAGE,			// 结义相关消息
		ERPO_BIGUAN_MESSAGE,		// 闭关消息

		ERPO_SECTION_MESSAGE,		// 分段消息
		EPRO_GET_RANKLIST_FROM_DB,	// 从DB得到排行榜数据
		EPRO_SCENE_MESSAGE,	// 关卡消息
		
		EPRO_SYNTIME_MESSAGE = 0x300,		// 逻辑时间同步相关消息
		
		EPRO_DAILY_MESSAGE = 0x79,		//玩家日常活动相关消息
		EPRO_SHOP_MESSAGE,	//商城消息
		EPRO_VIP_MESSAGE,	//VIP消息
		EPRO_FIGHTPET_MESSAGE,	//玩家战斗宠物消息
		//////////////////////////三国消息
		EPRO_STORAGE_MESSAGE, ///<背包消息
		EPRO_SANGUOPLAYER_MESSAGE, ///三国玩家消息
    };
    unsigned __int8 _protocol;
};

//#endif
