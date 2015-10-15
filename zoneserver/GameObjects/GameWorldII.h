#pragma    once
//#include "objectservice\Include\ObjectManager.h"
#include "playermanager.h"
#include "Peer.h"

class CGameWorldII : private CPlayerManager ,CPeer
{
public:
    //using CPlayerManager::Initilaize;
    
    using CPlayerManager::Run;
    using CPlayerManager::GetPlayerArray;
    using CPlayerManager::Destroy;
    using CPeer::Run;
    using CPeer::Initialize;
    using CPeer::UnInitialize;
private :
    BOOL DispatchSysMessage(int ClientIndex, struct SSysBaseMsg *pMsg); // �������Ϣ�ɷ�����
public:
    void Initilaize();
    void Run();
    void Destroy();
public:
    CGameWorldII();
    ~CGameWorldII();
};
extern CGameWorldII *&GetGW();



