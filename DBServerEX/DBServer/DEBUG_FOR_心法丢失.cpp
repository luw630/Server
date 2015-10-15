// ԭ��Ŀǰ�����ķ���ʧ�����ֺ�ȫ����ʧ��������д��ڣ�����
//       �������ڻ��޷�ȷ������Ϸ���񲿷ֻ������ݹ����ֵ�����
//       ����Ϊ��ר������һϵ��DEBUG����

// ԭ������������ҵ������춯��dump�����춯���ݣ���Ϊ��֤����
//       Ȼ���жϾ��������ı߳������⣡
//       ����ǴӴ������ݿ⵽ȡ�����ݵĹ����г����춯�Ļ���
//       ��ô˵����Ϊ���ݹ���������⵼�¶�ʧ
//       ����Ǵ�ȡ�����ݵ��������ݿ�Ĺ����г����춯��ͬʱ��Ϸ������û�и����춯��¼�Ļ���
//       ��ô˵����Ϊ��Ϸ����������⵼�¶�ʧ

// ����������һ�����ݿ⣬����������ҵ��ķ��Լ��ؼ����ݣ����ķ����ֱ仯��ʱ��������Ӧ��֪ͨ
#include "stdafx.h"

typedef unsigned __int64 QWORD;

#include "NETWORKMODULE/PlayerTypedef.h"
#include "ORBFRAMEWORK/ObjectService2.h"
#include "pub/traceinfo.h"

extern LPCSTR GetStringTime();

class cTelergyInfo
{
public:
    cTelergyInfo(STelergy *p6data = NULL)
    {
        Setp6Data(p6data, ST_NULL);
    }

    void Setp6Data(STelergy *p6data, int iState)
    {
        memset(m_Telergy, 0, sizeof(m_Telergy));

        if (p6data == NULL)
            return;

        m_State = (_STATE)iState;

        STelergy temp[MAX_EQUIPTELERGY];
        memcpy(temp, p6data, sizeof(temp));

        // ����wTelergyID��byTelergyLevel����
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {

            WORD idmin = 0xffff;
            int setindex = MAX_EQUIPTELERGY;

            // ѡ��һ��wTelergyID��С���ķ�
            for (int j=0; j<MAX_EQUIPTELERGY; j++)
            {
                // ��λ���������
                if (temp[j].byTelergyLevel == 0)
                    continue;

                if (temp[j].wTelergyID < idmin)
                {
                    idmin = temp[j].wTelergyID;
                    setindex = j;
                }
            }

            if (setindex < MAX_EQUIPTELERGY)
            {
                m_Telergy[i] = temp[setindex];
                temp[setindex].byTelergyLevel = 0;
            }
        }
    }

    bool Comparep6Data(STelergy *p6data)
    {
        // ����ıȽ���Ҫ�������ж��ķ���ʧ�����������Ȳ��Ƚ��ķ��������Լ��ķ�������
        // �����ķ��ȼ����㣨��ͬ�ڶ�ʧ������Ҫ�жϵģ�����
        if (p6data == NULL)
            return false;

        // ������Ҫ�ж�ID��Ϊ�㵫�ǵȼ�Ϊ������������BUG!!!
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {
            if ((p6data[i].wTelergyID != 0) &&
                (p6data[i].byTelergyLevel == 0))
                return true;
        }

        // �������ڱȽϵ��м�����
        cTelergyInfo data(p6data);
        
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {
            // ��������ݱȾ���������ʲô����˵���������ˣ����������Ȳ��ò�
            //if ((data.m_Telergy[i].byTelergyLevel == 0) &&
            //    (m_Telergy[i].byTelergyLevel != 0))
            //    return true;

            //if ((data.m_Telergy[i].wTelergyID == 0) &&
            //    (m_Telergy[i].wTelergyID != 0))
            //    return true;

            // ʵ����Ҳ���ǲ�ɵ��������ڻ��ڲ��ڣ�����

            if (m_Telergy[i].byTelergyLevel == 0)
                continue;

            bool check_success = false;

            for (int j=0; j<MAX_EQUIPTELERGY; j++)
            {
                if (m_Telergy[i].wTelergyID == data.m_Telergy[j].wTelergyID)
                {
                    if (data.m_Telergy[j].byTelergyLevel != 0)
                    {
                        // ����ķ������ڣ���Ч��
                        check_success = true;
                        break;
                    }
                }
            }

            if (!check_success)
            {
                // û��ͨ����⣬ʧ�ܣ������棡����
                return true;
            }
        }

        return false;
    }

    enum _STATE
    {
        ST_NULL,    // ��ʼ��   
        ST_RECV,    // ���߳�   
        ST_SAVE,    // �����߳� 
        ST_LOAD,    // ���߳�   ��Ϊ��load����ST_LOAD֮�󲻿�����ST_RECV
        ST_SEND     // ���߳�   
    };

    STelergy m_Telergy[MAX_EQUIPTELERGY];
    _STATE m_State;
};

typedef tObjectService<dwt::stringkey<char[11]>, cTelergyInfo> OS_TelergyInfo;

static OS_TelergyInfo s_datas;

void DEBUG_FOR_TelergyLost_UpdateTelergyData(int state, char name[11], STelergy *p6data)
{
    if (p6data == NULL)
        return;

    OS_TelergyInfo::LPOBJECT pInfo = s_datas.GetLocateObject(dwt::stringkey<char[11]>(name));
    if (pInfo == NULL)
    {
        // û�������ҵ����ݣ��½�һ��
        cTelergyInfo newdata(p6data);
        s_datas.AddObject(dwt::stringkey<char[11]>(name), newdata);
    }
    else
    {
        // ��������������Ѿ����ڣ�����Ҫ���бȽ�
        if (pInfo->Comparep6Data(p6data))
        {

#define _OLD_PARAM(i) pInfo->m_Telergy[i].wTelergyID, pInfo->m_Telergy[i].byTelergyLevel, pInfo->m_Telergy[i].dwTelergyVal
#define _NEW_PARAM(i) p6data[i].wTelergyID, p6data[i].byTelergyLevel, p6data[i].dwTelergyVal

            static char STINFO[8][10] = {"��ʼ��", "����", "����", "��ȡ", "����", "", "", ""};

            // ����ȽϽ����ͬ�������ݱ�����������
            TraceInfo("�ķ��춯��¼.txt", "[%s] [%s]\r\n"
                "old[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n"
                "new[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n",
                GetStringTime(), (LPCSTR)name, 
                STINFO[pInfo->m_State%8], _OLD_PARAM(0), _OLD_PARAM(1), _OLD_PARAM(2), _OLD_PARAM(3), _OLD_PARAM(4), _OLD_PARAM(5),
                STINFO[state%8], _NEW_PARAM(0), _NEW_PARAM(1), _NEW_PARAM(2), _NEW_PARAM(3), _NEW_PARAM(4), _NEW_PARAM(5));
        }

        // Ϊ����Ϊ��һ�αȽϵ����ݣ��Ƚ����ݸ���
        pInfo->Setp6Data(p6data, state);
    }
}
