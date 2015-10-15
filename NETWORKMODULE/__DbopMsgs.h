#pragma once

// #include "pub/LiteInterop.h"
#include "pub/rpcop.h"

//=============================================================================================
// һ��ͨ�ð�Ĳ����˺����ݿ�����Ϣ
DECLARE_MSG(_SDbopMsg, SMessage, SMessage::EPRO_DBOP_MESSAGE)

template < int buffersize >
struct RpcMsg : 
    public _SDbopMsg 
{
    // DNID dnidClient;
    WORD size;
    char data[ buffersize ];
    //static SDbopMsg * const CreateDbopMsg(LiteInterop::CLiteInterop &lpLiop, DWORD &dwSizeOut)
    //{
    //    if (lpLiop.GetValidSize() == 0)
    //        return NULL;

    //    SDbopMsg temp;
    //    dwSizeOut = sizeof(temp);
    //    return (SDbopMsg *) lpLiop.AddHeader(&temp, dwSizeOut);
    //}

    //DWORD GetSize()
    //{
    //    return ALIGN(sizeof(SDbopMsg), ALIGN_BYTE_4) + GetLiteInterop()->u32InteropCurSize;
    //}

    //LiteInterop::CLiteInterop *GetLiteInterop()
    //{
    //    return (LiteInterop::CLiteInterop *)( ((LPBYTE)this) + ALIGN(sizeof(SDbopMsg), ALIGN_BYTE_4) );
    //}
};

typedef RpcMsg<1> SDbopMsg;

//---------------------------------------------------------------------------------------------
