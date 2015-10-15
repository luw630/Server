#include "StdAfx.h"
#include "VenapointData.h"

DVenapointData  *g_pVenapointData;

DVenapointData::DVenapointData(void)
{
    memset( m_Data, 0, sizeof(SVenapointData)*VENA_COUNT*POINT_COUNT );
    memset( m_DataEx, 0, sizeof(SVenapointData)*VENA_COUNT*POINT_COUNT );
    memset( m_byVenapointCount, 0 , VENA_COUNT );
    memset( m_byVenapointCountEx, 0 , VENA_COUNT );
	memset( &m_TotalData, 0, sizeof(SVenapointData) );
	memset( &m_TotalDataEx, 0, sizeof(SVenapointDataEx) );
}

DVenapointData::~DVenapointData(void)
{
}

///////////////////////////////////////////////////////////////
BOOL    DVenapointData::LoadData(char* szPath)
{
    int         i,j;
    WORD        wID,wNum;
    char        szName[LEN_VENAPOINTNAME];
    LPSVenapointData    pData = NULL;

    dwt::ifstream    FileStream;
    char        szBuf[256];

    // 读取穴位数据
    FileStream.open( szPath );
    if( !FileStream.is_open() )
        return rfalse( "无法打开[%s]心法数据列表文件！", szPath );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "·") == 0)
			break;
	}
    i = 0;
    j = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szName;                              // 名称
		if( szName[0] == '\0' ) break;
        FileStream  >> wID;                                 // 编号（穴道）
        FileStream  >> wNum;                                // 编号（物品）

		// 根据编号确定经脉穴位位置
		i = wID >> 5;
		j = wID & 31;

		if( i==8 )
		{
			pData = &m_TotalData;
		}
		else
		{
			if( i>8 )
			{
				i = 8;
			}
			pData = &(m_Data[i][j]);

			if( j!=31 )
				m_byVenapointCount[i] ++;
		}

        if( pData )
        {
            dwt::strcpy( pData->szName, szName, LEN_VENAPOINTNAME );
            pData->wNum = wNum;
            pData->wID  = wID;
			FileStream  
						>> pData->data[SVenapointData::MAX_HP]							// 生命上限
						>> pData->data[SVenapointData::MAX_MP]							// 体力上限
						>> pData->data[SVenapointData::MAX_SP]							// 真气上限
						>> pData->data[SVenapointData::RE_HP]								// 生命恢复力（每次回复的量）
						>> pData->data[SVenapointData::RE_MP]								// 内力恢复力（每次回复的量）
						>> pData->data[SVenapointData::RE_SP]								// 真气恢复力（每次回复的量）
						>> pData->data[SVenapointData::OUT_ATTA_BASE]				// 外攻基础（单位1/1000）
						>> pData->data[SVenapointData::OUT_ATTA_MASTERY]		// 外攻精通
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// 内攻基础
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// 内攻精通
						>> pData->data[SVenapointData::OUT_DEF_BASE]				// 外防基础
						>> pData->data[SVenapointData::OUT_DEF_MASTERY]		// 外防精通
						>> pData->data[SVenapointData::INNER_DEF_BASE]			// 内防基础
						>> pData->data[SVenapointData::INNER_DEF_MASTERY]	// 内防精通
						>> pData->data[SVenapointData::AGILE]								// 身法
						>> pData->data[SVenapointData::AMUCK]							// 杀气
                        >> pData->dwUseTelergy;           // 需要消耗的心法数量
        }
    } 
    FileStream.close();

    return  TRUE;
}

// 获取一个穴位的数据，输入脉线与穴位序号。当byPointNum>=31表示脉线打通以后的数据；byVenaNum>=9表示全部脉线打通以后的数据。
LPSVenapointData    DVenapointData::GetVenapoint( BYTE byVenaNum, BYTE byPointNum )
{
    LPSVenapointData    pRet = NULL;

    if( byVenaNum>8 )
    {
        pRet = &m_TotalData;
    }
    else
    {
        if( byPointNum>31 ) 
            byPointNum = 31;

        pRet = &m_Data[byVenaNum][byPointNum];
    }

    return  pRet;
}

BYTE    DVenapointData::GetVenapointCount( BYTE byVenaNum )
{
    if( byVenaNum>=VENA_COUNT ) 
        return  0;

    return  m_byVenapointCount[byVenaNum];
}


///////////////////////////////////////////////////////////////
BOOL DVenapointData::LoadDataEx(char* szPath)
{
	return FALSE;

    int         i,j;
    WORD        wID,wNum;
    char        szName[250];
    LPSVenapointDataEx    pData = NULL;

    dwt::ifstream    FileStream;
    char        szBuf[256];

    // 读取穴位数据
    FileStream.open( szPath );
    if( !FileStream.is_open() )
        return rfalse( "无法打开[%s]心法数据列表文件！", szPath );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "·") == 0)
			break;
	}
    i = 0;
    j = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szName;                              // 名称
        FileStream  >> wID;                                 // 编号（物品）
        FileStream  >> wNum;                                // 编号（穴道）

        // 根据编号确定经脉穴位位置
        //i = wID >> 5;
        //j = wID & 31;
        i = ( wID / 31 );
        j = ( wID % 31 );
        if ( wID != 0 && j == 0 )
        {
            i --;
            j = 31;
        }

        //// 除了第一个编号为0特殊其他--
        if ( i != 0 && j != 0 && j != 31 )
            j--;

        if( i==8 )
        {
            pData = &m_TotalDataEx;
        }
        else
        {
            if( i > 8 )
                i = 8;

            pData = &(m_DataEx[i][j]);

            if( j != 31 )
                m_byVenapointCountEx[i] ++;
        }

        if( pData )
        {
            dwt::strcpy( pData->szName, szName, LEN_VENAPOINTNAME );
            pData->wNum = wNum;
            pData->wID  = wID;
            FileStream  
						>> pData->data[SVenapointData::MAX_HP]							// 生命上限
						>> pData->data[SVenapointData::MAX_MP]							// 体力上限
						>> pData->data[SVenapointData::MAX_SP]							// 真气上限
						>> pData->data[SVenapointData::RE_HP]								// 生命恢复力（每次回复的量）
						>> pData->data[SVenapointData::RE_MP]								// 内力恢复力（每次回复的量）
						>> pData->data[SVenapointData::RE_SP]								// 真气恢复力（每次回复的量）
						>> pData->data[SVenapointData::OUT_ATTA_BASE]				// 外攻基础（单位1/1000）
						>> pData->data[SVenapointData::OUT_ATTA_MASTERY]		// 外攻精通
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// 内攻基础
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// 内攻精通
						>> pData->data[SVenapointData::OUT_DEF_BASE]				// 外防基础
						>> pData->data[SVenapointData::OUT_DEF_MASTERY]		// 外防精通
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// 内防基础
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// 内防精通
						>> pData->data[SVenapointData::AGILE]								// 身法
						>> pData->data[SVenapointData::AMUCK]							// 杀气
                        >> pData->dwUseTelergy            // 需要消耗的心法数量
                        >> pData->wAddIN                  // 悟性
                        >> pData->wAddHitPro              // 命中加层万分比
                        >> pData->wAddDAMPro              // 外功加层万分比
                        >> pData->wAddPOWPro              // 内攻加层万分比
                        >> pData->wAddAGIPro              // 敏捷加层万分比
                        >> pData->wAddDodgePro            // 躲闪加层万分比
                        >> pData->wAddDefPro              // 防御加层万分比
                        >> pData->wAddSkillAdd[0]         // 技能加层[0：七杀诀 1：回生诀 2：战气诀 3：金钟罩 4：飞燕诀 5：天罡诀]
                        >> pData->wAddSkillAdd[1]
                        >> pData->wAddSkillAdd[2]
                        >> pData->wAddSkillAdd[3]
                        >> pData->wAddSkillAdd[4]
                        >> pData->wAddSkillAdd[5];
            //pData->dwUseTelergy = 80;
        }
    } 
    FileStream.close();
    return  TRUE;
}

LPSVenapointDataEx DVenapointData::GetVenapointEx( BYTE byVenaNum, BYTE byPointNum )
{
    if( byVenaNum>8 )
        return &m_TotalDataEx;
    else
    {
        if( byPointNum>31 ) 
            byPointNum = 31;

        return &m_DataEx[byVenaNum][byPointNum];
    }

    return  NULL;
}

BYTE DVenapointData::GetVenapointCountEx( BYTE byVenaNum )
{
    if ( byVenaNum >= 8 ) 
        return  0;

    return  m_byVenapointCountEx[byVenaNum];
}

std::string	DVenapointData::GetAddDataName( int data )
{
	std::string DataName[SVenapointData::VENA_PROPERTY_MAX] = {
			"生命上限",
			"内力上限",
			"真气上限",
			"生命恢复量",
			"内力恢复量",
			"真气恢复量",
			"外功基础",
			"外功精通",
			"内功基础",
			"内功精通",
			"外防基础",
			"外防精通",
			"内防基础",
			"内防精通",
			"身法",
			"杀气"
	};
	if( data<=SVenapointData::VENA_PROPERTY_MAX ) return DataName[data];
	else	return " ";
}
