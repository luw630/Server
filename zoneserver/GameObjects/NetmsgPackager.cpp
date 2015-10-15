#include "StdAfx.h"
#include "netmsgpackager.h"

// ���������е�����NetmsgPackager������һ�γɹ�ȡ�����ݵ�ʱ�䣬
// �������Ʊ��������е���ЧNetmsgPackager��������β�����ʱ����
DWORD s_dwPrevValidGetTime = timeGetTime();

extern void TalkToDnid(DNID, LPCSTR);

#define BEGIN_FRAME_BIT     (1)
#define DATA_FRAME_BIT      (2)
#define FINAL_FRAME_BIT     (4)

#define MAKEQWORD(low, hi) ((QWORD)((((QWORD)hi) << 32) | low))

struct SUploadFile
{
    SUploadFile() : 
        m_qwVerinfo(0), 
        m_hFile(INVALID_HANDLE_VALUE), 
        m_dwFilledSize(0)
    {}

    QWORD m_qwVerinfo;
    HANDLE m_hFile;
    DWORD m_dwFilledSize;
    DWORD m_dwOpTime;
};

typedef tObjectService<DNID, SUploadFile> OS_UploadFile;

static OS_UploadFile s_datas;

static DWORD String2ID(LPSTR szString)
{
	DWORD ID = 0;
	for (int i = 0; szString[i]; i++)
	{
		ID = (ID + (i + 1) * szString[i]) % 0x8000000b * 0xffffffef;
	}
	return (ID ^ 0x7ae8d31f);
}

DWORD CALLBACK CloseAllHandle(const DNID &Name, SUploadFile &Object, LPARAM lParam)
{
    CloseHandle(Object.m_hFile);
    return 0;
}

void UploadFile(DNID dnidClient, DWORD dwVerinfo, WORD wTransDataSize, BYTE byTransFlag, DWORD dwOffset, BYTE byDestFilenameLen, LPBYTE pDataEntry)
{
    // ������ݴ�С
    if (wTransDataSize > 65535)
    {
        TalkToDnid(dnidClient, "��������ݿ��С������");
        return;
    }

    OS_UploadFile::LPOBJECT pUFile = s_datas.GetLocateObject(dnidClient);

    // �������ʼ֡���
    if (byTransFlag & BEGIN_FRAME_BIT)
    {
        if (pUFile != NULL)
        {
            TalkToDnid(dnidClient, "��ǰ�Ѿ����ļ��ڴ����У�����");
            return;
        }

        // ��ȡĿ���ļ���
        if ((byDestFilenameLen == 0) || (byDestFilenameLen > wTransDataSize))
        {
            TalkToDnid(dnidClient, "�����Ŀ���ļ���������");
            return;
        }

        dwt::stringkey<char[256]> skDestFilename = (char*)pDataEntry;

        // �޸�������ںʹ�С
        pDataEntry += byDestFilenameLen;
        wTransDataSize -= byDestFilenameLen;

        char dir[256];
        int seg = 0, cur = 1;
        bool isPathFilename = false;
        LPSTR pIndex = skDestFilename;
        while (*pIndex++ != '\0')
        {
            if ((*pIndex == '\\') || (*pIndex == '/'))
            {
                if (cur-seg == 0)
                {
                    seg = cur+1;
                }
                else
                {
                    // ����Ŀ¼��
                    isPathFilename = true;
                    memcpy(dir, &skDestFilename[seg], cur-seg);
                    dir[cur-seg] = 0;
                    CreateDirectory(dir, NULL);
                    seg = cur+1;
                }
            }
            cur++;
        }

        if (PathFileExists(skDestFilename))
        {
            TalkToDnid(dnidClient, "��ǰ�ļ��Ѿ����ڣ����ǣ�����");
        }
        else
        {
            TalkToDnid(dnidClient, "��ʼ�������ļ�������");
        }

        HANDLE hFile = CreateFile(skDestFilename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            TalkToDnid(dnidClient, isPathFilename?"�޷�����ָ���ļ���������û��ָ��Ŀ¼������":"�޷�����ָ���ļ�������");

            s_datas.TraversalObjects((WPARAM)CloseAllHandle, 0);
            s_datas.Clear();

            TalkToDnid(dnidClient, "�ѽ���ǰ���д��Ͳ����رգ��ٴ�һ�����Կ���");

            return;
        }

        SUploadFile UFile;
        UFile.m_dwFilledSize = 0;
        UFile.m_hFile = hFile;
        UFile.m_qwVerinfo = dnidClient ^ String2ID(skDestFilename);
        UFile.m_dwOpTime = timeGetTime();
        dwOffset = 0;

        s_datas.AddObject(dnidClient, UFile);
        pUFile = s_datas.GetLocateObject(dnidClient);
    }

    if (pUFile == NULL)
    {
        TalkToDnid(dnidClient, "�ڲ�����(A)������");
        return;
    }

    if ((pUFile->m_hFile == INVALID_HANDLE_VALUE) ||
        (pUFile->m_qwVerinfo == 0))
    {
        TalkToDnid(dnidClient, "�ڲ�����(B)������");
        return;
    }

    // ���������֡���
    if (byTransFlag & DATA_FRAME_BIT)
    {
        if ((pUFile->m_qwVerinfo ^ dwVerinfo) != dnidClient)
        {
            TalkToDnid(dnidClient, "�ڲ�����(C)������");
            return;
        }

        if (pUFile->m_dwFilledSize != dwOffset)
        {
            TalkToDnid(dnidClient, "�ڲ�����(D)������");
            return;
        }

        DWORD dwOutSize = 0;
        if (!WriteFile(pUFile->m_hFile, pDataEntry, wTransDataSize, &dwOutSize, NULL))
        {
            TalkToDnid(dnidClient, "д��ʧ��(A)������");
            CloseHandle(pUFile->m_hFile);
            pUFile = OS_UploadFile::LPOBJECT();
            s_datas.DelObject(dnidClient);
            return;
        }

        if (dwOutSize != wTransDataSize)
        {
            TalkToDnid(dnidClient, "д��ʧ��(B)������");
            CloseHandle(pUFile->m_hFile);
            pUFile = OS_UploadFile::LPOBJECT();
            s_datas.DelObject(dnidClient);
            return;
        }

        pUFile->m_dwFilledSize += dwOutSize;
    }

    if (byTransFlag & FINAL_FRAME_BIT)
    {
        if ((pUFile->m_qwVerinfo ^ dwVerinfo) != dnidClient)
        {
            TalkToDnid(dnidClient, "�ڲ�����(E)������");
            return;
        }

        CloseHandle(pUFile->m_hFile);
        pUFile = OS_UploadFile::LPOBJECT();
        s_datas.DelObject(dnidClient);

        TalkToDnid(dnidClient, "�ļ�������ϣ�����");
    }
}
