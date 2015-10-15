#pragma once

#include <wtypes.h>
#include <assert.h>
#include <map>
#include <list>
#include "pub/ObjectManager.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

typedef boost::shared_ptr<class IObject> LPIObject;
typedef boost::weak_ptr<class IObject> WEAK_LPIObject;

struct FSMBase {
    virtual void SelfDestructor() = 0;
    virtual void EnterState( DWORD type ) = 0;
    virtual void LeaveState() = 0;
    virtual void ToDo() = 0;
};

class IObject
{
	friend struct deleter;

public:
	IObject *DynamicCast(const __int32 IID) { if (this == NULL) return NULL; return (IID == m_IID) ? this : VDC(IID); }
	virtual IObject *VDC(const __int32 IID) { return NULL; }

public:
	IObject() : actor(0), m_bValid(true), m_IID(0), m_GID(0) {}
	virtual ~IObject() { if ( actor ) actor->SelfDestructor(); }

public:
	// �����������ṹ
	virtual int AddObject(LPIObject pChild) = 0;	// �����Ӷ������Ŀ
	virtual int DelObject(LPIObject pChild) = 0;	// �����Ӷ������Ŀ

protected:
	// �����Լ���AI����
	virtual void OnRun(void) = 0;
	
	// ���������ٸ������ʱ�������Ĵ���
	virtual int OnCreate(_W64 long pParameter) = 0;
	virtual void OnClose() = 0;

public:
	const __int32 GetIID(void) { return m_IID; }
	const unsigned __int32 GetGID(void) { return m_GID; }

public:
    WEAK_LPIObject self;
	//const void *m_pRef;                     // ָ���ö����Ƿ�����
	const __int32 m_IID;					// ��ǰ�����е�һ����ЧΨһ����ֵ��ָ���ö��������������GUID
	const unsigned __int32 m_GID;			// ȫ��Ψһ��ʶ��
    FSMBase *actor;

protected:
	bool m_bValid;							// ���ڿ�����Ч�ڵ�����
	WORD m_wCount;							// �����õ����ü�����

public:
    bool &isValid(void) { return m_bValid; }
	friend class CObjectManager;
};

#define check_list std::list
LPIObject g_SearchInObjectList(check_list<LPIObject>& list, DWORD dwObjectID);

typedef BOOL (CALLBACK *TRAVERSALFUNC) (LPIObject &, LPARAM);
typedef LPIObject (CALLBACK *FINDFUNC)(LPIObject &, LPARAM);

class CObjectManager : public UGE::CObjectManager< unsigned int, LPIObject >
{
public:
    CObjectManager(void);
    virtual ~CObjectManager(void);

    //void Lock() { EnterCriticalSection(&m_MapLock); }
    //void Unlock() { LeaveCriticalSection(&m_MapLock); }

public:
	// �������������Ŀ��lMaxObject��
	bool Initialize(long lMaxObject=0); 

	// ִ��һЩ��Ҫ�Ĳ���
	bool Run(); 
	bool OnEveryDayManagerRun(int nIndex);

   // �ͷ����ж��󣬹رչ�����
   bool Destroy(); 

protected:
   virtual IObject *CreateObject(const __int32 IID) = 0;   // ����һ��IID���͵� ����

public:
    // ����һ���� GID ��Ϊ�ؼ��� pParameter ��Ϊ���ɲ����� IID ���͵� ����
    LPIObject GenerateObject(const __int32 IID, unsigned __int32 GID, LPARAM pParameter); 
    IObject *_Generate(LPIObject &pObject, DWORD GID);

    // �ùؼ���������Ŀ��
    LPIObject LocateObject(const __int32 GID); 

    // �����б�ص�����
    __int32 TraversalObjects(TRAVERSALFUNC TraversalFunc, LPARAM Param);

	LPIObject FindObjects(FINDFUNC FindFunc,LPARAM Param);

    // ȷ�ϵ�ǰ�������ж��ٸ�����
    size_t size() { return Size(); }
};



// վ��
struct StateStand : FSMBase {
};

// ����
struct StateZazen : FSMBase {
};

// �ƶ����ߡ��ܣ�
struct StateMove : FSMBase {
};

// ��
struct StateJump : FSMBase {
};

// ����
struct StateAttack : FSMBase {
};

// ����
struct StateWound : FSMBase {
};

// ����
struct StateDead : FSMBase {
};

// ʹ�õ���
struct StateUsingItem : FSMBase {
};

// �ɼ�
struct StateCollect : FSMBase {
};

