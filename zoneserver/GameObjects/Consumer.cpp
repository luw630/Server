#include "stdafx.h"
#include "Consumer.h"
#include "Producer.h"
#include "networkmodule\NetConsumerModule\NetConsumerModule.h"


BOOL CConsumer::ConnectProvider(LPCTSTR szIP,LPCTSTR szPort)
{
    LPIObject pObject;
    char ServerInfo[400];
    strcpy(ServerInfo,szIP);
    strcat(ServerInfo,":");
    strcat(ServerInfo,szPort);
    if( m_pNetConsumer->ConnectToServer(ServerInfo) > 0)
    {
        CProducer * pProducer;
        pProducer = new CProducer ;
        //pObject = CProducer::GenerateObject(IID_PRODUCEROBJECT,0,NULL);
        //pProducer = (CProducer*)pObject->DynamicCast(IID_PRODUCEROBJECT);
        pProducer->SetIP(szIP);
        pProducer->SetPort(szPort);
        m_ProviderList.push_back(pObject);
    }
    return FALSE;
}

CConsumer::CConsumer()
{

}

CConsumer::~CConsumer()
{

}
BOOL CConsumer::Initialize()
{
    m_pNetConsumer = new CNetConsumerModule;
    if(m_pNetConsumer)
    {
        m_pNetConsumer->Initialize();
    }
    return false;
}

void CConsumer::UnInitialize()
{
    if(m_pNetConsumer)
        delete m_pNetConsumer;

}
int CConsumer::AddObject(LPIObject pChild)
{
    return 0;
}
int CConsumer::DelObject(LPIObject pChild)
{
    return 0;
}

int CConsumer::OnCreate(_W64 long pParameter)
{
    return 0;
}

void CConsumer::OnRun(void)
{

}

void CConsumer::OnClose()
{
    
}

void CConsumer::SetIP(const char * IP)
{
    if(!IsBadReadPtr(IP,24))
    {
        strcpy(Info.m_szIP,IP);
    }
}

void CConsumer::SetPort(const char * Port)
{
    if(!IsBadReadPtr(Port,24))
    {
        strcpy(Info.m_szPort,Port);
    }
}


//bool CConsumer::Connect(char * szIP,char * szPort)
//{
//    char szName[256];
//    strcpy(szName,szIP);
//    strcat(szName,":");
//    strcat(szName,szPort);
//    return KNetClient::Connect(szName);
//}

