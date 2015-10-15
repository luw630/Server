#pragma once

#include "NetModule.h"
#include "itemtypedef.h"
#include "../pub/ConstValue.h"

#define GM_MODE 1       // 使用GM模式，便于测试

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// GM相关消息类
//=============================================================================================
DECLARE_MSG_MAP(SGameMngMsg, SMessage, SMessage::EPRO_GAMEMNG_MESSAGE)
END_MSG_MAP()
//---------------------------------------------------------------------------------------------



//=============================================================================================

// 唯一的一个GM消息？！……
struct SQGameMngMsg	:
    public SGameMngMsg
{
    enum E_GM_LIST
    {
        GM_MOVETO,
        GM_SETMONEY,
        GM_SETHPMPSP,
        GM_ADDGOODS,
        GM_SETFIGHT,
		GM_SETFLY,
		GM_SETPROTECT,
        GM_TEST,
        GM_OTHERMOVETO,
        GM_ONLINECHECK,
        GM_DELMEMBER,
        GM_RENAME,
        GM_CHANGEFIGHT,
        GM_SETMUTATE,           // 设置变身
        GM_SETSCAPEGOAT,        // 设置替身
        GM_SETICON,
        GM_UPLOAD,
        GM_SRESET,
        GM_ADD_GMC,             // 增加套装替身杀死怪物的计数
		GM_RLOADMC,				// 重新读入点卡换侠义币信息
		GM_RECOVERPLAYER,
		GM_MOVEBUILDTO,			//
		GM_MOVEBUILDIDTO,		// 按建筑UID移动
        GM_LOADGMCHECKURL,      // 加载GM check URL
        GM_ADDGOODSEX,          // 可以A有等级的装备
		GM_ADDEXP,				// 增加经验
		GM_ADDTELERGY,			// 增加心法熟练度
		GM_ADDSKILLPROF,		// 增加武功熟练度
		GM_ADDFLYPROF,			// 增加轻功熟练度
		GM_ADDPROTECTPROF,		// 增加护体熟练度
		GM_ADDMOUNTS,			// 加坐骑
		GM_ADDPETS,				// 加非战斗宠物
		GM_ADDMOUNT_PROPERTY,	// 加坐骑属性
		GM_ADDBUFF,				// 为自己添加一个BUFF
		GM_OPENVENA,			// 打开穴道
		GM_SET_PLAYER_PROPERTY,	// 设置角色属性
		GM_UPDATETELERGY,		// 升级某个心法到某个等级（控制数据：索引+等级）
		GM_SendMsg,
        GM_MAX,
		
    };

	DWORD	dwGlobalID;			// 本次运行全局唯一标识符
    WORD    wGMCmd;             // GM指令
    int     iNum[5];            // 控制数据

	char    m_Message[256];
	char	szUserpass[CONST_USERPASS];		// 在删除帮派（成员）时（或其他相关操作）要求输入密码
};

struct SQGameMngStrMsg : public SQGameMngMsg
{
    char szName[CONST_USERNAME];         // 与角色名字大小相同
};

struct SQGameMngStr2Msg : public SQGameMngStrMsg
{
    char szName2[CONST_USERNAME];        // 与角色名字大小相同
};

struct SQGMUploadFileMsg : public SQGameMngMsg
{
    DWORD dwVerinfo;        // 校验数据
    DWORD dwOffset;         // 偏移量
    WORD wTransDataSize;    // 当前数据块大小
    BYTE byTransFlag;       // 传送标记
    BYTE byDestFilenameLen; // 文件名长度
    BYTE pDataEntry[1];     // 数据入口
};

//---------------------------------------------------------------------------------------------
