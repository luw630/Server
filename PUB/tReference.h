#pragma once

namespace dwt
{
    template <typename _tObject> 
    class tReferencePtr
    {
    private:
        template <typename _tObject> 
        class tReferenceObject
        {
        private:
            tReferenceObject(const _tObject &Object) : m_ulCounter(1), m_Object(Object) {}

        public:
            tReferenceObject() : m_ulCounter(1) {}

            ~tReferenceObject() {}

        public:
            ULONG AddRef(void)
            {
                return InterlockedIncrement(&m_ulCounter);
            }

            ULONG Release(void)
            {
                if (InterlockedDecrement(&m_ulCounter) != 0)
                    return m_ulCounter;

                delete this;
                return 0;
            }

        private:
            LONG volatile m_ulCounter;

        private:
            _tObject m_Object;

            friend class tReferencePtr;
        };

    private:
        tReferencePtr(tReferenceObject<_tObject> *lpObject) : 
           m_lpRefedObject(lpObject) {}

    public:
        static tReferencePtr<_tObject> CreateInstance() 
        { 
            return tReferencePtr<_tObject>( new tReferenceObject<_tObject>() );
        }

    public:
        tReferencePtr() : m_lpRefedObject(NULL) {}

        tReferencePtr(const tReferencePtr<_tObject> &other) : 
           m_lpRefedObject(other.m_lpRefedObject) 
        { 
            if (m_lpRefedObject != NULL)
                m_lpRefedObject->AddRef();
        }

        ~tReferencePtr() 
        {
            if (m_lpRefedObject != NULL)
            {
                m_lpRefedObject->Release();
                m_lpRefedObject = NULL;
            }
        }

    public:
        _tObject* operator -> ()
        {
            if (m_lpRefedObject == NULL)
                return NULL;

            return &m_lpRefedObject->m_Object;
        }

        operator _tObject* ()
        {
            if (m_lpRefedObject == NULL)
                return NULL;

            return &m_lpRefedObject->m_Object;
        }

        operator const _tObject* () const
        {
            if (m_lpRefedObject == NULL)
                return NULL;

            return &m_lpRefedObject->m_Object;
        }

        const tReferencePtr<_tObject>& operator = (const tReferencePtr<_tObject> &other)
        {
            if (m_lpRefedObject != NULL)
                m_lpRefedObject->Release();

            if (other.m_lpRefedObject != NULL)
                other.m_lpRefedObject->AddRef();

            m_lpRefedObject = other.m_lpRefedObject;

            return *this;
        }

    private:
        tReferenceObject<_tObject> *m_lpRefedObject;

    };

}
