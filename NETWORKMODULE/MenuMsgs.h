#pragma once

#include "NetModule.h"

// 客户端菜单操作消息
//=============================================================================================
DECLARE_MSG(SMenuBaseMsg, SMessage, SMessage::EPRO_MENU_MESSAGE)
struct SQMenuMsg : public SMenuBaseMsg
{
    enum EMenuItem
    {
        EMI_EXCHANGE,
        EMI_CREATETEAM,
        EMI_JOINTEAM,
        EMI_INVITEJOINTEAM,
        EMI_WHISPER,
        EMI_JOINFACTION,
        EMI_OTHEREQUIPMENT,
		EMI_JOKINGWITHOTHER,
		EMI_WIFE_REMIT,				// 夫妻传送
        EMI_LAKELOG,				// 弹出江湖志面板
        EMI_BOSSINFO,				// BOSS 信息
		EMI_ASSOCIATION,			// 结义
        EMI_UNIONRECVEXP,			// 领取结义经验
        EMI_OTHEREQUIPMENTSID,		// 通过Sid查看玩家装备
        EMI_DOCTORADDBUFF,			// 医德BUFF
        EMI_XYBAPJIAN,				// 侠义宝鉴
        EMI_TASKINF,				// 任务
        EMI_ASK_ADDDOCTOR,          // 询问是否愿意附加医德BUFF
        EMI_AGREE_ADD_DOCTORBUFF,   // 同意附加医德BUFF
        EMI_REFUSE_ADD_DOCTORBUFF,  // 拒绝附加医德BUFF
        EMI_NEWS,                   // 新版介绍
        EMI_REQUESTDATA,            // 这里扩展一个用于向服务器请求数据的公用类型
        EMI_FLOWER,                 // 送花
        EMI_WULINRENMING,           // 武林任命
        EMI_WULINJIEPING,           // 武林解聘
        EMI_CLICKOTHER,             // 点击某人
        EMI_TRACKINGCANCEL,         // 取消追踪某人
        EMI_STANDINGSQUEST,         // 战斗统计请求相关
    };

    BYTE byMenuItem;    // 选择的菜单项
    DWORD dwDestGID;    // 点击目标的GID
};
//---------------------------------------------------------------------------------------------

struct SQClickPlayerMsg:
    public SQMenuMsg
{
    BYTE type;  // 0 表示第一次请求，1表示确认请求
};

struct SQJokingWitheOtherMsg :
	public SQMenuMsg
{
	WORD wItemIdx;
};

struct SQDoctorAddBuffMsg :
    public SQMenuMsg
{
    DWORD dwValue;  // 0x000000FF:DEF, 0x0000FF00:DAM, 0x00FF0000:POW, 0xFF000000:AGI, 任意组合
};

struct SQInviteJoinTeamMsg : public SQMenuMsg
{
	DWORD dwDestGID;		// 目标ID
};

struct SQPresentFlowerMsg :
    public SQMenuMsg
{
    char szDestName[ CONST_USERNAME ];
};