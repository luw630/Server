//#######################
//# xo checked at 09-24 #
//#######################
#include "NetConsumerModule.h"
#include <crtdbg.h>

extern BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

#include "new_net/netclientex.cpp"

#define DOTHROW

CNetConsumerModule::CNetConsumerModule(BOOL bUseVerify) :
    CNetClientEx(bUseVerify)
{
}

CNetConsumerModule::~CNetConsumerModule()
{
}

int CNetConsumerModule::Destroy()
{
    Disconnect();
    return 1;
}
