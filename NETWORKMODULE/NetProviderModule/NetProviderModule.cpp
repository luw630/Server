//#######################
//# xo checked at 09-24 #
//#######################
#include "NetProviderModule.h"

#include <crtdbg.h>

#define USE_CRC
#include "new_net/desdec.h"
#include "new_net/netserverex.cpp"

#define DOTHROW

extern BOOL rfalse(char lTraceType, char bUseEndl, LPCSTR szFormat, ...);

CNetProviderModule::CNetProviderModule(BOOL bUseVerify) :
    CNetServerEx(bUseVerify)
{

}

CNetProviderModule::~CNetProviderModule()
{

}

int CNetProviderModule::Destroy()
{
    Close();
    return 1;
}

