#pragma once

void g_CutClient(DNID dnidClient);
void g_CutClient(DNID dnidClient, int cutFlag);
int g_StoreMessage(DNID dnidClient, LPVOID pMsg, DWORD wSize);
DWORD g_GetClientCount();
BOOL g_GetAddrByDnid(DNID dnidClient, sockaddr_in *addr, size_t size);

int SendToLoginServer(struct SMessage *, long size);
int SendToORBServer(struct SORBMsg *data, long size);

LPIObject GetPlayerByName(LPCSTR szName);
LPIObject GetPlayerByGID(DWORD GID);

BOOL MoveSomebody(LPCSTR name, WORD regionid, WORD x, WORD y);

void TalkToDnid(DNID dnidClient, LPCSTR info,bool addPrefix,bool bPickUp,WORD wShowType);
void TalkToAll(LPCSTR info);

LPCSTR FormatString(LPCSTR szFormat, ...);
LPCSTR GetStringTime();

void TraceMemory(LPVOID, int, unsigned int, LPCSTR);
void DumpStack(LPCSTR);

WORD GetServerID();
DWORD GetGroupID();

struct SQGameServerRPCOPMsg;

struct LogNotify
{
    LogNotify( LPCSTR proc, BYTE dbcId = 100 );
    LogNotify& operator [] ( const lite::Variant &vt );
    void operator () ();

    const LPCSTR        proc;
    const BYTE          dbcId;
    int                 index;
    lite::Serializer    slm;

    static SQGameServerRPCOPMsg msg;
};

#define BEGIN_NOTIFY try { LogNotify
#define END_NOTIFY (); } catch ( lite::Xcpt& ) { rfalse( 2, 1, "END_NOTIFY catched exception" ); }

struct SimpleLogNotify
{
    SimpleLogNotify( DWORD tableId, BYTE dbcId = 2 );
    SimpleLogNotify& operator () ( int, QWORD );
    SimpleLogNotify& operator () ( int, int );
    SimpleLogNotify& operator () ( int, LPCSTR );
    void operator () ();

    const BYTE          dbcId;
    DWORD				tableId;
    lite::Serializer    slm;

    static SQGameServerRPCOPMsg msg;
};

#define BEGIN_SNOTIFY try { SimpleLogNotify
#define END_SNOTIFY (); } catch ( lite::Xcpt& ) { rfalse( 2, 1, "END_SNOTIFY catched exception" ); }
