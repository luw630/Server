#include "stdafx.h"
#include "rpcop.h"
#include "traceinfo.h"
#include "dbserver/querylib/queryclass.h"
#include "networkmodule/refreshmsgs.h"
#include "liteserializer/lite.h"

extern BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize );

#define LogErrorMsg( str ) TraceInfoDirectly( "DBRpcError.txt", str )

#undef Fields

void RpcProcedure( Query &query, DNID srcDnid, LPVOID rpcBuffer, DNID passDnid )
{
    #define COMMAND query.m_pCommand
    #define RECORDSET query.m_pRecordset
    #define GetVariant() ( ( dwRunIndex ++ ), ls() )

    DWORD dwRunIndex = 0;
    size_t lvtErrAddr = -1;

    // 涉及到循环遍历的数据！
    size_t lvtTraverseLoopAddr = -1;    // 循环起点
    DWORD loopRunIndexSegment = -1;     // 有限起点的执行索引 配合 dwRunIndex
    int loopNumber = -1;                // 有限循环次数

    const static HEADER_SIZE = offsetof( SAGameServerRPCOPMsg, SAGameServerRPCOPMsg::streamData );

    char retBuffer[ 4096 + HEADER_SIZE ];
    char errorBuffer[ 1024 ];

    // 如果分配的返回空间超过默认大小时，将会被重新分配！
    char *ptr_retBuffer = NULL;

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
                        _ParameterPtr lpParam = NULL;

                        if ( lvt.dataType == lite::Variant::VT_STRING )
                        {
                            lpParam = COMMAND->Parameters->GetItem( _bstr_t( lvt._string ) );
                        }
                        else if ( lvt.dataType == lite::Variant::VT_RPC_OP )
                        {
                            if ( lvt._rpcop == OP_END_PARAMS )
                                break;

                            // 如果遇到 VT_RPC_OP 那么应该一定是结束符！
                            throw lite::InvalidConvert();
                        }
                        else
                        {
                            ULONG dwTemp = 0;
                            _variant_t vPtr;

                            IEnumVARIANTPtr vtEnum = COMMAND->Parameters->_NewEnum();
                            vtEnum->Reset();
                            vtEnum->Skip( lvt );
                            vtEnum->Next( 1, &vPtr, &dwTemp );

                            lpParam = vPtr;
                        }

                        if ( lpParam == NULL )
                            throw lite::NullPointer();

                        lite::Variant lvtParamValue = GetVariant();

                        if ( lvtParamValue.dataType == lite::Variant::VT_RPC_OP )
                            throw lite::InvalidConvert();

                        lpParam->Value = lvtParamValue;
                    }
                }
                break;

            case OP_BOX_VARIANT:
                retBox.PushVariant( GetVariant() );
                break;

            case OP_BOX_PARAM:
                {
                    lite::Variant lvt = GetVariant();

                    _ParameterPtr lpParam = NULL;

                    if ( lvt.dataType == lite::Variant::VT_STRING )
                    {
                        lpParam = COMMAND->Parameters->GetItem( _bstr_t( lvt._string ) );
                    }
                    else
                    {
                        ULONG dwTemp = 0;
                        _variant_t vPtr;

                        IEnumVARIANTPtr vtEnum = COMMAND->Parameters->_NewEnum();
                        vtEnum->Reset();
                        vtEnum->Skip( lvt );
                        vtEnum->Next( 1, &vPtr, &dwTemp );

                        lpParam = vPtr;
                    }

                    if ( lpParam == NULL )
                        throw lite::NullPointer();

                    retBox.PushVariant( lite::Variant( lpParam->Value ) );
                }
                break;

            case OP_BOX_FIELD:
                {
                    lite::Variant lvt = GetVariant();
                    FieldPtr lpField = NULL;

                    if ( lvt.dataType == lite::Variant::VT_STRING )
                        lpField = RECORDSET->Fields->GetItem( _bstr_t( lvt._string ) );
                    else
                        lpField = RECORDSET->Fields->GetItem( ( long )lvt );

                    if ( lpField == NULL )
                        throw lite::NullPointer();

                    retBox.PushVariant( lite::Variant( lpField->Value ) );
                }
                break;

            case OP_PREPARE_STOREDPROC:
                {
                    lite::Variant lvtCallName = GetVariant();
                    COMMAND->CommandText = lvtCallName;
                    COMMAND->CommandType = adCmdStoredProc;
                    COMMAND->Parameters->Refresh();
                }
                break;

            case OP_CALL_STOREDPROC:
                RECORDSET = COMMAND->Execute( NULL, NULL, adCmdStoredProc );
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
                    if (  RECORDSET == NULL || ( !RECORDSET->State ) || RECORDSET->adoEOF)
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
                    if ( ( RECORDSET != NULL ) && ( RECORDSET->adoEOF || ( ( -- loopNumber ) == 0 ) ) )
                    {
                        // 循环结束
                        RECORDSET->Close();
                        break;
                    }

                    RECORDSET->MoveNext();

                    if ( RECORDSET->adoEOF )
                    {
                        // 循环结束
                        RECORDSET->Close();
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

            case OP_RPC_END:
                goto __finally_release;
            }

            lvt = GetVariant();
        }

        throw lite::CurrentIsEnd();
    }
    catch ( _com_error &e )
    {
        _variant_t nLine = dwRunIndex;
        nLine.ChangeType( VT_BSTR );
        _bstr_t info = e.Description();
        if ( info.length() == 0 )
        {
            _variant_t hErr = e.Error();
            hErr.ChangeType( VT_BSTR );
            info = hErr.bstrVal;
        }

        LogErrorMsg( "RPC_OP opNo.[" + (_bstr_t)nLine + "] ComError[" + info + "]" );

        try
        {
            COMMAND->CommandText = "null_sp";
            COMMAND->CommandType = adCmdStoredProc;
            COMMAND->Parameters->Refresh();
        }
        catch ( _com_error &e )
        {
            _variant_t nLine = dwRunIndex;
            nLine.ChangeType( VT_BSTR );
            _bstr_t info = e.Description();
            if ( info.length() == 0 )
            {
                _variant_t hErr = e.Error();
                hErr.ChangeType( VT_BSTR );
                info = hErr.bstrVal;
            }

            LogErrorMsg( "RPC_OP com_inner opNo.[" + (_bstr_t)nLine + "] ComError[" + info + "]" );
        }
    }
    catch ( lite::Xcpt &e )
    {
        _variant_t nLine = dwRunIndex;
        nLine.ChangeType( VT_BSTR );
        LogErrorMsg( "RPC_OP opNo.[" + (_bstr_t)nLine + "] LiteError[" + e.GetErrInfo() + "]" );
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
        _variant_t nLine = dwRunIndex;
        nLine.ChangeType( VT_BSTR );
        LogErrorMsg( "RPC_OP opNo.[" + (_bstr_t)nLine + "] LiteError handle [" + e.GetErrInfo() + "]" );
    }

    errorBox.EndEdition();

    SendMessage( srcDnid, errorBuffer, HEADER_SIZE + errorBox.maxSize() );

__finally_release:

    // 需要释放创建出来的临时空间！
    if ( ptr_retBuffer != retBuffer )
        delete ptr_retBuffer;

    if ( ( RECORDSET != NULL ) && ( RECORDSET->State ) )
        RECORDSET->Close();
}
