#include "StdAfx.h"
#include <map>
#include "monster.h"
#include "gameworld.h"
#include "area.h"
#include "item.h"
#include "SingleItem.h"
#include "region.h"
#include "Player.h"
#include "Trigger.h"
#include "ScriptManager.h"
extern CScriptManager g_Script;

#include "TeamManager.h"
#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/upgrademsgs.h"
#include "NetWorkModule/TeamMsgs.h"
#include "���������/Environment.h"
#include "DSpcialItemData.h"
#include "BuffBaseData.h"
#include "Random.h"
#include "CMonsterService.h"
#include "CItemDropService.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "CMonsterDefine.h"
#include "Pet.h"
#include "DynamicRegion.h"
#include "DynamicDailyArea.h"
#include "CSkillService.h"

static long s_lMonsterSize = 0;                   // ������������б��С

static SMonsterSkill m_SMSData[MAX_MSKILLCOUNT];  // ���＼������

DWORD CMonster::monsterDeadMoney = 0;

int MutateRank[10] = {18182, 36364, 54545, 72727, 90909, 109091, 127273, 145455, 163636, 181818};

extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern LPIObject GetPlayerByGID(DWORD);
extern LPIObject GetObjectByGID(DWORD GID);
extern float GetDirection(float x1, float y1, float x2, float y2);

// �������ĸ��ȼ���������
int  g_LevelAttributesArray[ 4 ][ 3 ];
int LoadLevelAttributeArray( void )
{
    memset( g_LevelAttributesArray, -1, sizeof( g_LevelAttributesArray ) );
    dwt::ifstream file( "data\\LevelAttributesList.txt" );
    int AttrIndex = 0;
    if ( file.is_open() )
    {
        int i = 0, j = 0;
        for ( i = 0; i < 4; ++i )
        {
            for ( j = 0; j < 3; ++j )
            {
                if ( file.eof() )
                    break;

                file >> AttrIndex;
                if ( AttrIndex < 0 || AttrIndex >= 57 )
                {
                    file.close();
                    return rfalse( 2, 1, "��ȡ�����ȼ������б��ļ�����" );
                }
                g_LevelAttributesArray[ i ][ j ] = AttrIndex;
            }
        }

        if ( i != 4 || j != 3 )
        {
            file.close();
            return rfalse( 2, 1, "��ȡ�����ȼ������б��ļ�����" );
        }
    }
    else
        return rfalse( 2, 1, "�޷��򿪼����ȼ������б��ļ���" );
    
    file.close();
    return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------------
//��������
//--------------------------------------------------------------------------------------------------------------------------------------

//����������Ը������ʱ�
int CMonster::LoadEquipDropAttNum()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\���Ը������ʱ�.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "�޷���[Data\\item\\dropEquipTable\\���Ը������ʱ�.txt]�ļ���");

	char linemax[2048],tmp[32];
	int maxid = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> tmp;
		int i = 0,tmpMem[512],MaxNum = 0;
		memset(&tmpMem,0,sizeof(tmpMem));
		while ( !linebuf.eof())
		{
			linebuf >> tmpMem[i++];
		}
		g_wAttNumOdds = new WORD[i];

		i = 0;
		while ( tmpMem[i] != 0)
		{
			g_wAttNumOdds[i] = tmpMem[i++];
		}	
	}
	stream.close();
	return 1;
}

//��������������ͼ��ʱ�
int CMonster::LoadEquipDropAttType()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\���Գ��ּ��ʱ�.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "�޷���[Data\\item\\dropEquipTable\\���Գ��ּ��ʱ�.txt]�ļ���");

	char linemax[2048];
	int maxid = 0,i = 0;

	g_sAttTypeOdds = new SEquipAttTypeOddsData[Max_Att_Num];

	memset(g_sAttTypeOdds, 0, sizeof(SEquipAttTypeOddsData) * Max_Att_Num);

	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> g_sAttTypeOdds[i].name;
		linebuf >>  g_sAttTypeOdds[i++].OddsValue;
	}
	stream.close();
	return 1;
}

//����װ��ÿ������ȡÿ��ֵ�ļ���
int CMonster::LoadEquipDropAttVale()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\������Ե�������ֵ.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "�޷���[Data\\item\\dropEquipTable\\������Ե�������ֵ.txt]�ļ���");

	g_sAttValueOdds = new SEquipAttValueOddsData[Max_Att_Num];
	memset(g_sAttValueOdds, 0, sizeof(SEquipAttValueOddsData) * Max_Att_Num);

	char linemax[2048];
	int maxid = 0,i = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		int id = 0;
		linebuf >> g_sAttValueOdds[i].name;
		while(!linebuf.eof())
		{
			linebuf >>  g_sAttValueOdds[i].OddsValue[id++];
		}
		i++;
	}
	stream.close();
	return 1;
}

//����װ������������������
int CMonster::LoadEquipDropAttRule()
{
	dwt::ifstream stream("Data\\item\\dropEquipTable\\װ���������������.txt");
	if (!stream.is_open())
		return rfalse(0, 0, "�޷���[Data\\item\\dropEquipTable\\װ���������������.txt]�ļ���");

	g_sAttNumRule = new SEquipAttNumRuleData[Max_Att_Num_Rule];
	memset(g_sAttNumRule, 0, sizeof(SEquipAttNumRuleData) * Max_Att_Num_Rule);

	char linemax[2048];
	int tmp = 0,i = 0;
	while ( !stream.eof() ) {
		stream.getline( linemax, sizeof( linemax ) );
		if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//'  )
			continue;

		std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );

		linebuf >> tmp >> g_sAttNumRule[i].levle >> g_sAttNumRule[i++].number;	
	}
	stream.close();
	return 1;
}

//������������ȫ�ֵ��ߵ����
int CMonster::LoadItemDropTable()
{
	//memset(s_pDropTable, 0, sizeof(SDropTable) * 256);

	static WORD itemId = 0;
	static char str[128];
	g_vItemDropIdTable.clear();
	dwt::ifstream stream("Data\\item\\xysjDropItemTable\\�������ñ�.def");
	if (!stream.is_open())
	{
		return rfalse(0, 0, "�޷���[Data\\item\\xysjDropItemTable\\�������ñ�.def]���ߵ��������б��ļ���");
	}
	
	while (!stream.eof())
	{	
		ITEMDROPTABLE curDropTable;
		SItemDropTableData curDataStruct;

		curDropTable.clear();
		std::string fname( "Data\\item\\xysjDropItemTable\\" );
		stream >> str;
		fname += str;
		dwt::ifstream tfile( fname.c_str() );
		if (!tfile.is_open())
		{
			rfalse(0, 0, "�޷���[%s]���ߵ��������б��ļ���", fname.c_str());
		}
		else
		{
			int i = 0;
			while (!tfile.eof())
			{
				char tmp[1024];
				SItemOddsTabledata curTable;
				tfile.getline(tmp,sizeof(tmp));
				std::strstream lineStream( tmp, ( int )strlen( tmp ), ios_base::in );

				lineStream >> curTable.itemId >> curTable.wOdds;
				curDropTable.push_back(curTable);	

				//�鿴������ȷ��д��
				itemId = (WORD)curDropTable.size();
				itemId = curDropTable[i++].itemId;
			}
			
			tfile.close();
		}
		char tmp[64] = "";
		_snprintf(tmp,5,str);
		curDataStruct.itemId	  = atoi(tmp);
		curDataStruct.itemIdTable = curDropTable;
		g_vItemDropIdTable.push_back(curDataStruct);
	}

	stream.close();

	return 1;
}

//������������������������Ʒ��
int CMonster::LoadTaskDropTable()
{

	static WORD itemId = 0;
	static char str[128];
	g_vTaskDropIdTable.clear();
	dwt::ifstream stream("Data\\item\\xysjTaskDropTable\\�������ñ�.def");
	if (!stream.is_open())
	{
		return rfalse(0, 0, "�޷���[Data\\item\\xysjTaskDropTable\\�������ñ�.def]���ߵ��������б��ļ���");
	}

	while (!stream.eof())
	{	
		ITEMDROPTABLE curDropTable;
		SItemDropTableData curDataStruct;

		curDropTable.clear();
		std::string fname( "Data\\item\\xysjTaskDropTable\\" );
		stream >> str;
		fname += str;
		dwt::ifstream tfile( fname.c_str() );
		if (!tfile.is_open())
		{
			rfalse(0, 0, "�޷���[%s]���ߵ��������б��ļ���", fname.c_str());
		}
		else
		{
			int i = 0;
			while (!tfile.eof())
			{
				char tmp[1024];
				SItemOddsTabledata curTable;
				tfile.getline(tmp,sizeof(tmp));
				std::strstream lineStream( tmp, ( int )strlen( tmp ), ios_base::in );

				lineStream >> curTable.itemId >> curTable.wOdds;
				curDropTable.push_back(curTable);	

				//�鿴������ȷ��д��
				itemId = (WORD)curDropTable.size();
				itemId = curDropTable[i++].itemId;
			}

			tfile.close();
		}
		char tmp[64] = "";
		_snprintf(tmp,5,str);
		curDataStruct.itemId	  = atoi(tmp);
		curDataStruct.itemIdTable = curDropTable;
		g_vTaskDropIdTable.push_back(curDataStruct);
	}

	stream.close();

	return 1;
}

int CMonster::LoadEquipDropAttributeTable()
{
	//m_wAttNumOdds
	//m_wAttTypeOdds
	//m_wAttValueOdds

	//����������Ը������ʱ�
	LoadEquipDropAttNum();

	//��������������ͼ��ʱ�
	LoadEquipDropAttType();

	//����װ��ÿ������ȡÿ��ֵ�ļ���
	LoadEquipDropAttVale();

	//����װ������������������
	LoadEquipDropAttRule();

	//������������ȫ�ֵ��ߵ����
	LoadItemDropTable();

	//���������������������Ʒ��
	LoadTaskDropTable();
	return 1;
}
//----------------------------------------------------------------------------------------------------------	
int CMonster::LoadMonsterData()
{
//     // ���õط����ù�
//     void LoadDropItemData();
//     LoadDropItemData();
// 
// 	//���ص���װ������������ñ�
// 	LoadEquipDropAttributeTable();
// 
// 	//
// 	static char str[128];
// 	dwt::ifstream stream("Data\\�������ñ�.txt");
// 	if (!stream.is_open())
// 		return rfalse(0, 0, "�޷���[Data\\�������ñ�.txt]���������б��ļ���");
// 
//     // �µĹ����
//     // ��� ���� ͼ���� �ȼ� �Ƿ���BOSS 
//     // ����ֵ �⹦���� �ڹ����� ������� �ڷ����� 
//     // ���⹥���������ӷ����� �书���� �书ʹ�ü��� �� �������[�����ٶ�] ����ʱ�� ������Χ 
//     // �����⹦ɫ�ּ��� �����ڹ�ɫ�ּ��� �������ɫ�ֱ��� �����ڷ�ɫ�ּ��� ������Ǯɫ�֡����� 
//     // Я������ֵ Я����Ǯ Я������ֵ Я����Ʒ1�б� ��Ʒ1������� ��Ʒ1���������� Я����Ʒ�б�n ��Ʒ�������n ��Ʒn����������...
// 
//     std::list< SMonsterData > lst;
//     char linemax[2048];
//     int maxid = 0;
//     while ( !stream.eof() ) {
//         stream.getline( linemax, sizeof( linemax ) );
//         if ( linemax[0] == 0 || ( *(LPWORD)linemax ) == '//' || ( *(LPWORD)linemax ) == '--' )
//             continue;
// 
//         std::strstream linebuf( linemax, (std::streamsize)strlen( linemax ) );
//         SMonsterData d;
// 		memset(&d, 0, sizeof(SMonsterData));
//         linebuf >> d.id >> str >> d.image >> d.iHeadImage >> d.level >> d.boss;
//         linebuf >> d.hp >> d.outAtkBase >> d.innerAtkBase >> d.outDefBase >> d.innerDefBase;
// 		linebuf >> d.outAtkChance >> d.innerAtkChance >> d.avgAtkChance;
//         linebuf >> d.specAtk >> d.bWeaponOdds>> d.specPro>> d.bArmourOdds>>d.skillScale >> d.skillRate >> d.agility >> d.hitSpeed >> d.hitTick >> d.hitArea;
//         linebuf >> d.ChangeRate;
//         linebuf >> d.exp >> d.money >> d.xydvalue;
// 		for (int i = 0;i < 5;++i)
// 		{
// 			linebuf >> d.skillInfo[i][0] >> d.skillInfo[i][1];
// 		}
// 		for (int i = 0;i < 8;++i)
// 		{
// 			linebuf >> d.moveBuffInfo[i][0] >> d.moveBuffInfo[i][1];
// 		}
// 		linebuf >> d.scriptId >> d.dropMaxNum >> d.dropTaskTable;
// 
//         strncpy( d.name, str, sizeof( d.name ) );
//         d.name[ sizeof( d.name ) - 1 ] = 0;
//         int c = 0;
//         while ( !linebuf.eof() && c < ( sizeof(d.dropTable) / sizeof(d.dropTable[0]) ) ) {
//             linebuf >> d.dropTable[c][0];
//             linebuf >> d.dropTable[c][1];
//             if ( d.dropTable[c][1] == 0 )
//                 break;
//             c++;
//         }
// 		linebuf >> d.extraScriptId;
// 
//         if ( d.id > maxid )
//             maxid = d.id;
//         lst.push_back( d );
//     }
// 
//     if ( maxid > (int)lst.size() * 2 )
//         return rfalse(0, 0, "�����б���������ID[%d] > MAXIDSIZE[%d]", maxid, lst.size() * 2);
// 
//     s_lMonsterSize = maxid+1;
// 	s_pMonsterDatas = new SMonsterData[s_lMonsterSize];
//     memset(s_pMonsterDatas, 0, sizeof(SMonsterData) * s_lMonsterSize);
//     for ( std::list< SMonsterData >::iterator it = lst.begin(); it != lst.end(); it++ )
//         s_pMonsterDatas[it->id] = *it;
// 
// 	stream.close();

	return 1;
}

//----------------------------------------------------------------------------------------------------------	
// SMonsterData *CMonster::GetMonsterData(int index)
// {
// 	if (index >= s_lMonsterSize) 
//         return NULL;
// 
// 	return &s_pMonsterDatas[index];
// }

//----------------------------------------------------------------------------------------------------------	
void CMonster::ClearMonsterData()
{
//	if (s_pMonsterDatas != NULL) 
//		delete s_pMonsterDatas;

    s_lMonsterSize = 0;
//    s_pMonsterDatas = NULL;

	if(g_wAttNumOdds != NULL)
		delete []g_wAttNumOdds;
	g_wAttNumOdds = NULL;

	if(g_sAttTypeOdds != NULL)
		delete []g_sAttTypeOdds;
	g_sAttTypeOdds = NULL;

	if(g_sAttValueOdds != NULL)
		delete []g_sAttValueOdds;
	g_sAttValueOdds = NULL;

	if(g_sAttNumRule != NULL)
		delete []g_sAttNumRule;
	g_sAttNumRule = NULL;
}

//----------------------------------------------------------------------------------------------------------	
void CMonster::LoadMonsterSkillData()   
{
    int         i, iDataCount;
    dwt::ifstream    FileStream( FILENAME_MSKILL );
    char        szBuf[256];

    if( !FileStream.is_open() )
    {
        rfalse( "�޷���[%s]�ڹ������б��ļ���", FILENAME_MSKILL );
        return;
    }

    while (!FileStream.eof())
    {
        FileStream >> szBuf;
        if (strcmp(szBuf, "��") == 0)
			break;
	}
    i = 0;
    iDataCount = MAX_MSKILLCOUNT;
    char tmpStr[1024];
    while( !FileStream.eof() )
    {
        FileStream.getline( tmpStr, sizeof( tmpStr ) );
        if ( tmpStr[0] == 0 )
            continue;

        std::strstream FileStream( tmpStr, (std::streamsize)strlen( tmpStr ) );

        DWORD dwTemp = 0;
        char nameTemp[16] = "";

        m_SMSData[i].type = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ��������	

        FileStream >> nameTemp;
        if ( nameTemp[0] == 0 )
            continue;
        strncpy( m_SMSData[i].szName, nameTemp, 15 );               // ����[δ��]
            m_SMSData[i].szName[15] = '\0';

        m_SMSData[i].effectId        = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ��Ӧ����
        m_SMSData[i].rating          = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ��1/10000ΪС����λ,������ָ���               
        m_SMSData[i].vitalsodds      = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // �����������ֵ�ᱻ����ķ��ֿ�(Ӱ�����и���)   
        m_SMSData[i].effectTime      = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ����ʱ��                       
        m_SMSData[i].waiting         = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // �ӷ��������еĵȴ�ʱ��
                                                                                              
        m_SMSData[i].wRedDefRate     = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ���ͷ���������p                       
        m_SMSData[i].wRedAgiRate     = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ���������ȱ���p                           
        m_SMSData[i].wToxicRedHPRate = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // �ж�����������p             
        m_SMSData[i].wDemToHPRate    = (WORD)( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // �����˺�ת��������   
        m_SMSData[i].dwDamageArea    = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // Ⱥ�˵Ĺ�����Χ
        m_SMSData[i].dwDamageTotal   = ( ( FileStream >> dwTemp ), dwTemp ); dwTemp = 0;    // ���˺�

        m_SMSData[i].dwSkillTimes = -1;  // ��ʼ״̬
        i ++;
        if( i>=iDataCount ) break;
    } 
    FileStream.close();
}

//----------------------------------------------------------------------------------------------------------	
SMonsterSkill *CMonster::GetMonsterSkillData(int SkillType)
{
    if( SkillType < 1 || SkillType > MAX_MSKILLCOUNT )   return  NULL;

    return  &m_SMSData[ SkillType - 1 ]; 
}

int CMonster::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter*)pParameter;

	if (!pParam)
		return 0;

    memset(&m_Property, 0, sizeof(SMonsterProperty));
	memset(m_Name, 0, CONST_USERNAME);

	m_Property.m_BirthPosX = pParam->wX;
	m_Property.m_BirthPosY = pParam->wY;

	// ��ȡ��������
	const SMonsterBaseData *monster = CMonsterService::GetInstance().GetMonsterBaseData(pParam->wListID);
	if (!monster)
		return 0;

	SMonsterBaseData *temp = &m_Property;
	memcpy(temp, monster, sizeof(SMonsterBaseData));
	if (pParam->PartolRadius == 0)
	{
		
	}
	else
	{
		m_Property.m_PartolRadius = pParam->PartolRadius;
	}
	if (pParam->dwRefreshTime)
		m_Property.m_ReBirthInterval = pParam->dwRefreshTime;

	m_Property.controlId = 0;
	m_wCuruseskill = 0;
	//AI����������������Ч
	memcpy(&m_AIParameter, pParam, sizeof(SParameter));
	memset(&m_AIProperty, 0, sizeof(SMonsterAIProperty));

	// �����3�ƶ�
	m_curX		= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	m_curY		= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_curZ		= 0.0f;
	m_Direction = m_Property.m_Direction;
	m_curTileX	= m_Property.m_BirthPosX;
	m_curTileY	= m_Property.m_BirthPosY;
	m_BaseSpeed	= monster->m_WalkSpeed;
	m_OriSpeed	= monster->m_WalkSpeed;
	m_Speed = (float)monster->m_WalkSpeed/300;
	if (m_Speed < 0.1f)
	{
		m_Speed += 0.1f;
	}
	

	m_MaxHp		= m_Property.m_MaxHP;
	m_MaxMp		= m_Property.m_MaxMP;
	m_Level		= m_Property.m_level;
	m_CurHp		= m_MaxHp;
	m_CurMp		= m_MaxMp;
	
	m_GongJi	= m_Property.m_GongJi;
	m_FangYu	= m_Property.m_FangYu;
	m_BaoJi		= m_Property.m_BaoJi;
	m_ShanBi	= m_Property.m_ShanBi;
	m_AtkSpeed	= m_Property.m_AtkSpeed;
	m_StaticpreUpdateTime = 0;

	//��ҹ�������Գ�ʼ��
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = m_Property.m_uncrit;
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = m_Property.m_wreck;
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = m_Property.m_unwreck;
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = m_Property.m_puncture;
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = m_Property.m_unpuncture;
	m_Hit = m_Property.m_Hit;
// 	m_MaxHp	= m_Property.m_MaxHP;
// 	m_CurHp = m_MaxHp;
// 	m_MaxMp = m_Property.m_MaxMP;
// 	m_CurMp = m_MaxMp;
// 	m_Level	= m_Property.m_level;
// 	

// 
// 	m_PhyEscapeFix	= 0;
// 	m_PhyHitFix		= 0;
// 	m_PhyCriHitFix	= 0;
// 	m_PhyDamageFix	= 0;
// 	m_PhyDefenceFix	= 0;
// 	m_FpEscapeFix	= 0;
// 	m_FpHitFix		= 0;
// 	m_FpCriHitFix	= 0;
// 	m_FpDamageFix	= 0;
// 	m_FpDefenceFix	= 0;

	// �տ�ʼ�����AIΪվ�������Գ�ʼ��վ����ʱ��
	m_ActionFrame		= 5/*CRandom::RandRange(100, 150)*/;
	m_SearchTargetTime	= timeGetTime();
	m_BackProtection	= false;

    m_bStopToStand = FALSE;

    m_dwEndAttackTime = 0;

    m_bDeadEvent = 0;
    m_dwDeadDelayScript = 0;
    m_dwKillerPlayerGID = 0;
    m_byMutateState = 0;			
    m_wDeadTimes = 0;
    targetDamage = 0;
    targetSegTime = 0;
	m_nlastdropTime=0;

    bySelectType = 0;        
    byPriorityExceptType = 0;
    dwPriorityTarget = 0xffffffff;  

    m_dwTelesportTime = 0;
	
	///���ڹ���AIʱ�����
	m_nAttackTime=0;
	m_nRandomSkillIndex=0;
	m_nMoveStartTime = 0;
	///��¼��һ�η��͵�Ŀ�����Ϣ
	m_vdesPos = new D3DXVECTOR2;

	///����ͼ��ײ�赲��ʱ
	m_nCollisionTime = 0;
	// ��ʼ������ID
	int i=0;
	while(i<MAX_MONTER_SKILLCOUNT)
	{
		m_pSkills[i].wTypeID		= monster->m_SkillID[i];
		m_pSkills[i].byLevel		= 1;
		m_pSkills[i].dwProficiency	= 0;
		i++;
	}

	m_gmakeparm1 = 1;
	m_gmakeparm2 = 1;
	m_gmakeparm3 = 1;
	m_gmakeparm4 = 1;
	m_CopySceneGID = 0;

	m_dCreatePlayerID = 0; //����Ĵ�����
	m_dCreatePlayerID = pParam->wCreatePlayerID;
	m_dRankIndex = 0;
	m_dRankIndex = pParam->dRankIndex;
	m_ShowState = 0;
	strcpy(m_Name, m_Property.m_Name);
	m_Level = m_Property.m_level;
	m_dModID = m_Property.m_LookID;
	//��ʼ������ű��¼�
	InitScriptEvent();


	if (IsBoss()) {	//����BOSS
		UpdateMyData(pParam->reginID,0,0,"��",1);
	}

	

	return CFightObject::OnCreate(pParameter);
}



void CMonster::Thinking()
{
	///����1���ӵ���һ�νű�
// 	if(GetTickCount()-m_nBirthTime>60000)
// 	{
// 	///��ʱ�����³�ʼ���Ľű�����
// 		g_Script.SetCondition(this, NULL, NULL);
// 		g_Script.CallFunc("AttackMonsterScript");
// 		g_Script.CleanCondition();
// 		m_nBirthTime=GetTickCount();
// 	}

	if (m_BackProtection)
		return;

	const SMonsterBaseData *pData = CMonsterService::GetInstance().GetMonsterBaseData(m_Property.m_ID);
	if (!pData)
		return;

	switch (GetCurActionID())
	{
	case EA_STAND:
		if (0xffffffff == m_SearchTargetTime)
		{
			m_SearchTargetTime = timeGetTime();
//			rfalse(2, 1, "������ϴ���������ʼ�µ���������T:%d��", m_SearchTargetTime);
		}

		// ������ƶ����򱣳�վ��
		if (MMT_NOMOVE == pData->m_MoveType)
			break;

		// ����Ǳ������������ҵ�ǰ��Ŀ���ڹ�������ôת�빥������
		if (MAT_PASSIVE == pData->m_AttType && 0 != m_dwLastEnemyID)
			break;

		if (MAT_ACTIVELY == pData->m_AttType && 0 == m_dwLastEnemyID && 0xffffffff != m_SearchTargetTime && 
								timeGetTime()-m_SearchTargetTime>5000 && m_ParentRegion && m_ParentArea)
		{
			m_SearchTargetTime = 0xffffffff;

			// ��Ѱһ��Ŀ��
			check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);
			
			if (playerList)
			{
				for (check_list<LPIObject>::iterator it = playerList->begin(); it != playerList->end(); ++it)
				{
					LPIObject target = *it;
					CPlayer *player = (CPlayer *)target->DynamicCast(IID_PLAYER);
					if (player && 0 != player->m_CurHp)
					{
						m_dwLastEnemyID = player->GetGID();	
//						rfalse(2, 1, "�ҵ�һ��û����Ŀ�꡾%d����׼�����𹥻���", m_dwLastEnemyID);
						break;
					}
				}				
			}
		}

		if (--m_ActionFrame <= 0)
		{
			if ((m_fightState & FS_DIZZY) || (m_fightState & FS_DINGSHENG))
			{
				m_ActionFrame = 1;
				return;
			}

			// վ����ʱ���������ʼ����ƶ�
			SQSynPathMsg msg;
			
			MakeRadomPath(m_Property.m_ActiveRadius, m_Property.m_BirthPosX << TILE_BITH, m_Property.m_BirthPosY << TILE_BITH, msg);
			
			// �������·��ʧ�ܣ������վ��
			if (!SetMovePath(&msg))
				m_ActionFrame = CRandom::RandRange(m_Property.m_StayTimeMin, m_Property.m_StayTimeMax);
			
			return;
		}
		break;
	
	case EA_RUN:
		// ������ƶ������У����˹�������ôת�빥������
		if (SQSynPathMsg::normal == m_IsMove2Attack && 0 != m_dwLastEnemyID)
		{
			SetCurActionID(EA_STAND);
			SetBackupActionID(EA_STAND);
		}
		break;

	case EA_DEAD:
		MY_ASSERT(0 == m_CurHp);

		if (--m_ActionFrame <= 0)
			SetRenascence(0, 0, m_MaxHp, 2);
		break;

	default:
		break;
	}

	// �ж��Ƿ������ڹ���
	if (GetCurActionID() == EA_STAND && m_dwLastEnemyID != 0 && MAT_RUBBER != pData->m_AttType)
	{
		///�ڶ��鹥��ģʽ���漴�л�Ŀ��
// 		LPIObject tempobject=m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
// 		if (!tempobject)return;
// 		CPlayer *pDestattk = (CPlayer *)tempobject->DynamicCast(IID_PLAYER);
// 		if (pDestattk&&pDestattk->m_dwTeamID>0&&pDestattk->m_CurHp>0)
// 		{
// 			///�����������ʼ���㹥����ʼʱ��
// 			if (m_nAttackTime==0)m_nAttackTime=timeGetTime();
// 
// 			///����5���漴�л�һ��Ŀ��
// 			if (m_nAttackTime>0&&timeGetTime()-m_nAttackTime>5000)
// 			{
// 				GetRandomEnemyID();
// 				m_nAttackTime=timeGetTime();
// 				m_nRandomSkillIndex=rand()%5;
// 			}
// 		}

		LPIObject object=m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
		if (!object)return;
		
		float DefenderWorldPosX = 0.0f;
		float DefenderWorldPosY = 0.0f;

		CPlayer *pDest = (CPlayer *)object->DynamicCast(IID_PLAYER);
		CFightPet	*pfpet =  (CFightPet *)object->DynamicCast(IID_FIGHT_PET);
		if  (pDest && pDest->m_CurHp > 0)
		{
			DefenderWorldPosX = pDest->m_curX;
			DefenderWorldPosY = pDest->m_curY;
		}

		//2014/06/10 ly ע�͵�����ϣ�����﹥������
		//else if(pfpet && pfpet->m_CurHp > 0)
		//{
		//	DefenderWorldPosX = pfpet->m_curX;
		//	DefenderWorldPosY = pfpet->m_curY;
		//}
		else
		{
			m_dwLastEnemyID = 0;
		}

		if(m_dwLastEnemyID == 0)return;

		SQuestSkill_C2S_MsgBody msg;
		msg.mDefenderGID				= m_dwLastEnemyID;
		msg.mAttackerGID				= GetGID();
		msg.mDefenderWorldPosX			= DefenderWorldPosX;
		msg.mDefenderWorldPosY			= DefenderWorldPosY;
		msg.dwSkillIndex				= m_nRandomSkillIndex;

		ProcessQuestSkill(&msg);
		
	}

}

SASynMonsterMsg *CMonster::GetStateMsg()
{
    static SASynMonsterMsg msg;

	msg.dwGlobalID		= GetGID();
	msg.bChangeType		= m_byMutateState;
	msg.bMonsterType = m_Property.m_LevelType;
	//msg.mMoveSpeed = (float)m_Property.m_FightSpeed/1000;
	msg.mMoveSpeed = (float)m_Property.m_WalkSpeed ;
	msg.direction		= m_Direction;
	msg.ilevel = m_Level;
	msg.mImageID = m_dModID;
	msg.monsterID = m_Property.m_ID;
	msg.fScale			= m_Property.m_BodySize/100.0f;
	msg.dfightState = m_fightState;
	dwt::strcpy(msg.sName, m_Name, strlen(m_Name)+1);
	FillSynPos(&msg.ssp);

// 	char s[255]; memset(s, 0, 255);
// 	rfalse(2,1,"SASynMonsterMsg | GID:%d ", msg.dwGlobalID);
// 	for (int i = 0; i < msg.ssp.m_wayPoints; i++)
// 	{
// 		sprintf(s, " -- Idx=%d,X=%4.2f,Y=%4.2f", i , msg.ssp.m_path[i].m_X, msg.ssp.m_path[i].m_Y);
// 		rfalse(2,1,s);
// 	}

    return &msg;
}

BOOL CMonster::GetDropItem(CPlayer *Killer)
{
	if (!m_ParentRegion || !Killer)
		return FALSE;

	if (m_dCreatePlayerID)
	{
		CPlayer *player = (CPlayer*)GetPlayerByGID(m_dCreatePlayerID)->DynamicCast(IID_PLAYER);
		if (player)
		{
			Killer = player;
			m_dCreatePlayerID = 0;
		}
	}

	CItem::SParameter args;
	std::vector<SRawItemBuffer> item;

	const SMonsterDropTable *MDTable = CItemDropService::GetInstance().GetMonsterDropTable(m_Property.m_ID);
	if (!MDTable)
		return FALSE;

	WORD MoneyMountRate = CRandom::RandRange(70, 130);
	WORD tempMoney = MDTable->m_MoneyMount * MoneyMountRate / 100;

	SMonsterDropTable *mtab = const_cast<SMonsterDropTable*>(MDTable);

	if (Killer->limitedState == CPlayer::LIMITED_HALF){  //���ڷ�����ʱ�䣬�����Ǯ���ʼ���
		mtab->m_MoneyRate /= 2;
		Killer->ShowLimitTips();
	}
	else if (Killer->limitedState == CPlayer::LIMITED_ZERO) {	//����ƣ��ʱ�䣬����Ϊ0
		mtab->m_MoneyRate = 0;
		Killer->ShowLimitTips();
	}

	MDTable->m_MoneyRate > CRandom::RandRange(1, 10000) ? (args.Money = tempMoney, args.MoneyType = MDTable->m_MoneyType) 
														: (args.Money = 0, args.MoneyType = 0);

	if (args.Money)
		MY_ASSERT(args.MoneyType);

	for (int i = 0; i < 10; i++)
	{
		DWORD itemID		= MDTable->m_DropTable[i][0];
		DWORD itemRate		= MDTable->m_DropTable[i][1];
		DWORD itemNum		= MDTable->m_DropTable[i][2];
		DWORD itemUpdate	= MDTable->m_DropTable[i][3];
		
		if (!itemID)
			continue;

		if (0 == itemNum || itemRate > 10000 || itemUpdate > 10)
			return FALSE;

		if (Killer->limitedState == CPlayer::LIMITED_HALF){  //���ڷ�����ʱ�䣬������Ʒ���ʼ���
			itemRate /= 2;
			Killer->ShowLimitTips();
		}
		else if (Killer->limitedState == CPlayer::LIMITED_ZERO) {	//����ƣ��ʱ�䣬����Ϊ0
			itemRate = 0;
			Killer->ShowLimitTips();
		}

		// ���ȶ϶����ʣ����ʧ�ܣ�����Ҫ�������������
		DWORD tempRate = CRandom::RandRange(1, 10000);
		if (tempRate > itemRate)
			continue;

		// ������Ʒ
		if (itemID >= 200000001)				// ȫ�ֵ����
		{
			const std::vector<DWORD> &temp = CItemDropService::GetInstance().GetGlobalDropTable(itemID);
			if (temp.empty())
				return FALSE;

			WORD rateNum = CRandom::RandRange(1, temp.size()) - 1;

			itemID = temp[rateNum];
		}		
		
		const SItemBaseData *pItemData = CItemService::GetInstance().GetItemBaseData(itemID);
		if (!pItemData)
			return FALSE;

		// Ҫ�����ĸ������ܴ�����Ʒ����������
		if (itemNum > pItemData->m_Overlay)
			return FALSE;

		SRawItemBuffer temp;
		BOOL rt = CItemUser::GenerateNewItem(temp, SAddItemInfo(itemID, itemNum));
		if (FALSE == rt)
			return FALSE;

		// �����������ߣ�ֱ�Ӽӵ��˵�����
		if (ITEM_IS_TASK(pItemData->m_Type))
		{
			if (pItemData->m_TaskID && TS_ACCEPTED == Killer->GetTaskStatus(pItemData->m_TaskID))
			{
				if (Killer->GetItemNum(pItemData->m_ID, XYD_FT_WHATEVER) < pItemData->m_TaskNeed)
				{
					std::list<SAddItemInfo> itemList;
					itemList.push_back(SAddItemInfo(pItemData->m_ID, 1));		// �������ֻ�ܵ���1��

					if (Killer->CanAddItems(itemList))
						Killer->StartAddItems(itemList);
				}
			}
		}
		else
			item.push_back(temp);
	}
	
	if (item.empty() && 0 == args.Money)	// û�����ɵ�
		return FALSE;

	CSingleItem::SParameter Singleargs;

	if (args.Money)
	{
		SRawItemBuffer itemTemp;
		int moneyid = Killer->_L_GetLuaValue("OnGetMoneyItemID"); //�ж��Ƿ�Ϊ��Ǯ����
		if (!moneyid)return FALSE;
		
		BOOL rt = CItemUser::GenerateNewItem(itemTemp, SAddItemInfo(moneyid, args.Money)); //�����Ǯ��Ϊһ������ĵ��ߴ���
		if (FALSE == rt)
			return FALSE;

		Singleargs.PackageModel = IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=Killer->GetGID();
		Singleargs.ProtectTeamID	= Killer->m_dwTeamID;
		Singleargs.MoneyType = args.MoneyType;
		
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "�������")))
			return FALSE; 
	}

	std::vector<SRawItemBuffer>::iterator iter = item.begin();
	while(iter != item.end())
	{
		SRawItemBuffer itemTemp = *iter;
	

		Singleargs.PackageModel = IPM_TANMUBOX;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=Killer->GetGID();
		Singleargs.ProtectTeamID	= Killer->m_dwTeamID;
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "�������")))
			return FALSE; 

		iter++;
	}

	//if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, args, item, LogInfo(m_Property.m_ID, "�������")))
		//return FALSE; 
	return TRUE;
}

int CMonster::Attack()
{
    return WatchArea();
}

int CMonster::WatchArea()
{
    if (m_ParentRegion == NULL)
        return 0;

    int iRet = 0;

    /*

    WORD Area_X;
    WORD Area_Y;

    WORD NowX;
    WORD NowY;

    WORD DistanceNow;
    WORD DistanceBest;

    CArea * pArea;
    CPlayer* pTarget = NULL;          // ѡ���Ŀ��
    CPlayer* pPriorityTarget = NULL;  // ���ڼ�¼���ȵ�Ŀ��

    Area_X = m_ParentArea->m_X;
    Area_Y = m_ParentArea->m_Y;

    int ix;
    int iy;
    DistanceBest = 20;
    //Search around 9 areas for life 
    for (int iLooper = 0;iLooper < 9 ;iLooper ++)
    {
        switch(iLooper) 
        {
        case 0:
            ix = 0;
            iy = 0;
            break;
        case 1:
            ix = 0;
            iy = -1;
            break;
        case 2:
            ix = 1;
            iy = -1;
            break;
        case 3:
            ix = 1;
            iy = 0;
            break;
        case 4:
            ix = 1;
            iy = 1;
            break;
        case 5:
            ix = 0;
            iy = 1;
            break;
        case 6:
            ix = -1;
            iy = 1 ;
            break;
        case 7:
            ix = -1;
            iy = 0;
            break;
        case 8:
            ix = -1;
            iy = -1;
            break;
        default:
            break;
        }

        pArea = ( CArea* )m_ParentRegion->GetArea( Area_X + ix ,Area_Y + iy )->DynamicCast( IID_AREA );
        if ( pArea == NULL)
            continue;

        check_list< LPIObject >::iterator it = pArea->m_PlayerList.begin();
        while ( it != pArea->m_PlayerList.end() )
        {
            pTarget = ( CPlayer* )( *it )->DynamicCast( IID_PLAYER );
            
            // ���ﲻ��������������GM
            if ( ( pTarget->m_Property.m_GMLevel != 0 ) && ( ( LPBYTE )&pTarget->m_Property.m_dwConsumePoint )[ 4 ] == 0 )
            {
                it ++;
                continue;
            }

            BOOL isPriority = ( bySelectType == 1 );  // �Ƿ�Ϊ����
            BOOL isExcept   = ( bySelectType == 2 );  // �Ƿ�Ϊ�ų�
            BOOL type       = FALSE;
            switch ( byPriorityExceptType ) // ���Ȼ��ų�ѡ��Ŀ�����ͣ� 1,��ɫ  2,����  3,�Ա�  4,����  5,����
            {
            case 1:  // ĳ����ɫ���Ȼ��ų�
                type = ( pTarget->m_Property.m_dwStaticID == dwPriorityTarget );
                break;
            case 2: //  ĳ���������Ȼ��ų�
                type = ( pTarget->m_Property.m_School == ( BYTE )dwPriorityTarget );
                break;
            case 3: //  ĳ���Ա����Ȼ��ų�
                type = ( pTarget->m_Property.m_Sex == ( BYTE )dwPriorityTarget );
                break;
            case 4: //  ĳ���������Ȼ��ų�
                type = ( pTarget->m_Property.factionId == ( WORD )dwPriorityTarget );
                break;
            case 5: //  ĳ���������Ȼ��ų�
                type = ( pTarget->m_dwTeamID == dwPriorityTarget );
                break;
            }

            if ( type && isExcept )  // ������ų���ֱ�ӽ�����һ��ѭ��
            {
                it++;
                continue;
            }

			pTarget->GetCurPos( NowX, NowY );

			DistanceNow = max( abs( NowX - m_wCurX ), abs( NowY - m_wCurY ) );
			if ( ( DistanceNow < DistanceBest ) && ( DistanceNow <= m_Property.m_wMoveArea ) && ( pTarget->m_CurHp != 0 ) )
			{
                if ( type && isPriority )  // �˶���Ϊ���ȶ���,��¼����
                    pPriorityTarget = pTarget;

                m_pAttackTarget = pTarget;
				targetDamage = 0;
				targetSegTime = timeGetTime() + ( rand() % 10000 );

				m_dwEndAttackTime = timeGetTime() + 40000;

				iRet = 1;
				DistanceBest = DistanceNow;
			}
            it++;
        }

        if ( pPriorityTarget != NULL )         // ��������ȵĶ��󣬸ı�֮ǰѡ��Ŀ���Ϊ��ͨ�����ָ��
            m_pAttackTarget = pPriorityTarget; // �������ʱ��̣ܶ��Ͳ������¼�¼m_dwEndAttackTimeʱ����
    }
*/
    return iRet;
}


//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::CheckTarget(void)
{
	/*
    if (m_ParentRegion == NULL)
        return false;

    if (m_dwLastEnemyID != 0)
    {
        m_pAttackTarget = m_ParentRegion->SearchObjectListInAreas(m_dwLastEnemyID, m_ParentArea->m_X, m_ParentArea->m_Y);
        m_dwLastEnemyID = 0;
        targetDamage = 0;
        targetSegTime = timeGetTime() + ( rand() % 10000 );
        m_fAttackWaitFrames = (float)(m_Property.hitSpeed >> 1);
        return true;
    }//*/

    return false;
}

bool CMonster::StopTracing()
{
	// ���ֹͣ׷�ٵ�����
	float m_BX	= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	float m_BY	= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;

	float distance = sqrt(pow((m_curX-m_BX),2)+pow((m_curY-m_BY),2));
	if ((WORD)distance > m_Property.m_TailLength)  //ֹͣ׷����ͬʱ�Ƴ�׷��
	{
		SetStopTracing(EA_RUN);
		return true;
	}


	
		CFightObject *pfight =(CFightObject*) GetObjectByGID(m_dwLastEnemyID)->DynamicCast(IID_FIGHTOBJECT);
		if (pfight)
		{
			const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(m_wCuruseskill);
			if (pData)
			{
				D3DXVECTOR2 curPos((m_curX - pfight->m_curX), (m_curY - pfight->m_curY));
				float fdistance = D3DXVec2Length(&curPos);
				if (fdistance > (pData->m_SkillMaxDistance + 100)) //����Զ
				{
					D3DXVECTOR2 vPos((pfight->m_curX - m_vdesPos->x), (pfight->m_curY - m_vdesPos->y));
					fdistance = D3DXVec2Length(&vPos);
					if (fdistance > 100) //׷��Ŀ���Ѿ��ƶ�����
					{
						SendMove2TargetForAttackMsg(pData->m_SkillMaxDistance, m_wCuruseskill, pfight);
						//rfalse(2, 1, "׷��Ŀ���Ѿ��ƶ�����");
						return false;
					}
				}
				else  //�����Ѿ���Ŀ����ӽ�
				{
					SetCurActionID(EA_STAND);
					SetBackupActionID(EA_STAND);
					Stand(EA_STAND);
					m_isRunEnd = false;
					OnRunEnd();
					//rfalse(2, 1, "�����Ѿ��ӽ�Ŀ��");
					return true;
				}

			}
			return false;
		}
	
		
	SQSynPathMsg msg;
	
	msg.m_GID		= GetGID();
	msg.m_move2What = SQSynPathMsg::attack;
	msg.m_Path.m_wayPoints		= 2;
	msg.m_Path.m_moveType		= EA_RUN;
	msg.m_Path.m_path[0].m_X	= m_curX;
	msg.m_Path.m_path[0].m_Y	= m_curY;
	msg.m_Path.m_path[1].m_X	= m_BX;
	msg.m_Path.m_path[1].m_Y	= m_BY;

	// �������·��ʧ�ܣ���˲�ƹ�ȥ
	if (!SetMovePath(&msg))
	{
		// զ��˲��ૣ�

	}
	else
	{
		m_IsMove2Attack  = SQSynPathMsg::normal;
		m_BackProtection = true;
	}

	m_dwLastEnemyID = 0;
	//m_BackProtection = true;
	//m_Speed = m_Property.m_WalkSpeed / 1000;
	return true;
}

//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::AttackTarget(void)
{
    // ��������
	/*
    if (GetCurActionID() == EA_STAND)
    {
        CFightObject *pDest = (CFightObject *)m_pAttackTarget->DynamicCast(IID_FIGHTOBJECT);
        if (pDest == NULL)
        {
            // ������񲻴���
            m_pAttackTarget = NULL;
        }
        else if (pDest->m_CurHp == 0)
        {
            // �����������
            m_pAttackTarget = NULL;
        }
        else if (pDest->m_ParentRegion != m_ParentRegion)
        {
            // ���������ͬһ������
            m_pAttackTarget = NULL;
        }
        else
        {
            WORD xTarget;
            WORD yTarget;
            // Wonly 2003-10-31 Ϊ������ұ������ƶ��󣬹��﹥�����Ҫ�ߵ��ĵ㣬ʵ������ҹ���ȥ
            pDest->GetCurPos(xTarget, yTarget);
            //pDest->GetMarkOffPos(xTarget, yTarget);
            // ����������̫Զ(1.2��)
            if ( m_Property.hitArea != 2 )
            {
                if (abs((int)xTarget - (int)m_wCurX) > (15))
                    m_pAttackTarget = NULL;
                if (abs((int)yTarget - (int)m_wCurY) > 21)
                    m_pAttackTarget = NULL;
            }
        }
        
        if (m_pAttackTarget)
        {
            WORD xTarget;
            WORD yTarget;
            // Wonly 2003-10-31 Ϊ������ұ������ƶ��󣬹��﹥�����Ҫ�ߵ��ĵ㣬ʵ������ҹ���ȥ
            pDest->GetCurPos(xTarget, yTarget);
            //pDest->GetMarkOffPos(xTarget, yTarget);
            // ��������ڹ�����Χ���ڣ���������ƶ�

            // ������޷��ƶ������ͣ���ô�Ͳ�����Ŀ���ƶ���,��һ���ƶ�����MoveToTarget���
            int ret = 0;
			ret = MoveToTarget( m_Property.hitArea );

            if ( ret == -1 && m_Property.hitArea != 2 )
            {
                // ����Ҳ����ƶ���·����ȡ����ǰ�Ĺ���Ŀ��
                m_pAttackTarget = NULL;
            }
            else if (!m_fAttackWaitFrames && ret == 0)
            {
                SQAFightObjectMsg iMsg;
                iMsg.sa.dwGlobalID = GetGID();
                iMsg.sa.wPosX = m_wCurX;
                iMsg.sa.wPosY = m_wCurY;
                iMsg.sa.byWaitFrames = 0;
                //iMsg.sa.byAttackType = 0;
                //iMsg.sa.bySkillIndex = 0;
                iMsg.sa.byFightID = 0;
                iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
                SetAttackMsg(&iMsg);
            }
        }
    }
	//*/
    return  FALSE;
}

void CMonster::SetRenascence(DWORD dwLast, DWORD dwCur, DWORD dwHP, WORD wTime)
{
	if (!m_ParentRegion)
		return;

	SetCurActionID(EA_STAND);
	SetBackupActionID(EA_STAND);

	m_dwLastEnemyID = dwLast;

	m_CurHp = dwHP;

	m_curX		= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	m_curY		= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_curZ		= 0.0f;
	m_Direction = 0.0f;
	m_curTileX	= m_Property.m_BirthPosX;
	m_curTileY	= m_Property.m_BirthPosY;

	m_BackProtection	= false;
	m_SearchTargetTime	= timeGetTime();

	m_ActionFrame = 5/*CRandom::RandRange(100, 150)*/;
    
	if (m_ParentRegion->Move2Area(self.lock(), m_curTileX, m_curTileY, true))
		Stand();
	//����BOSS
	if (m_Property.m_LevelType > 1) {
		UpdateMyData(m_ParentRegion->m_wRegionID,0,0,"ľ���˻�ɱ",1);
	}
}

//----------------------------------------------------------------------------------------------------------	
void SetRandAttributes( DWORD level, SEquipment &equip )
{
    extern int randomatt_levelLimit, randomatt_below, randomatt_above;
    extern DWORD tickRandom();

    DWORD rand_ratting = randomatt_below;
    if ( level > (DWORD)randomatt_levelLimit )
        rand_ratting = randomatt_above;

    // �����rand_ratting�����֮һ�ı�������
    if ( RandomAbilityTable::RATable.validate && ( ( tickRandom() % 10000 ) < rand_ratting ) )
    {
        int rand_times = 1;
	    int rand_ratting = rand() % 10000;
        if ( rand_ratting < 3 )         rand_times = 7;
        else if ( rand_ratting < 10 )   rand_times = 6;
        else if ( rand_ratting < 20 )   rand_times = 5;
        else if ( rand_ratting < 40 )   rand_times = 4;
        else if ( rand_ratting < 100 )  rand_times = 3;
        else if ( rand_ratting < 250 )  rand_times = 2;

        // ѭ��ָ���������г�����
        for ( int slot_index = 0; rand_times != 0; rand_times -- )
        {
            // ���һ������Ȼ����ѭ���еݼ�������������
            rand_ratting = tickRandom() % 100000000;
            for ( int i = 1; ( i < 32 ) && ( rand_ratting > 0 ); i ++ )
            {
                if ( RandomAbilityTable::RATable.units[i].randomType == RandomAbilityTable::TYPE_NULL )
                    break;

                if ( (DWORD)rand_ratting > RandomAbilityTable::RATable.units[i].hitRating )
                {
                    rand_ratting -= RandomAbilityTable::RATable.units[i].hitRating;
                    continue;
                }

                //// ���У���˳���ȡattributes
                //SEquipment::ExtraAttribute &attex = equip.attributes[ slot_index ++ ];
                //attex.type = RandomAbilityTable::RATable.units[i].randomType;

                //// ����ȥ��ȡ���漸�ָ��߼�����
                //int j = rand() % 7;
                //if ( ( RandomAbilityTable::RATable.units[i].array[ 1 + j ].rating == 0 ) || 
                //    ( ( tickRandom() % 100000000 ) > RandomAbilityTable::RATable.units[i].array[ 1 + j ].rating ) )
                //    j = 0; // ���ʧ�����趨ΪĬ��ֵ��

                //attex.value = j;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
#define RATE(_x, _r)    \
    if(_x < 200) { _r = (_x % 100) * 100;  } \
    else if(_x < 2000) { _r = (_x % 1000) * 10;  } \
    else if(_x < 20000) { _r = (_x % 10000);  }  \
	else { _r = 0;  }  
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ֱ����ָ���������������
// ���ñ��������������ID
extern WORD mSpecialAttIdx[ 57 ];

/*
extern void GetAddAttribute( SEquipment::Attribute::Unit *unit, WORD id, int maxNum, WORD coff[ 57 ] = NULL, BYTE level = 0, BYTE itemType = 0 )
{
    #define CALCVALUE() ( ( ( m - n ) <= 0 ) ? n : ( rand() % ( ( m - n ) + 1 ) ) + n )

	if ( maxNum > 6 )
		return;

    SGemAtt* genAtt = CItem::GetSpecialItemAttribute( id );
    if ( genAtt == NULL )
        return;

    BYTE selectIdx[ 60 ];
    BYTE maxSelectIdx[ 60 ];        // ��¼���������һ��
    bool levelIndex[ 4 ] = { 0 };   // �����������ļ����Եļ�������
    
    int  maxOddssIdx = 0;
    int  selectNum   = 0;
    int  maxSelNum   = 0;
    int  dropNum     = 0;           // ��Ϊͬʱ���ֲ�Ӧ��ͬʱ���ֵ����Զ���������������

    BYTE byTimes   = 1;             // ����ѭ��������ٴ�
    BYTE byAttrNum = 1;             // Ӧ�ó���������

    int firstAttr = 0;
    if ( itemType == 104 ) 
        firstAttr = 14;  // �������⹦
    else if ( itemType == 102 || itemType == 105 || itemType == 106 || itemType == 107 || itemType == 108 ) 
        firstAttr = 16;  // ���߼ӷ���
    else if ( itemType == 101 || itemType == 103 || itemType == 112 || itemType == 113 || itemType == 110 || itemType == 111 ) 
        firstAttr = 17;  // ���μ���
    g_LevelAttributesArray[ 0 ][ 0 ] = firstAttr;

    if ( level == 0 && coff == NULL )      // Ϊ�˼��ݵ����
        byTimes = 1;
    else if ( level == 3 )                 // ��Ӧ��ϵΪ��1,2��ѡ1��  3�����ѡ2�� 4, 5�����ѡ3�� 6�����ѡ4��
        byTimes = 2;                       // �������ѡ����������Ծ�ֹͣѡ��
    else if ( level == 4 )
        byTimes = 3;
    else if ( level == 5 || level == 6 )
        byTimes = level - 2;
    
    byAttrNum = byTimes;
    
    while ( byTimes )
    {
        // ���г�ȡ�������и��ʵ��������
        for ( int i = 0; i < 57; ++i )
        {
            // ��ƫ��ӳɾ� �ӳ� = ���� * �ӳ�����( float )�����򲻼ӳ�
            DWORD oddss = genAtt->att[ i ][ 0 ] * ( ( coff == NULL || coff[ i ] == 0 ) ? 1 : coff[ i ]  );  
            oddss = ( coff == NULL || coff[ i ] == 0 ) ? oddss : ( oddss / 100 );
            if ( oddss > genAtt->att[ maxOddssIdx ][ 0 ] )  // ��¼����ODDSS����ֵ
                maxOddssIdx = i;

            if( IN_ODDSS( oddss ) )
                selectIdx[ selectNum++ ] = i;
        }

        if ( selectNum > maxSelNum )
        {
            maxSelNum = selectNum;
            memcpy( maxSelectIdx, selectIdx, sizeof( maxSelectIdx ) );  // ��¼����һ��
        }

        byTimes--;

        // ������һ��û�г���2�����ԣ��ͼ�����������ѭ��2��
        if ( ( level == 3 || level == 4 ) && selectNum >= 2 )  
            break;

        // ������һ��û�г���level - 1�����ԣ��ͼ�����������ѭ��level - 1��
        if ( ( level == 5 || level == 6 ) && selectNum >= level - 2 )  
            break;

        if ( byTimes > 0 )  // �����Ҫ�����������������
        {
            memset( selectIdx, 0, sizeof( selectIdx ) );
            selectNum = 0;
        }
    }

    BYTE* pIdxArray = ( selectNum >= maxSelNum ) ? selectIdx : maxSelectIdx;
    selectNum = max( selectNum, maxSelNum );
    
    // ����Ѿ�����������Աȼƻ������Զ࣬����Ҫ�������
    BOOL randSet = maxNum < selectNum;  
    maxNum = min( maxNum, selectNum );
	for ( int i = 0; i < maxNum; i++ ) 
	{
        int idx = 0;
        if ( randSet == FALSE )  // ����Ҫ�����
		    idx = pIdxArray[ i ];
        else
        {
            // �ȳ����������е��������ݣ���Ϊ��һ�γ�ȡ׼������
            int selPos = rand() % selectNum;
		    idx  = pIdxArray[ selPos ];
            pIdxArray[ selPos ] = pIdxArray[ --selectNum ];
        }

        // �ж��Ƿ������������ͬʱ���ֵ�����
        bool isFound = false;
        for ( int j = 0; j < i; ++j )
        {
            if ( ( mSpecialAttIdx[ idx ] == 28 && unit[ j ].type == 29 ) || 
                 ( mSpecialAttIdx[ idx ] == 29 && unit[ j ].type == 28 ) ||  // ����������
                 ( mSpecialAttIdx[ idx ] == 18 && unit[ j ].type == 19 ) || 
                 ( mSpecialAttIdx[ idx ] == 19 && unit[ j ].type == 18 ) )   // �ڵֺ����
            {
                dropNum++;
                isFound = true; 
                break;
            }
        }

        if ( isFound ) // ��������в���ͬʱ���ֵ�ֱ�Ӷ���( ���ﲻ�������Ϊ˭��д�뱾����������ģ��������������治�ô���)
            continue;
        
		unit[ i ].type = mSpecialAttIdx[ idx ];
		int n = genAtt->att[ idx ][ 1 ];
		int m = genAtt->att[ idx ][ 2 ];

        if ( unit[ i ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ i ].type == SGemData::GEMDATA_MOVESPEED )
            unit[ i ].value = 500 + CALCVALUE();
        else
            unit[ i ].value = CALCVALUE();

        // ֻ�м����������Ը���������С�����Ž���
        if ( coff != NULL && level != 0 && ( ( maxNum - dropNum ) < byAttrNum ) ) 
        {                                                       
            for ( int i = 0; i < 4; ++i )
            {
                for ( int j = 0; j < 3; ++j )
                {
                    if ( g_LevelAttributesArray[ i ][ j ] == idx ) // �����ļ����Ա�������û����������
                    {
                        levelIndex[ i ] = true;                    // ��ʾ�⼶�����Ѿ�������������
                        goto __stopSearch;
                    }
                }
            }
        }

__stopSearch:
        continue;
    }

    // ����������������Ը����������ٸ���
    if ( coff != NULL && level != 0 && ( ( maxNum - dropNum ) < byAttrNum ) )  
    {
        for ( int i = 0; i < ( byAttrNum - ( maxNum - dropNum ) ); ++i )  // ���ϲ�ļ�������
        {
            for ( int j = 0; j < 4; ++j )
            {
                if ( levelIndex[ j ] == true ) 
                    continue;

                int index = g_LevelAttributesArray[ j ][ rand() % 3 ];
                if ( index < 0 || index >= 57 )
                    continue;

                int n = genAtt->att[ index ][ 1 ];
                int m = genAtt->att[ index ][ 2 ];

                unit[ maxNum + i ].type  = mSpecialAttIdx[ index ];
                if ( unit[ maxNum + i ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ maxNum + i ].type == SGemData::GEMDATA_MOVESPEED )
                    unit[ maxNum + i ].value = 500 + CALCVALUE();
                else
                    unit[ maxNum + i ].value = CALCVALUE();

                levelIndex[ j ] = true;
                break;
            }
        }
    }
    else if ( coff == NULL && maxNum == 0 && level == 0 )  // ����ֱ��ѡ����
    {
        int n = genAtt->att[ maxOddssIdx ][ 1 ];
        int m = genAtt->att[ maxOddssIdx ][ 2 ];
        unit[ 0 ].type  = mSpecialAttIdx[ maxOddssIdx ];
        if ( unit[ 0 ].type == SGemData::GEMDATA_ACTIONSPEED || unit[ 0 ].type == SGemData::GEMDATA_MOVESPEED )
            unit[ 0 ].value = 500 + CALCVALUE();
        else
            unit[ 0 ].value = CALCVALUE();
    }
}//*/

/**��������˥������
	��������ȼ�50���󣬱���˥��100%

	���������
	����������2��3��4�Ĺ�����ʲ�˥��
*/
BOOL CMonster::SendDropItemOnDead(BOOL bDoubleDrop, CPlayer *pActPlayer)
{
	/*
    if (pActPlayer == NULL)
        return FALSE;

    if (m_ParentRegion == NULL)
        return FALSE;

    g_Script.SetCondition( this, pActPlayer, NULL );
	BOOL SelectDropResult( CPlayer *pActPlayer, DWORD did, DWORD gid, DWORD tid, POINT pos, CRegion *region, CMonster *monster );
    POINT pos = { m_wCurX, m_wCurY };
    if ( m_Property.boss == 2)
        SelectDropResult( pActPlayer, m_Property.id, 0xffffffff, 0, pos, m_ParentRegion, this );
    else
    	SelectDropResult( pActPlayer, m_Property.id, pActPlayer->GetGID(), pActPlayer->m_dwTeamID, pos, m_ParentRegion, this );
    g_Script.CleanCondition();
	// */
    return  FALSE;
}
BOOL CMonster::SendDropItemOnDeadXYSJ( BOOL bDoubleDrop, class CPlayer *pActPlayer )
{
	/* For XYD3
	//�������ϵ�����Ʒ��������
	int Maxdrop = m_Property.dropMaxNum;
	//�����������������Ʒ����
	int TaskdropTable = m_Property.dropTaskTable;

	extern bool NewItemDropGroundXYSJ(CPlayer *pActPlayer,DWORD did, DWORD count,DWORD protectedGID, 
						DWORD protectedTeamID,POINT pos,CRegion *region,DWORD &dropCount, 
						CMonster *monster,BOOL bind,int dropItemTableId);

	DWORD dropCount = 0;
	
	POINT pos = { m_wCurX, m_wCurY };

	if (pActPlayer == NULL)
		return FALSE;

	if (m_ParentRegion == NULL)
		return FALSE;

	g_Script.SetCondition( this, pActPlayer, NULL );

	if ( m_Property.boss == 2)
		NewItemDropGroundXYSJ(pActPlayer,m_Property.id,Maxdrop,0xffffffff,0,pos,m_ParentRegion,
								dropCount,this,false,TaskdropTable);	
	else
		NewItemDropGroundXYSJ(pActPlayer,m_Property.id,Maxdrop,pActPlayer->GetGID(),pActPlayer->m_dwTeamID,
								pos,m_ParentRegion,dropCount,this,false,TaskdropTable);	

	g_Script.CleanCondition();
	//*/
	return  FALSE;
}

BOOL CMonster::CheckAction(int eCA)        // ��⶯��ִ�е�����
{
    return  TRUE;
}
	
void CMonster::OnDead(CFightObject *PKiller)
{
	_OnDead(PKiller);
	if (CDynamicRegion *pDynRegion = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
	{
		pDynRegion->m_Monster.erase(GetGID());
		if (pDynRegion->m_Monster.empty())
		{
			pDynRegion->m_CanMoveOut		= true;
			pDynRegion->m_MoveOutTimeSeg	= timeGetTime();
			pDynRegion->m_MoveOutTimeleft= timeGetTime();
			pDynRegion->m_MoveOutOT			= 10000;
		}
		CPlayer *pPlayer = (CPlayer *)PKiller->DynamicCast(IID_PLAYER);
		if (pPlayer)
		{
			if (pPlayer->m_ParentRegion->GetGID() == pDynRegion->GetGID())//˵����һ�������е�
			{
				if (g_Script.m_pPlayer)
				{
					CScriptManager::TempCondition temp;
					g_Script.PushCondition(temp);
					LuaFunctor(g_Script,"OnTFPlayerValue")[pDynRegion->m_DynamicIndex]();
					g_Script.PopCondition(temp);
				}
				else
				{
					g_Script.SetCondition(0,pPlayer,0);
					LuaFunctor(g_Script,"OnTFPlayerValue")[pDynRegion->m_DynamicIndex]();
					g_Script.CleanCondition();
				}
			}
		}
	}
	
	if (PKiller)
	{
		g_Script.SetFightCondition(PKiller);
		g_Script.SetCondition(this,0,0);

		CDynamicDailyArea *pDailyScene = (CDynamicDailyArea *)PKiller->m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
		if (pDailyScene != NULL)
		{
			if (g_Script.PrepareFunction("OnDailyObjectDead"))
			{
				g_Script.PushParameter(GetGID());
				g_Script.PushParameter(GetSceneRegionID());
				g_Script.PushParameter(m_ParentRegion->m_wRegionID);//��ͼregionid
				if (PKiller)
				{
					g_Script.PushParameter(PKiller->GetGID());	//ɱ����ҵ�gid
				}
				g_Script.PushParameter(this->GetGID());	//��ǰ�����gid
				g_Script.Execute();
			}
		}
		else
		{
			if (g_Script.PrepareFunction("OnObjectDead"))
			{
				g_Script.PushParameter(GetGID());
				g_Script.PushParameter(GetSceneRegionID());
				g_Script.PushParameter(m_ParentRegion->m_wRegionID);//��ͼregionid
				if (PKiller)
				{
					g_Script.PushParameter(PKiller->GetGID());	//gid
				}
				else
				{
					g_Script.PushParameter(0);//����������
				}
				g_Script.PushParameter(m_dRankIndex);//����������
				g_Script.Execute();
			}
		}

		if (g_Script.PrepareFunction("OnDropItem"))
		{
			g_Script.PushParameter(m_Property.m_ID);
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());
			}
			g_Script.Execute();
		}
		g_Script.CleanFightCondition();
		g_Script.CleanCondition();
	}



	
	//���ڣ���̬�����Ĺ����ˢ�µ�ʱ��Ҳ��������Ϊ-1�ˣ������������ͨ���������
	if (m_Property.m_ReBirthInterval == -1 && m_ParentRegion)
		m_ParentRegion->DelMonster(GetGID());
	
	if (IsBoss()) //����BOSS
	{
		char Killname[CONST_USERNAME];
		CPlayer *player = (CPlayer*)PKiller->DynamicCast(IID_PLAYER);
		if (!player)
		{
			CFightPet *pet =  (CFightPet*)PKiller->DynamicCast(IID_FIGHT_PET);
			{
				if(!pet){
					dwt::strcpy(Killname, "δ֪��һ�ɱ", CONST_USERNAME);
				}
				else{
					player = pet->m_owner;
					if(player){
						dwt::strcpy(Killname, player->getname(), CONST_USERNAME);
					}
				}
			}
		}else{
			dwt::strcpy(Killname, player->getname(), CONST_USERNAME);
		}
		SYSTEMTIME stime;
		GetLocalTime(&stime);
		WORD whour = m_Property.m_ReBirthInterval / 1000/60/60 + stime.wHour;
		WORD wMinute =  m_Property.m_ReBirthInterval / 1000/60+ stime.wMinute;
		UpdateMyData(m_ParentRegion->m_wRegionID,whour,wMinute,Killname,0);
	}

	
}

void CMonster::KillMonsterTaskWithSingle(CPlayer *pCurrPlayer)
{
	if (!pCurrPlayer) return;
	CPlayer::TASKKILL::iterator it = pCurrPlayer->m_KillTask.find(this->m_Property.m_ID);
	//CPlayer::TASKKILL::iterator it = pCurrPlayer->m_KillTask.find(this->m_Property.m_KillTaskID);
	if (it != pCurrPlayer->m_KillTask.end())
	{
		for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
		{
			DWORD flagIndex = *ti;

			if (CRoleTask *task = pCurrPlayer->m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
			{
				if (STaskFlag *flag = task->GetFlag(flagIndex))
				{
					if (flag->m_Complete)			// ����Ѿ�����ˣ������κδ���
						continue;

					BYTE oldComp = flag->m_Complete;

					if (++flag->m_TaskDetail.Kill.CurKillNum == flag->m_TaskDetail.Kill.KillNum)
						flag->m_Complete = 1;

					// ������������Ϣ
					pCurrPlayer->SendUpdateTaskFlagMsg(*flag, oldComp != flag->m_Complete);
				}
				else
				{
					rfalse("�������ɱ����Ϣ����Ȼ�Ҳ���������꣡");
				}
			}
			else
			{
				rfalse("�������ɱ����Ϣ����Ȼ�Ҳ�������");
			}
		}
	}
}

void CMonster::KillMonsterTask(CPlayer *pKiller)
{
	// �����������
	if (!pKiller) return;

	if(0 == pKiller->m_dwTeamID)
	{
		KillMonsterTaskWithSingle(pKiller);
	}
	else
	{
		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
		{
			if(NULL == m_PlayerInDis[i]) continue;
			KillMonsterTaskWithSingle(m_PlayerInDis[i]);
		}
	}
}

void CMonster::HandlePlayerInDis(CPlayer *pKiller)
{
	if (!pKiller) return;

	m_dwPlayerInDisCount = 0;
	for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)	m_PlayerInDis[i] = NULL; //ÿ�ζ���������

	if (!pKiller->m_dwTeamID) return; 

	extern std::map< DWORD, Team > teamManagerMap;
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(pKiller->m_dwTeamID);

	if (iter == teamManagerMap.end())
	{
		rfalse("�ж���ID��ȴ�Ҳ������ڶ��飡");
		return;
	}

	Team &team = iter->second;
	MY_ASSERT(pKiller->m_ParentRegion);
	WORD mapID	= pKiller->m_Property.m_CurRegionID;

	///��Ҫ�����ж��Ƿ��ڶ�̬������
	DWORD Regiongid=pKiller->GetDynamicRegionID();
	// ���������Ա,�ҳ���������Ч��Χ�ĳ�Ա
	for (size_t i=0; i<team.byMemberNum; ++i)
	{
		if (team.stTeamPlayer[i].wRegionID != mapID)
			continue;

		extern LPIObject GetPlayerByGID(DWORD);
		CPlayer *temp = (CPlayer *)GetPlayerByGID(team.stTeamPlayer[i].dwGlobal)->DynamicCast(IID_PLAYER);
		if (!temp || 0 == temp->m_CurHp)
			continue;

		///���Regiongid����0 ���ж��Ƿ���ͬһ��̬������
		if (Regiongid!=0&&Regiongid!=temp->GetDynamicRegionID())continue;
		// �������
		DWORD distance = sqrt(pow((pKiller->m_curX-temp->m_curX),2)+pow((pKiller->m_curY-temp->m_curY),2));
		if (distance > 1500)
			continue;

		if(i>=MAX_TEAM_MEMBER)
		{
			rfalse("��ҵĶ���Index���������ֵ��");
			continue;
		}

		m_dwPlayerInDisCount ++;
		m_PlayerInDis[i] = temp; //��������
	}
}

DWORD GetExp(DWORD dwMonsterExp, DWORD dwPlayerLevel, DWORD dwMonsterLevel)
{
	DWORD dwAddExp=0 , dwLevel=0 ;
	if(dwPlayerLevel <= dwMonsterLevel)
	{
		dwLevel = dwMonsterLevel - dwPlayerLevel;
		dwLevel = (dwLevel>20) ? 20 : dwLevel;

		dwAddExp = dwMonsterExp * (1 - dwLevel * 0.045);
	}
	else
	{
		dwLevel = dwPlayerLevel - dwMonsterLevel;
		dwLevel = (dwLevel > 40) ? 40 : dwLevel;

		dwAddExp = (40 == dwLevel) ? (dwMonsterExp * 0.1) : (dwMonsterExp * (1 - dwLevel * 0.02));
	}

	dwAddExp = (dwAddExp<1) ? 1 : dwAddExp;

	return dwAddExp;
}
//���ɱ������2���Ĺ�������2������ֵ
void CMonster::HandleGetSP(CPlayer *pKiller)
{
	if (!pKiller) return;
	
	if (m_Property.m_Sp > 0)
	{
		pKiller->AddPlayerSp(m_Property.m_Sp);
		pKiller->syneffects(GetGID(), SASynEffectsMsg::EFFECT_ZHENQI, (float)m_curTileX, (float)m_curTileY);
	}
	
}
void CMonster::HandleExpWithTeam(CPlayer *pKiller)
{
 	if (!pKiller) return;

	//����ж�
	if(!pKiller->m_dwTeamID)
	{	//û�����
		DWORD dwExp = GetExp(m_Property.m_Exp, pKiller->m_Property.m_Level, m_Level);
		if (m_ParentRegion){
			lite::Variant ret;
			LuaFunctor(g_Script, "reginaddexp")[m_ParentRegion->m_wRegionID](&ret);
			if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)return;
			if (0 == ( __int32 )ret)return ;
			dwExp *= ( __int32 )ret;
		}

		if (g_Script.GetMonsterVipExp(pKiller,"VipAddMonsterExp",dwExp,dwExp) == 0)
		{
			rfalse(2, 1, "GetMonsterVipExp Fail");
		}

		if (pKiller->limitedState == CPlayer::LIMITED_HALF){  //���ڷ�����ʱ�䣬����ֱ�Ӽ���
			dwExp /= 2;
			pKiller->ShowLimitTips();
		}
		else if (pKiller->limitedState == CPlayer::LIMITED_ZERO) {	//����ƣ��ʱ�䣬����Ϊ0
			dwExp = 0;
			pKiller->ShowLimitTips();
		}
		
		pKiller->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
	}
	else
	{
		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
		{
			if(NULL == m_PlayerInDis[i]) continue;

			CPlayer* lpPlayer = m_PlayerInDis[i];
			DWORD dwExp = GetExp(m_Property.m_Exp, lpPlayer->m_Property.m_Level, m_Level);
			//����������þ��飽�������������1��������������1����3.5%�����¶�������
			dwExp =  dwExp * (1 + (m_dwPlayerInDisCount-1) * 0.05) / m_dwPlayerInDisCount;
			dwExp = (dwExp<1) ? 1 : dwExp;
			//���ѹ�ϵ����ӳ�
			DWORD addexp = 0 ;
			for (size_t j=0; j<MAX_TEAM_MEMBER; ++j)
			{
				if(NULL == m_PlayerInDis[j]) continue;
				CPlayer* pdest = m_PlayerInDis[j];
				if(pdest->GetGID()!=lpPlayer->GetGID())
				{
//					DWORD temp = pdest->GetAppendExp(lpPlayer->GetName(),dwExp);
//					addexp = addexp<temp?temp:addexp;
				}
			}
			dwExp  = dwExp +addexp ;

			if (pKiller->limitedState == CPlayer::LIMITED_HALF){  //���ڷ�����ʱ�䣬����ֱ�Ӽ���
				dwExp /= 2;
				pKiller->ShowLimitTips();
			}
			else if (pKiller->limitedState == CPlayer::LIMITED_ZERO) {	//����ƣ��ʱ�䣬����Ϊ0
				dwExp = 0;
				pKiller->ShowLimitTips();
			}
			dwExp += dwExp * (lpPlayer->m_ExtraExpPercent * 0.01);
			lpPlayer->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
		}
	}

	// ��������
	//pKiller->ModifyCurrentSP(m_Property.m_Sp, 0, false);
}

void CMonster::_OnDead(CFightObject *PKiller)
{
	// ֻ����ҿ��Թ�������������Ҽ���һ����ѪBuff�����ߣ���ôPKiller����Ϊ�գ�
	CPlayer *pPlayer = (CPlayer *)PKiller->DynamicCast(IID_PLAYER);

	CFightObject::OnDead(PKiller);

	ChangeFightSpeed(false);

	m_ActionFrame = (m_Property.m_ReBirthInterval/1000) * 5;
	if(!pPlayer)return;

	// ����ɱ�󴥷��ж� --ahj--
	//SetNineWordsCondition(pPlayer);

	// �����boss��д�뵽����ṹ������
	if (IsBoss())
	{
		++pPlayer->m_Property.m_KillBossNum;
	}

	if (!pPlayer)
	{
		CFightPet *pet=(CFightPet*)PKiller->DynamicCast(IID_FIGHT_PET);
		if (pet)
		{
			pPlayer=pet->m_owner;
		}
	}

	HandlePlayerInDis(pPlayer);	// ��������е�����Ƿ�����Ч��Χ��
	HandleExpWithTeam(pPlayer);	// ɱ�־��鴦��
	HandleGetSP(pPlayer);		// ɱ����������
	KillMonsterTask(pPlayer);	// ����������
	GetDropItem(pPlayer);		// ������䴦��
	
	HandleExpWithObj(PKiller);//	����ɱ�־��鴦��
	

	MY_ASSERT(0 == m_CurHp);

	if (pPlayer)
	{ 
		//pPlayer->RemoveFollow(this);
	}

	return;
}

CMonster::SHitData::SHitData()
{
    memset(this, 0, sizeof(SHitData));
    dwFristHitTime = dwLastHitTime = timeGetTime();
}

void CMonster::SetHit(DWORD GID, DWORD TeamID, DWORD Damage)
{
    SHitData *pHitData = &m_HitMap[GID];
    pHitData->dwDamage += Damage;
    pHitData->dwHitTimes ++;
    pHitData->dwTeamID = TeamID;
    pHitData->dwGID = GID;

    DWORD margin = timeGetTime() - pHitData->dwLastHitTime;
    pHitData->dwLastHitTime = timeGetTime();

    if (pHitData->dwTimeMargin < margin)
        pHitData->dwTimeMargin = margin;
}

void CMonster::UpdateHitMap()
{
    std::map<DWORD, SHitData>::iterator it = m_HitMap.begin();
    while (it != m_HitMap.end())
    {
        SHitData *pHitData = &it->second;

        if (timeGetTime() - pHitData->dwLastHitTime > 20000)
        {
            m_HitMap.erase(it);
            break;
        }

        it ++;
    }
}

bool CMonster::IsBoss(void)
{
	const SMonsterBaseData *pData = CMonsterService::GetInstance().GetMonsterBaseData(m_Property.m_ID);
	MY_ASSERT(pData);
	return pData->m_LevelType>MT_ELITE&&pData->m_LevelType<MT_MAX;
}

void RandomPos( CRegion *parentRegion, WORD &wDesPosX, WORD &wDesPosY, WORD wCenterX, WORD wCenterY, WORD wBRArea, DWORD dwSearchTimes )
{
    if ( parentRegion == NULL )
        return;

    RECT rcBRArea;          // �ļ����÷�Χ
    rcBRArea.left = (wCenterX > wBRArea) ? (wCenterX - wBRArea) : 0;
    rcBRArea.right = wCenterX + wBRArea;
    rcBRArea.top = (wCenterY > wBRArea) ? (wCenterY - wBRArea) : 0;
    rcBRArea.bottom = wCenterY + wBRArea;

    RECT rcRegion;          // ��ͼ��Χ
    rcRegion.left = 0;
    rcRegion.right = parentRegion->m_RegionW;
    rcRegion.top = 0;
    rcRegion.bottom = parentRegion->m_RegionH;

    RECT rcAvaiRegion;
    if ( ( !::IntersectRect(&rcAvaiRegion, &rcBRArea, &rcRegion) ) || ( rcAvaiRegion.left == rcAvaiRegion.right && rcAvaiRegion.top == rcAvaiRegion.bottom ) )
    {
        rfalse(2, 1, "��ͼ���[%d]ѡȡ��Χ����[fileLRTB]:%d, %d, %d, %d; [mapLRTB]:%d, %d, %d, %d;", 
            parentRegion->m_wRegionID ,rcBRArea.left, rcBRArea.right, rcBRArea.top, rcBRArea.bottom, rcRegion.left, rcRegion.right, rcRegion.top, rcRegion.bottom);
        return;
    }

    WORD wBRAreaX = (WORD)(( rcAvaiRegion.right - rcAvaiRegion.left ) >> 1);
    WORD wBRAreaY = (WORD)(( rcAvaiRegion.bottom - rcAvaiRegion.top ) >> 1);
    wCenterX = (WORD)(rcAvaiRegion.left + wBRAreaX);
    wCenterY = (WORD)(rcAvaiRegion.top + wBRAreaY);
    

    DWORD dwTempSearchTimes = 0;
    do 
    {
        ++dwTempSearchTimes;
        wDesPosX = 222/*GetRandomPos(wCenterX, wBRAreaX)*/;
        wDesPosY = 222/*GetRandomPos(wCenterY, wBRAreaY)*/;
    }
    // �����赲��ѡ���������500
    while ( dwTempSearchTimes < dwSearchTimes &&  !parentRegion->isTileWalkAble(wDesPosX, wDesPosY) );

    if ( !parentRegion->isTileWalkAble(wDesPosX, wDesPosY) )     // ���⴦����Ϊ���ĵ�������ѡ�ģ� �϶������赲
    {
        wDesPosX = wCenterX;
        wDesPosY = wCenterY;
    }
}

void CMonster::OnDamage(INT32 *nDamage, CFightObject *pFighter)
{
	// ��ǰ��HP�Ѿ���0��
	if (0 == m_CurHp)
		return;

	//CMonster *monter=(CMonster*)pFighter->DynamicCast(IID_MONSTER);
	//if (monter)
	//	return;
	
	static int WearPos[EQUIP_P_MAX];
	
	CPlayer *attacker = (CPlayer *)pFighter->DynamicCast(IID_PLAYER);
	if (attacker)
	{
		// ���ٹ����������;�
		if (0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex && 0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].attribute.currWear)
		{
			const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex);
			// �ж��Ƿ�������ĥ�������
			if (0 != itemData->m_MaxWear)
				attacker->UpdateEquipWear(EQUIP_P_WEAPON, 1);
		}

		// ������Ϊ��ң���������Ϊ�����Ϊ�Ѿ�ɱ��9���Ǿ�Ӣ�ֵ�ʱ�򣬴�������ɱ��
// 		if (9 == attacker->m_NineWordsKill && !IsBoss())
// 		{
// 			ModifyCurrentHP(-m_CurHp, 0, pFighter);
// 			return;
// 		}
	}

	// �鿴�˺�����
	/*if (m_SorbDamageValue)
		*nDamage -= m_SorbDamageValue;

	if (*nDamage < 0)
		*nDamage = 0;

	if (*nDamage && m_SorbDamagePercent)
		*nDamage -= (*nDamage * 100 / m_SorbDamagePercent);*/

	ModifyCurrentHP(-(*nDamage), 0, pFighter);

	if (0 != *nDamage)
		TriggerHpScriptEvent(*nDamage,pFighter);
		
	if (0 != m_CurHp)
	{
		//m_dwLastEnemyID = pFighter->GetGID();
	}

	if (pFighter)
	{
		m_Direction = GetDirection(m_curX, m_curY, pFighter->m_curX, pFighter->m_curY);
	}

	return;
}

void CMonster::SendPropertiesUpdate()
{
	CFightObject::SendPropertiesUpdate();

	memset(m_FightPropertyStatus, 0, sizeof(m_FightPropertyStatus));
}

void CMonster::OnRebound(int rebound, CFightObject* pFighter)
{
}

void CMonster::SwitchFightState(bool IsIn)
{
	return;
// 	if (IsIn)
// 		ChangeFightSpeed(true);
// 	else
// 	{
// 		RestoreFullHPDirectly();
// 		RestoreFullMPDirectly();
// 		ChangeFightSpeed(false);
// 	}
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByWoundState()
{
	/*
	if (!m_pAttackTarget)
	{
		CheckTarget();
	}

	// ���˿��ܻ���ɴ���AI�ű�
	if(m_CurHp <= (m_dwMaxHP / 2)  &&  m_Property.m_wAIScriptID > 0)     
	{
		CTrigger trigger;
		trigger.m_dwType = CTrigger::TT_MONSTER_HALF_HP;
		trigger.SMonsterHalfHP.pMonster = this;
		trigger.SMonsterHalfHP.dwScriptID = m_Property.m_wAIScriptID;
		OnTriggerActivated(&trigger);
	} //*/

	return true;
}
//----------------------------------------------------------------------------------------------------------
bool CMonster::ProcessEndAttack()
{
	if (m_dwEndAttackTime != 0)
	{
		if ( (int)(m_dwEndAttackTime - timeGetTime()) <= 0 )
		{
			// ׷��ʱ�����������׷��Ŀ��
			m_dwEndAttackTime = 0;
			m_dwLastEnemyID = 0;
		}
	}

	return true;
}
//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID2()
{
	/*
	if(m_bStopToStand )
	{
		if(!m_pAttackTarget)
			m_wIdleTime = 10 + rand() % 25;
		else
			m_wIdleTime = 1;
		m_bStopToStand = FALSE;
	}

	m_wIdleTime -- ;
	if(m_wIdleTime<=0)
	{
		Attack();

		if (!m_pAttackTarget)
		{
			SQSynWayTrackMsg WayMsg;
			int iXEnd = 0;
			int iYEnd = 0;
			int iRet = -1;
			MakeRadomPath(m_Property.m_wMoveArea,m_Property.m_wSrcX,m_Property.m_wSrcY,iXEnd,iYEnd);
			iRet = FindPath(iXEnd,iYEnd,WayMsg);
			{
				if(iRet>= 0)
				{
					SetMoveWay(&WayMsg);
					//add by yuntao.liu
					m_bIsMove2AttackObject = SQSynWayTrackMsg::normal;
				}
			}
		}
		else
		{
			SQAFightObjectMsg iMsg;
			iMsg.sa.dwGlobalID = GetGID();
			iMsg.sa.wPosX = m_wCurX;
			iMsg.sa.wPosY = m_wCurY;
			iMsg.sa.byWaitFrames = 0;
			iMsg.sa.byFightID = 0;
			iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
			SetAttackMsg(&iMsg);
		}

		m_bStopToStand = TRUE;
	}
	//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID1()
{
	/*
	if (m_bStopToStand)
	{
		m_bStopToStand = FALSE;
		m_wIdleTime = 10 + rand() % 25;
	}

	m_wIdleTime -- ;
	if(m_wIdleTime<=0)
	{
		SQSynWayTrackMsg WayMsg;
		int iXEnd = 0;
		int iYEnd = 0;
		int iRet = -1;
		MakeRadomPath(m_Property.m_wMoveArea,m_Property.m_BirthPosX << TILE_BITW,m_Property.m_BirthPosY << TILE_BITH, iXEnd,iYEnd);
		iRet = FindPath(iXEnd,iYEnd,WayMsg);
		{
			if(iRet>= 0)
			{
				SetMoveWay(&WayMsg);
				m_bIsMove2AttackObject = SQSynWayTrackMsg::normal;
			}
		}
		m_bStopToStand = TRUE;
	}//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByStandState_DoAIByID3Or4()
{
// 	if ( m_bStopToStand )
// 	{
// 		if(!m_pAttackTarget)
// 			m_wIdleTime = 5;
// 		else
// 			m_wIdleTime = 1;
// 		m_bStopToStand = FALSE;
// 	}
// 
// 	m_wIdleTime -- ;
// 	if(m_wIdleTime<=0)
// 	{
// 		Attack();
// 
// 		if ( m_pAttackTarget )
// 		{
// 			SQAFightObjectMsg iMsg;
// 			iMsg.sa.dwGlobalID = GetGID();
// 			iMsg.sa.wPosX = m_wCurX;
// 			iMsg.sa.wPosY = m_wCurY;
// 			iMsg.sa.byWaitFrames = 0;
// 			iMsg.sa.byFightID = 0;
// 			iMsg.dwDestGlobalID = m_pAttackTarget->GetGID();
// 			SetAttackMsg(&iMsg);
// 		}
// 
// 		m_bStopToStand = TRUE;
// 	}

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::DoRenascenceScript()
{
	/*
	if(m_Property.m_wRefreshScriptID > 0)   
	{
		CTrigger trigger;
		trigger.m_dwType = CTrigger::TT_MONSTER_NEWLIFE;
		trigger.SMonsterNewLife.pMonster = this;
		trigger.SMonsterNewLife.dwScriptID = m_Property.m_wAIScriptID;
		OnTriggerActivated(&trigger);
	}
	else
	{
		SetRenascence(0, 0, m_dwMaxHP, 2);
	}//*/

	return true;
}
//----------------------------------------------------------------------------------------------------------	
bool CMonster::DeDeadEvent()
{
	SADelObjectMsg msg;
	msg.dwGlobalID = GetGID();
	if (m_ParentArea) 
		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);

	if (m_dwDeadDelayScript)
	{
		extern LPIObject GetPlayerByGID(DWORD);
		if (CPlayer *pPlayer = (CPlayer *)GetPlayerByGID(m_dwKillerPlayerGID)->DynamicCast(IID_PLAYER))
		{
			CTrigger trigger;
			trigger.m_dwType = CTrigger::TT_MONSTERDEAD;
			trigger.SMonsterDead.dwScriptID = m_dwDeadDelayScript;
			trigger.SMonsterDead.pPlayer = pPlayer;
			trigger.SMonsterDead.pMonster = this;

			pPlayer->OnTriggerActivated(&trigger);
		}
	}

	m_bDeadEvent = FALSE;
	m_dwDeadDelayScript = 0;
	m_dwKillerPlayerGID = 0;
/*	m_fActionFrame = 20.0f;*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::ProcessDeadWaitTime()
{
	/*
	if ( m_Property.m_dwBRTeamNumber == 0 || m_Property.m_wRefreshStyle == 1 )
	{
		m_DeadWaitTime --;
	}
	else if (m_Property.m_dwBRTeamNumber != 0
		&& m_Property.m_wRefreshStyle == 2 && m_ParentRegion != NULL)
	{
		if (m_Property.m_dwRefreshTime 
			&&  ( (m_ParentRegion->m_dwTickTime % m_Property.m_dwRefreshTime) >= 0 ) 
			&& ( (m_ParentRegion->m_dwTickTime % m_Property.m_dwRefreshTime) <= 19 ))
		{
			m_DeadWaitTime = 0;
		}
	}//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CMonster::EndPrevActionByDeadState()
{
// 	// �������ű��Ļ�
// 	if (m_DeadWaitTime == 0)
// 	{
// 		DoRenascenceScript();    
// 	}
// 	else if (m_bDeadEvent)
// 	{
// 		DeDeadEvent();
// 	}
// 	else 
// 	{
// 		ProcessDeadWaitTime();
// 	}
// 
	return true;
}

bool CMonster::EndPrevActionByStandState()
{
	/*
	ProcessEndAttack();

	// ����й���Ŀ�����������
	if (m_pAttackTarget != NULL)
	{
		AttackTarget();
	}
	// ����͸���AI���ͽ��ж���
	else 
	{
		switch (m_Property.m_wAIType)
		{
		case 2:
			{
				// ����
				EndPrevActionByStandState_DoAIByID2();
			}
			break;

		case 1:
			{
				// ����߶�
				EndPrevActionByStandState_DoAIByID1();
			}
			break;

		case 3:
		case 4:
			{
				// ��������
				EndPrevActionByStandState_DoAIByID3Or4();
			}
			break;
		}
	}
	//*/

	return true;
}

//----------------------------------------------------------------------------------------------------------	
BOOL CMonster::EndPrevAction()
{
	/*
 	Parent::EndPrevAction();
 
 	switch (GetCurActionID())
 	{
 	case EA_STAND:
 		{
			switch (m_Property.m_wAIType)
			{
			case 2:
				{
					// ����
					EndPrevActionByStandState_DoAIByID2();
				}
				break;

			case 1:
				{
					// ����߶�
					EndPrevActionByStandState_DoAIByID1();
				}
				break;

			case 3:
			case 4:
				{
					// ��������
					EndPrevActionByStandState_DoAIByID3Or4();
				}
				break;
			case 5:	//�һ�����(�ֲ��������,��ͨ����)
				{
					return true;
				}
				break;
			case 6://�һ�����(��������,����)
				{
					m_dwLastEnemyID = 0;
					EndPrevActionByStandState_DoAIByID3Or4();
				}
				break;
			}

 			if(m_dwLastEnemyID != 0)
 			{
				extern LPIObject GetPlayerByGID(DWORD GID);
				LPIObject object = GetPlayerByGID(m_dwLastEnemyID);

				CPlayer* player = (CPlayer*)(object->DynamicCast(IID_PLAYER));
				if(!player)
				{	
					m_dwLastEnemyID = 0;
				}
				else if(player->m_CurHp == 0)
 				{
					m_dwLastEnemyID = 0;
				}
				else
				{
					SQuestSkill_C2S_MsgBody msg;

					msg.mDefenderGID = m_dwLastEnemyID;
					msg.dwSkillIndex = 0;
					msg.mAttackWorldPosX = 0;
					msg.mAttackWorldPosY = 0;
					msg.mDefenderWorldPosX = 0;
					msg.mDefenderWorldPosY = 0;
// 					msg.mQuestTileX = -1;
// 					msg.mQuestTileY = -1;
					ProcessQuestSkill(&msg);
				}

				//m_dwLastEnemyID = 0;
 			}
 		}
 		break;
	case EA_WOUND:
		{
			EndPrevActionByWoundState();
		}

		break;

	case EA_DEAD:
		{
			EndPrevActionByDeadState();
		}

		break;
 	default:
 		{}
 		break;
 	}
	//*/

	return TRUE;
}

BOOL CMonster::DoCurAction()
{
	return Parent::DoCurAction();
}

bool CMonster::CreatePath(SQSynPathMsg& WayMsg, CFightObject *pLife)
{
	if (NULL == m_ParentRegion || NULL == pLife)
		return false;

	// Ŀǰ�򵥵Ĵ�����
	WayMsg.m_GID = GetGID();
	WayMsg.m_Path.m_moveType = EA_RUN;
	WayMsg.m_Path.m_wayPoints = 2;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;
	WayMsg.m_Path.m_path[1].m_X = pLife->m_curX;
	WayMsg.m_Path.m_path[1].m_Y = pLife->m_curY;

	return true;
}

int CMonster::CreatePath(SQSynWayTrackMsg& WayMsg,CFightObject* pLife)
{
// 	WayMsg.dwGlobalID = 0;
// 	ZeroMemory(&WayMsg.ssw, sizeof(WayMsg.ssw));
// 
// 	WayMsg.dwGlobalID = GetGID();
// 
// 	if (m_ParentRegion == NULL) 
// 		return 0;
// 
// 	if (!m_ParentRegion->isTileWalkAble(m_wCurX, m_wCurY))
// 	{
// 		return 0;
// 	}
// 
// 	int direction = GetDirection(m_wCurX,m_wCurY,pLife->m_wCurX,pLife->m_wCurY);
// 
// 	INT32 maxOffset = 1; //max(abs(pLife->m_wCurX - m_wCurX),abs(pLife->m_wCurY - m_wCurY));
// 
// 	for (int i = 0;i < 8;i ++)
// 	{
// 		direction &= 7;
// 
// 		INT32 len = 1;
// 		bool isFind = false;
// 
// 		while(
// 			m_ParentRegion->isTileWalkAble(
// 				m_wCurX + g_dir[ direction * 2 ] * len, 
// 				m_wCurY + g_dir[ direction * 2 + 1 ] * len
// 			)
// 			&&
// 			m_ParentRegion->isTileStopAble(
// 				m_wCurX + g_dir[ direction * 2 ] * len, 
// 				m_wCurY + g_dir[ direction * 2 + 1 ] * len,
// 				GetGID())	
// 			)
// 		{
// 			len ++;
// 			isFind = true;
// 			if(len >= maxOffset)
// 			{
// 				break;
// 			}
// 		}
// 
// 		if (isFind)
// 		{
// 			WayMsg.ssw.byAction = EA_WALK;
// 			WayMsg.ssw.wSegX = m_wCurX;
// 			WayMsg.ssw.wSegY = m_wCurY;
// 			WayMsg.ssw.Track[ 0 ].len = len;
// 			WayMsg.ssw.Track[ 0 ].dir = direction;
// 
// 			break;
// 		}
// 		direction ++;
// 	}
// 
// 	return WayMsg.ssw.Track[0].len;

	return 0;
}

bool CMonster::SendMove2TargetForAttackMsg(INT32 skillDistance, INT32 skillIndex, CFightObject *pLife)
{
	if (!m_ParentRegion || !pLife)
		return false;
	 
	// [2012-5-11 18-09 gw: + ����״̬���ǲ����ƶ�ȥ������]
	if (m_fightState & FS_DIZZY || m_fightState & FS_JITUI)
	{ 
		//rfalse(2, 1, "����״̬���ǲ����ƶ�ȥ������");
		return false;
	}

	if (skillIndex < 0) //������ҵ�֪ͨ׷���Ĺ������Ѿ��ı��˷����������׷��״ֱ̬�ӷ���
	{
		if (GetCurActionID() != EA_RUN )
		{
			return false;
		}
	}

	CPlayer *pPlayer = (CPlayer *)pLife->DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		SASynPathMonsterMsg  spathmonster;  //������Ҫ��ͻ�������1��·��
		spathmonster.dwGlobalID = GetGID();
		spathmonster.m_path[0].m_X = m_curX;
		spathmonster.m_path[0].m_Y = m_curY;
		spathmonster.m_path[1].m_X = pPlayer->m_curX;
		spathmonster.m_path[1].m_Y = pPlayer->m_curY;
		g_StoreMessage(pPlayer->m_ClientIndex, &spathmonster, sizeof(SASynPathMonsterMsg));
	}
	if (skillIndex >= 0)
	{
		int nCuruseskill = GetSkillIDBySkillIndex(skillIndex);
		if (nCuruseskill >= 0)
		{
			m_wCuruseskill = nCuruseskill;
		}
	}
	WORD maxdistance = 200;
	WORD mindistance = 50;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(m_wCuruseskill);
	if (pData)
	{
		maxdistance = pData->m_SkillMaxDistance;
		mindistance = pData->m_SkillMinDistance;
		skillDistance = maxdistance;
	}

	SQSynPathMsg WayMsg;

	// ���ȼ������
	D3DXVECTOR2 ver((m_curX - pLife->m_curX), (m_curY - pLife->m_curY));
	float distance = D3DXVec2Length(&ver);  //Ŀ������Լ��ľ���
	//float distance = sqrt(pow((m_curX - pLife->m_curX), 2) + pow((m_curY - pLife->m_curY), 2));
	if (distance< skillDistance)
	{
		return true;
	}

/*	rfalse(2, 1, "Move2TargetForAttack %4.2f,%4.2f", m_curX, m_curY);*/

	float b = m_Property.m_WalkSpeed /2;  //200 ����ÿ֡

	// ����һ���Ĳ�����λ�𲽱ƽ�������100��
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	if (CreateRadomPoint(pLife,desPos,mindistance,maxdistance)== 0)
	{
		rfalse(2, 1, "CreateRadomPoint Faile");
	}
	
	//D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	float fRadian = atan2(desVector.y, desVector.x);

	D3DXVECTOR2 vMovePos;
	vMovePos.x = m_curX + b * cos(fRadian);
	vMovePos.y = m_curY + b * sin(fRadian);
	float newlen = D3DXVec2Length(&(vMovePos - curPos)); //ÿ���ƶ��ľ���



	WORD wnode = 1;
	while (1)
	{
		//distance = sqrt(pow((pLife->m_curX - DestPosX), 2) + pow((pLife->m_curY - DestPosY), 2));
		WayMsg.m_Path.m_path[wnode].m_X = vMovePos.x;
		WayMsg.m_Path.m_path[wnode].m_Y = vMovePos.y;

		//distance = sqrt(pow((m_BX - DestPosX), 2) + pow((m_BY - DestPosY), 2));
		distance = D3DXVec2Length(&(desPos - vMovePos));
		newlen = D3DXVec2Length(&(vMovePos - curPos));
		if (newlen >= distance ) //�ƶ�������Ŀ���ʱ
		{
			WayMsg.m_Path.m_path[wnode].m_X = desPos.x;   //���1�������Ŀ�����
			WayMsg.m_Path.m_path[wnode].m_Y = desPos.y;
			//rfalse(2, 1, "CreateRadomPoint id = %d , x = %f,y = %f ", GetGID(),desPos.x, desPos.y);
			break;
		}
// 		if (skillDistance >= distance)
// 		{
// 			//WayMsg.m_Path.m_path[wnode].m_X = desPos.x;
// 			//WayMsg.m_Path.m_path[wnode].m_Y = desPos.y;
// 			rfalse(2,1,"skillDistance = %d,distance = %f",skillDistance,distance);
// 			break;
// 		}

		wnode++;
		curPos = vMovePos;

		vMovePos.x = vMovePos.x + b * cos(fRadian);
		vMovePos.y = vMovePos.y + b * sin(fRadian);

		if (wnode+1 >= MAX_TRACK_LENGTH)
		{
			ver.x = pLife->m_curX - m_curX;
			ver.y = pLife->m_curY - m_curY;
			distance = D3DXVec2Length(&ver);
			rfalse("wnode to long Lenth = %f",distance);
			break;
		}
	}

	m_vdesPos->x = pLife->m_curX;
	m_vdesPos->y = pLife->m_curY;

// 	if (wnode > 2) //�ù���������뿪���2����
// 	{
// 		wnode -= 2;
// 		m_vdesPos.x = WayMsg.m_Path.m_path[wnode].m_X;
// 		m_vdesPos.y = WayMsg.m_Path.m_path[wnode].m_Y;
// 	}



	//rfalse("SendMove2TargetForAttackMsg x = %f,y = %f,destX = %f,destY = %f", m_curX,m_curY,DestPosX,DestPosY);

	WayMsg.m_GID				= GetGID();
	WayMsg.m_move2What			= SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType	= EA_RUN;
	WayMsg.m_Path.m_wayPoints	= wnode+1;
	WayMsg.m_Path.m_path[0].m_X = m_curX;
	WayMsg.m_Path.m_path[0].m_Y = m_curY;
// 	WayMsg.m_Path.m_path[1].m_X = pLife->m_curX;
// 	WayMsg.m_Path.m_path[1].m_Y = pLife->m_curY;
	
	//rfalse("MonsterGID =%d, MoveTime = %d", GetGID(),wnode * 200);
	// ���·���ɹ����ɣ����ƶ���ȥ����
	m_IsMove2Attack = SQSynPathMsg::attack;
	if(!SetMovePath(&WayMsg))
	{
		// ���·�����ɣ����ã�ʧ�ܣ���ô��ʾ�������⣬��ʱ����ѡ����ַ���
		// �磺�ص�ԭ�㡢ԭ�ز����ȣ��˴���ʱ���������Ǻ�����ȻҪ����true��
	}
	m_nMoveStartTime = GetTickCount64();
	ver.x = m_curX - pLife->m_curX;
	ver.y = m_curY - pLife->m_curY;
	distance = D3DXVec2Length(&ver);
	//rfalse(2, 1, "Move2TargetForAttack %4.2f,%4.2f ---%4.2f,%4.2f ", m_curX, m_curY, WayMsg.m_Path.m_path[wnode].m_X, WayMsg.m_Path.m_path[wnode].m_Y);
//	rfalse(2, 1, "Move2Target Target %4.2f,%4.2f", WayMsg.m_Path.m_path[wnode].m_X, WayMsg.m_Path.m_path[wnode].m_Y);
	return true;
}

INT32 CMonster::GetCurrentSkillLevel(DWORD dwSkillIndex)
{
	return 1;
}

INT32 CMonster::GetSkillIDBySkillIndex(INT32 index)
{
	//SSkill* pSkill = &m_pSkills[index];
	return (index < 0 || index >= MAX_MONTER_SKILLCOUNT)?-1:( (m_pSkills[index].byLevel == 0)?-1:m_pSkills[index].wTypeID);
}

void CMonster::OnChangeState(EActionState newActionID)
{
	if (newActionID == EA_STAND)
	{

	}
}

void CMonster::OnRunEnd()
{
	if (m_IsMove2Attack == SQSynWayTrackMsg::move_2_attack)
 	{
		// rfalse(2, 1, "���﹥���ƶ���ϣ���ǰ����%f, %f", m_curX, m_curY);

 		m_backUpQusetSkillMsg = m_AtkContext;
 		__ProcessQuestSkill(&m_backUpQusetSkillMsg, true);
 	}
	else
	{
		if (m_BackProtection)
		{
			m_BackProtection = false;
			m_dwLastEnemyID	 = 0;
			m_Attacker.clear();
			
			// [2012-6-11 18-40 gw: +todo..] �����ǰ�����б�Ϊ��
			if (m_Attacker.empty())
				SwitchFightState(m_IsInFight = false);
		}

		// ����վ��״̬
		//m_ActionFrame = 5/*CRandom::RandRange(100, 150)*/;
		m_ActionFrame = CRandom::RandRange(m_Property.m_StayTimeMin, m_Property.m_StayTimeMax);

	}
	//rfalse(2,1,"MonsterMoveEnd GID = %d,MoveTime = %d,MoveNode = %d", GetGID(), (GetTickCount64() - m_nMoveStartTime), m_NodeCount);
}

bool CMonster::CheckAddBuff(const SBuffBaseData *pBuff)
{
	if (m_BackProtection)	// ������ڷ��ر������޷�����Buff
	{
		rfalse(2, 1, "���ﴦ�ڱ���״̬���޷�����buff����");
		return false;
	}

	return true;
}

void CMonster::ChangeFightSpeed(bool inFight)
{
	if (0 == m_Property.m_WalkSpeed)
		return;
	
	if (inFight && m_BaseSpeed == m_Property.m_FightSpeed) return;
	if (!inFight && m_BaseSpeed == m_Property.m_WalkSpeed) return;

	m_BaseSpeed = inFight ? m_Property.m_FightSpeed : m_Property.m_WalkSpeed;

	UpdateAllProperties();

	if (m_ParentArea)
	{
		SAMonsterWRChange msg;
		msg.bStatus		= inFight ? SAMonsterWRChange::SMWR_RUN : SAMonsterWRChange::SMWR_WALK;
		msg.dwGlobal	= GetGID();

		m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
	}
}

void CMonster::UpdateAllProperties()
{
	WORD oldSpeed = m_OriSpeed;

	WORD wSpeed			= m_BaseSpeed;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;

	m_MaxHp += GetBuffModifyValue(BAP_MHP, m_MaxHp);
	m_MaxMp += GetBuffModifyValue(BAP_MMP, m_MaxMp);
	m_MaxTp += GetBuffModifyValue(BAP_MTP, m_MaxTp);
	m_GongJi += GetBuffModifyValue(BAP_GONGJI, m_GongJi);
	m_FangYu += GetBuffModifyValue(BAP_FANGYU, m_FangYu);
	m_BaoJi += GetBuffModifyValue(BAP_BAOJI, m_BaoJi);
	m_ShanBi += GetBuffModifyValue(BAP_SHANBI, m_ShanBi);
	m_Hit += GetBuffModifyValue(BAP_HIT, m_Hit);

	m_SorbDamageValue	+= GetBuffModifyValue(BAP_SORBDAM,		0);
	m_SorbDamagePercent += GetBuffModifyValue(BAP_SORBDAM_PER,	0);
	wSpeed += GetBuffModifyValue(BAP_SPEED, m_BaseSpeed);
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNCRIT, m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_WRECK, m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNWRECK, m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_PUNCTURE, m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNPUNCTURE, m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);

	if (oldSpeed != wSpeed)
	{
		m_OriSpeed = wSpeed;
		//m_Speed = m_OriSpeed / 1000.0f;
		m_FightPropertyStatus[XA_SPEED] = true;
	}

	// ������Ըı�
	for (size_t i = 0; i < XA_MAX_EXP; i++)
		m_FightPropertyStatus[i] = true;
}

void CMonster::InitScriptEvent()
{
	///����ű�������
	CScriptManager::TempCondition tCondition;
	g_Script.PushCondition(tCondition);

	///���ô˽ű�
	g_Script.SetCondition(this,0,0);
	//LuaFunctor(g_Script, FormatString("OnMonsterBirth%d",m_Property.m_ID))[m_Property.m_ID]();
	LuaFunctor(g_Script, "OnMonsterBirth")[m_Property.m_ID][m_dRankIndex]();
	g_Script.CleanCondition();

	//�ָ��ű�����
	g_Script.PopCondition(tCondition);
	
	m_nBirthTime=GetTickCount64();
}

void CMonster::TriggerHpScriptEvent(int nDamage, CFightObject *pFighter)
{
	
	if (g_Script.PrepareFunction("OnObjectDamage"))
	{
		g_Script.PushParameter(GetGID());
		g_Script.PushParameter(nDamage);
		if (pFighter)
		{
			g_Script.PushParameter(pFighter->GetGID());
		}
		g_Script.Execute();
	}

}
#define NUM 100
void CMonster::HandleExpWithObj( CFightObject *pKiller )
{
	if (!pKiller) return;
	CFightPet *pFightPet=  (CFightPet *)pKiller->DynamicCast(IID_FIGHT_PET);
	if (!pFightPet)
	{
		CPlayer *player =  (CPlayer *)pKiller->DynamicCast(IID_PLAYER);
		if(player&&player->m_pFightPet)
		{
			pFightPet = player->m_pFightPet;
		}
	}
	if(!pFightPet)return;
	DWORD dwExp = GetExp(m_Property.m_Exp, pFightPet->m_Level, m_Level);
	//dwExp*=NUM;//����100������*/
	dwExp/=2;
	pFightPet->SendAddFightPetExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");

	//����ж�
// 	if(!pKiller->m_dwTeamID)
// 	{	//û�����
// 		DWORD dwExp = GetExp(m_Property.m_Exp, pKiller->m_Property.m_Level, m_Level);
// 		pKiller->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
// 		if (pKiller->m_pFightPet)
// 		{	
// 
// 		}
// 	}
// 	else
// 	{
// 		for (size_t i=0; i<MAX_TEAM_MEMBER; ++i)
// 		{
// 			if(NULL == m_PlayerInDis[i]) continue;
// 
// 			CPlayer* lpPlayer = m_PlayerInDis[i];
// 			DWORD dwExp = GetExp(m_Property.m_Exp, lpPlayer->m_Property.m_Level, m_Level);
// 			//����������þ��飽�������������1��������������1����3.5%�����¶�������
// 			dwExp =  dwExp * (1 + (m_dwPlayerInDisCount-1) * 0.035) / m_dwPlayerInDisCount;
// 			dwExp = (dwExp<1) ? 1 : dwExp;
// 			lpPlayer->SendAddPlayerExp(dwExp, SAExpChangeMsg::MONSTERDEAD, "Killed Monster");
// 		}
// 	}
}

BOOL CMonster::GetDropItem( DWORD itemID,INT32 itemNum )
{
	if (!m_ParentRegion )
		return FALSE;
	
	if (!g_Script.m_pAttacker)return FALSE;
	CPlayer *player=(CPlayer*)g_Script.m_pAttacker->DynamicCast(IID_PLAYER);
	if(!player)return FALSE;
	
	
	WORD Droproute = CRandom::RandRange(1, 100);
	if (player->limitedState == CPlayer::LIMITED_HALF){  //���ڷ�����ʱ�䣬������Ʒ���ʼ���
		player->ShowLimitTips();
		if (Droproute < 50) {
			return FALSE;
		}
	}
	else if (player->limitedState == CPlayer::LIMITED_ZERO) {	//����ƣ��ʱ�䣬����Ϊ0
		player->ShowLimitTips();
		return FALSE;
	}

	CItem::SParameter args;
	std::vector<SRawItemBuffer> item;


	SRawItemBuffer temp;
	BOOL rt = CItemUser::GenerateNewItem(temp, SAddItemInfo(itemID, itemNum));
	if (FALSE == rt)
		return FALSE;

	item.push_back(temp);

 	if (item.empty() && 0 == args.Money)	// û�����ɵ�
		return FALSE;
// 	args.PackageModel	= IPM_BUDAI;
// 	args.dwLife			= MakeLifeTime(5);
// 	args.xTile			= m_curTileX;
// 	args.yTile			= m_curTileY;
// 	args.ItemsInPack	= item.size();
// 	args.ProtectedGID	= player->GetGID();
// 	args.ProtectTeamID	= player->m_dwTeamID;
// 
// 	if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, args, item, LogInfo(m_Property.m_ID, "����ű��������")))
// 		return FALSE; 

	CSingleItem::SParameter Singleargs;
	if (args.Money)
	{
		SRawItemBuffer itemTemp;
		int moneyid = player->_L_GetLuaValue("OnGetMoneyItemID"); //�ж��Ƿ�Ϊ��Ǯ����
		if (!moneyid)return FALSE;

		BOOL rt = CItemUser::GenerateNewItem(itemTemp, SAddItemInfo(moneyid, args.Money)); //�����Ǯ��Ϊһ������ĵ��ߴ���
		if (FALSE == rt)
			return FALSE;

		Singleargs.PackageModel	= IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=player->GetGID();
		Singleargs.ProtectTeamID	= player->m_dwTeamID;

		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "����ű��������")))
			return FALSE; 
	}

	std::vector<SRawItemBuffer>::iterator iter = item.begin();
	while(iter != item.end())
	{
		SRawItemBuffer itemTemp = *iter;

		Singleargs.PackageModel	= IPM_BUDAI;
		Singleargs.dwLife			= MakeLifeTime(5);
		Singleargs.xTile			= m_curTileX;
		Singleargs.yTile			= m_curTileY;
		Singleargs.ItemsNum = itemTemp.overlap;
		Singleargs.ItemsID = itemTemp.wIndex;
		Singleargs.ProtectedGID=player->GetGID();
		Singleargs.ProtectTeamID	= player->m_dwTeamID;
		if (!CSingleItem::GenerateNewGroundItem(m_ParentRegion, 3, Singleargs, itemTemp, LogInfo(m_Property.m_ID, "����ű��������")))
			return FALSE; 

		iter++;
	}

	return TRUE;
}

DWORD CMonster::GetRandomEnemyID()
{
	// ��Ѱһ��Ŀ��
	check_list<LPIObject> *playerList = m_ParentRegion->GetPlayerListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);

	if (playerList)
	{
		size_t lenth=playerList->size();
		if(lenth==0)return 0;
		int ntemp=rand()%lenth;
		check_list<LPIObject>::iterator it = playerList->begin();
		while(ntemp--)it++;
		LPIObject target = *it;
		CPlayer *player = (CPlayer *)target->DynamicCast(IID_PLAYER);
		if (player && 0 != player->m_CurHp)
		{
			m_dwLastEnemyID = player->GetGID();	
			return m_dwLastEnemyID;
		}
			
	}
	return 0;
}

bool CMonster::SetStopTracing(EActionState action)//�ص�������
{
	float m_BX	= ((DWORD)m_Property.m_BirthPosX) << TILE_BITW;
	float m_BY	= ((DWORD)m_Property.m_BirthPosY) << TILE_BITH;
	m_BackProtection = true;
	SQSynPathMsg msg;
	//float distance = sqrt(pow((m_curX - m_BX), 2) + pow((m_curY - m_BY), 2));
	D3DXVECTOR2 ver((m_curX - m_BX), (m_curY - m_BY));
	float distance = D3DXVec2Length(&ver);
	float b = m_Property.m_WalkSpeed  ;  //200 ����ÿ֡

	// ����һ���Ĳ�����λ�𲽱ƽ�������100��
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(m_BX, m_BY);
	D3DXVECTOR2 desVector = desPos - curPos;
	D3DXVec2Normalize(&desVector, &desVector);

	float fRadian = atan2(desVector.y, desVector.x);

	D3DXVECTOR2 vMovePos;
	vMovePos.x = m_curX + b * cos(fRadian);
	vMovePos.y = m_curY + b * sin(fRadian);
	float newlen = D3DXVec2Length(&(vMovePos - curPos));

	WORD wnode = 1;
	while (1)
	{

		msg.m_Path.m_path[wnode].m_X = vMovePos.x;
		msg.m_Path.m_path[wnode].m_Y = vMovePos.y;

		//distance = sqrt(pow((m_BX - DestPosX), 2) + pow((m_BY - DestPosY), 2));
		distance = D3DXVec2Length(&(desPos - curPos));
		newlen = D3DXVec2Length(&(vMovePos - curPos));
		if (newlen >= distance)
		{
			msg.m_Path.m_path[wnode].m_X = desPos.x;
			msg.m_Path.m_path[wnode].m_Y = desPos.y;
			break;
		}


		wnode++;
		curPos = vMovePos;

		vMovePos.x = vMovePos.x + b * cos(fRadian);
		vMovePos.y = vMovePos.y + b * sin(fRadian);


		if (wnode+1 >= MAX_TRACK_LENGTH)
		{
// 			ver.x = m_curX - m_BX;
// 			ver.y = m_curY - m_BY;
			//distance = D3DXVec2Length(&ver);
			//distance = sqrt(pow((m_curX - m_BX), 2) + pow((m_curY - m_BY), 2));
			rfalse("wnode to long distanceLeft = %f",distance);
			//m_BackProtection = false;
			wnode = MAX_TRACK_LENGTH - 1;
			msg.m_Path.m_path[wnode].m_X = m_BX;
			msg.m_Path.m_path[wnode].m_Y = m_BY;
			break;
		}
	}

	

	msg.m_GID		= GetGID();
	msg.m_move2What	= SQSynPathMsg::normal;
	msg.m_Path.m_wayPoints		= wnode + 1;
	msg.m_Path.m_moveType		= action;
	msg.m_Path.m_path[0].m_X	= m_curX;
	msg.m_Path.m_path[0].m_Y	= m_curY;
	m_IsMove2Attack = SQSynPathMsg::normal;

	//rfalse(2,1,"BackToBirth %f,%f",m_BX,m_BY);
	// �������·��ʧ�ܣ���˲�ƹ�ȥ
	if (!SetMovePath(&msg))
	{
		// զ��˲��ૣ�
		MoveToBirth();
	}
	else
	{
		m_IsMove2Attack  = SQSynPathMsg::normal;
		m_BackProtection = true;
	}

	m_nMoveStartTime = GetTickCount64();
	ver.x = m_curX - m_BX;
	ver.y = m_curY - m_BY;
	distance = D3DXVec2Length(&ver);
	//rfalse("MonsterMoveStart GID = %d,MoveTime = %d,MoveNode = %d,Lenth = % f", GetGID(), (wnode * 200), wnode+1, distance);
	//if (m_pFightFollow)
	//{
		//m_pFightFollow->RemoveFollow(this);
	//}

	m_dwLastEnemyID = 0;

	return true;
}

bool CMonster::MoveToBirth()
{
	//rfalse("MoveToBirth");
	MY_ASSERT(m_ParentRegion);
	return m_ParentRegion->Move2Area(this->self.lock(),m_Property.m_BirthPosX,m_Property.m_BirthPosY,true);
}

// ˽�к���������ɱ�������ж�
void CMonster::SetNineWordsCondition(CPlayer* player)
{
	if(player == NULL) 
		return;

	if (!IsBoss())
	{
		++player->m_NineWordsKill;
		//TalkToDnid(player->m_ClientIndex, FormatString("������ɱ�󡿵�%d������", player->m_NineWordsKill));
		SANineWordMsg msg;
		msg.num = player->m_NineWordsKill;

		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SANineWordMsg));
	}

	if (10 == player->m_NineWordsKill)
	{
		TalkToDnid(player->m_ClientIndex, "�١����������ߡ��ԡ����С�ǰ���У�ɱ��");
		
		//DWORD gid = this->GetGID();
		
		//MY_ASSERT(gid);
		SANineWordMsg msg;
		msg.num = player->m_NineWordsKill;
		//msg.gid = gid;

		g_StoreMessage(player->m_ClientIndex, &msg, sizeof(SANineWordMsg));
		
		player->m_NineWordsKill = 0;
	}
	
	return;
}
 
void CMonster::OnRun()
{
	if(m_AIParameter.wAIID != 0)
	{	//AI�ű����ƵĹ���
		SendPropertiesUpdate();
		ProcessScript();
		Parent::OnRun();
	}
	else
	{	//��ͨ����
		SendPropertiesUpdate();
		//Thinking();
		if ( m_StaticpreUpdateTime == 0 )
		{
			g_Script.SetCondition( this, NULL, NULL);
			LuaFunctor(g_Script, FormatString("StaticScene_Monster_Init"))[m_Property.m_StaticAI][GetGID()]();
			g_Script.CleanCondition();
			m_StaticpreUpdateTime = ::timeGetTime();
		}
		else
		{   
			g_Script.SetCondition( this, NULL, NULL);
			LuaFunctor(g_Script, FormatString("StaticScene_Monster_Update"))[m_Property.m_StaticAI][GetGID()]();
			g_Script.CleanCondition();
		}
		Parent::OnRun();
	}
}

void CMonster::OnClose()
{
	g_Script.SetCondition(this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("StaticScene_Monster_End"))[m_Property.m_StaticAI][GetGID()]();
	g_Script.CleanCondition();

// 	if(m_AIParameter.wAIID != 0)
// 	{
// 		g_Script.SetCondition( this, NULL, NULL );
// 		LuaFunctor(g_Script, FormatString( "Monster_End_%d", m_AIParameter.wAIID ))[m_ParentRegion->GetGID()][GetGID()][m_AIParameter.wListID]();
// 		g_Script.CleanCondition();
// 	}
// 	else
// 	{
// 		g_Script.SetCondition( this, NULL, NULL );
// 		LuaFunctor(g_Script, FormatString( "StaticScene_Monster_End", m_Property.m_StaticAI))[GetGID()]();
// 		g_Script.CleanCondition();
// 	}
	return;
}

void CMonster::OnDisppear()
{
	m_buffMgr.CheckDelBuffSpot(0x00000004);
	SAPlayerDeadMsg DeadMsg;
	DeadMsg.dwGID = GetGID();
	DeadMsg.killGID =  0;
	if (m_ParentArea)
		m_ParentArea->SendAdj(&DeadMsg, sizeof(DeadMsg), -1);

	SetCurActionID(EA_DEAD);
	SetBackupActionID(EA_DEAD);
}

int CMonster::OnCreateEnd()
{
	return 1;
}

void CMonster::UpdateMyData( WORD reginID,WORD wHour,WORD wMinute,char *killmeName,WORD mystate )
{
	BossDeadData bossdata;
	bossdata.m_ID = m_Property.m_ID;
	bossdata.m_level = m_Property.m_level;
	bossdata.reginid= reginID;				//����ID
	bossdata.wX= m_Property.m_BirthPosX;					//����X	
	bossdata.wY= m_Property.m_BirthPosY	;					//����Y
	bossdata.wHour =  wHour;			//����
	bossdata.wMinute=  wMinute;		
	bossdata.wState = mystate;
	bossdata.m_UpdateTime = timeGetTime();
	dwt::strcpy(bossdata.m_KillMyName, killmeName, CONST_USERNAME);
	GetGW()->UpdateBossData(bossdata);
}

void CMonster::OnRecvPath(SQSynPathMonsterMsg *pMsg)
{
	SQSynPathMsg WayMsg;
	WayMsg.m_GID = GetGID();
	WayMsg.m_move2What = SQSynWayTrackMsg::move_2_attack;
	WayMsg.m_Path.m_moveType = EA_RUN;
	WayMsg.m_Path.m_wayPoints = pMsg->m_Path.m_wayPoints;
	for (WORD i = 0; i < pMsg->m_Path.m_wayPoints; i++)
	{
		WayMsg.m_Path.m_path[i] = pMsg->m_Path.m_path[i];
	}
	if (WayMsg.m_Path.m_wayPoints > 1)
	{
		m_curX = WayMsg.m_Path.m_path[0].m_X;
		m_curY= WayMsg.m_Path.m_path[0].m_Y;
		SetMovePath(&WayMsg);
	}
}

int CMonster::CreateRadomPoint(CFightObject* pLife, D3DXVECTOR2 &vtargetPos, WORD mindistance, WORD maxdistance)
{
	if (!pLife)
	{
		return 0;
	}
	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(pLife->m_curX, pLife->m_curY);
	D3DXVECTOR2 desVector = curPos - desPos;
	D3DXVec2Normalize(&desVector, &desVector);
	
	float fRadian = atan2(desVector.y, desVector.x); //��ǰ����
	int nRadian = CRandom::RandRange(fRadian - D3DX_PI / 2, fRadian + D3DX_PI/2);  //��ǰ�����������90���ֿ�
	int randdistance = CRandom::RandRange(mindistance, maxdistance);

	vtargetPos.x = pLife->m_curX + randdistance * cosf(nRadian);
	vtargetPos.y = pLife->m_curY + randdistance * sinf(nRadian);
	//rfalse(2, 1, "CreateRadomPoint TargetPosX = %4.2f, TargetPosY = %4.2f",vtargetPos.x,vtargetPos.y);
	return 1;
}

float CMonster::GetWalkSpeed()
{
	return m_Property.m_WalkSpeed;
}
