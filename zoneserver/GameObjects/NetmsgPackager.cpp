#include "StdAfx.h"
#include "netmsgpackager.h"

// 本服务器中的所有NetmsgPackager对象上一次成功取出数据的时间，
// 用于限制本服务器中的有效NetmsgPackager对象的两次操作的时间间隔
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
    // 检测数据大小
    if (wTransDataSize > 65535)
    {
        TalkToDnid(dnidClient, "错误的数据块大小！！！");
        return;
    }

    OS_UploadFile::LPOBJECT pUFile = s_datas.GetLocateObject(dnidClient);

    // 如果有起始帧标记
    if (byTransFlag & BEGIN_FRAME_BIT)
    {
        if (pUFile != NULL)
        {
            TalkToDnid(dnidClient, "当前已经有文件在传送中！！！");
            return;
        }

        // 获取目标文件名
        if ((byDestFilenameLen == 0) || (byDestFilenameLen > wTransDataSize))
        {
            TalkToDnid(dnidClient, "错误的目标文件名！！！");
            return;
        }

        dwt::stringkey<char[256]> skDestFilename = (char*)pDataEntry;

        // 修改数据入口和大小
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
                    // 含有目录！
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
            TalkToDnid(dnidClient, "当前文件已经存在，覆盖！！！");
        }
        else
        {
            TalkToDnid(dnidClient, "开始传送新文件！！！");
        }

        HANDLE hFile = CreateFile(skDestFilename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            TalkToDnid(dnidClient, isPathFilename?"无法创建指定文件，可能是没有指定目录！！！":"无法创建指定文件！！！");

            s_datas.TraversalObjects((WPARAM)CloseAllHandle, 0);
            s_datas.Clear();

            TalkToDnid(dnidClient, "已将当前所有传送操作关闭，再传一次试试看！");

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
        TalkToDnid(dnidClient, "内部错误(A)！！！");
        return;
    }

    if ((pUFile->m_hFile == INVALID_HANDLE_VALUE) ||
        (pUFile->m_qwVerinfo == 0))
    {
        TalkToDnid(dnidClient, "内部错误(B)！！！");
        return;
    }

    // 如果有数据帧标记
    if (byTransFlag & DATA_FRAME_BIT)
    {
        if ((pUFile->m_qwVerinfo ^ dwVerinfo) != dnidClient)
        {
            TalkToDnid(dnidClient, "内部错误(C)！！！");
            return;
        }

        if (pUFile->m_dwFilledSize != dwOffset)
        {
            TalkToDnid(dnidClient, "内部错误(D)！！！");
            return;
        }

        DWORD dwOutSize = 0;
        if (!WriteFile(pUFile->m_hFile, pDataEntry, wTransDataSize, &dwOutSize, NULL))
        {
            TalkToDnid(dnidClient, "写入失败(A)！！！");
            CloseHandle(pUFile->m_hFile);
            pUFile = OS_UploadFile::LPOBJECT();
            s_datas.DelObject(dnidClient);
            return;
        }

        if (dwOutSize != wTransDataSize)
        {
            TalkToDnid(dnidClient, "写入失败(B)！！！");
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
            TalkToDnid(dnidClient, "内部错误(E)！！！");
            return;
        }

        CloseHandle(pUFile->m_hFile);
        pUFile = OS_UploadFile::LPOBJECT();
        s_datas.DelObject(dnidClient);

        TalkToDnid(dnidClient, "文件传输完毕！！！");
    }
}
