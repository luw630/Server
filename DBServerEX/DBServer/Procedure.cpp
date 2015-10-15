#include "stdafx.h"
#include "netdbmoduleex.h"
#include "networkmodule/refreshmsgs.h"
#include "liteserializer/lite.h"
#include "pub/rpcop.h"

extern BOOL SendMessage( DNID, LPCVOID, size_t );
#define LogErrorMsg( str ) LogMsg( "debugError.txt", str )
const char *RPCERROR = "RPCERR.txt";
const static int HEADER_SIZE = offsetof( SAGameServerRPCOPMsg, SAGameServerRPCOPMsg::streamData );
__declspec( thread ) char retBuffer[ 0xF000 + HEADER_SIZE ]; // ���л��������ֻ֧��0xffff��С�Ļ�����������
__declspec( thread ) BYTE RPCLocalBuffer[ 0x8000 ]; // �����л�RPC���ݱ���ֵʱʹ��

namespace lite
{
    DeclareXcpt( PrepareFailure, 0x00000009, "׼���洢����ʧ��" )
    DeclareXcpt( ExecFailure,    0x0000000a, "ִ�д洢����ʧ��" )
}

int CNetDBModuleEx::RpcProcedure( DNID srcDnid, LPVOID rpcBuffer, DNID passDnid )
{

#define GetVariant() ( ( dwRunIndex ++ ), ls() )

    ProcedureHelper helper( *this );

    MYSQL_RES *RECORDSET = NULL;

    DWORD dwRunIndex = 0;
    size_t lvtErrAddr = -1;

    // �漰��ѭ�����������ݣ�
    size_t lvtTraverseLoopAddr = -1;    // ѭ�����
    DWORD loopRunIndexSegment = -1;     // ��������ִ������ ��� dwRunIndex
    int loopNumber = -1;                // ����ѭ������

    char errorBuffer[ 1024 ];
    char tmpErr[10] = { 0 };

    // �������ķ��ؿռ䳬��Ĭ�ϴ�Сʱ�����ᱻ���·��䣡
    char *ptr_retBuffer = NULL;
    LPCSTR procName = "null";

    lite::Serialreader ls;
    lite::Serializer retBox, errorBox;

    try
    {
        // ������һ�������������Ϊ��������ܲ����쳣�����ⲿ����Ҫʹ�õ�ls�����Ծ��������ˣ���
        new ( &ls ) lite::Serialreader( rpcBuffer );

        lite::Variant lvt = GetVariant();

        while ( lvt.dataType == lite::Variant::VT_RPC_OP )
        {
            switch ( lvt._rpcop )
            {
            case OP_BEGIN_PARAMS:
                {
                    while ( TRUE )
                    {
                        lite::Variant lvt = GetVariant();
                        if ( lvt.dataType == lite::Variant::VT_RPC_OP )
                        {
                            if ( lvt._rpcop == OP_END_PARAMS )
                                break;

                            // ������� VT_RPC_OP ��ôӦ��һ���ǽ�������
                            throw lite::InvalidConvert();
                        }
                        else
                        {
                            lite::Variant lvtParamValue = GetVariant();

                            if ( lvtParamValue.dataType == lite::Variant::VT_RPC_OP )
                                throw lite::InvalidConvert();

                            // ֱ�ӽ���������󶨵�������
                            helper.BindParam( ( int )lvt, lvtParamValue );
                        }
                    }
                }
                break;

            case OP_BOX_VARIANT:
                retBox.PushVariant( GetVariant() );
                break;

            case OP_BOX_PARAM:
                {
                    // ֱ�ӽ��洢���̵Ĳ������������������ѹ�����
                    lite::Variant lvt = GetVariant();
                    retBox.PushVariant( helper.GetParam( ( int )lvt ) );
                }
                break;

            case OP_BOX_FIELD:
                {
                    // �����ǽ����ý�����е�ĳ�������ݣ�ѹ�����
                    lite::Variant lvt = GetVariant();
                    retBox.PushVariant( helper.GetField( ( int )lvt ) );
                }
                break;

            case OP_PREPARE_STOREDPROC:
                {
                    lite::Variant lvtCallName = GetVariant();
                    LPCSTR proc = lvtCallName;
                    procName = proc;

                    // ȥ��ǰ���script.
                    if ( lvtCallName.dataSize > 7 && memcmp( procName, "script.", 7 ) == 0 )
                        procName += 7;

                    if ( helper.Prepare( procName ) != 0 ) 
                        throw lite::PrepareFailure();
                }
                break;

            case OP_CALL_STOREDPROC:
                if ( helper.Execute() != 0 )
                    throw lite::ExecFailure();
                break;

            case OP_INIT_RETBOX:
                {
                    int allocSize = GetVariant();
                    if ( ptr_retBuffer != NULL )
                        throw lite::InvalidBuffer();

                    if ( allocSize >= sizeof( retBuffer ) - HEADER_SIZE )
                        ptr_retBuffer = new char[ allocSize + HEADER_SIZE ];
                    else
                        ptr_retBuffer = retBuffer;

                    retBox.Assign( ptr_retBuffer + HEADER_SIZE, allocSize );
                }
                break;

            case OP_RETURN_BOX:
                {
                    SAGameServerRPCOPMsg *pMsg = new ( ptr_retBuffer ) SAGameServerRPCOPMsg();
                    pMsg->dnidClient = passDnid;
                    retBox.EndEdition();
                    SendMessage( srcDnid, ptr_retBuffer, HEADER_SIZE + retBox.maxSize() );
                }
                break;

            case OP_TRAVERSE_RECORDSET:
                {
                    loopNumber = GetVariant();

                    // ���������ѭ����һ��ʼ���Ѿ�û�����ݣ���ֱ�Ӻ��Ե�ѭ������
                    if ( helper.num_rows == 0 || helper.num_fields == 0 || helper.FetchResult() != 0 )
                    {
                        while ( TRUE )
                        {
                            lite::Variant lvt = GetVariant();

                            if ( ( lvt.dataType == lite::Variant::VT_RPC_OP ) && 
                                ( lvt._rpcop == OP_CHECK_TRAVERSE_LOOP ) )
                                break;
                        }
                    }
                    else
                    {
                        // ��¼��ǰ��ѭ��������ַ���Ͳ�ѯ��ַ
                        lvtTraverseLoopAddr = ls.curSize();
                        loopRunIndexSegment = dwRunIndex;
                    }
                }
                break;

            case OP_CHECK_TRAVERSE_LOOP:
                {
                    // ѭ����ȡ���ݼ�����һ�����ݣ�ֱ��������ѭ����ָ���Ĵ���
                    if ( ( -- loopNumber <= 0 ) || helper.FetchResult() != 0 )
                    {
                        // helper.FreeResult();
                        break;
                    }

                    // �ָ�ѭ��������ַ���Ͳ�ѯ��ַ
                    ls.curSize() = static_cast< WORD >( lvtTraverseLoopAddr );
                    dwRunIndex = loopRunIndexSegment;
                }
                break;

            case OP_BEGIN_ERRHANDLE:
                {
                    // ��¼��ǰ��ERRHANDLE��ַ��
                    lvtErrAddr = ls.curSize();

                    while ( TRUE )
                    {
                        lite::Variant lvt = GetVariant();

                        if ( ( lvt.dataType == lite::Variant::VT_RPC_OP ) && ( lvt._rpcop == OP_END_ERRHANDLE ) )
                            break;
                    }
                }
                break;

			case OP_TABLE_DATA:
				{
					int retNumber = GetVariant(); // ֻ��ȡָ�������ļ�¼��
					int tableType = GetVariant(); // �������ݵı�����ʽ

					retNumber = min( retNumber, ( int )helper.num_rows );
					if ( helper.num_rows == -1 || helper.num_fields == 0xffffffff || retNumber == 0 || helper.num_fields == 0 )
                    {
                        lite::Variant lvt;
                        lvt.dataType = lite::Variant::VT_NULL;
                        retBox.PushVariant( lvt ); // ������ݼ�Ϊ�գ��򷵻�null
                    }
                    else
                    {
                        // ���л��������ֻ֧��0xffff��С�Ļ�����������
					    lite::Serializer ls( RPCLocalBuffer, sizeof( RPCLocalBuffer ) ); 

				        // �ȴ����ֶ������ٴ������������������ݿ�������fetchʧ�ܣ����²�׼ȷ��
					    ls( helper.num_fields )( ( DWORD )retNumber );

                        if ( helper.FetchResult() == 0  )
                        {
                            for ( int i = 0; i < retNumber; i++ ) 
                            {
                                for ( DWORD j = 0; j < helper.num_fields; j++ )
                                    ls( helper.GetField( j ) );

                                if ( helper.FetchResult() != 0  )
                                    break;
                            }
                        }

                        // ������ǰ����������ݣ�������ߵ�2�����л�������Ҫ�������Ϊlua_table
                        if ( tableType != '@' ) retBox[ OP_TABLE_DATA ] ( tableType ); 

                        // ѹ��������л����ݡ�����
                        retBox( RPCLocalBuffer, ( sizeof( RPCLocalBuffer ) - ls.EndEdition() ) ); 
                    }
                }
                break;

            case OP_RPC_END:
                goto __finally_release;
            }

            lvt = GetVariant();
        }

        throw lite::CurrentIsEnd();
    }
    catch ( lite::Xcpt &e )
    {
        std::string err = std::string( "Proc[" ) + procName + "] opNo.[" + itoa( dwRunIndex, tmpErr, 10 ) + "] LiteError[" + e.GetErrInfo() + "]" ;
        LogError( RPCERROR, err.c_str() );
    }

    if ( lvtErrAddr == -1 )
        goto __finally_release;

    // Error-handle
    errorBox.Assign( errorBuffer + HEADER_SIZE, sizeof( errorBuffer ) - HEADER_SIZE );

    // �ش�dnidClient�����ڱ�ʶ�ǵ�½������ֱ�����󣬻������������������
    ( new ( errorBuffer ) SAGameServerRPCOPMsg() )->dnidClient = passDnid;

    ls.curSize() = (WORD)lvtErrAddr;

    try
    {
        while ( TRUE )
        {
            lite::Variant lvt = GetVariant();

            if ( ( lvt.dataType == lite::Variant::VT_RPC_OP ) && ( lvt._rpcop == OP_END_ERRHANDLE ) )
                break;

            errorBox.PushVariant( lvt );
        }
    }
    catch ( lite::Xcpt &e )
    {
        std::string err = std::string( "RPC_OP ehandle [" ) + procName + "] opNo.[" + itoa( dwRunIndex, tmpErr, 10) + "] LiteError handle [" + e.GetErrInfo() + "]" ;
        LogError( RPCERROR, err.c_str()  );
    }

    errorBox.EndEdition();

    SendMessage( srcDnid, errorBuffer, HEADER_SIZE + errorBox.maxSize() );

__finally_release:

    // ��Ҫ�ͷŴ�����������ʱ�ռ䣡
    if ( ptr_retBuffer != retBuffer )
        delete ptr_retBuffer;

    return true;
}
