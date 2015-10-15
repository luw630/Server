#pragma once

#include <winsock2.h>

#ifndef DIRECTNET_API
#define DIRECTNET_API __declspec(dllimport)
#endif

typedef UINT64 DNID;

#define INVALID_DNID ( (DNID)-1 )

typedef UINT16 DNPORT;

typedef INT32 DNRESULT;

#define DNR_FAILED(dnr) ( (INT32)dnr < 0 )

#define DNR_S_OK                    ( (INT32)0x00000000 )
#define DNR_S_PENDING               ( (INT32)0x00000001 )

#define DNR_E_FAIL                  ( (INT32)0x80000000 )
#define DNR_E_INVALID_PARAMETER     ( (INT32)0x80000001 )
#define DNR_E_OUT_OF_MEMORY         ( (INT32)0x80000002 )
#define DNR_E_NOT_INITIALIZED       ( (INT32)0x80000003 )
#define DNR_E_INVALID_PLAYER        ( (INT32)0x80000004 )

// desc: abstract interface for data security that is implemented externally
struct iDirectNetCryption {
    virtual UINT32 CRC32_compute(/* [in] */PVOID pvBuffer, DWORD dwLength) { return 0; }
    virtual void DES_encrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength) {}
    virtual void DES_decrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength) {}
};

// desc: abstract interface that is implemented by user application
struct iDirectNetServerEventHandler {
    virtual void OnCreatePlayer(DNID dnidPlayer, PVOID* ppvPlayerContext) = 0;
    virtual void OnReceivePacket(DNID dnidPlayer, PVOID pvPlayerContext,
        PVOID pvBuffer, DWORD dwLength, DWORD dwSequence) = 0;
    virtual void OnDeletePlayer(DNID dnidPlayer, PVOID pvPlayerContext) = 0;
};

// desc: the primary application programming interface
class DIRECTNET_API iDirectNetServer {
public:
    static iDirectNetServer* CreateInstance();

    virtual DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
        iDirectNetServerEventHandler* pDirectNetServerEventHandler, DWORD dwMaxNumPlayers) = 0;
    virtual DNRESULT Host(DNPORT dnPort) = 0;
    virtual DNRESULT SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength) = 0;
    virtual DNRESULT DeletePlayer(DNID dnidPlayer) = 0;
    virtual DNRESULT Close() = 0;

    void Release();

protected:
    virtual ~iDirectNetServer() {}
};

