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
	// 用来产生树结构
	virtual int AddObject(LPIObject pChild) = 0;	// 返回子对象的数目
	virtual int DelObject(LPIObject pChild) = 0;	// 返回子对象的数目

protected:
	// 运行自己的AI部分
	virtual void OnRun(void) = 0;
	
	// 创建与销毁该物件的时候所作的处理
	virtual int OnCreate(_W64 long pParameter) = 0;
	virtual void OnClose() = 0;

public:
	const __int32 GetIID(void) { return m_IID; }
	const unsigned __int32 GetGID(void) { return m_GID; }

public:
    WEAK_LPIObject self;
	//const void *m_pRef;                     // 指明该对象是否被引用
	const __int32 m_IID;					// 当前程序中的一个有效唯一的数值，指明该对象的类型类似于GUID
	const unsigned __int32 m_GID;			// 全局唯一标识符
    FSMBase *actor;

protected:
	bool m_bValid;							// 用于控制有效期的数据
	WORD m_wCount;							// 测试用的引用计数器

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
	// 根据最大容纳数目（lMaxObject）
	bool Initialize(long lMaxObject=0); 

	// 执行一些必要的操作
	bool Run(); 
	bool OnEveryDayManagerRun(int nIndex);

   // 释放所有对象，关闭管理器
   bool Destroy(); 

protected:
   virtual IObject *CreateObject(const __int32 IID) = 0;   // 生成一个IID类型的 对象

public:
    // 生成一个以 GID 作为关键字 pParameter 作为生成参数的 IID 类型的 对象
    LPIObject GenerateObject(const __int32 IID, unsigned __int32 GID, LPARAM pParameter); 
    IObject *_Generate(LPIObject &pObject, DWORD GID);

    // 用关键字来查找目标
    LPIObject LocateObject(const __int32 GID); 

    // 遍历列表回调函数
    __int32 TraversalObjects(TRAVERSALFUNC TraversalFunc, LPARAM Param);

	LPIObject FindObjects(FINDFUNC FindFunc,LPARAM Param);

    // 确认当前容器中有多少个对象
    size_t size() { return Size(); }
};



// 站立
struct StateStand : FSMBase {
};

// 打坐
struct StateZazen : FSMBase {
};

// 移动（走、跑）
struct StateMove : FSMBase {
};

// 跳
struct StateJump : FSMBase {
};

// 攻击
struct StateAttack : FSMBase {
};

// 受伤
struct StateWound : FSMBase {
};

// 死亡
struct StateDead : FSMBase {
};

// 使用道具
struct StateUsingItem : FSMBase {
};

// 采集
struct StateCollect : FSMBase {
};

