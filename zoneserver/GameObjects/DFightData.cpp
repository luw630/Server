#include "StdAfx.h"
#include "DFIghtData.h"


DFightData  *g_pFightData = NULL;
WORD SkillActualCalcLevelList[ MAX_ACTURLEVEL ];  // 一些武功实际计算的等级列表,在计算伤害等时使用这些等级

DFightData::DFightData(void)
{
    //!3 初始化基本受伤数据
    m_WoundData[EW_WOUND].byWoundFrame     = 2*2;      // 受伤速度
    m_WoundData[EW_WOUND2].byWoundFrame     = 2*2;      // 受伤速度
    m_WoundData[EW_WOUND3].byWoundFrame     = 2*2;      // 受伤速度
    m_WoundData[EW_NKOCKOUT].byWoundFrame   = 2*2;      // 受伤速度
    m_WoundData[EW_DIZZY].byWoundFrame      = 2*2;      // 受伤速度

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

// 载入武功数据
BOOL    DFightData::LoadFightDate(void)
{
    int         i,j,iDataCount;
    dwt::ifstream    FileStream;
    char        szBuf[256];

    // 读取武功招式数据
    FileStream.open( FILENAME_SKILL );
    if( !FileStream.is_open() )
        return rfalse( "无法打开[%s]内功数据列表文件！", FILENAME_SKILL );

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "·") == 0)
			break;
	}
    i = 0;
    iDataCount = MAX_SKILLCOUNT;
    while( !FileStream.eof() )
    {
        FileStream  >> m_Data[i].wIndex;            // 编号	
        FileStream  >> m_Data[i].szName             // 名称         
                    >> m_Data[i].wPicId             // 对应动画               
                    >> m_Data[i].wIsAtack           // 是否辅助攻击           
                    >> m_Data[i].wWoundNum          // 攻击段位               
                    >> m_Data[i].wAttackType        // 攻击方式               
                    >> m_Data[i].wDistance          // 攻击距离               
                    >> m_Data[i].wTargetArea        // 攻击范围               
                    >> m_Data[i].wAttackTime        // 攻击速度（时间）       
                    >> m_Data[i].wHitTime           // 命中时间               
                    >> m_Data[i].wAttackCoef        // 攻击系数               
                    >> m_Data[i].wHitOdds           // 命中率                 
                    >> m_Data[i].wHarmHP            // 生命伤害               
                    >> m_Data[i].wHarmMP            // 内力伤害               
                    >> m_Data[i].wHarmSP            // 体力伤害               
                    >> m_Data[i].wKnockoutOdds      // 击飞几率               
                    >> m_Data[i].wKnockoutDist      // 击飞距离               
                    >> m_Data[i].wVitalsOdds        // 点穴几率               
                    >> m_Data[i].wVitalsTime        // 点穴时间               
                    >> m_Data[i].wDizzyOdds         // 晕眩几率               
                    >> m_Data[i].wDizzyTime         // 晕眩时间    
                    >> m_Data[i].wDropWeaponOdda    // 打落武器几率p         
                    >> m_Data[i].wDropEquipmentOdds // 打落防具几率         
                    >> m_Data[i].wLimitAttackOdds   // 封招几率               
                    >> m_Data[i].wLimitAttackTime   // 封招时间               
                    >> m_Data[i].wAddHPRate         // 恢复生命比率p          
                    >> m_Data[i].wAddSPRate         // 恢复体力比率p          
                    >> m_Data[i].wAddDamTime        // 提升外功攻击力时间     
                    >> m_Data[i].wAddDamRate        // 提升外功攻击力比率p    
                    >> m_Data[i].wAddPowTime        // 提升内功攻击力时间     
                    >> m_Data[i].wAddPowRate        // 提升内功攻击力比率p    
                    >> m_Data[i].wAddDefTime        // 提升防御力时间         
                    >> m_Data[i].wAddDefRate        // 提升防御力比率p        
                    >> m_Data[i].wAddAgiTime        // 提升灵敏度时间         
                    >> m_Data[i].wAddAgiRate        // 提升灵敏度比率p        
                    >> m_Data[i].wRedAgiTime        // 降低灵敏度时间         
                    >> m_Data[i].wRedAgiRate        // 降低灵敏度比率         
                    >> m_Data[i].wToxicOdds         // 中毒几率               
                    >> m_Data[i].wToxicTime         // 中毒总时间             
                    >> m_Data[i].wToxicRedHPRate    // 中毒损生命比率         
                    >> m_Data[i].wAddAmuckTime      // 提升杀气时间           
                    >> m_Data[i].wAddAmuckVal       // 提升杀气数量p          
                    >> m_Data[i].wDoubleMoneyOdds   // 双倍钱物几率m    
                    >> m_Data[i].wAddWeaponTime     // 提升武器性能时间       
                    >> m_Data[i].wAddWeaponRate     // 提升武器性能比率p      
                    >> m_Data[i].wAddEquipTime      // 提升防具性能时间       
                    >> m_Data[i].wAddEquipRate      // 提升防具性能比率p      
                    >> m_Data[i].wDemToHPRate       // 吸收伤害转生命比率     
                    >> m_Data[i].wDemToSPRate       // 吸收伤害转体力比率     
                    >> m_Data[i].wUseupSP           // 消耗体力   
                    >> m_Data[i].wUseupMP           // 消耗内力   
                    >> m_Data[i].wUseupHP           // 消耗生命
                    >> m_Data[i].wAtkHardCoef       // 武功难度系数           
                    >> m_Data[i].wAttackRvs         // 武功攻击修正           
                    >> m_Data[i].wDemMPToHPRate     // 吸收内力转生命比率    
                    >> m_Data[i].wZeroSPOdds        // 直接将体力变成零的几率 
                    >> m_Data[i].wAddHPMaxTime      // 提升HP时间 
                    >> m_Data[i].wAddHPMaxRate;     // 提升HP比率
       i ++;
       if( i>=iDataCount ) break;
    } 
    FileStream.close();

    // 读取基本武功
    FileStream.open( FILENAME_BASEF );
    if( !FileStream.is_open() )
    {
        rfalse( "无法打开[%s]基本攻击数据文件！",FILENAME_BASEF );
        return   true;
    }

  	while (!FileStream.eof())
	{
		FileStream >> szBuf;
		if (strcmp(szBuf, "·") == 0)
			break;
	}
    i = 0;
    while( !FileStream.eof() )
    {
        FileStream  >> szBuf          // 名称
                    >> m_BaseFightList[i].wAttackTime   // 攻击速度（整个攻击的总帧数）
                    >> m_BaseFightList[i].wHitTime      // 命中速度（在第几帧命中）
                    >> m_BaseFightList[i].byNextTime    // 下次攻击时间间隔
                    >> m_BaseFightList[i].byDist;       // 攻击距离
        i++;
        if( i>=6 )  break;
    }
    FileStream.close();

    // 读取攻击范围矩阵
    FileStream.open( FILENAME_ATTACKMATIX );
    if( !FileStream.is_open() )
    {
        rfalse( "无法打开[%s]攻击范围矩阵数据文件！",FILENAME_ATTACKMATIX );
        return   true;
    }

 // 	while (!FileStream.eof())
	//{
	//	FileStream >> szBuf;
	//	if (strcmp(szBuf, "·") == 0)
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

    if( iSkillID<2 || iSkillID>5 )      return  NULL;   // 不在范围攻击的招式里面

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
            return  0;   // 不在范围攻击的招式里面
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

    // 读取特殊掉落物品列表 
    FileStream.open( FILENAME_SITEM );
    if( !FileStream.is_open() )
    {
        rfalse( "无法打开[%s]特殊掉落物品数据列表文件！", FILENAME_SITEM );
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

    // 读取随机掉落物品列表 
    FileStream.open( FILENAME_DRITEM );
    if( !FileStream.is_open() )
    {
        rfalse( "无法打开[%s]特殊掉落物品数据列表文件！", FILENAME_DRITEM );
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
