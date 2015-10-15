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
#define DNR_E_NOT_CONNECTED         ( (INT32)0x80000005 )
#define DNR_E_CONNECT_AGAIN         ( (INT32)0x80000006 )

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
#ifdef  NOTMAKE_DLL
class iDirectNetServer {
#else
class DIRECTNET_API iDirectNetServer {
#endif
public:
    static iDirectNetServer* CreateInstance();

    virtual DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
        iDirectNetServerEventHandler* pDirectNetServerEventHandler, DWORD dwMaxNumPlayers) = 0;
    virtual DNRESULT Host(DNPORT dnPort) = 0;
    virtual DNRESULT SendTo(DNID dnidPlayer, PVOID pvBuffer, DWORD dwLength) = 0;
    virtual DNRESULT DeletePlayer(DNID dnidPlayer) = 0;
    virtual DNRESULT Close() = 0;

    struct CONNECTIONINFO {
        UINT32 addr;
        UINT16 port;
    };

    virtual DNRESULT GetPlayerConnectionInfo(DNID dnidPlayer, CONNECTIONINFO* pConnectionInfo) = 0;

    void Release();

protected:
    virtual ~iDirectNetServer() {}
};

// desc: abstract interface that is implemented by user application
struct iDirectNetClientEventHandler {
    virtual void OnConnectComplete(DNRESULT dnr) = 0;
    virtual void OnReceivePacket(PVOID pvBuffer, DWORD dwLength, DWORD dwSequence) = 0;
    virtual void OnTerminateSession() = 0;
};

// desc: the primary application programming interface
#ifdef  NOTMAKE_DLL
class iDirectNetClient {
#else
class DIRECTNET_API iDirectNetClient {
#endif
public:
    static iDirectNetClient* CreateInstance();

    virtual DNRESULT Initialize(iDirectNetCryption* pDirectNetCryption,
        iDirectNetClientEventHandler* pDirectNetClientEventHandler) = 0;
    virtual DNRESULT Connect(LPCSTR szServerIpAddress, DNPORT dnPort) = 0;
    virtual DNRESULT Send(PVOID pvBuffer, DWORD dwLength) = 0;
    virtual DNRESULT Disconnect() = 0;
    virtual DNRESULT Close() = 0;

    void Release();

protected:
    virtual ~iDirectNetClient() {}
};

