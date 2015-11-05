#pragma once

#include <winsock2.h>

#ifndef DIRECTNET_API
#define DIRECTNET_API __declspec(dllimport)
#endif

typedef UINT16 DNPORT;

typedef INT32 DNRESULT;

#define DNR_FAILED(dnr) ( (INT32)dnr < 0 )

#define DNR_S_OK                ( (INT32)0x00000000 )
#define DNR_S_PENDING           ( (INT32)0x00000001 )

#define DNR_E_FAIL              ( (INT32)0x80000000 )
#define DNR_E_INVALID_PARAMETER ( (INT32)0x80000001 )
#define DNR_E_OUT_OF_MEMORY     ( (INT32)0x80000002 )
#define DNR_E_NOT_INITIALIZED   ( (INT32)0x80000003 )
#define DNR_E_NOT_CONNECTED     ( (INT32)0x80000004 )
#define DNR_E_CONNECT_AGAIN     ( (INT32)0x80000005 )

// desc: abstract interface for data security that is implemented externally
struct iDirectNetCryption {
    virtual UINT32 CRC32_compute(/* [in] */PVOID pvBuffer, DWORD dwLength) { return 0; }
    virtual void DES_encrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength) {}
    virtual void DES_decrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength) {}
};

// desc: abstract interface that is implemented by user application
struct iDirectNetClientEventHandler {
    virtual void OnConnectComplete(DNRESULT dnr) = 0;
    virtual void OnReceivePacket(PVOID pvBuffer, DWORD dwLength, DWORD dwSequence) = 0;
    virtual void OnTerminateSession() = 0;
};

// desc: the primary application programming interface
class DIRECTNET_API iDirectNetClient {
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

