#include "stdafx.h"
#include "Producer.h"
#include "Consumer.h"


#include "Item.h"
#include "Area.h"
#include "Monster.h"
#include "Npc.h"
#include "Player.h"
#include "Region.h"

#include "networkmodule\netprovidermodule\NetProviderModule.h"
 
CProducer::CProducer()
{

}

CProducer::~CProducer()
{
    
}


bool CProducer::Initialize(char * PortName)
{
    for(int iLooper = 0;iLooper<MAX_CONSUMER_AFFORD;iLooper++)
    {
        m_ConsumerArray[iLooper] = NULL;
    }
    m_ConsumerCount = 0;
    m_pNetProviderModule = new CNetProviderModule();
    if(m_pNetProviderModule)
    {
        return m_pNetProviderModule->Initialize(PortName);
    }
    else
        return false;
}

void CProducer::UnInitialize()
{
    if(m_pNetProviderModule)
        delete m_pNetProviderModule;
}

void CProducer::Run()
{
    //m_pNetProviderModule->OnDispatch()

    CObjectManager::Run();
    m_pNetProviderModule->Execution();

}


int CProducer::AddObject(LPIObject pChild)
{
    return 0 ;
}

int CProducer::DelObject(LPIObject pChild)
{
    return 0;
}


void CProducer::OnRun()
{
    //CObjectManager::Run();
    //m_pNetProviderModule->Execution();
}
void CProducer::SetIP(const char * IP)
{
    if(!IsBadReadPtr(IP,24))
    {
        strcpy(m_szIP,IP);
    }
}

void CProducer::SetPort(const char * Port)
{
    if(!IsBadReadPtr(Port,24))
    {
        strcpy(m_szPort,Port);
    }
}
int CProducer::OnCreate(_W64 long pParameter)
{
    return 0;
}


void CProducer::OnClose()
{

}

IObject * CProducer::CreateObject(const __int32 IID)
{
    //if(IID == IID_PRODUCEROBJECT)
    //{
    //    return new CProducer;
    //}
    if(IID == IID_AREA)
        return new CArea;
    else if(IID == IID_ITEM)
        return new CItem;
    else if(IID == IID_MONSTER)
        return new CMonster;
    else if(IID == IID_NPC)
        return new CNpc;
    else if(IID == IID_PLAYER)
        return new CPlayer;
    else if(IID == IID_REGION)
        return new CRegion;
    else 
        return NULL;
}

void CProducer::AddConsumer(LPIObject pConsumer,WORD wClientIndex)
{
    m_ConsumerArray[wClientIndex] = pConsumer;    
    m_ConsumerCount ++;
    //for (int iLooper = 0 ; iLooper < MAX_CONSUMER_AFFORD ; iLooper ++)
    //{
    //    if(m_ConsumerArray[iLooper]!=NULL)
    //    {
    //        m_ConsumerArray[iLooper] = pConsumer;
    //        return;
    //    }
    //}

}

void CProducer::FillConsumerInfo(CConsumer * pConsumer,void * pConsumerInfo)
{
    memcpy(&pConsumer->Info,(CConsumer::SConsumerInfo*)pConsumerInfo,sizeof(CConsumer::SConsumerInfo));
}

