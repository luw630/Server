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

 //   // ��ȡ�ķ�����
 //   FileStream.open( szPath );
 //   if( !FileStream.is_open() )
 //       return rfalse( "�޷���[%s]�ķ������б��ļ���", szPath );

 // 	while (!FileStream.eof())
	//{
	//	FileStream >> szBuf;
	//	if (strcmp(szBuf, "��") == 0)
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

 //      m_Data[i].wDamCoef               = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �⹥��ϵ��	
 //      m_Data[i].wPowCoef               = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ڹ���ϵ��	
 //      m_Data[i].wDamAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �⹦����	
 //      m_Data[i].wPowAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ڹ�����	
 //      m_Data[i].wDefAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��������	
 //      m_Data[i].wAgiAddCoef            = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��������	
 //      m_Data[i].wHPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// Ѫ������	
 //      m_Data[i].wMPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��������	
 //      m_Data[i].wSPMaxAddCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��������	
 //      m_Data[i].wDamDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫���˺�	
 //      m_Data[i].wPowDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �������˺�	
 //      m_Data[i].wDefDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫���˺�	
 //      m_Data[i].wAgiDam                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �������˺�	
 //      m_Data[i].wDamDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫������	
 //      m_Data[i].wPowDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����������	
 //      m_Data[i].wDefDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫������	
 //      m_Data[i].wAgiDef                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����������	
 //      m_Data[i].wDamMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫������	
 //      m_Data[i].wPowMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����������	
 //      m_Data[i].wDefMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��̫������	
 //      m_Data[i].wAgiMed                = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����������	
 //      m_Data[i].wKnockOdds             = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ���ɼ���ϵ��	
 //      m_Data[i].wKnockDefCoef          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����ϵ��	
 //      m_Data[i].wOnVitalsTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��Ѩʱ��ϵ��	
 //      m_Data[i].wOffVitalsTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��Ѩʱ��ϵ��	
 //                                       
 //      m_Data[i].wOnLmtAtkTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����ʱ��ϵ��	
 //      m_Data[i].wOffLmtAtkTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����ʱ��ϵ��	
 //                                       
 //      m_Data[i].wOnDizzyTimeCoef       = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ѣ��ʱ��ϵ��	
 //      m_Data[i].wOffDizzyTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����ʱ��ϵ��	
 //      m_Data[i].wOnToxicTimeCoef       = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ж�ʱ��ϵ��	
 //      m_Data[i].wOffToxicTimeCoef      = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ⶾʱ��ϵ��	
 //      m_Data[i].wDamAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ṥʱ��ϵ��	
 //      m_Data[i].wDefAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ���ʱ��ϵ��	
 //      m_Data[i].wAgiAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ����ʱ��ϵ��	
 //      m_Data[i].wAmuAddTimeCoef        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ��ɱʱ��ϵ��	
 //      m_Data[i].wOffZeroSPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ֿ���������Ϊ0ʱ��ϵ��
 //      m_Data[i].wDiffCoef              = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ķ��Ѷ�ϵ��
 //      m_Data[i].wMaxLevel              = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ķ���������  

 //      m_Data[i].wOffZeroMPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// �ֿ���������ʱ��ϵ��
 //      m_Data[i].wOffZeroHPTimeCoef     = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = -1;// �ֿ���������ʱ��ϵ��
 //      m_Data[i].nMutexTelergy          = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;// ���⼼��
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
		return rfalse( "�޷���[%s]���������ķ������б��ļ���", fileName );

	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "��") == 0)
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
	if (iHard == 0)	//һ����ķ���ֱ�Ӷ�ȡ���������
	{
		return pTempData->m_hardBase;
	}
	else	//ͨ������õ�
	{
		return	(DWORD)((UINT64)pTempData->m_hardBase * iLevel * iLevel * iLevel / iHard + 10);
	}
}

DWORD DTelergyData::GetCurPractice( WORD wTime )
{
	WORD	iBase = 24;	//�չػ���
	return wTime * iBase;
}