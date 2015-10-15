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

#include "ORBFRAMEWORK/ObjectService2.h"
#include "pub/traceinfo.h"

extern LPCSTR GetStringTime();

class cTelergyInfo
{
public:
    cTelergyInfo(STelergy *p6data = NULL)
    {
        Setp6Data(p6data, false);
    }

    void Setp6Data(STelergy *p6data, BOOL isSave)
    {
        memset(m_Telergy, 0, sizeof(m_Telergy));

        if (p6data == NULL)
            return;

        m_isSave = isSave;

        STelergy temp[6];
        memcpy(temp, p6data, sizeof(temp));

        // ����wTelergyID��byTelergyLevel����
        for (int i=0; i<6; i++)
        {

            WORD idmin = 0xffff;
            int setindex = 6;

            // ѡ��һ��wTelergyID��С���ķ�
            for (int j=0; j<6; j++)
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

            if (setindex < 6)
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
        for (int i=0; i<6; i++)
        {
            if ((p6data[i].wTelergyID != 0) &&
                (p6data[i].byTelergyLevel == 0))
                return true;
        }

        // �������ڱȽϵ��м�����
        cTelergyInfo data(p6data);
        
        for (int i=0; i<6; i++)
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

            for (int j=0; j<6; j++)
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

    STelergy m_Telergy[6];
    BOOL m_isSave;
};

typedef tObjectService<dwt::stringkey<char[11]>, cTelergyInfo> OS_TelergyInfo;

static OS_TelergyInfo s_datas;

void DEBUG_FOR_TelergyLost_UpdateTelergyData(bool isSave, dwt::stringkey<char[11]> name, STelergy *p6data)
{
    if (p6data == NULL)
        return;

    OS_TelergyInfo::LPOBJECT pInfo = s_datas.GetLocateObject(name);
    if (pInfo == NULL)
    {
        // û�������ҵ����ݣ��½�һ��
        cTelergyInfo newdata(p6data);
        s_datas.AddObject(name, newdata);
    }
    else
    {
        // ��������������Ѿ����ڣ�����Ҫ���бȽ�
        if (pInfo->Comparep6Data(p6data))
        {

#define _OLD_PARAM(i) pInfo->m_Telergy[i].wTelergyID, pInfo->m_Telergy[i].byTelergyLevel, pInfo->m_Telergy[i].dwTelergyVal
#define _NEW_PARAM(i) p6data[i].wTelergyID, p6data[i].byTelergyLevel, p6data[i].dwTelergyVal

            // ����ȽϽ����ͬ�������ݱ�����������
            TraceInfo("�ķ��춯��¼.txt", "[%s] [%s]\r\n"
                "old[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n"
                "new[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n",
                GetStringTime(), (LPCSTR)name, 
                pInfo->m_isSave ? "����" : "��ȡ", _OLD_PARAM(0), _OLD_PARAM(1), _OLD_PARAM(2), _OLD_PARAM(3), _OLD_PARAM(4), _OLD_PARAM(5),
                isSave ? "����" : "��ȡ", _NEW_PARAM(0), _NEW_PARAM(1), _NEW_PARAM(2), _NEW_PARAM(3), _NEW_PARAM(4), _NEW_PARAM(5));
        }

        // Ϊ����Ϊ��һ�αȽϵ����ݣ��Ƚ����ݸ���
        pInfo->Setp6Data(p6data, isSave);
    }
}
