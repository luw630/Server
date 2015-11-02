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
            // ׼���洢���̣����㣩
        slm [OP_PREPARE_STOREDPROC] ( "dbo.testSP" ) 

		    // �趨���ò���
		    [OP_BEGIN_PARAMS]
		    (1) ( 12 )
		    (2) ( 21 )
		    (3) ( "asdsd" )
		    [OP_END_PARAMS]

		    // ���ô洢����
		    [OP_CALL_STOREDPROC]

            [OP_RPC_END];

        // ȫ��׼���ú��ٷ�����Ϣ����½�������������ɵ�½��������ת���˺ŷ�����
        msg.dstDatabase = 2;
        SendToLoginServer( &msg, sizeof( msg ) - slm.EndEdition() );
    }
    catch ( lite::Xcpt & )
	{
        // ֻ��������жϵĴ���
	}
}
