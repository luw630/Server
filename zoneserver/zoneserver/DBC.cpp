#include "StdAfx.h"

#include "Networkmodule/refreshMsgs.h"
#include "Networkmodule/dbopMsgs.h"

extern int SendToLoginServer(SMessage *data, long size);

void SendTestInfo()
{
	SQGameServerRPCOPMsg msg;

	lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

	try
	{
            // 准备存储过程（减点）
        slm [OP_PREPARE_STOREDPROC] ( "dbo.testSP" ) 

		    // 设定调用参数
		    [OP_BEGIN_PARAMS]
		    (1) ( 12 )
		    (2) ( 21 )
		    (3) ( "asdsd" )
		    [OP_END_PARAMS]

		    // 调用存储过程
		    [OP_CALL_STOREDPROC]

            [OP_RPC_END];

        // 全都准备好后，再发送消息到登陆服务器！并且由登陆服务器再转到账号服务器
        msg.dstDatabase = 2;
        SendToLoginServer( &msg, sizeof( msg ) - slm.EndEdition() );
    }
    catch ( lite::Xcpt & )
	{
        // 只捕获可以判断的错误
	}
}
