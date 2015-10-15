#include "StdAfx.h"
#include "DFIghtData.h"


DFightData  *g_pFightData = NULL;
WORD SkillActualCalcLevelList[ MAX_ACTURLEVEL ];  // һЩ�书ʵ�ʼ���ĵȼ��б�,�ڼ����˺���ʱʹ����Щ�ȼ�

DFightData::DFightData(void)
{
    //!3 ��ʼ��������������
    m_WoundData[EW_WOUND].byWoundFrame     = 2*2;      // �����ٶ�
    m_WoundData[EW_WOUND2].byWoundFrame     = 2*2;      // �����ٶ�
    m_WoundData[EW_WOUND3].byWoundFrame     = 2*2;      // �����ٶ�
    m_WoundData[EW_NKOCKOUT].byWoundFrame   = 2*2;      // �����ٶ�
    m_WoundData[EW_DIZZY].byWoundFrame      = 2*2;      // �����ٶ�

    m_iSpecialItem = NULL;
    m_DropItemBaseRand.pItemList = NULL;
    m_iDropItemCount = 0;
}

DFightData::~DFightData(void)
{
    ReleaseDropItemData();
    ReleaseRandDropItemData();
}

void    DFightData::ReleaseDropItemData(void)
{
    if( m_iSpecialItem )
    {
        delete[]  m_iSpecialItem;
        m_iSpecialItem = NULL;
        m_iDropItemCount = 0;
    }
}

void    DFightData::ReleaseRandDropItemData(void)
{
    if( m_DropItemBaseRand.pItemList )
    {
        delete[]  m_DropItemBaseRand.pItemList;
        m_DropItemBaseRand.pItemList = NULL;
        m_DropItemBaseRand.wItemCount = 0;
    }
}

// �����书����
BOOL    DFightData::LoadFightDate(void)
{
    int         i,j,iDataCount;
    dwt::ifstream    FileStream;
    char        szBuf[256];

    // ��ȡ�书��ʽ����
    FileStream.open( FILENAME_SKILL );
    if( !FileStream.is_open() )
        return rfalse( "�޷���[%s]�ڹ������б��ļ���", FILENAME_SKILL );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "��") == 0)
			break;
	}
    i = 0;
    iDataCount = MAX_SKILLCOUNT;
    while( !FileStream.eof() )
    {
        FileStream  >> m_Data[i].wIndex;            // ���	
        FileStream  >> m_Data[i].szName             // ����         
                    >> m_Data[i].wPicId             // ��Ӧ����               
                    >> m_Data[i].wIsAtack           // �Ƿ�������           
                    >> m_Data[i].wWoundNum          // ������λ               
                    >> m_Data[i].wAttackType        // ������ʽ               
                    >> m_Data[i].wDistance          // ��������               
                    >> m_Data[i].wTargetArea        // ������Χ               
                    >> m_Data[i].wAttackTime        // �����ٶȣ�ʱ�䣩       
                    >> m_Data[i].wHitTime           // ����ʱ��               
                    >> m_Data[i].wAttackCoef        // ����ϵ��               
                    >> m_Data[i].wHitOdds           // ������                 
                    >> m_Data[i].wHarmHP            // �����˺�               
                    >> m_Data[i].wHarmMP            // �����˺�               
                    >> m_Data[i].wHarmSP            // �����˺�               
                    >> m_Data[i].wKnockoutOdds      // ���ɼ���               
                    >> m_Data[i].wKnockoutDist      // ���ɾ���               
                    >> m_Data[i].wVitalsOdds        // ��Ѩ����               
                    >> m_Data[i].wVitalsTime        // ��Ѩʱ��               
                    >> m_Data[i].wDizzyOdds         // ��ѣ����               
                    >> m_Data[i].wDizzyTime         // ��ѣʱ��    
                    >> m_Data[i].wDropWeaponOdda    // ������������p         
                    >> m_Data[i].wDropEquipmentOdds // ������߼���         
                    >> m_Data[i].wLimitAttackOdds   // ���м���               
                    >> m_Data[i].wLimitAttackTime   // ����ʱ��               
                    >> m_Data[i].wAddHPRate         // �ָ���������p          
                    >> m_Data[i].wAddSPRate         // �ָ���������p          
                    >> m_Data[i].wAddDamTime        // �����⹦������ʱ��     
                    >> m_Data[i].wAddDamRate        // �����⹦����������p    
                    >> m_Data[i].wAddPowTime        // �����ڹ�������ʱ��     
                    >> m_Data[i].wAddPowRate        // �����ڹ�����������p    
                    >> m_Data[i].wAddDefTime        // ����������ʱ��         
                    >> m_Data[i].wAddDefRate        // ��������������p        
                    >> m_Data[i].wAddAgiTime        // ����������ʱ��         
                    >> m_Data[i].wAddAgiRate        // ���������ȱ���p        
                    >> m_Data[i].wRedAgiTime        // ����������ʱ��         
                    >> m_Data[i].wRedAgiRate        // ���������ȱ���         
                    >> m_Data[i].wToxicOdds         // �ж�����               
                    >> m_Data[i].wToxicTime         // �ж���ʱ��             
                    >> m_Data[i].wToxicRedHPRate    // �ж�����������         
                    >> m_Data[i].wAddAmuckTime      // ����ɱ��ʱ��           
                    >> m_Data[i].wAddAmuckVal       // ����ɱ������p          
                    >> m_Data[i].wDoubleMoneyOdds   // ˫��Ǯ�Ｘ��m    
                    >> m_Data[i].wAddWeaponTime     // ������������ʱ��       
                    >> m_Data[i].wAddWeaponRate     // �����������ܱ���p      
                    >> m_Data[i].wAddEquipTime      // ������������ʱ��       
                    >> m_Data[i].wAddEquipRate      // �����������ܱ���p      
                    >> m_Data[i].wDemToHPRate       // �����˺�ת��������     
                    >> m_Data[i].wDemToSPRate       // �����˺�ת��������     
                    >> m_Data[i].wUseupSP           // ��������   
                    >> m_Data[i].wUseupMP           // ��������   
                    >> m_Data[i].wUseupHP           // ��������
                    >> m_Data[i].wAtkHardCoef       // �书�Ѷ�ϵ��           
                    >> m_Data[i].wAttackRvs         // �书��������           
                    >> m_Data[i].wDemMPToHPRate     // ��������ת��������    
                    >> m_Data[i].wZeroSPOdds        // ֱ�ӽ����������ļ��� 
                    >> m_Data[i].wAddHPMaxTime      // ����HPʱ�� 
                    >> m_Data[i].wAddHPMaxRate;     // ����HP����
       i ++;
       if( i>=iDataCount ) break;
    } 
    FileStream.close();

    // ��ȡ�����书
    FileStream.open( FILENAME_BASEF );
    if( !FileStream.is_open() )
    {
        rfalse( "�޷���[%s]�������������ļ���",FILENAME_BASEF );
        return   true;
    }

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "��") == 0)
			break;
	}
    i = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szBuf          // ����
                    >> m_BaseFightList[i].wAttackTime   // �����ٶȣ�������������֡����
                    >> m_BaseFightList[i].wHitTime      // �����ٶȣ��ڵڼ�֡���У�
                    >> m_BaseFightList[i].byNextTime    // �´ι���ʱ����
                    >> m_BaseFightList[i].byDist;       // ��������
        i++;
        if( i>=6 )  break;
    }
    FileStream.close();

    // ��ȡ������Χ����
    FileStream.open( FILENAME_ATTACKMATIX );
    if( !FileStream.is_open() )
    {
        rfalse( "�޷���[%s]������Χ���������ļ���",FILENAME_ATTACKMATIX );
        return   true;
    }

 // 	while (!FileStream.eof())
	//{
	//	FileStream >> szBuf;
	//	if (strcmp(szBuf, "��") == 0)
	//		break;
	//}

    i = 0;
    j = 0;
    while( !FileStream.eof() )
    {
        j = 0;
        while( !FileStream.eof() )
        {
            FileStream >> m_AttackArea[i][j];
            j++;
            if( j>=MAX_ATTACKAREASIZE )
            {
                break;
            }
        }
        i++;
        if( i>=MAX_ATTACKAREASIZE*8 )  break;
    }
    FileStream.close();

    LoadDropItemData();
    LoadRandDropItemData();
    
    return  TRUE;
}

SSkillData  *DFightData::GetSkill( int iSkillID )
{
    if( iSkillID<0      || iSkillID>=MAX_SKILLCOUNT     )   return  NULL;

    return  &m_Data[iSkillID];
}

int     *DFightData::GetAreaMatrix( int iSkillID, BYTE byDir )
{
    int     iMatrixID, iIsBevel;

    if( iSkillID<2 || iSkillID>5 )      return  NULL;   // ���ڷ�Χ��������ʽ����

    iMatrixID = iSkillID - 2;
    iIsBevel = byDir&1;

    return  m_AttackArea[(iMatrixID<<1)+iIsBevel];
}

int     DFightData::GetAttackPosCoef( int iSkillID, BYTE byDir, int x, int y )
{
    int     iMatrixID, iIsBevel;

    if( iSkillID<2 || iSkillID>5 )      
    {
        if( iSkillID>=18 )
            iSkillID -= 16;
        else
            return  0;   // ���ڷ�Χ��������ʽ����
    }
    if( x<-4 || x>4 )                   return  0;
    if( y<-4 || y>4 )                   return  0;

    iMatrixID = iSkillID - 2;
    iIsBevel = byDir&1;

    switch( byDir>>1 )
    {
    case 0:
        return  m_AttackArea[((iMatrixID<<1)+iIsBevel)*MAX_ATTACKAREASIZE+y+4][4+x];
    case 1:
        return  m_AttackArea[((iMatrixID<<1)+iIsBevel)*MAX_ATTACKAREASIZE-x+4][4+y];
    case 2:
        return  m_AttackArea[((iMatrixID<<1)+iIsBevel)*MAX_ATTACKAREASIZE-y+4][4-x];
    case 3:
        return  m_AttackArea[((iMatrixID<<1)+iIsBevel)*MAX_ATTACKAREASIZE+x+4][4-y];
    case 4:
        return  m_AttackArea[((iMatrixID<<1)+iIsBevel)*MAX_ATTACKAREASIZE+x+4][4+y];
    }

    return  0;
}

WORD    DFightData::GetRandDropItem(void)
{
    if( m_iSpecialItem==NULL || m_iDropItemCount==0 )  return  0;
    return  (WORD)m_iSpecialItem[ rand()%m_iDropItemCount ];
}

SBaseFightType  *DFightData::GetBaseFightOnSchool( BYTE school )
{
    if( school>=5 )  return NULL;
    return  &(m_BaseFightList[school]);
}

WORD    DFightData::GetDropItemByRand( WORD wBaseRand )
{
    if( m_DropItemBaseRand.pItemList==NULL )
        return  0;

    if( (rand()%wBaseRand)==0 )
    {
        return  m_DropItemBaseRand.pItemList[ rand()%m_DropItemBaseRand.wItemCount ];
    }

    return  0;
}

bool    DFightData::LoadDropItemData(void)
{
    int     i;
    dwt::ifstream    FileStream;
    char        szBuf[256];

    ReleaseDropItemData();

    // ��ȡ���������Ʒ�б� 
    FileStream.open( FILENAME_SITEM );
    if( !FileStream.is_open() )
    {
        rfalse( "�޷���[%s]���������Ʒ�����б��ļ���", FILENAME_SITEM );
        return   false;
    }

    m_iDropItemCount = 0;
    if (!FileStream.eof())
    {
        FileStream >> szBuf;
        if( strcmp(szBuf,"DropItems") == 0)
            FileStream >> m_iDropItemCount;
    }
    if( m_iDropItemCount )
    {
        m_iSpecialItem = new int[m_iDropItemCount];
        for( i=0; i<m_iDropItemCount; i++ )
        {
            FileStream >> m_iSpecialItem[i];
        }
    }
    FileStream.close();
    return false;
}

bool    DFightData::LoadRandDropItemData(void)
{
    int     i;
    dwt::ifstream    FileStream;

    ReleaseRandDropItemData();

    // ��ȡ���������Ʒ�б� 
    FileStream.open( FILENAME_DRITEM );
    if( !FileStream.is_open() )
    {
        rfalse( "�޷���[%s]���������Ʒ�����б��ļ���", FILENAME_DRITEM );
        return   false;
    }

    FileStream  >> m_DropItemBaseRand.wMax
                >> m_DropItemBaseRand.wMin
                >> m_DropItemBaseRand.wReduceVal
                >> m_DropItemBaseRand.wItemCount;

    if( m_DropItemBaseRand.wItemCount )
    {
        m_DropItemBaseRand.pItemList = new WORD[ m_DropItemBaseRand.wItemCount ];
        for( i=0; i<m_DropItemBaseRand.wItemCount; i++ )
        {
            FileStream >> m_DropItemBaseRand.pItemList[i];
        }
    }
    FileStream.close();

    return  true;
}
