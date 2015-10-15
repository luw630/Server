#include "stdafx.h"
#include "GameWorldII.h"
BOOL CGameWorldII::DispatchSysMessage(int ClientIndex, struct SSysBaseMsg *pMsg)
{
    return FALSE;
}

void CGameWorldII::Initilaize()
{
    CPlayerManager::Initilaize();
    CPeer::Initialize();
}

void CGameWorldII::Run()
{
    CPlayerManager::Run();
    CPeer::Run();
}

void CGameWorldII::Destroy()
{
    CPlayerManager::Destroy();
    CPeer::UnInitialize();

}
CGameWorldII::CGameWorldII()
{

}

CGameWorldII::~CGameWorldII()
{

}
CGameWorldII *&GetGW()
{
    static CGameWorldII *s_pGameWorld;
    return s_pGameWorld;
}
