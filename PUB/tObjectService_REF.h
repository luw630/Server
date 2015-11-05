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
                throw new exception("��������������������һ����ڵ㣡�ᵼ���޷�Ԥ�ϵĽ����");
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
                throw new exception("���������������ɾ����һ����ڵ㣡�ᵼ���޷�Ԥ�ϵĽ����");
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
        cLocker m_MapLocker;    // ʹ���ٽ�������ֹͬһʱ�������߳�ͬʱ��MAP�����޸ģ�
        BOOL    m_bAnitModify;  // Ϊ�˱�֤�ڱ����Ĺ����в����޸�MAP�ڽڵ㣬������Ҫ���м�飡

    private:
        CRITICALOBJECTMAP m_ObjectMap;
    };
}