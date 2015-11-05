#pragma once

#include <map>

#include "tReference.h"
#include "tLockedPtr.h"

namespace dwt
{
    template <typename _tObject>
    struct sCriticalObject : public cLocker
    {
        sCriticalObject() : cLocker() {}
        // sCriticalObject(const _tObject Object) : cLocker(), m_Object(Object) {}

        ~sCriticalObject() {}

        operator _tObject const & () const 
        { 
            return reinterpret_cast<_tObject&>(*this); 
        }

        _tObject m_Object;
    };

    template <typename _tNameStub, typename _tObject>
    class tObjectService_REF
    {
    public:
        #define TCR_BREAK       (0x0001L)
        #define TCR_REMOVE      (0x0010L)

        typedef tLockedPtr< sCriticalObject<_tObject>, _tObject >       OBJECTPTR;
        typedef tReferencePtr< sCriticalObject<_tObject> >              LPCRITICALOBJECT;
        typedef std::map< _tNameStub, LPCRITICALOBJECT >                CRITICALOBJECTMAP;

        typedef DWORD (CALLBACK * TRAVERSALCALLBACK)(const _tNameStub &NameStub, const OBJECTPTR &Object, LPARAM lParam);

    public:
        OBJECTPTR NewObject(const _tNameStub &NameStub)
        {
            OBJECTPTR ret;

            m_MapLocker.Lock();

            if (m_bAnitModify)
            {
                m_MapLocker.Unlock();
                throw new exception("在链表遍历过程中添加了一个表节点！会导致无法预料的结果！");
                return ret;
            }

            if (m_ObjectMap.find(NameStub) == m_ObjectMap.end())
            {
                LPCRITICALOBJECT t = LPCRITICALOBJECT::CreateInstance();
                m_ObjectMap[NameStub] = t;
                ret = OBJECTPTR(t);
            }

            m_MapLocker.Unlock();

            return ret;
        }

        BOOL DelObject(_tNameStub &NameStub)
        {
            BOOL ret = FALSE;

            m_MapLocker.Lock();

            if (m_bAnitModify)
            {
                m_MapLocker.Unlock();
                throw new exception("在链表遍历过程中删除了一个表节点！会导致无法预料的结果！");
                return ret;
            }

            CRITICALOBJECTMAP::iterator it = m_ObjectMap.find(NameStub);
            if (it != m_ObjectMap.end())
            {
                m_ObjectMap.erase(it);
                ret = TRUE;
            }

            m_MapLocker.Unlock();

            return ret;
        }

        OBJECTPTR GetLocateObject(_tNameStub &NameStub)
        {
            OBJECTPTR ret;

            m_MapLocker.Lock();

            CRITICALOBJECTMAP::iterator it = m_ObjectMap.find(NameStub);
            if (it != m_ObjectMap.end())
                ret = OBJECTPTR(it->second);

            m_MapLocker.Unlock();

            return ret;
        }

        void Clear()
        {
            m_MapLocker.Lock();

            m_ObjectMap.clear();

            m_MapLocker.Unlock();
        }

        DWORD GetSize()
        {
            return (DWORD)m_ObjectMap.size();
        }

        void TraversalObjects(TRAVERSALCALLBACK traversal, LPARAM lParam)
        {
            m_MapLocker.Lock();

            m_bAnitModify = TRUE;

            CRITICALOBJECTMAP::iterator it = m_ObjectMap.begin();
            while (it != m_ObjectMap.end())
            {
                DWORD ret = traversal(it->first, OBJECTPTR(it->second), lParam);

                if (ret & TCR_REMOVE)
                    it = m_ObjectMap.erase(it);
                else
                    it ++;

                if (ret & TCR_BREAK)
                    break;
            }

            m_bAnitModify = FALSE;

            m_MapLocker.Unlock();
        }

    public:
        tObjectService_REF(void) :
            m_bAnitModify(FALSE)
        {
        }

        ~tObjectService_REF(void) 
        {
        }

    private:
        cLocker m_MapLocker;    // 使用临界锁，防止同一时刻两个线程同时对MAP进行修改！
        BOOL    m_bAnitModify;  // 为了保证在遍历的过程中不能修改MAP内节点，所以需要进行检查！

    private:
        CRITICALOBJECTMAP m_ObjectMap;
    };
}