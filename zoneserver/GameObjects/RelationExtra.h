#pragma once

// ================================================================================
#include "networkmodule\chatmsgs.h"
#include "networkmodule\datatransmsgs.h"
#include "networkmodule\relationmsgs.h"
// �������Ƿ����ߣ�д������ֻ��Ϊ�˱����޸�ͷ�ļ��������з���������Ҫ���±���
DECLARE_MSG( _SCheckFriendStateMsg, SDataTransMsg, SDataTransMsg::EPRO_TONGONE + 1 )
struct SDT_CheckFriendStateMsg : public _SCheckFriendStateMsg
{
    char srcName[11];       // Դ��ҵ����ƣ����ڻش�
    SRefreshFriendsMsg msg; // check ��Ϣ
};
// ================================================================================
