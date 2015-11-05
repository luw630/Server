#pragma once

#include "tReference.h"
#include "cLocker.h"

namespace dwt
{
    template <typename _tObject, typename _tSubObject>
    class tLockedPtr
    {
    private:
        tLockedPtr(const _tObject &Object) :
            m_ObjectPtr(&Object)
        {
            if (m_ObjectPtr != NULL)
                m_ObjectPtr->Lock();
        }

    public:
        tLockedPtr() {}

        tLockedPtr(const tLockedPtr &other) :
            m_ObjectPtr(other.m_ObjectPtr)
        {
            if (m_ObjectPtr != NULL)
                m_ObjectPtr->Lock();
        }

        tLockedPtr(const tReferencePtr<_tObject> &ObjectPtr) : 
            m_ObjectPtr(ObjectPtr)
        {
            if (m_ObjectPtr != NULL)
                m_ObjectPtr->Lock();
        }

        ~tLockedPtr()
        {
            if (m_ObjectPtr != NULL)
                m_ObjectPtr->Unlock();
        }

    public:
        operator _tSubObject* ()
        {
            if (m_ObjectPtr == NULL)
                return NULL;

            return &m_ObjectPtr->m_Object;
        }

        _tSubObject* operator -> ()
        {
            if (m_ObjectPtr == NULL)
                return NULL;

            return &m_ObjectPtr->m_Object;
        }

        void operator = (const tLockedPtr &other)
        {
            if (m_ObjectPtr != NULL)
                m_ObjectPtr->Unlock();

            if (other.m_ObjectPtr != NULL)
                const_cast<tReferencePtr<_tObject>&>(other.m_ObjectPtr)->Lock();

            m_ObjectPtr = other.m_ObjectPtr; 
        }

    private:
        tReferencePtr<_tObject> m_ObjectPtr;

        friend class tObjectService;
    };

}
