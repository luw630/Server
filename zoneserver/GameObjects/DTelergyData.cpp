#include "StdAfx.h"
#include "networkmodule/PlayerTypedef.h"

#include "DTelergyData.h"
bool rfalse( LPCSTR, LPCSTR )
{
    return false;
}

DTelergyData  *g_pTelergyData = NULL;
//---------------------------------------------------------
DTelergyData::DTelergyData(void)
{
    m_iMaxTelergy = 0;
}
//---------------------------------------------------------
DTelergyData::~DTelergyData(void)
{
}

//---------------------------------------------------------
BOOL DTelergyData::LoadData(char* szPath)
{
	// Zeb 2009-11-19
 //   int         i,iDataCount;

 //   dwt::ifstream    FileStream;
 //   char        szBuf[256];

 //   // 读取心法数据
 //   FileStream.open( szPath );
 //   if( !FileStream.is_open() )
 //       return rfalse( "无法打开[%s]心法数据列表文件！", szPath );

 // 	while (!FileStream.eof())
	//{
	//	FileStream >> szBuf;
	//	if (strcmp(szBuf, "·") == 0)
	//		break;
	//}
 //   i = 0;
 //   iDataCount = MAX_TELETGY;
 //   char tempStr[1024];
 //   while( !FileStream.eof() )
 //   {
 //       FileStream.getline( tempStr, sizeof( tempStr ) );
 //       if ( tempStr[0] == 0 )
 //           continue;

 //       std::strstream FileStream( tempStr, strlen( tempStr ) );

 //       int dwTemp = 0;
 //       char nameTemp[LEN_TELERGYNAME] = ""; 

 //       m_Data[i].wIndex = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;

 //       FileStream >> nameTemp;
 //       if ( nameTemp[0] == 0 )
 //           continue;

 //       strncpy( m_Data[i].szName, nameTemp, sizeof( m_Data[i].szName ) );
 //       m_Data[i].szName[LEN_TELERGYNAME-1] = 0;

 //      m_Data[i].wDamCoef               = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 外攻击系数	
 //      m_Data[i].wPowCoef               = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 内攻击系数	
 //      m_Data[i].wDamAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 外功修炼	
 //      m_Data[i].wPowAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 内功修炼	
 //      m_Data[i].wDefAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 防御修炼	
 //      m_Data[i].wAgiAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 敏捷修炼	
 //      m_Data[i].wHPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 血量上限	
 //      m_Data[i].wMPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 内力上限	
 //      m_Data[i].wSPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 体力上限	
 //      m_Data[i].wDamDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手太阳伤害	
 //      m_Data[i].wPowDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手少阳伤害	
 //      m_Data[i].wDefDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足太阴伤害	
 //      m_Data[i].wAgiDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足少阴伤害	
 //      m_Data[i].wDamDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手太阳抵制	
 //      m_Data[i].wPowDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手少阳抵制	
 //      m_Data[i].wDefDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足太阴抵制	
 //      m_Data[i].wAgiDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足少阴抵制	
 //      m_Data[i].wDamMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手太阳疗伤	
 //      m_Data[i].wPowMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 手少阳疗伤	
 //      m_Data[i].wDefMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足太阴疗伤	
 //      m_Data[i].wAgiMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 足少阴疗伤	
 //      m_Data[i].wKnockOdds             = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 击飞几率系数	
 //      m_Data[i].wKnockDefCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 抗击系数	
 //      m_Data[i].wOnVitalsTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 点穴时间系数	
 //      m_Data[i].wOffVitalsTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 解穴时间系数	
 //                                       
 //      m_Data[i].wOnLmtAtkTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 封招时间系数	
 //      m_Data[i].wOffLmtAtkTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 解招时间系数	
 //                                       
 //      m_Data[i].wOnDizzyTimeCoef       = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 眩晕时间系数	
 //      m_Data[i].wOffDizzyTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 解晕时间系数	
 //      m_Data[i].wOnToxicTimeCoef       = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 中毒时间系数	
 //      m_Data[i].wOffToxicTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 解毒时间系数	
 //      m_Data[i].wDamAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 提攻时间系数	
 //      m_Data[i].wDefAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 提防时间系数	
 //      m_Data[i].wAgiAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 提敏时间系数	
 //      m_Data[i].wAmuAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 提杀时间系数	
 //      m_Data[i].wOffZeroSPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 抵抗减体力降为0时间系数
 //      m_Data[i].wDiffCoef              = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 心法难度系数
 //      m_Data[i].wMaxLevel              = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 心法层数上限  

 //      m_Data[i].wOffZeroMPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 抵抗内力减少时间系数
 //      m_Data[i].wOffZeroHPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = -1;// 抵抗生命减少时间系数
 //      m_Data[i].nMutexTelergy          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// 互斥技能
 //                                       
 //       i ++;
 //       m_iMaxTelergy ++;
 //       if( i>=iDataCount ) break;
 //   } 
 //   FileStream.close();
// Zeb 2009-11-19
    return  LoadDataXiaYiShiJie(szPath);
}

//---------------------------------------------------------
TelergyData *DTelergyData::GetTelergy( WORD wIndex )
{
    if( wIndex>m_iMaxTelergy )     return  NULL;

    return  m_Data + wIndex;
}

//---------------------------------------------------------
BOOL DTelergyData::LoadDataXiaYiShiJie(char* fileName)
{
	int i = 0;
	int iDataCount = 0;
	dwt::ifstream FileStream;
	char szBuf[ 256 ];

	FileStream.open( fileName );
	if( !FileStream.is_open() )
		return rfalse( "无法打开[%s]侠义世界心法数据列表文件！", fileName );

	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "·") == 0)
			break;
	}

	iDataCount = MAX_TELETGY;
	char tempStr[1024];

	while( !FileStream.eof() )
	{
		FileStream.getline( tempStr, sizeof( tempStr ) );
		if ( tempStr[0] == 0 )
			continue;

		std::strstream FileStream( tempStr, ( int )strlen( tempStr ) );

		int dwTemp = 0;
		char nameTemp[LEN_TELERGYNAME] = ""; 

		m_Data[i].m_index = ( ( FileStream >> dwTemp ), dwTemp );dwTemp = 0;

		FileStream >> nameTemp;
		if ( nameTemp[0] == 0 )
			continue;

		strncpy( m_Data[i].m_name, nameTemp, sizeof( m_Data[ i ].m_name ) );
		FileStream >> m_Data[i].m_nSchool;
		
		for(int j = 0;j < TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE ;++ j)
		{
			FileStream >> m_Data[ i ].m_data[ j ];
		}
		FileStream	>> m_Data[i].m_hardBase
					>> m_Data[i].m_hardCoefficient
					>> m_Data[i].m_maxLevel;
		

		char szInfoBuf[1024];
		FileStream >> szInfoBuf;
		memset(m_Data[ i ].information,0,MAX_INFO_LENTH);
		strncpy(m_Data[ i ].information,szInfoBuf,MAX_INFO_LENTH);
		m_Data[ i ].information[MAX_INFO_LENTH - 1] = 0;

		++ i;
		++m_iMaxTelergy;
		if( i >= iDataCount )
			break;
	}
	FileStream.close();

	return TRUE;
}

DWORD DTelergyData::GetCurLevelMax(TelergyData* pTempData ,int iLevel)
{
	UINT64 iHard = (UINT64)pTempData->m_hardCoefficient;
	if (iHard == 0)	//一般的心法。直接读取最大熟练度
	{
		return pTempData->m_hardBase;
	}
	else	//通过计算得到
	{
		return	(DWORD)((UINT64)pTempData->m_hardBase * iLevel * iLevel * iLevel / iHard + 10);
	}
}

DWORD DTelergyData::GetCurPractice( WORD wTime )
{
	WORD	iBase = 24;	//闭关基数
	return wTime * iBase;
}