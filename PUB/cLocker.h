#pragma once

namespace dwt
{
    class cLocker
    {
    public:
        cLocker()
        {
            InitializeCriticalSection(&m_csLocker);
        }

        ~cLocker()
        {
            DeleteCriticalSection(&m_csLocker);
        }

    public:
        void Lock()
        {
            EnterCriticalSection(&m_csLocker);
        }

        void Unlock()
        {
            LeaveCriticalSection(&m_csLocker);
        }

    private:
        CRITICAL_SECTION m_csLocker;
    };


}
