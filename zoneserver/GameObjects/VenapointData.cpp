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

    // ��ȡѨλ����
    FileStream.open( szPath );
    if( !FileStream.is_open() )
        return rfalse( "�޷���[%s]�ķ������б��ļ���", szPath );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "��") == 0)
			break;
	}
    i = 0;
    j = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szName;                              // ����
		if( szName[0] == '\0' ) break;
        FileStream  >> wID;                                 // ��ţ�Ѩ����
        FileStream  >> wNum;                                // ��ţ���Ʒ��

		// ���ݱ��ȷ������Ѩλλ��
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
						>> pData->data[SVenapointData::MAX_HP]							// ��������
						>> pData->data[SVenapointData::MAX_MP]							// ��������
						>> pData->data[SVenapointData::MAX_SP]							// ��������
						>> pData->data[SVenapointData::RE_HP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::RE_MP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::RE_SP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::OUT_ATTA_BASE]				// �⹥��������λ1/1000��
						>> pData->data[SVenapointData::OUT_ATTA_MASTERY]		// �⹥��ͨ
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// �ڹ�����
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// �ڹ���ͨ
						>> pData->data[SVenapointData::OUT_DEF_BASE]				// �������
						>> pData->data[SVenapointData::OUT_DEF_MASTERY]		// �����ͨ
						>> pData->data[SVenapointData::INNER_DEF_BASE]			// �ڷ�����
						>> pData->data[SVenapointData::INNER_DEF_MASTERY]	// �ڷ���ͨ
						>> pData->data[SVenapointData::AGILE]								// ��
						>> pData->data[SVenapointData::AMUCK]							// ɱ��
                        >> pData->dwUseTelergy;           // ��Ҫ���ĵ��ķ�����
        }
    } 
    FileStream.close();

    return  TRUE;
}

// ��ȡһ��Ѩλ�����ݣ�����������Ѩλ��š���byPointNum>=31��ʾ���ߴ�ͨ�Ժ�����ݣ�byVenaNum>=9��ʾȫ�����ߴ�ͨ�Ժ�����ݡ�
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

    // ��ȡѨλ����
    FileStream.open( szPath );
    if( !FileStream.is_open() )
        return rfalse( "�޷���[%s]�ķ������б��ļ���", szPath );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "��") == 0)
			break;
	}
    i = 0;
    j = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szName;                              // ����
        FileStream  >> wID;                                 // ��ţ���Ʒ��
        FileStream  >> wNum;                                // ��ţ�Ѩ����

        // ���ݱ��ȷ������Ѩλλ��
        //i = wID >> 5;
        //j = wID & 31;
        i = ( wID / 31 );
        j = ( wID % 31 );
        if ( wID != 0 && j == 0 )
        {
            i --;
            j = 31;
        }

        //// ���˵�һ�����Ϊ0��������--
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
						>> pData->data[SVenapointData::MAX_HP]							// ��������
						>> pData->data[SVenapointData::MAX_MP]							// ��������
						>> pData->data[SVenapointData::MAX_SP]							// ��������
						>> pData->data[SVenapointData::RE_HP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::RE_MP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::RE_SP]								// �����ָ�����ÿ�λظ�������
						>> pData->data[SVenapointData::OUT_ATTA_BASE]				// �⹥��������λ1/1000��
						>> pData->data[SVenapointData::OUT_ATTA_MASTERY]		// �⹥��ͨ
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// �ڹ�����
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// �ڹ���ͨ
						>> pData->data[SVenapointData::OUT_DEF_BASE]				// �������
						>> pData->data[SVenapointData::OUT_DEF_MASTERY]		// �����ͨ
						>> pData->data[SVenapointData::INNER_ATTA_BASE]			// �ڷ�����
						>> pData->data[SVenapointData::INNER_ATTA_MASTERY]	// �ڷ���ͨ
						>> pData->data[SVenapointData::AGILE]								// ��
						>> pData->data[SVenapointData::AMUCK]							// ɱ��
                        >> pData->dwUseTelergy            // ��Ҫ���ĵ��ķ�����
                        >> pData->wAddIN                  // ����
                        >> pData->wAddHitPro              // ���мӲ���ֱ�
                        >> pData->wAddDAMPro              // �⹦�Ӳ���ֱ�
                        >> pData->wAddPOWPro              // �ڹ��Ӳ���ֱ�
                        >> pData->wAddAGIPro              // ���ݼӲ���ֱ�
                        >> pData->wAddDodgePro            // �����Ӳ���ֱ�
                        >> pData->wAddDefPro              // �����Ӳ���ֱ�
                        >> pData->wAddSkillAdd[0]         // ���ܼӲ�[0����ɱ�� 1�������� 2��ս���� 3�������� 4������� 5�����]
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
			"��������",
			"��������",
			"��������",
			"�����ָ���",
			"�����ָ���",
			"�����ָ���",
			"�⹦����",
			"�⹦��ͨ",
			"�ڹ�����",
			"�ڹ���ͨ",
			"�������",
			"�����ͨ",
			"�ڷ�����",
			"�ڷ���ͨ",
			"��",
			"ɱ��"
	};
	if( data<=SVenapointData::VENA_PROPERTY_MAX ) return DataName[data];
	else	return " ";
}
