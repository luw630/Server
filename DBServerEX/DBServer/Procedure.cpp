#include "stdafx.h"
#include "netdbmoduleex.h"
#include "networkmodule/refreshmsgs.h"
#include "liteserializer/lite.h"
#include "pub/rpcop.h"

extern BOOL SendMessage( DNID, LPCVOID, size_t );
#define LogErrorMsg( str ) LogMsg( "debugError.txt", str )
const char *RPCERROR = "RPCERR.txt";
const static int HEADER_SIZE = offsetof( SAGameServerRPCOPMsg, SAGameServerRPCOPMsg::streamData );
__declspec( thread ) char retBuffer[ 0xF000 + HEADER_SIZE ]; // 序列化器，最大只支持0xffff大小的缓冲区。。。
__declspec( thread ) BYTE RPCLocalBuffer[ 0x8000 ]; // 作序列化RPC数据表返回值时使用

namespace lite
{
    DeclareXcpt( PrepareFailure, 0x00000009, "准备存储过程失败" )
    DeclareXcpt( ExecFailure,    0x0000000a, "执行存储过程失败" )
}

int CNetDBModuleEx::RpcProcedure( DNID srcDnid, LPVOID rpcBuffer, DNID passDnid )
{

#define GetVariant() ( ( dwRunIndex ++ ), ls() )

    ProcedureHelper helper( *this );

    MYSQL_RES *RECORDSET = NULL;

    DWORD dwRunIndex = 0;
    size_t lvtErrAddr = -1;

    // 涉及到循环遍历的数据！
    size_t lvtTraverseLoopAddr = -1;    // 循环起点
    DWORD loopRunIndexSegment = -1;     // 有限起点的执行索引 配合 dwRunIndex
    int loopNumber = -1;                // 有限循环次数

    char errorBuffer[ 1024 ];
    char tmpErr[10] = { 0 };

    // 如果分配的返回空间超过默认大小时，将会被重新分配！
    char *ptr_retBuffer = NULL;
    LPCSTR procName = "null";

    lite::Serialreader ls;
    lite::Serializer retBox, errorBox;

    try
    {
        // 这里是一个特殊情况，因为本身构造可能产生异常，而外部又需要使用到ls，所以就这样干了：）
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

                            // 如果遇到 VT_RPC_OP 那么应该一定是结束符！
                            throw lite::InvalidConvert();
                        }
                        else
                        {
                            lite::Variant lvtParamValue = GetVariant();

                            if ( lvtParamValue.dataType == lite::Variant::VT_RPC_OP )
                                throw lite::InvalidConvert();

                            // 直接将变量结果绑定到参数上
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
                    // 直接将存储过程的参数（包括输入输出）压入队列
                    lite::Variant lvt = GetVariant();
                    retBox.PushVariant( helper.GetParam( ( int )lvt ) );
                }
                break;

            case OP_BOX_FIELD:
                {
                    // 这里是将调用结果集中的某个域数据，压入队列
                    lite::Variant lvt = GetVariant();
                    retBox.PushVariant( helper.GetField( ( int )lvt ) );
                }
                break;

            case OP_PREPARE_STOREDPROC:
                {
                    lite::Variant lvtCallName = GetVariant();
                    LPCSTR proc = lvtCallName;
                    procName = proc;

                    // 去掉前面的script.
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

                    // 如果在启动循环的一开始就已经没有数据，则直接忽略到循环结束
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
                        // 记录当前的循环遍历地址！和查询地址
                        lvtTraverseLoopAddr = ls.curSize();
                        loopRunIndexSegment = dwRunIndex;
                    }
                }
                break;

            case OP_CHECK_TRAVERSE_LOOP:
                {
                    // 循环获取数据集的下一条数据，直到结束或循环到指定的次数
                    if ( ( -- loopNumber <= 0 ) || helper.FetchResult() != 0 )
                    {
                        // helper.FreeResult();
                        break;
                    }

                    // 恢复循环遍历地址！和查询地址
                    ls.curSize() = static_cast< WORD >( lvtTraverseLoopAddr );
                    dwRunIndex = loopRunIndexSegment;
                }
                break;

            case OP_BEGIN_ERRHANDLE:
                {
                    // 记录当前的ERRHANDLE地址！
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
					int retNumber = GetVariant(); // 只获取指定行数的记录。
					int tableType = GetVariant(); // 返回数据的表现形式

					retNumber = min( retNumber, ( int )helper.num_rows );
					if ( helper.num_rows == -1 || helper.num_fields == 0xffffffff || retNumber == 0 || helper.num_fields == 0 )
                    {
                        lite::Variant lvt;
                        lvt.dataType = lite::Variant::VT_NULL;
                        retBox.PushVariant( lvt ); // 如果数据集为空，则返回null
                    }
                    else
                    {
                        // 序列化器，最大只支持0xffff大小的缓冲区。。。
					    lite::Serializer ls( RPCLocalBuffer, sizeof( RPCLocalBuffer ) ); 

				        // 先存入字段数，再存入行数（行数的数据可能由于fetch失败，导致不准确）
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

                        // 在数据前插入控制数据，表明后边的2次序列化数据需要反向解码为lua_table
                        if ( tableType != '@' ) retBox[ OP_TABLE_DATA ] ( tableType ); 

                        // 压入二次序列化数据。。。
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

    // 回传dnidClient，用于标识是登陆服务器直接请求，还是区域服务器的请求
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

    // 需要释放创建出来的临时空间！
    if ( ptr_retBuffer != retBuffer )
        delete ptr_retBuffer;

    return true;
}
