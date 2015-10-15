#pragma once

// ================================================================================
#include "networkmodule\chatmsgs.h"
#include "networkmodule\datatransmsgs.h"
#include "networkmodule\relationmsgs.h"
// 检查好友是否在线，写在这里只是为了避免修改头文件导致所有服务器都需要重新编译
DECLARE_MSG( _SCheckFriendStateMsg, SDataTransMsg, SDataTransMsg::EPRO_TONGONE + 1 )
struct SDT_CheckFriendStateMsg : public _SCheckFriendStateMsg
{
    char srcName[11];       // 源玩家的名称，用于回传
    SRefreshFriendsMsg msg; // check 信息
};
// ================================================================================
