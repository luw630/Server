#pragma once
#include "pub\ConstValue.h"
///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

enum OPR_RESULT
{
    OPR_FAIL,
    OPR_SUCCESS
};

// 操作请求方式
struct SQTeamOperation
{
    bool useBroadcast;              // 是否进行全服务器广播，否则回传
    DWORD dwSrcStaticID;            // 发起操作的源对象StaticID(回传)
    DWORD dwReserved;               // 预留的StaticID
};

// 操作返回结果
struct SATeamOperation
{
    struct SOpPlayer
    {
        // 索引关键字
        DWORD   dwStaticID;

        // 定位关键字
        WORD    wServerID;          // 该玩家区域服务器ID
        DNID    dnidClient;         // 基本连接编号

        // 校验关键字
        DWORD   dwClientGID;        // 该玩家本次登录的全局关键字

        // 基本数据
		char	szName[CONST_USERNAME];         // 该玩家的名字[自己做末尾截断]
        bool    isLeader;           // 队内称号，队长或队员
        WORD    wCurRegionID;       // 所在场景的ID
    };

    DWORD dwSrcStaticID;            // 发起操作的源对象StaticID(回传)
    DWORD dwSrcGlobalID;            // 发起操作的源对象GlobalID(回传)

    DWORD dwTeamID;                 // 当前队伍的ID

    bool useBroadcast;              // 是否进行全服务器广播，否则回传
    BYTE byMemberNumber;            // 返回的成员数量
    SOpPlayer aMembers[8];          // 返回的成员数据

    SOpPlayer* GetMember(int n)
    {
        if (byMemberNumber > 8) return NULL;
        if (n > byMemberNumber-1) return NULL;
        return &aMembers[n];
    }
};

