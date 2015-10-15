#pragma    once
//#include "KNetClient.h"
//#include "NetModule.h"
//#include "NetClientModule.h"
#include "objectservice\Include\ObjectManager.h"
#include <list>
const __int32 IID_CONSUMER = 0x110ef9d2; 

class CNetConsumerModule;
class CProvider;


class CConsumer  :  public IObject,public CObjectManager
{
    //The Consumer Information 
    typedef enum 
    {
        CONSUMERSTATE_RUN,
    }CONSUMERSTATE;
    struct SConsumerInfo
    {
        char m_szIP[24];
        char m_szPort[24];
        CONSUMERSTATE STATE;
    };

    SConsumerInfo Info;
    
    
    //using KNetClient::Connect;

    //using KNetClient::SendMsg;
    //using KNetClient::SendPackage;
    //using KNetClient::GetRecvStatus;
    CNetConsumerModule * m_pNetConsumer;
    std::list<LPIObject> m_ProviderList;



public:
    //bool Connect(char * szIP,char * szPort);
    IObject *CreateObject(const __int32 IID)// ����һ��IID���͵� ����
    {


        if(IID == IID_CONSUMER)

            return new CConsumer;
        return NULL;
    }


    //IObject * GenerateObject(const __int32 IID );
    BOOL Initialize();
    void UnInitialize();
    BOOL ConnectProvider(LPCTSTR szIP,LPCTSTR szPort);

    void SetIP(const char * IP);
    void SetPort(const char * Port);

    CConsumer();
    ~CConsumer();
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

    friend class CProducer;
    
};

