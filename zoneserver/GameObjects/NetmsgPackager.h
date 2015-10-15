#pragma once

#define CONTAINER_MAX_SIZE          (0xffff)
#define CONTAINER_DEFAULT_SPACE     (0x10)
#define GET_MARGIN_DEFAULT_LIMIT    (1000)
#define GET_MARGIN_MAX_LIMIT        (5000)
#define GET_MARGIN_PUBLIC_LIMIT     (100)

// 网络消息打包容器，本容器暂时不考虑多线层下的临界操作
template <class _TBuffer>
class CNetmsgPackager
{
public:
    CNetmsgPackager(void)
    {
        m_dwWorkThreadID = ::GetCurrentThreadId();
        m_bUsePublicLimit = FALSE;
        m_bPrevPushFail = FALSE;
        m_dwValidGetTime = timeGetTime();
        m_dwCurrentSize = 0;
        m_dwGetMarginDefault = GET_MARGIN_DEFAULT_LIMIT;

        if (sizeof(_TBuffer) > CONTAINER_MAX_SIZE)
            MessageBox(NULL, "(CNetmsgPackager::cons) invalid contaner size", 0, 0);
    }

    ~CNetmsgPackager(void)
    {
    }

public:
    // 将需要广播的数据打包，一般情况下，返回FALSE表示打包容器满了，
    // 这时候应该将本容器中的数据先行取出以清空打包容器
    BOOL PushMsg(LPVOID pMsg, WORD wSize)
    {
        if (m_dwWorkThreadID != GetCurrentThreadId())
            return FALSE;

        if ( (wSize + m_dwCurrentSize + CONTAINER_DEFAULT_SPACE) > sizeof(_TBuffer) )
            return FALSE;

        if (pMsg == NULL)
            return false;

        // copied from directnetserver
        struct DNPHDR 
        {
            UINT16 seqnum;
            UINT16 paclen;
        } *pHeader = (DNPHDR *)&m_MsgContainer[m_dwCurrentSize];
        
        pHeader->seqnum = 0;
        pHeader->paclen = wSize;
        m_dwCurrentSize += sizeof(DNPHDR);

        memcpy(&m_MsgContainer[m_dwCurrentSize], pMsg, wSize);
        m_dwCurrentSize += wSize;

        return TRUE;
    }

    // 取出已经打包的数据，一般情况下，返回FALSE的原因可能是本容器中没有数据，
    // 或者是因为时间限制关系无法取出数据，这种情况下等待下次操作即可，
    // 存在一种特殊情况，在PushMsg操作失败（容器已满）的情况下，本操作会忽略时间限制，
    // 当取出数据以后，时间限制被重设
    BOOL GetPackagedMsg(LPVOID &pPackage, WORD &wSize)
    {
        extern DWORD s_dwPrevValidGetTime;

        if (m_dwWorkThreadID != GetCurrentThreadId())
            return FALSE;

        if (m_dwCurrentSize == 0)
            return FALSE;

        // 如果上次push操作没有失败（容器空间未满）
        if (!m_bPrevPushFail)
        {
            int margin = ((int)(timeGetTime() - m_dwValidGetTime));

            // 如果和上次操作的时间间隔在限制以内
            if (margin < (int)m_dwGetMarginDefault)
                return FALSE;

            // 如果存在全局操作时间限制
            if (m_bUsePublicLimit)
            {
                // 如果在限制以内
                if ( ((int)(timeGetTime() - s_dwPrevValidGetTime)) < GET_MARGIN_PUBLIC_LIMIT )
                {
                    // 如果本身的操作还没有超过最大时限
                    if (margin < GET_MARGIN_MAX_LIMIT)
                    {
                        // 返回
                        return FALSE;
                    }
                }
            }
        }

        wSize = (WORD)m_dwCurrentSize;
        pPackage = m_MsgContainer;

        m_dwCurrentSize = 0;
        m_dwValidGetTime = timeGetTime();

        if (m_bUsePublicLimit)
        {
            s_dwPrevValidGetTime = m_dwValidGetTime;
        }

        return TRUE;
    }

public:
    void EnablePublicGetMargin(BOOL bEnable);
    void SetGetMarginDefault(DWORD dwMargin)
    {
        m_dwGetMarginDefault = dwMargin;
    }

private:
    // 工作线程ID
    DWORD m_dwWorkThreadID;

    DWORD m_dwGetMarginDefault;

    // 是否启用s_dwPrevValidGetTime做全局操作时间限制
    BOOL m_bUsePublicLimit;

    // 上一次push操作是否失败（打包容器已满）
    BOOL m_bPrevPushFail;

    // 允许发送的时间，本成员和m_dwPrevValidGetTime成员用于限制两次操作的时间间隔，
    // 但是如果因为m_dwPrevValidGetTime限制造成时间差异太大，时间限制将无效
    DWORD m_dwValidGetTime;

    // 打包容器相关数据
    DWORD m_dwCurrentSize;
    char m_MsgContainer[sizeof(_TBuffer)];
};

