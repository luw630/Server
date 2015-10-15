// 原因：目前存在心法丢失，部分和全部丢失的情况均有存在！！！
//       由于现在还无法确定是游戏服务部分还是数据管理部分的问题
//       所以为此专门做的一系列DEBUG功能

// 原理：分析所有玩家的数据异动，dump所有异动数据，作为查证依据
//       然后判断究竟是在哪边出的问题！
//       如果是从存入数据库到取出数据的过程中出现异动的话，
//       那么说明因为数据管理出现问题导致丢失
//       如果是从取出数据到存入数据库的过程中出现异动，同时游戏服务又没有给出异动记录的话，
//       那么说明因为游戏服务出现问题导致丢失

// 方法：建立一个数据库，保存所有玩家的心法以及关键数据，当心法出现变化的时候作出对应的通知
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

        // 根据wTelergyID和byTelergyLevel排序！
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {

            WORD idmin = 0xffff;
            int setindex = MAX_EQUIPTELERGY;

            // 选出一个wTelergyID最小的心法
            for (int j=0; j<MAX_EQUIPTELERGY; j++)
            {
                // 空位不参与计算
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
        // 这里的比较主要是用于判断心法丢失，所以现在先不比较心法熟练度以及心法的升级
        // 但是心法等级归零（等同于丢失）是需要判断的！！！
        if (p6data == NULL)
            return false;

        // 首先需要判断ID不为零但是等级为零的情况，这是BUG!!!
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {
            if ((p6data[i].wTelergyID != 0) &&
                (p6data[i].byTelergyLevel == 0))
                return true;
        }

        // 创建用于比较的中间数据
        cTelergyInfo data(p6data);
        
        for (int i=0; i<MAX_EQUIPTELERGY; i++)
        {
            // 如果新数据比旧数据少了什么，就说明有问题了，多了数据先不用查
            //if ((data.m_Telergy[i].byTelergyLevel == 0) &&
            //    (m_Telergy[i].byTelergyLevel != 0))
            //    return true;

            //if ((data.m_Telergy[i].wTelergyID == 0) &&
            //    (m_Telergy[i].wTelergyID != 0))
            //    return true;

            // 实际上也就是查旧的数据现在还在不在！！！

            if (m_Telergy[i].byTelergyLevel == 0)
                continue;

            bool check_success = false;

            for (int j=0; j<MAX_EQUIPTELERGY; j++)
            {
                if (m_Telergy[i].wTelergyID == data.m_Telergy[j].wTelergyID)
                {
                    if (data.m_Telergy[j].byTelergyLevel != 0)
                    {
                        // 这个心法还存在，有效！
                        check_success = true;
                        break;
                    }
                }
            }

            if (!check_success)
            {
                // 没有通过检测，失败，返回真！！！
                return true;
            }
        }

        return false;
    }

    enum _STATE
    {
        ST_NULL,    // 初始化   
        ST_RECV,    // 主线程   
        ST_SAVE,    // 保存线程 
        ST_LOAD,    // 主线程   因为是load所以ST_LOAD之后不可能是ST_RECV
        ST_SEND     // 主线程   
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
        // 没有这个玩家的数据，新建一个
        cTelergyInfo newdata(p6data);
        s_datas.AddObject(dwt::stringkey<char[11]>(name), newdata);
    }
    else
    {
        // 如果这个玩家数据已经存在，就需要进行比较
        if (pInfo->Comparep6Data(p6data))
        {

#define _OLD_PARAM(i) pInfo->m_Telergy[i].wTelergyID, pInfo->m_Telergy[i].byTelergyLevel, pInfo->m_Telergy[i].dwTelergyVal
#define _NEW_PARAM(i) p6data[i].wTelergyID, p6data[i].byTelergyLevel, p6data[i].dwTelergyVal

            static char STINFO[8][10] = {"初始化", "接收", "保存", "读取", "发送", "", "", ""};

            // 如果比较结果不同，将数据保存下来备查
            TraceInfo("心法异动记录.txt", "[%s] [%s]\r\n"
                "old[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n"
                "new[%s] = [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\r\n",
                GetStringTime(), (LPCSTR)name, 
                STINFO[pInfo->m_State%8], _OLD_PARAM(0), _OLD_PARAM(1), _OLD_PARAM(2), _OLD_PARAM(3), _OLD_PARAM(4), _OLD_PARAM(5),
                STINFO[state%8], _NEW_PARAM(0), _NEW_PARAM(1), _NEW_PARAM(2), _NEW_PARAM(3), _NEW_PARAM(4), _NEW_PARAM(5));
        }

        // 为了作为下一次比较的依据，先将数据更新
        pInfo->Setp6Data(p6data, state);
    }
}
