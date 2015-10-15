#pragma    once
//#include "KNetServer.h"
//#include "NetModule.h"
#include <list>
#include "objectservice\Include\ObjectManager.h"

#define  MAX_CONSUMER_AFFORD 32 

class CItem;
class CArea;
class CMonster;
class CNpc;
class CPlayer;
class CRegion;

//const __int32 IID_AREA = 0x110304ce; 
//const __int32 IID_ITEM = 0x1168eb14; 
//const __int32 IID_MONSTER = 0x112100a4; 
//const __int32 IID_NPC = 0x11210094; 
//const __int32 IID_PLAYER = 0x110ef9d3; 
//const __int32 IID_REGION = 0x11030636; 





const __int32 IID_PRODUCEROBJECT = 0x117c95d7; 

class CNetProviderModule ;
class CConsumer;
class CProducer : public IObject,public CObjectManager
    
{

    //IObject *CreateObject(const __int32 IID)// ����һ��IID���͵� ����
    //{
    //    return new CProducer;
    //    //return NULL;
    //}

    char m_szIP[24];
    char m_szPort[24];



    CNetProviderModule * m_pNetProviderModule;
    //std::list<LPIObject> m_ConsumerList;

    LPIObject m_ConsumerArray[MAX_CONSUMER_AFFORD];
    DWORD m_ConsumerCount;

    //Get infomation from config file
    //void virtual GetConfig(LPCTSTR szConfigFilePath) = 0;

    //Add a consumer's infomation 
    void AddConsumer(LPIObject pConsumer,WORD wClientIndex);
    //Fill a consumer's information from a structure
    void FillConsumerInfo(CConsumer * pConsumer,void * pConsumerInfo);


    //������Ϣ��client
    //using KNetServer::RecieveData;
    //using KNetServer::RecieveMessage; 
    //using KNetServer::MessagePreProcess;
    //// invoke IDreamNetServer::SendTo internally
    //using KNetServer::SendMessage;
    //// invoke IDreamNetServer::DeletePlayer internally
    //using KNetServer::DelOneClient; 

public:
    IObject * CreateObject(const __int32 IID);
    bool Initialize(char * PortName);
    void UnInitialize();
    void Run();
    void SetIP(const char * IP);
    void SetPort(const char * Port);

    CProducer();
    ~CProducer();
private:
    IObject *VDC(const __int32 IID) { return NULL; }


public:
    //IObject() : m_bValid(true), m_pRef(NULL), m_IID(0), m_GID(0) { }
    //~IObject() { }

public:
    // �����������ṹ
    int AddObject(LPIObject pChild);	// �����Ӷ������Ŀ
    int DelObject(LPIObject pChild);	// �����Ӷ������Ŀ

protected:
    // �����Լ���AI����
    void OnRun(void);

    // ���������ٸ������ʱ�������Ĵ���
    int OnCreate(_W64 long pParameter);
    void OnClose();

    friend class CConsumer;


};