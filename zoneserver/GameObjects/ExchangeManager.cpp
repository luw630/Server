#include "StdAfx.h"
#include "exchangegoods.h"
#include "exchangemanager.h"

#include "pub/tObjectService_REF.h"

typedef dwt::tObjectService_REF<SExchangeVerifyInfo, SExchangeFullInfo> CExchangeManager;

CExchangeManager g_EM;

BOOL PushExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi)
{
    CExchangeManager::OBJECTPTR ObjPtr = g_EM.NewObject(vi);
    if (ObjPtr == NULL)
        return FALSE;

    *ObjPtr = fi;

    return true;
}

BOOL PopExchangeInfo(SExchangeVerifyInfo &vi, SExchangeFullInfo &fi)
{
    CExchangeManager::OBJECTPTR ObjPtr = g_EM.GetLocateObject(vi);
    if (ObjPtr == NULL)
        return FALSE;

    fi = *ObjPtr;

    return g_EM.DelObject(vi);
}
