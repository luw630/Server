#pragma once

#define CONTAINER_MAX_SIZE          (0xffff)
#define CONTAINER_DEFAULT_SPACE     (0x10)
#define GET_MARGIN_DEFAULT_LIMIT    (1000)
#define GET_MARGIN_MAX_LIMIT        (5000)
#define GET_MARGIN_PUBLIC_LIMIT     (100)

// ������Ϣ�����������������ʱ�����Ƕ��߲��µ��ٽ����
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
    // ����Ҫ�㲥�����ݴ����һ������£�����FALSE��ʾ����������ˣ�
    // ��ʱ��Ӧ�ý��������е���������ȡ������մ������
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

    // ȡ���Ѿ���������ݣ�һ������£�����FALSE��ԭ������Ǳ�������û�����ݣ�
    // ��������Ϊʱ�����ƹ�ϵ�޷�ȡ�����ݣ���������µȴ��´β������ɣ�
    // ����һ�������������PushMsg����ʧ�ܣ�����������������£������������ʱ�����ƣ�
    // ��ȡ�������Ժ�ʱ�����Ʊ�����
    BOOL GetPackagedMsg(LPVOID &pPackage, WORD &wSize)
    {
        extern DWORD s_dwPrevValidGetTime;

        if (m_dwWorkThreadID != GetCurrentThreadId())
            return FALSE;

        if (m_dwCurrentSize == 0)
            return FALSE;

        // ����ϴ�push����û��ʧ�ܣ������ռ�δ����
        if (!m_bPrevPushFail)
        {
            int margin = ((int)(timeGetTime() - m_dwValidGetTime));

            // ������ϴβ�����ʱ��������������
            if (margin < (int)m_dwGetMarginDefault)
                return FALSE;

            // �������ȫ�ֲ���ʱ������
            if (m_bUsePublicLimit)
            {
                // �������������
                if ( ((int)(timeGetTime() - s_dwPrevValidGetTime)) < GET_MARGIN_PUBLIC_LIMIT )
                {
                    // �������Ĳ�����û�г������ʱ��
                    if (margin < GET_MARGIN_MAX_LIMIT)
                    {
                        // ����
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
    // �����߳�ID
    DWORD m_dwWorkThreadID;

    DWORD m_dwGetMarginDefault;

    // �Ƿ�����s_dwPrevValidGetTime��ȫ�ֲ���ʱ������
    BOOL m_bUsePublicLimit;

    // ��һ��push�����Ƿ�ʧ�ܣ��������������
    BOOL m_bPrevPushFail;

    // �����͵�ʱ�䣬����Ա��m_dwPrevValidGetTime��Ա�����������β�����ʱ������
    // ���������Ϊm_dwPrevValidGetTime�������ʱ�����̫��ʱ�����ƽ���Ч
    DWORD m_dwValidGetTime;

    // ��������������
    DWORD m_dwCurrentSize;
    char m_MsgContainer[sizeof(_TBuffer)];
};

