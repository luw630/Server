#include "StdAfx.h"
#include "BaseObject.h"
#include "globalfunctions.h"
#include "factionmanager.h"

#include "networkmodule/chatmsgs.h"
#include "networkmodule/orbmsgs.h"
#include "region.h"
#include "time.h"
#include "scriptmanager.h"
#include "pub/smcacheex.h"
#include "networkmodule/refreshmsgs.h"
#include "liteserializer/lite.h"

#include "factionregion.h"
#include "networkmodule/netprovidermodule/netprovidermoduleadv.h"
#include "���������/dmainapp.h"
#include "gameworld.h"

#include "SanguoCode\MailMoudle\MailMoudle.h"
extern "C" {
#include "lauxlib.h"
}

extern LPIObject FindRegionByID( DWORD ID );
#define CureentDayNum 3

struct FactionMsg
{
	DWORD FactionID;
	bool State;
	FactionMsg()
	{
		FactionID = 0;
		State = false;
	}
};
struct Map_FactionMsg
{

	int count;
	bool state;
	DWORD times;
	FactionMsg FMsg[CureentDayNum];
	Map_FactionMsg()
	{
		count = 0;
		state = false;
		memset(FMsg, 0, sizeof(FMsg));

	}
};

std::map< DWORD, Map_FactionMsg > RequestFactionMsgMap;

struct PlayerMsg
{
	char delteerName[CONST_USERNAME];
	PlayerMsg()
	{
		memset(delteerName, 0, sizeof(delteerName));
		
	}
};
std::map< std::string, PlayerMsg > DelteOfFaction;

//..
static void SendToAllMember( SFactionData &faction, LPVOID pMsg, DWORD dwSize, DNID exceptDnid = 0);
BOOL GetFactionInfo( const char* szFactionName, SFactionData& stFactionInfo );
//BOOL GetFactionHeaderInfo( LPCSTR szFactionName, SFactionData::SFaction &factionHeader );
static void CheckMember( SFactionData::SMember &stMember, int nCheckValue );
static BOOL SetFactionMemberJoinTime( CPlayer *pPlayer, const BOOL isAdd );
static void SendFactionInfo( const SFactionData &stFaction, const DNID dnidClient );
static LPCSTR szfactionTitle[9] = { "����","��ҽ", "��ҽ", "����", "̳��","�һ���", "�󻤷�","������", "����" };
static LPCSTR szKingCityTitle[6] = { "̫ξ", "ة��", "͢ξ", "�ٸ�", "��ξ", "����" };

BOOL factionLoadSuccess = FALSE;
// �ڴ�����õ�
//ObjectCacheEx< SFactionData, MAX_FACTION_NUMBER * 2 >  mFactionManager( "FactionManager", 20 );
std::map< std::string, SFactionData > factionManagerMap;
std::map< WORD, std::string > factionIdMap;        // ����ID������ӳ���
WORD TopFactionID[ 5 ] = { 0 };                    // ���ٶ�ǰ5λ����ID�б�
string TopFactionName[ 5 ] = { "", "", "", "", ""};// ���ٶ�ǰ5λ���ɰ����б�
DWORD FactionFightInf[2][5] = { 0 };               // ��ǿ��ս��Ϣ [0]ʤ������ [1]�ܴ���

// ȫ���õ�����
SAInitFactionInfoMsg stInitFacMsg;	// ��ʼ����ҽ�ɫ�İ�������

static SFactionData factionBuffer;

static const int WeeklySalary[] = { 4000, 50000, 50000, 10000, 30000, 40000, 40000, 60000, 100000 };

struct callbackForDelMember
{
	void operator () ( SFactionData &faction, DWORD pos )
	{
		memmove( &faction.stMember[pos], &faction.stMember[pos + 1], 
			sizeof( SFactionData::SMember ) * ( faction.stFaction.byMemberNum - pos ) );
		faction.stFaction.byMemberNum--;
	}
};

struct CallbackForUpdateMember
{
	CallbackForUpdateMember() { memset( this, 0, sizeof( *this ) ); }

	void operator () ( SFactionData &faction, DWORD _pos )
	{
        pos = _pos;
        SFactionData::SMember &member = faction.stMember[pos];
        if ( pos == 0 && member.Title != 8 )
        {
            reinterpret_cast< DWORD& >( member ) |= ~0x7ffff;
            member.Title = 8;
        }

        if ( member.Title <= 8 )    // �޸���н 
            member.dwWeeklySalary = WeeklySalary[ member.Title ];

        if ( player )
        {
		    member.Level = player->m_Property.m_Level;
		    member.dnidClient = player->m_ClientIndex;
		    member.wServerID = GetServerID();
		    member.IsOnline = isOnline;
            member.School = player->m_Property.m_School;
	        if ( player->m_Property.m_Sex )
	        {
		        if ( member.School == 1 ) 
			        member.School = 5;
		        else if( member.School >= 3 ) 
			        member.School = member.School + 3;
	        }

		    //if ( player->m_Property.m_sXValue > 0 )         member.Camp = 0;
		    //else if ( player->m_Property.m_sXValue < 0 )    member.Camp = 2;
		    //else                                            member.Camp = 1;

            // �������ID, ֱ�Ӹ���
//		    player->m_Property.factionId = faction.stFaction.factionId;
        }

        if ( newTitle ) 
            dwt::strcpy( member.szCustomTitle, newTitle, sizeof( member.szCustomTitle ) );

		// �����ҪCOPYһ�ݳ�ȥ�Ļ�......
		if ( copyFaction != NULL )  *copyFaction = faction;
        if ( copyMember != NULL )   *copyMember = member;
	}

	SFactionData            *copyFaction;
    SFactionData::SMember   *copyMember;
	CPlayer                 *player;
    BOOL                    isOnline;
	LPCSTR                  newTitle;
    DWORD                   pos;
};

struct CallbackForSetEmpower
{
	CallbackForSetEmpower() { memset( this, 0, sizeof( *this ) ); }

	void operator () ( SFactionData &faction, DWORD pos )
	{
        SFactionData::SMember &member = faction.stMember[pos];

        if ( ( member.Title < rawTitle ) && ( member.Title >= 3 ) )
        {
            reinterpret_cast< DWORD& >( member ) |= ( 1 << ( newEmpower + 18 ) );

            if ( copyMember != NULL )
                *copyMember = member;

            result = true;
        }
	}

    SFactionData::SMember   *copyMember;
    DWORD                   rawTitle;
    DWORD                   newEmpower;
    BOOL                    result;
};

struct CallbackForShelve
{
	CallbackForShelve() { memset( this, 0, sizeof( *this ) ); }

	void operator () ( SFactionData &faction, DWORD pos )
	{
        SFactionData::SMember &member = faction.stMember[pos];

        if ( rawTitle == -2 )
        {
            // ��������ڼ���˰�������
            result = ( member.Title == 0 );
        }
        else if ( ( rawTitle == -1 ) || ( member.Title <= 2 && rawTitle == 8 ) || ( member.Title < rawTitle ) )
        {
			// ֻ�а������ܽ�ʹ�ʦ�ֺ�ʦ��
			if ( rawTitle < 8 && member.Title <= 2 ) 
				return ;

            reinterpret_cast< DWORD& >( member ) &= 0x7ffff;
            member.Title = 0;
            member.dwWeeklySalary = WeeklySalary[ 0 ];

            if ( copyMember != NULL )
                *copyMember = member;

            result = true;
        }
    }

    SFactionData::SMember   *copyMember;
    DWORD                   rawTitle;
    BOOL                    result;
};

void CallbackForDeleteFaction( CPlayer *player )
{
    player->UpdateMemberData( NULL, NULL, 0, true );
	
}

template < typename _func > int lockedFindMember( LPCSTR factionName, LPCSTR playerName, _func &func )
{
    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( factionName );
    if ( iter == factionManagerMap.end() )
        return -2;

    SFactionData &faction = iter->second;
    if ( faction.stFaction.byMemberNum >= MAX_MEMBER_NUMBER )
        return -2;

    int result = -1;
    for ( int i = 0; i < faction.stFaction.byMemberNum; i++ )
    {
        if ( dwt::strcmp( faction.stMember[i].szName, playerName, sizeof( faction.stMember[i].szName ) ) == 0 )
        {
            result = i;
            func( iter->second, i );
            break;
        }
    }

    return result;
    
	//DWORD dwHandle = mFactionManager.Locate( factionName );
	//if ( dwHandle == 0xffffffff )
	//	return result;

	//if ( SFactionData *faction = mFactionManager.Useseg( dwHandle ) )
	//{
	//	// �����������
	//	result = -1;
	//	for ( DWORD n = 0; n < faction->stFaction.byMemberNum; n++ )
	//	{
	//		if( dwt::strcmp( faction->stMember[n].szName, 
	//			playerName, sizeof( faction->stMember[n].szName ) ) == 0 )
	//		{
	//			result = n;
	//			func( faction, n );
	//			break;
	//		}
	//	}

	//	mFactionManager.Useend( faction );
	//}

	//return result;
}

template < typename _func > void traverseMembersOnLocal( SFactionData &faction, _func &func )
{
	for ( DWORD i = 0; i < faction.stFaction.byMemberNum; i++ )
	{
        // ����ֻ��һ������
		//if ( faction.stMember[i].wServerID != GetServerID() )
		//	continue;

		CPlayer *pPlayer = ( CPlayer* )
			GetPlayerByName( faction.stMember[i].szName )->DynamicCast( IID_PLAYER );

		if ( pPlayer )
			func( pPlayer );
	}
}

BOOL UpdateFaction( SFactionData::SFaction &faction )
{
    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( faction.szFactionName );
    if ( iter == factionManagerMap.end() )
        return FALSE;

    SFactionData &update = iter->second;

    update.stFaction.buildingStub.parentId = faction.buildingStub.parentId;
    update.stFaction.buildingStub.uniqueId = faction.buildingStub.uniqueId;
    update.stFaction.byNpcNum = faction.byNpcNum;
    update.stFaction.byOpenRApplyJoin = faction.byOpenRApplyJoin;
    update.stFaction.dwFactionProsperity = faction.dwFactionProsperity;  // ���ٶ�

    return TRUE;

	//if ( faction.szFactionName[0] == 0 )
	//	return FALSE;

	//DWORD dwHandle = mFactionManager.Locate( faction.szFactionName );
	//if ( dwHandle == 0xffffffff )
	//	return FALSE;

	//SFactionData *pFaction = mFactionManager.Useseg( dwHandle );
	//if ( pFaction == NULL )
	//	return FALSE;

	//pFaction->stFaction.buildingStub.parentId = faction.buildingStub.parentId;
	//pFaction->stFaction.buildingStub.uniqueId = faction.buildingStub.uniqueId;
	//pFaction->stFaction.byBuildingNum = faction.byBuildingNum; 
	//pFaction->stFaction.byNpcNum = faction.byNpcNum;
	////pFaction->stFaction.factionId = faction.stFaction.factionId;

	////memcpy( &pFaction->stFaction , &faction.stFaction , sizeof( pFaction->stFaction ) );
	//mFactionManager.Useend( pFaction );

	//return TRUE;
}

static CFactionManager *uniqueThis = NULL;

CFactionManager::CFactionManager(void) 
{
    uniqueThis = this;
	m_bRefused = false;
}

CFactionManager::~CFactionManager(void)
{
    uniqueThis = NULL;
}

//���ɰ���ID
DWORD CFactionManager::GetFactionAIID()
{
	int fid = 0;
	int now = factionManagerMap.size();
	int SId = (int)GetGroupID();
	char sSId[4];
	char sSId4[7];
	

	sprintf(sSId4, "%d", SId);
	
	//��ȡ����λ
	strncpy(sSId, sSId4 + 3, 4);
	fid = now * 10000 + atoi(sSId);
	


	std::map< std::string, SFactionData >::iterator iter;

	bool flag = false;

	for (iter = factionManagerMap.begin(); iter != factionManagerMap.end(); iter++)
	{		
		//�������ID
		if (iter->second.stFaction.factionId >= fid)
		{
			fid = iter->second.stFaction.factionId;

		}
	}
	now++;
	fid = now * 10000 + atoi(sSId);
	return fid;
}
// ��������
BOOL CFactionManager::CreateFaction(LPCSTR szFactionName, CPlayer* pPlayer, DWORD factionId, SQCreatetchTong *Msg)
{
	SACreatetchTong *pMsg = new SACreatetchTong();


     if (pPlayer == NULL || szFactionName[0] == 0 )
         return FALSE;
 
	 //��������ID
	 factionId = GetFactionAIID();

     // �����Ӳ������������
     /*if ( factionId < 10000 || factionId > 65535 )
         return FALSE;*/
 
   //  if (pPlayer->m_Property.m_szTongName[0] != 0)
   //  {
   //      //TalkToDnid(pPlayer->m_ClientIndex, "�����㻹���������ɵİ���Ŷ��");
   //      //TalkToDnid(pPlayer->m_ClientIndex, "�����ɲ������㴴���µİ��ɣ�");
		 //pMsg->_result = Enum_TongMsg_ErrorCode::Tong_IntoIng;
		 //g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(*pMsg));
   //      return FALSE;

   //  }

 
     if (!IsValidName(szFactionName))
     {
         //TalkToDnid(pPlayer->m_ClientIndex, "�����İ������ֿ��µò�������ͬ�����Ͽɡ���");
		 /*pMsg->_result = Enum_TongMsg_ErrorCode::Tong_VaildName;
		 g_StoreMessage(pPlayer->m_ClientIndex, &pMsg, sizeof(*pMsg));
         return FALSE;*/
     }
 
     // �Ƿ��Ѿ����ﴴ�����ޡ�����������
     if ( factionManagerMap.size() >= MAX_FACTION_NUMBER )
     {
         //TalkToDnid(pPlayer->m_ClientIndex, "�������ֵİ��������Ѿ��ﵽ���ޣ�");
         //TalkToDnid(pPlayer->m_ClientIndex, "���¿��Կ����߽��ĳ������Ȼ��ȡ����֮����");
		 pMsg->_result = Enum_TongMsg_ErrorCode::Tong_MaxCount;
		 g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(*pMsg));
         return FALSE;
     }
// 
//     
     // �����ֵİ���Ѿ�����
 	//if ( mFactionManager.Locate( szFactionName ) != 0xffffffff )
     if ( factionManagerMap.find( szFactionName ) != factionManagerMap.end() )
     {
         //TalkToDnid(pPlayer->m_ClientIndex, "���������ã���ϧ�������Ѿ����������ֵİ��ɴ����ˡ���");
		 pMsg->_result = Enum_TongMsg_ErrorCode::Tong_ReName;

		 g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(*pMsg));
         return FALSE;
     }
 
     // ���ͨ������������
     SFactionData temp;
 	ZeroMemory( &temp.stFaction, sizeof( temp.stFaction ) );
 
 	// ���ð���ͷ����
     temp.stFaction.factionId            = factionId;

	 //memcpy(temp.stFaction.factionIcon, Msg->factionIcon, sizeof(Msg->factionIcon));
	temp.stFaction.factionIcon = Msg->factionIcon;
 	temp.stFaction.byMemberNum			= 1;
 	temp.stFaction.byFactionLevel		= 1;
 	temp.stFaction.dwFactionProsperity	= 0;
 	temp.stFaction.iMoney				= 0;
 	dwt::strcpy( temp.stFaction.szCreatorName, pPlayer->m_FixData.m_Name,  sizeof( temp.stFaction.szCreatorName ) );
 	dwt::strcpy( temp.stFaction.szFactionName, szFactionName,	    sizeof( temp.stFaction.szFactionName ) );
 	temp.stFaction.szFactionMemo[0] = 0;
	temp.stFaction.Rhlistcount = 0;
	memset(temp.stFaction.RjoinList, 0, sizeof(temp.stFaction.RjoinList));

 	// ���������뵽��Ա�б���ȥ
 	ZeroMemory( &temp.stMember[0], sizeof( temp.stMember[0] ) );
 	temp.stMember[0].dnidClient			= pPlayer->m_ClientIndex;
 	temp.stMember[0].wServerID			= GetServerID();
	temp.stMember[0].Level = pPlayer->m_FixData.m_dwLevel;
 	temp.stMember[0].Title = 8;
     temp.stMember[0].dwWeeklySalary = WeeklySalary[ 8 ];
 	temp.stMember[0].School = pPlayer->m_Property.m_School;
 	if ( pPlayer->m_Property.m_Sex )
 	{
 		if( temp.stMember[0].School == 1 ) 
 			temp.stMember[0].School = 5;
 		else if( temp.stMember[0].School >= 3 ) 
 			temp.stMember[0].School = temp.stMember[0].School + 3;
 	}
 
 	/*if ( pPlayer->m_Property.m_sXValue < 0 )
 		temp.stMember[0].Camp = 2;
 	else if ( pPlayer->m_Property.m_sXValue > 0 )
 		temp.stMember[0].Camp = 0;	
 	else*/
 		temp.stMember[0].Camp = 1;
 
 	temp.stMember[0].Appoint = TRUE;
 	temp.stMember[0].Accept = TRUE;
 	temp.stMember[0].Authorize = TRUE;
 	temp.stMember[0].Dismiss = TRUE;
 	//temp.stMember[0].EspecialTitle = TRUE;
 	temp.stMember[0].Editioner = TRUE;
 	//temp.stMember[0].Resignation = TRUE;
 	temp.stMember[0].CastOut = TRUE;
 	//temp.stMember[0].QuitFaction = TRUE;
 	temp.stMember[0].IsOnline = TRUE;
 	temp.stMember[0].iDonation = 0;
 	temp.stMember[0].dwAccomplishment = 0;
 	temp.stMember[0].dwWeeklySalary = 0;
	dwt::strcpy(temp.stMember[0].szName, pPlayer->m_FixData.m_Name, CONST_USERNAME);
 	temp.stMember[0].szCustomTitle[0] = 0;
 	temp.stMember[0].dwJoinTime = (DWORD)time(NULL);
	__int64 now;
	temp.stMember[0].LastLineTime = _time64(&now);
 	BOOL succeeded = FALSE;
 	SFactionData& pFaction = factionManagerMap[szFactionName];
	temp.stMember[0].PlayerIconID = pPlayer->m_FixData.m_PlayerIconID;
    factionManagerMap[szFactionName] = temp;
    
 	if ( pFaction.stFaction.byMemberNum == 0 )
 		memcpy( &pFaction, &temp, sizeof(SFactionData) ), succeeded = TRUE;
 
 	//mFactionManager.Useend( &pFaction );
 
 	// �ݴ������ȡ��һ���Ѿ���������ע������
 	/*if ( !succeeded )
 		return FALSE;*/
 
 	// ������ҵİ�������
 	dwt::strcpy( pPlayer->m_Property.m_szTongName, szFactionName, CONST_USERNAME );
 	memcpy( &pPlayer->m_stFacRight, &temp.stMember[0], sizeof( pPlayer->m_stFacRight ) );
 	SetFactionMemberJoinTime( pPlayer, TRUE );

	//���������Ϣ���뵽SFixData
//	memccpy(pPlayer->m_FixData.m_FactionName, szFactionName, 0, sizeof(pPlayer->m_FixData.m_FactionName));

	//�۳�Ԫ��
	pPlayer->m_FixData.m_dwDiamond -= CConfigManager::getSingleton()->globalConfig.CreateFactionNeedMoney;

	//�������  ���ؿͻ��� �ɹ�����
	pMsg->_result = Enum_TongMsg_ErrorCode::Tong_OK;
	g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(*pMsg));
    /*if (pPlayer == NULL || szFactionName[0] == 0 )
        return FALSE;*/



 	/*pPlayer->m_Property.factionId = ( WORD )factionId;
 	pPlayer->m_Property.m_dwSysTitle[1] = 3008;*/


 	//pPlayer->SendMyState();
 
   //  TalkToAll( "һ���°��ɳ����ˣ���ӭ���ӻԾ������" );
 	 //TalkToAll( FormatString( "������Ϣ������[%s]����[%s]", szFactionName, pPlayer->GetName() ) );
   //  TalkToDnid( pPlayer->m_ClientIndex, "��İ����Ѿ��ɹ������ˣ�" );
   //  TalkToDnid( pPlayer->m_ClientIndex, "���������ͨ���������������������Ҽ�����İ��ɣ�" );
   //  TalkToDnid( pPlayer->m_ClientIndex, "���Ѿ����Լ����ɵ���������ѡ���俪����" );
 
     // ���°���������Ч���ݵ��ͻ��˴�
 //SendFactionInfo( temp, pPlayer->m_ClientIndex );


 //   // �����Ӳ������������
 //   if ( factionId < 10000 || factionId > 65535 )
 //       return FALSE;

 //   if (pPlayer->m_Property.m_szTongName[0] != 0)
 //   {
 //       TalkToDnid(pPlayer->m_ClientIndex, "�����㻹���������ɵİ���Ŷ��");
 //       TalkToDnid(pPlayer->m_ClientIndex, "�����ɲ������㴴���µİ��ɣ�");
 //       return FALSE;
 //   }

 //   if (!IsValidName(szFactionName))
 //   {
 //       TalkToDnid(pPlayer->m_ClientIndex, "�����İ������ֿ��µò�������ͬ�����Ͽɡ���");
 //       return FALSE;
 //   }

 //   // �Ƿ��Ѿ����ﴴ�����ޡ�����������
 //   if ( factionManagerMap.size() >= MAX_FACTION_NUMBER )
 //   {
 //       TalkToDnid(pPlayer->m_ClientIndex, "�������ֵİ��������Ѿ��ﵽ���ޣ�");
 //       TalkToDnid(pPlayer->m_ClientIndex, "���¿��Կ����߽��ĳ������Ȼ��ȡ����֮����");
 //       return FALSE;
 //   }

 //   
 //   // �����ֵİ���Ѿ�����
	////if ( mFactionManager.Locate( szFactionName ) != 0xffffffff )
 //   if ( factionManagerMap.find( szFactionName ) != factionManagerMap.end() )
 //   {
 //       TalkToDnid(pPlayer->m_ClientIndex, "���������ã���ϧ�������Ѿ����������ֵİ��ɴ����ˡ���");
 //       return FALSE;
 //   }

 //   // ���ͨ������������
 //   //SFactionData temp;
	//ZeroMemory( &temp.stFaction, sizeof( temp.stFaction ) );

	//// ���ð���ͷ����
 //   temp.stFaction.factionId            = ( WORD )factionId;
	//temp.stFaction.byMemberNum			= 1;
	//temp.stFaction.byFactionLevel		= 1;
	//temp.stFaction.dwFactionProsperity	= 0;
	//temp.stFaction.iMoney				= 0;
	//dwt::strcpy( temp.stFaction.szCreatorName, pPlayer->GetName(),  sizeof( temp.stFaction.szCreatorName ) );
	//dwt::strcpy( temp.stFaction.szFactionName, szFactionName,	    sizeof( temp.stFaction.szFactionName ) );
	//temp.stFaction.szFactionMemo[0] = 0;

	//// ���������뵽��Ա�б���ȥ
	//ZeroMemory( &temp.stMember[0], sizeof( temp.stMember[0] ) );
	//temp.stMember[0].dnidClient			= pPlayer->m_ClientIndex;
	//temp.stMember[0].wServerID			= GetServerID();
	//temp.stMember[0].Level = pPlayer->m_Property.m_Level;
	//temp.stMember[0].Title = 8;
 //   temp.stMember[0].dwWeeklySalary = WeeklySalary[ 8 ];
	//temp.stMember[0].School = pPlayer->m_Property.m_School;
	//if ( pPlayer->m_Property.m_Sex )
	//{
	//	if( temp.stMember[0].School == 1 ) 
	//		temp.stMember[0].School = 5;
	//	else if( temp.stMember[0].School >= 3 ) 
	//		temp.stMember[0].School = temp.stMember[0].School + 3;
	//}
	///*
	//if ( pPlayer->m_Property.m_sXValue < 0 )
	//	temp.stMember[0].Camp = 2;
	//else if ( pPlayer->m_Property.m_sXValue > 0 )
	//	temp.stMember[0].Camp = 0;	
	//else
	//	temp.stMember[0].Camp = 1;
	//*/
	//temp.stMember[0].Appoint = TRUE;
	//temp.stMember[0].Accept = TRUE;
	//temp.stMember[0].Authorize = TRUE;
	//temp.stMember[0].Dismiss = TRUE;
	////temp.stMember[0].EspecialTitle = TRUE;
	//temp.stMember[0].Editioner = TRUE;
	////temp.stMember[0].Resignation = TRUE;
	//temp.stMember[0].CastOut = TRUE;
	////temp.stMember[0].QuitFaction = TRUE;
	//temp.stMember[0].IsOnline = TRUE;
	//temp.stMember[0].iDonation = 0;
	//temp.stMember[0].dwAccomplishment = 0;
	//temp.stMember[0].dwWeeklySalary = 0;
	//dwt::strcpy( temp.stMember[0].szName, pPlayer->GetName(),  sizeof( temp.stMember[0].szName  ) );
	//temp.stMember[0].szCustomTitle[0] = 0;
	//temp.stMember[0].dwJoinTime = (DWORD)time(NULL);

	////BOOL succeeded = FALSE;
	////SFactionData& pFaction = factionManagerMap[szFactionName];

 //   factionManagerMap[szFactionName] = temp;
 //  
	////if ( pFaction.stFaction.byMemberNum == 0 )
	////	memcpy( &pFaction, &temp, sizeof(SFactionData) ), succeeded = TRUE;

	////mFactionManager.Useend( &pFaction );

	//// �ݴ������ȡ��һ���Ѿ���������ע������
	////if ( !succeeded )
	////	return FALSE;

	//// ������ҵİ�������
	//dwt::strcpy( pPlayer->m_Property.m_szTongName, szFactionName, CONST_USERNAME );
	//memcpy( &pPlayer->m_stFacRight, &temp.stMember[0], sizeof( pPlayer->m_stFacRight ) );
	//SetFactionMemberJoinTime( pPlayer, TRUE );

	////pPlayer->m_Property.factionId = ( WORD )factionId;
	////pPlayer->m_Property.m_dwSysTitle[1] = 3008;
	////pPlayer->SendMyState();

 //   TalkToAll( "һ���°��ɳ����ˣ���ӭ���ӻԾ������" );
	//TalkToAll( FormatString( "������Ϣ������[%s]����[%s]", szFactionName, pPlayer->GetName() ) );
 //   TalkToDnid( pPlayer->m_ClientIndex, "��İ����Ѿ��ɹ������ˣ�" );
 //   TalkToDnid( pPlayer->m_ClientIndex, "���������ͨ���������������������Ҽ�����İ��ɣ�" );
 //   TalkToDnid( pPlayer->m_ClientIndex, "���Ѿ����Լ����ɵ���������ѡ���俪����" );

 //   // ���°���������Ч���ݵ��ͻ��˴�
	//SendFactionInfo( temp, pPlayer->m_ClientIndex );


    return TRUE;
}

// ɾ������(����Ҫ����ʺź�����������)
BOOL CFactionManager::DeleteFaction(LPCSTR szFactionName, CPlayer	*pPlayer)
{
	SADisposeFaction pMsg;
    if( szFactionName == NULL || szFactionName[0] == 0 )
        return FALSE;

    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( szFactionName );
    if ( iter == factionManagerMap.end() )
        return FALSE;

    /*g_Script.SetCondition( NULL, NULL, NULL );
    LuaFunctor( g_Script, "OnFactionClear" )[ szFactionName ]();
    g_Script.CleanCondition();*/

    SFactionData &faction = iter->second;
	int fid = iter->second.stFaction.factionId;
	// ������е�ǰ�����ϵ���Ұ��ɹ�ϵ
	//traverseMembersOnLocal( faction, CallbackForDeleteFaction );

    // �����ɵĴ���ɾ������������ڵĻ���
    /*if ( faction.stFaction.buildingStub.uniqueId != 0 )
    {
        CRegion *region = ( CRegion* )FindRegionByID( 
            ( WORD )faction.stFaction.buildingStub.parentId )->DynamicCast( IID_REGION );

        if ( region && region->RemoveBuilding( faction.stFaction.buildingStub.uniqueId ) )
            reinterpret_cast< QWORD& >( faction.stFaction.buildingStub ) = 0;
    }*/


   
	/*TalkToAll( FormatString( "[%s]������������ظ������ڽ�ɢ�ˡ���", szFactionName ) );
	TalkToAll( "һ���µİ���������˳�������ҸϿ���ע����" );*/
    
	LPCSTR szfname = szFactionName;
	SADletePalyer  SAmsg;
	//�޸���Ұ������
	CPlayer	*Player;
	for (int i = 0; i < faction.stFaction.byMemberNum; i++)
	{

		Player = (CPlayer *)(GetPlayerByName(faction.stMember[i].szName)->DynamicCast(IID_PLAYER));
		if (Player == NULL)
			continue;
//		memset(Player->m_FixData.m_FactionName, 0, sizeof(pPlayer->m_FixData.m_FactionName));
		SAmsg._protocol = Tong_MemberLeave;
		g_StoreMessage(Player->m_ClientIndex, &SAmsg, sizeof(SAmsg));
	}
	

	//���ȫ�ֵ���������
	std::map< DWORD, Map_FactionMsg >::iterator riter;
	for (riter = RequestFactionMsgMap.begin(); riter != RequestFactionMsgMap.end(); riter++)
	{
		for (int i = 0; i < riter->second.count; i++)
		{
			if (riter->second.FMsg[i].FactionID == fid)
			{
				memset(&riter->second.FMsg[i], 0, sizeof(riter->second.FMsg[i]));
			}
		}
	}
	faction.stFaction.byMemberNum = 0;
	factionManagerMap.erase(iter);
	g_StoreMessage(pPlayer->m_ClientIndex, &pMsg, sizeof(pMsg));
	//SFactionData *faction = mFactionManager.Useseg( mFactionManager.Locate( pMsg->szFactionName ) );
	//if ( faction == NULL )
	//	return;

    //reinterpret_cast< QWORD& >( faction.stFaction.buildingStub ) = 
    //    reinterpret_cast< QWORD& >( factionBuffer.stFaction.buildingStub );

    // ȷ��ÿ�������������ɾ����
	//DWORD lastTimes = -- faction.stFaction.byDelete;
	//if ( lastTimes == 0 ) faction.stFaction.byMemberNum = 0;
	//mFactionManager.Useend( faction );
	//if ( lastTimes == 0 )
	//{
	//	//mFactionManager.Erase( pMsg->szFactionName );
 //       factionManagerMap.erase( szFactionName );
	//	TalkToAll(FormatString("[%s]������������ظ������ڽ�ɢ�ˡ���", pMsg->szFactionName));
	//	TalkToAll("һ���µİ���������˳�������ҸϿ���ע����");
	//}

    

    //DWORD handle = mFactionManager.Locate( szFactionName );
	//if ( handle == 0xffffffff )
	//	return FALSE;

	//SQSendServerMsg msg;
	//dwt::strcpy( msg.szFactionName, szFactionName, sizeof(msg.szFactionName) );
	//SendToLoginServer( &msg, sizeof( msg ) );
    return TRUE;
}



//���뵽���ŵ������б�
BOOL CFactionManager::RequestFactionListToList(CPlayer *pPlayer, LPCSTR szFactionName)
{
	//CPlayer *pPlayer = (CPlayer *)(GetPlayerByName(pname)->DynamicCast(IID_PLAYER));
	SARequestJoinFactionToList msg;

	std::map< std::string, SFactionData >::iterator iter;
	iter = factionManagerMap.find(szFactionName);


	if (iter == factionManagerMap.end())
	{
		msg.result = Enum_TongMsg_ErrorCode::Tong_VaildName;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
		return false;
	}

	//�ı��������������
	SimFactionData::RequestJionPlayer fmsg;
	fmsg.headIconID = pPlayer->m_FixData.m_PlayerIconID;

	fmsg.level = pPlayer->m_FixData.m_dwLevel;
	LPCSTR pname = pPlayer->m_FixData.m_Name;
	memcpy(fmsg.name, pname, sizeof(fmsg.name));
	
	int num = iter->second.stFaction.Rhlistcount;

	if (iter->second.stFaction.Rhlistcount < MAX_JoinListCount )
	{
		for (int j = 0; j < MAX_JoinListCount; j++)
		{
			if (iter->second.stFaction.RjoinList[j].level == 0)
			{
				iter->second.stFaction.RjoinList[j] = fmsg;
				iter->second.stFaction.Rhlistcount++;
				break;
			}
		}
	}
	else
	{
		msg.result = Enum_TongMsg_ErrorCode::Tong_RequestJionCurDayToMax;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
		return false;
	}

	//Ϊȫ�������б���󹹽�����
	FactionMsg famsg;
	Map_FactionMsg map_fmsg;
	std::map< DWORD, Map_FactionMsg >::iterator fiter;
	fiter = RequestFactionMsgMap.find(pPlayer->GetSID());
	if (fiter == RequestFactionMsgMap.end())
	{
		
		if (map_fmsg.count == CureentDayNum)
			{
				msg.result = Enum_TongMsg_ErrorCode::Tong_RequestJionCurDayToMax;
				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
				return false;
			}

		famsg.FactionID = iter->second.stFaction.factionId;
		famsg.State = false;
		int a = fiter->second.count;
		map_fmsg.FMsg[0] = famsg;

		__int64 now;
		map_fmsg.times = _time64(&now);;
		map_fmsg.count += 1;

		RequestFactionMsgMap[pPlayer->GetSID()] = map_fmsg;
	}
	else
	{
		famsg.FactionID = iter->second.stFaction.factionId;
		famsg.State = false;
		__int64 now;
		map_fmsg.times = _time64(&now);
		fiter->second.FMsg[fiter->second.count] = famsg;
		fiter->second.count += 1;
	}

	//���뵽ȫ������map
	//RequestFactionMsgMap[pPlayer->GetSID()]= 

	//������� ������Ϣ
	//memcpy(pPlayer->m_FixData.m_FactionName, iter->second.stFaction.szFactionName, sizeof(pPlayer->m_FixData.m_FactionName));
	msg.result = Tong_OK;
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	return true;
	//fmsg.name = pPlayer->GetName();
	//std::map< std::string, SimFactionData::RequestJionPlayer >::iterator itr;

	//iter->second.stFaction.RJlist[pname] = fmsg;

	/*itr = iter->second.stFaction.RJlist.find(pname);
	if (itr == iter->second.stFaction.RJlist.end())
	{
		msg.result = Enum_TongMsg_ErrorCode::Tong_VaildName;
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
		return false;
	}*/

}



//�˳�����
BOOL CFactionManager::DelMember(CPlayer *pPlayer, LPCSTR szTongName, LPCSTR szName, BOOL isLeave)
{
 	if( szTongName[0] == 0 )
 		return FALSE;
	SADletePalyer  msg;
 	CPlayer * Player = (CPlayer*)(GetPlayerByName(szName)->DynamicCast(IID_PLAYER));

	//����Ƿ�����
	if (Player == NULL)
	{
		PlayerMsg pmsg;
		//strcpy_s(pmsg.delteerName, szName);
		memcpy(pmsg.delteerName, szName, strlen(szName));
		//strcpy(pmsg.delteerName, szName);
		DelteOfFaction[pmsg.delteerName] = pmsg;
		
		lockedFindMember(szTongName, szName, callbackForDelMember());
		g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
		return false;
	}
 	if (GetGW()->m_FactionManager.m_bRefused)
 	{   
 		if (Player && isLeave)
 			TalkToDnid(Player->m_ClientIndex, GetGW()->m_FactionManager.m_sErrInfo.c_str());
 		return FALSE;
 	}
 
 	// lock and update //���Ҳ�ɾ����Ա
     if ( lockedFindMember( szTongName, szName, callbackForDelMember( ) ) < 0 )
         return FALSE;
 	
 	// ��������˰�ʱ��
 	if (Player)
 	{
 		//pPlayer->m_Property.dwLastLeaveFactionTime = static_cast<DWORD>( time( NULL ) );
 		//pPlayer->m_Property.wContribution = 0; // �˰���հﹱֵ
 	}
 
 	// ���������ϵİ�������,���Ȼ�㲥��Ϣ���������������ϵ�����
 	// ��������ߣ���ô���һ���߾ͻ����¼��
 	SAUpdateMemberMsg deletemsg;
 	deletemsg.byType = isLeave ? SAUpdateMemberMsg::E_DELETE_MEMBER_LEAVE : SAUpdateMemberMsg::E_DELETE_MEMBER;
 	strcpy( deletemsg.stInfo.szName, szName );
 	//SendFactionAllServer( szTongName, SQSendServerMsg::UPDATEMEMBER, &deletemsg, sizeof(deletemsg) );

	//�����Ϣ���뵽SFixData
//	memset(Player->m_FixData.m_FactionName, 0, sizeof(Player->m_FixData.m_FactionName));

	//�������ų�
	g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
	//������T����
	msg._protocol = Tong_MemberLeave;
	g_StoreMessage(Player->m_ClientIndex, &msg, sizeof(msg));
	return TRUE;
}

BOOL CFactionManager::SendFactionHeadInfo( CPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return FALSE;

	if( pPlayer->m_Property.m_szTongName[0] == 0 )
		return FALSE;

    SFactionData::SFaction stFaction;
// 	if( !GetFactionHeaderInfo( pPlayer->m_Property.m_szTongName, stFaction ) )
// 		return FALSE;

	SAUpdataFactionHeadMsg msg;
	dwt::strcpy( msg.szFactionMemo, stFaction.szFactionMemo, sizeof( msg.szFactionMemo ) );
	msg.byFactionLevel	 = stFaction.byFactionLevel;
	msg.dwFactionProsperity	 = stFaction.dwFactionProsperity;

    // ���ɵȼ� * ( 5 + ���ɳ�Ա�� * 1 + NPC * 2 + ������ * 2 )
    // msg.dwTotalMoney = stFaction.byFactionLevel * ( 5 + stFaction.byMemberNum * 1 + 3 * 2 + 2 * 1 );
    // ���Ե����ɵȼ���ά���ѵ�Ӱ��
    //msg.dwTotalMoney = 5 + stFaction.byMemberNum * 1 + 3 * 2 + 2 * 1;
    msg.dwTotalMoney = 5 + stFaction.byMemberNum * 1 + stFaction.byNpcNum * 2 + stFaction.byNpcNum * 2;

    /*
	msg.dwTotalMoney	 = ( ( stFaction.stFaction.byMemberNum * stFaction.stFaction.byFactionLevel * 50000 ) +
			( 3 * stFaction.stFaction.byFactionLevel * 50000 ) + 
			( 3 * 10000 ) +
			( stFaction.stFaction.byFactionLevel * 50000 ) );
    */

	msg.iMoney			= stFaction.iMoney;
	msg.dwWood			= stFaction.dwWood;
	msg.dwStone			= stFaction.dwStone;
	msg.dwMine			= stFaction.dwMine;
	msg.dwPaper			= stFaction.dwPaper;
	msg.dwJade			= stFaction.dwJade;
    msg.byOpenRApplyJoin= stFaction.byOpenRApplyJoin;

	g_StoreMessage( pPlayer->m_ClientIndex, &msg, sizeof( msg ) );

	return TRUE;
}

BOOL UpdateFactionMember( CPlayer *player )
{
    return uniqueThis->UpdateMember( NULL, player, true, true );
}

BOOL CheckFactionMember( CPlayer *player )
{
    // ����ʱ�����δ������ɵ�����Ƿ���ĳһ�������У�
    return uniqueThis->CheckMemberAtLogin( player );
}

BOOL CFactionManager::CheckMemberAtLogin( CPlayer *pPlayer )
{
    LPCSTR playerName = pPlayer->GetName();
	for (std::map< std::string, SFactionData >::iterator iter = factionManagerMap.begin();
		iter != factionManagerMap.end(); iter++)
	{
		SFactionData &faction = iter->second;
		for (int i = 0; i < faction.stFaction.byMemberNum; i++)
		{
			if (dwt::strcmp(faction.stMember[i].szName, playerName, sizeof(faction.stMember[i].szName)) == 0)
			{
				dwt::strcpy(pPlayer->m_Property.m_szTongName, faction.stFaction.szFactionName,
					sizeof(pPlayer->m_Property.m_szTongName));
				return true;
			}
		}
	}
    return false;
}

// ���¶�Ա��Ϣ
BOOL CFactionManager::UpdateMember( SFactionData *pFaction, CPlayer* pPlayer, BOOL isLogin, BOOL sendDisable )
{
	if ( pPlayer == NULL || pPlayer->m_Property.m_szTongName[0] == 0 )
		return FALSE;

    SFactionData::SMember member;
    CallbackForUpdateMember checker;
    checker.player = pPlayer;
    checker.copyFaction = pFaction;
    checker.copyMember = &member;
    checker.isOnline = isLogin;
    int result = lockedFindMember( pPlayer->m_Property.m_szTongName, pPlayer->GetName(), checker );
    if ( result < 0 )
    {
        pPlayer->UpdateMemberData( NULL, NULL, 0, true );
		TalkToDnid( pPlayer->m_ClientIndex, ( result == -1 ) ? 
			"�ܿ�ϧ���㱻����İ��������ˡ�����" : "�����ڵİ����Ѿ�����ɢ���ˡ�����" );

        return FALSE;
    }

    if ( !sendDisable )
    {
		// �㲥�¸�����Ϣ
		SAUpdateMemberMsg msg;
		msg.byType = SAUpdateMemberMsg::E_UPDATE_MEMBER;
        msg.stInfo = member;
		SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::UPDATEMEMBER, &msg, sizeof( msg ) );
    }

    // ���������UPDATEMEMBER����ʱ����������!
	pPlayer->UpdateMemberData( &member, NULL, NULL, false );
    return TRUE;
}

// �㲥��Ϣ���������������
//SQSendServerMsg stMsg;
static DWORD prev = timeGetTime(); 
void CFactionManager::SendFactionAllServer( LPCSTR szFaction, const BYTE byMsgTyte, LPVOID pMsg, WORD wSize, WORD wRegion, DNID exceptDnid)
{
	if ( szFaction[0] == 0 || pMsg == NULL || wSize == 0 || byMsgTyte > SQSendServerMsg::MSGCOPY )
		return;

	//stMsg.byMsgType = byMsgTyte;
	//stMsg.wRegion = wRegion;
	//dwt::strcpy( stMsg.szFactionName, szFaction, sizeof(stMsg.szFactionName) );
	//memcpy( stMsg.szBuffer, pMsg, wSize );

    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( szFaction );
    if ( iter == factionManagerMap.end() )
        return ;

    SFactionData &faction = iter->second;

    if ( byMsgTyte == SQSendServerMsg::TALK )
    {
        SAChatFactionMsg msg;
        memcpy( &msg, pMsg, sizeof( msg ) );
		SendToAllMember( iter->second, &msg, msg.GetMySize()/*sizeof( msg )*/ ); /// ֻ����ʵ�ʵĳ���
    }
    else if ( byMsgTyte == SQSendServerMsg::UPDATEMEMBER )
    {
        SAUpdateMemberMsg stUpdateMsg;
        memcpy( &stUpdateMsg, pMsg, sizeof(stUpdateMsg) );

        CPlayer *pPlayer = ( CPlayer* )GetPlayerByName( stUpdateMsg.stInfo.szName )->DynamicCast( IID_PLAYER );
       /* if ( pPlayer == NULL )
            return ;*/ //�Ҳ�������������

        // ��Ϊ�漰��ͬ�������Ի�����Ҫȡ����������
        //if ( !GetFactionInfo( pMsg->szFactionName, factionBuffer ) )
        //    return;

        switch ( stUpdateMsg.byType )
        {
        case SAUpdateMemberMsg::E_UPDATE_MEMBER:
            // ���ͼ������˱仯, �͸�ͬ��������
            // ��ʾ����
//             if ( pPlayer )
//             {
//                 pPlayer->UpdateMemberData( &stUpdateMsg.stInfo, szFaction, faction.stFaction.factionId, 
//                     ( pPlayer->m_Property.m_dwSysTitle[1] != stUpdateMsg.stInfo.Title + 3000 ) );
//             }
            break;

        case SAUpdateMemberMsg::E_DELETE_MEMBER:
        case SAUpdateMemberMsg::E_DELETE_MEMBER_LEAVE:
            if ( pPlayer )
                pPlayer->UpdateMemberData( NULL, NULL, 0, true );
            break;

        default:
            return;
        }

        SendToAllMember( iter->second, &stUpdateMsg, sizeof( stUpdateMsg ) );
    }
	else if ( byMsgTyte == SQSendServerMsg::CITYOWENTALK )//�����ڰ���˵�������ˡ���������֪ͨ���а���
	{
		/*
		SAChatFactionMsg msg;
		memcpy( &msg, pMsg, sizeof( msg ) );

		if ( abs( ( int )( timeGetTime() - prev ) ) < 1000 * 30)//���ͼ��30��
		{		
			CPlayer *pPlayer = ( CPlayer* )GetPlayerByName( msg.cTalkerName )->DynamicCast( IID_PLAYER );	
			if ( pPlayer == NULL )
				return ;	

			TalkToDnid(pPlayer->m_ClientIndex, "�������������(��)��ʱ����˵̫��Ŷ��");
			return;
		}

		prev = timeGetTime();


		std::map< std::string, SFactionData >::iterator factioniter;
		for ( factioniter = factionManagerMap.begin(); factioniter != factionManagerMap.end(); factioniter++ )
			SendToAllMember( factioniter->second, &msg, sizeof( msg ) );
		//*/
	}
	else if ( byMsgTyte == SQSendServerMsg::MSGCOPY )
	{
		SendToAllMember( iter->second, pMsg, wSize, exceptDnid);
	}
}

// ORBת����������Ϣ���������ﴦ��
// 3in1�����
void CFactionManager::ProcessSSMsg( SQSendServerMsg* pMsg )
{
	if ( pMsg == NULL )
		return;

	// ��������,������Ϣ�Ĳ�ͬ���в�ͬ��ת��
	if ( pMsg->byMsgType == SQSendServerMsg::TALK )
	{
		//if ( pMsg->wRegion == 0 )
		//{
  //          // ����������ܵĻ���������ֱ�ӻ���������ݣ��ٶ�ʱ����
	 //       if ( !GetFactionInfo( pMsg->szFactionName, factionBuffer ) )
		//        return;

		//	SAChatFactionMsg msg;
		//	memcpy( &msg, pMsg->szBuffer, sizeof( msg ) );
		//	SendToAllMember( factionBuffer, &msg, sizeof( msg ) );
		//}
        // �ݲ������������
        //else if ( CRegion *region = ( CRegion* )FindRegionByID( pMsg->wRegion )->DynamicCast( IID_REGION ) )
        //{
        //    SFactionData::SFaction faction;
        //    if ( !GetFactionHeaderInfo( pMsg->szFactionName, faction ) )
        //        return;
        //}
	}
	else if ( pMsg->byMsgType == SQSendServerMsg::DELFACTION )
	{
		//// �������˵�ǿ���Ϣ,ֻ��Ҫֱ�Ӵ���
	 //   if ( !GetFactionInfo( pMsg->szFactionName, factionBuffer ) )
		//    return;

		//// ������е�ǰ�����ϵ���Ұ��ɹ�ϵ
		//traverseMembersOnLocal( factionBuffer, CallbackForDeleteFaction );

  //      // �����ɵĴ���ɾ������������ڵĻ���
  //      if ( factionBuffer.stFaction.buildingStub.uniqueId != 0 )
  //      {
  //          CRegion *region = ( CRegion* )FindRegionByID( 
  //              ( WORD )factionBuffer.stFaction.buildingStub.parentId )->DynamicCast( IID_REGION );

  //          if ( region && region->RemoveBuilding( factionBuffer.stFaction.buildingStub.uniqueId ) )
  //              reinterpret_cast< QWORD& >( factionBuffer.stFaction.buildingStub ) = 0;
  //      }

		//SFactionData *faction = mFactionManager.Useseg( mFactionManager.Locate( pMsg->szFactionName ) );
		//if ( faction == NULL )
		//	return;

  //      reinterpret_cast< QWORD& >( faction->stFaction.buildingStub ) = 
  //          reinterpret_cast< QWORD& >( factionBuffer.stFaction.buildingStub );
		//DWORD lastTimes = -- faction->stFaction.byDelete;
		//if ( lastTimes == 0 ) faction->stFaction.byMemberNum = 0;

		//mFactionManager.Useend( faction );

		//if ( lastTimes == 0 )
		//{
		//	mFactionManager.Erase( pMsg->szFactionName );
		//	TalkToAll(FormatString("[%s]������������ظ������ڽ�ɢ�ˡ���", pMsg->szFactionName));
		//	TalkToAll("һ���µİ���������˳�������ҸϿ���ע����");
		//}
	}
	else if( pMsg->byMsgType == SQSendServerMsg::UPDATEMEMBER )
	{
		SAUpdateMemberMsg stUpdateMsg;
		memcpy( &stUpdateMsg, pMsg->szBuffer, sizeof(stUpdateMsg) );

        CPlayer *pPlayer = ( CPlayer* )GetPlayerByName( stUpdateMsg.stInfo.szName )->DynamicCast( IID_PLAYER );

		// ��Ϊ�漰��ͬ�������Ի�����Ҫȡ����������
	    if ( !GetFactionInfo( pMsg->szFactionName, factionBuffer ) )
		    return;

		switch ( stUpdateMsg.byType )
		{
		case SAUpdateMemberMsg::E_UPDATE_MEMBER:
            // ���ͼ������˱仯, �͸�ͬ��������
//             if ( pPlayer ) pPlayer->UpdateMemberData( &stUpdateMsg.stInfo, pMsg->szFactionName, 
//                                 factionBuffer.stFaction.factionId, 
//                                 pPlayer->m_Property.m_dwSysTitle[1] != stUpdateMsg.stInfo.Title + 3000 );
			break;

		case SAUpdateMemberMsg::E_DELETE_MEMBER:
		case SAUpdateMemberMsg::E_DELETE_MEMBER_LEAVE:
            if ( pPlayer ) pPlayer->UpdateMemberData( NULL, NULL, 0, true );
			break;
		}

		SendToAllMember( factionBuffer, &stUpdateMsg, sizeof( stUpdateMsg ) );
	}
}

void CFactionManager::EditFactionMemo(LPCSTR szFaction, LPCSTR szMemo, CPlayer * pPlayer)
{
	SAChangeFactionMemo msg;
    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( szFaction );
    if ( iter == factionManagerMap.end() )
        return ;

    SFactionData &faction = iter->second;
    //dwt::strcpy( faction.stFaction.szFactionMemo, szMemo, sizeof( faction.stFaction.szFactionMemo ) );
	memccpy(faction.stFaction.szFactionMemo, szMemo, 0, sizeof(faction.stFaction.szFactionMemo));
	//DWORD dwHandle = mFactionManager.Locate( szFaction );
	//if ( dwHandle == 0xffffffff )
	//	return;
	//if ( SFactionData *faction = mFactionManager.Useseg( dwHandle ) )
	//{
	//	dwt::strcpy( faction->stFaction.szFactionMemo, szMemo, sizeof( faction->stFaction.szFactionMemo ) );
	//	mFactionManager.Useend( faction );
	//}
	SAChangeFactionMemo Smsg;
	g_StoreMessage(pPlayer->m_ClientIndex, &Smsg, sizeof(Smsg));
}

void CFactionManager::SendEditFactionMemoMsg( CPlayer *pPlayer )
{
	if( pPlayer == NULL )
		return;

	if( pPlayer->m_Property.m_szTongName[0] == 0 )
		return;

    SFactionData::SFaction faction;
// 	if( !GetFactionHeaderInfo( pPlayer->m_Property.m_szTongName, faction ) )
// 		return;

	// if( dwt::strcmp( faction.stMember[0].szName, pPlayer->GetName(), sizeof( faction.stMember[0].szName ) ) != 0 )
    if ( pPlayer->m_stFacRight.Title != 8 )
	{
		TalkToDnid( pPlayer->m_ClientIndex, "��û��Ȩ�����޸İ�������!!!" );
		return;
	}

	SAInputMemoMsg msg;
	dwt::strcpy( msg.szFactionMemo, faction.szFactionMemo, sizeof( msg.szFactionMemo ) );
	g_StoreMessage( pPlayer->m_ClientIndex, &msg, sizeof( msg ) );
}

void SendFactionInfo( const SFactionData &stFaction, const DNID dnidClient )
{
	if ( ( dnidClient == 0 ) || ( stFaction.stFaction.byMemberNum == 0 ) )
		return;

	dwt::strcpy( stInitFacMsg.szFactionName, stFaction.stFaction.szFactionName, sizeof( stInitFacMsg.szFactionName ) );
	dwt::strcpy( stInitFacMsg.szCreatorName, stFaction.stFaction.szCreatorName, sizeof( stInitFacMsg.szCreatorName ) );
	stInitFacMsg.byMemberNum = stFaction.stFaction.byMemberNum;

	for( DWORD n = 0; n < stFaction.stFaction.byMemberNum; n++ )
		memcpy( &stInitFacMsg.stMemberInfo[n], &stFaction.stMember[n], sizeof( stInitFacMsg.stMemberInfo[n] ) );

	WORD wSize = sizeof( stInitFacMsg ) - sizeof( stInitFacMsg.stMemberInfo[0] ) * MAX_MEMBER_NUMBER;
	wSize += sizeof( stInitFacMsg.stMemberInfo[0] ) * stInitFacMsg.byMemberNum;
	g_StoreMessage( dnidClient, &stInitFacMsg, wSize );
}

static void SendToAllMember( SFactionData &stFaction, LPVOID pMsg, DWORD dwSize ,DNID exceptDnid)
{
	if( stFaction.stFaction.byMemberNum == 0 )
		return;

	for( DWORD n = 0; n < stFaction.stFaction.byMemberNum; n++ )
	{
		if (exceptDnid != stFaction.stMember[n].dnidClient)
		{
			g_StoreMessage( stFaction.stMember[n].dnidClient, pMsg, ( WORD )dwSize );
		}

	}

}

struct _GetFactionHeaderInfo
{
    BOOL operator () ( LPCSTR key, SFactionData &data )
    {
        if ( factionId == data.stFaction.factionId )
        {
            *copyOut = data.stFaction;
            return -1;
        }

        return true;
    }

    WORD factionId;
    SFactionData::SFaction *copyOut;
};


// ��İ��ɵ�����
BOOL GetFactionInfo( LPCSTR szFactionName, SFactionData& stFactionInfo )
{
    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( szFactionName );
    if ( iter == factionManagerMap.end() )
        return FALSE;

    stFactionInfo = iter->second;
	//DWORD handle = mFactionManager.Locate( szFactionName );
	//if ( handle == 0xffffffff )
	//	return FALSE;

	//SFactionData* temp = mFactionManager.Useseg( handle );
	//if ( temp == NULL )
	//	return FALSE;

	//memcpy( &stFactionInfo, temp, sizeof(stFactionInfo) - 
	//	( MAX_MEMBER_NUMBER - temp->stFaction.byMemberNum ) * sizeof( SFactionData::SMember ) );

	//mFactionManager.Useend( temp );

	return TRUE;
}

static BOOL SetFactionMemberJoinTime( CPlayer *pPlayer, const BOOL isAdd )
{
// 	if(pPlayer == NULL)
// 		return FALSE;
// 
// 	int TaskID = (int)g_Script.GetGlobalValue("FACTION_CONFIG_JOINTIME_TASKID");
// 	
// 	if ( TaskID <= 0 )
// 		return FALSE;
// 
// 	SRoleTask  task;
// 	ZeroMemory(&task, sizeof(SRoleTask));
// 	task.wTaskID = TaskID;
// 	
// 	if ( !isAdd )
// 	{
// 		SRoleTask *pTask = pPlayer->FindTask(TaskID);
// 		if(pTask)
// 			pPlayer->DelTask(pTask);
// 	}
// 	else
// 	{
// 		task.bSave = true;
// 		task.wComplete = (DWORD)time(0);
// 		pPlayer->AddTask(task, FALSE);
// 	}

	return TRUE;
}

BOOL SetAccess( CPlayer *player, LPCSTR memberName, DWORD value )
{
    // Ȩ����ߵ�����
    if ( value > 8 )
        return FALSE;

    // ԽȨ�ж�, ��������
    if ( ( player->m_stFacRight.Title <= value ) && ( value < 8 ) )
        return FALSE;

    // �������²�����������Ȩ!
    if ( player->m_stFacRight.Title < 3 ) 
        return FALSE;

    // ֻ�а�����������ʦ��\��ʦ��
    if ( ( value < 3 ) && ( player->m_stFacRight.Title != 8 ) )
        return FALSE;

	SAUpdateMemberMsg msga, msgb;
    msgb.byType = 0xff;

    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( player->m_Property.m_szTongName );
    if ( iter == factionManagerMap.end() )
        return FALSE;

    SFactionData& faction = iter->second;
	//DWORD handle = mFactionManager.Locate( player->m_Property.m_szTongName );
	//if ( handle == 0xffffffff )
	//	return FALSE;

	//SFactionData* faction = mFactionManager.Useseg( handle );
	//if ( faction == NULL )
	//	return FALSE;

    int check[9] = { 0,0,0,0,0,0,0,0,0 };
    int checkMax[9] = { 999,1,1,6,4,1,1,1,1 };

    SFactionData::SMember *member = NULL;

	for ( int i = 0; i < faction.stFaction.byMemberNum; i++ )
	{
        if ( dwt::strcmp( faction.stMember[i].szName, memberName, 
            sizeof( faction.stMember->szName ) ) == 0 )
			member = &faction.stMember[i];
        else
            check[ faction.stMember[i].Title ] ++;
	}

    BOOL succCheck = FALSE;

    if ( member == NULL )
        goto _err;

    // ԽȨ�ж�
    if ( player->m_stFacRight.Title <= member->Title )
        goto _err;

	if ( member->Title == value )
		goto _err;

    if ( value == 8 )
	{
        BOOL isUpdatWS = player->IsInFactionFirst();

        SFactionData::SMember temp = faction.stMember[0];
		double dElapsed_time = abs( difftime(time( NULL ), ( time_t )temp.dwJoinTime ));
		if ( dElapsed_time / 3600 < (7 * 24) )
		{
			TalkToDnid(temp.dnidClient, "�����β������죬���ܽ��а���ְ���ƽ�����");
			goto _err;
		}

        // �����°���������Ȩ��
        reinterpret_cast< DWORD& >( *member ) |= ~0x7ffff;
        member->Title = 8;
        member->dwWeeklySalary = WeeklySalary[ 8 ];
        faction.stMember[0] = *member;

        // ����ɰ���������Ȩ��
        reinterpret_cast< DWORD& >( temp ) &= 0x7ffff;
        temp.Title = 0;
        temp.dwWeeklySalary = WeeklySalary[ 0 ];
        *member = temp;

        // �����޸ĺ�, �°����ĵ�ַ�����˱仯, ������Ҫ�����޸ĵ�ַ����Ӧmember
        member = faction.stMember;

        // ������ͬ��״̬, ���þɰ���������
		msgb.byType = SAUpdateMemberMsg::E_UPDATE_MEMBER;
		msgb.stInfo = temp;

        // ������������
        if ( isUpdatWS )
        {
            g_Script.SetCondition( NULL, player, NULL );
            LuaFunctor( g_Script, "OnKCAuthorize" )[ 4008 ][ member->szName ]();
            g_Script.CleanCondition();
        }
    }
    else
    {
        // ְ�������ж�, �ų���������Ϊ������ְ��ת��
        if ( check[value] >= checkMax[value] )
        {
            TalkToDnid(player->m_ClientIndex, "��ǰְλ�������������Ƚ�Ƹ���������ְλ��");
            goto _err;
        }

        if ( value < 3 )
        {
           /* if ( ( value == 1 ) && ( member->School == 0 || member->School > 4 ) ) goto _err;
            if ( ( value == 2 ) && ( member->School > 0 && member->School <= 4 ) ) goto _err;*/

            // ��ʦ��\���ʼ��û���κ�Ȩ����
            reinterpret_cast< DWORD& >( *member ) &= 0x7ffff;
        }
    }

    // ����Ȩ��, ���ݰ������ֵ����⴦��
    member->Title = value;
    member->dwWeeklySalary = WeeklySalary[ value ];

    member->dwJoinTime = static_cast<DWORD>( time( NULL ) );

    msga.byType = SAUpdateMemberMsg::E_UPDATE_MEMBER;
	msga.stInfo = *member;

    succCheck = TRUE;

_err:
	//mFactionManager.Useend( faction );

    if ( !succCheck )
        return FALSE;

	// ͨ��
	SAChatFactionMsg msgchat;
	msgchat.wResult = TRUE;
	dwt::strcpy( msgchat.cTalkerName, "", sizeof( msgchat.cTalkerName ) );
	_snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "��ϲ[%s]������Ϊ[%s]", 
        msga.stInfo.szName, szfactionTitle[ msga.stInfo.Title ] );
	msgchat.cChatData[ sizeof( msgchat.cChatData ) - 1 ] = 0;
	CFactionManager::SendFactionAllServer( player->m_Property.m_szTongName, SQSendServerMsg::TALK, &msgchat, sizeof( msgchat ) );

	CFactionManager::SendFactionAllServer( player->m_Property.m_szTongName, SQSendServerMsg::UPDATEMEMBER, &msga, sizeof( msga ) );

    if ( msgb.byType == SAUpdateMemberMsg::E_UPDATE_MEMBER )
    {
		CFactionManager::SendFactionAllServer( player->m_Property.m_szTongName, SQSendServerMsg::UPDATEMEMBER, &msgb, sizeof( msgb ) );
        player->UpdateMemberData( &msgb.stInfo, NULL, 0, true );
    }

    for ( int i = 0; i < 9; i ++ )
    {
        if ( check[i] > checkMax[i] )
            rfalse( 2, 1, "[%s][%s]ְ�����������쳣��", player->m_Property.m_szTongName, szfactionTitle[i] );
    }

    return TRUE;
}

// ������Ա�������ز���
void CFactionManager::RecvMemberManager( CPlayer *pPlayer, SQMemberManager* pMsg )
{
	if ( pPlayer == NULL || pMsg == NULL || pPlayer->m_Property.m_szTongName[0] == 0 )
		return ;

	LPCSTR destName = NULL;
	LPCSTR titleName = NULL;
	int value = 0;

	try
	{
		lite::Serialreader slr( pMsg->streamData, sizeof( pMsg->streamData ) );
		destName = slr();

		if ( pMsg->tpye == SQMemberManager::TITLENAME )
			titleName = slr();
		else if (	pMsg->tpye == SQMemberManager::AUTHORIZE || 
					pMsg->tpye == SQMemberManager::EMPOWER ||
                    pMsg->tpye == SQMemberManager::KCAUTHORIZE||
                    pMsg->tpye == SQMemberManager::KCQUIT )
			value = slr();
	}
	catch ( lite::Xcpt & )
	{
		return ;
	}

    SimFactionData::SimMemberInfo &access = pPlayer->m_stFacRight;

    SAUpdateMemberMsg msg;
    SAChatFactionMsg msgchat;
    SFactionData::SMember member;
    member.szName[0] = 0;
    msgchat.cChatData[0] = 0;

	switch ( pMsg->tpye )
	{
	case SQMemberManager::AUTHORIZE:// ����
		{
			if ( access.Appoint == 0 || destName == NULL )
				return ;

            SetAccess( pPlayer, destName, value );
		}
		break;

    case SQMemberManager::EMPOWER:	// ��Ȩ
        {
            if ( access.Authorize == 0 || destName == NULL )
                return ;

            CallbackForSetEmpower checker;
            checker.copyMember = &member;
            checker.newEmpower = value;
            checker.rawTitle = access.Title;

            // ����ʧ�ܺ�ֱ�ӷ���
			if ( ( lockedFindMember( pPlayer->m_Property.m_szTongName, destName, checker ) < 0 ) || !checker.result )
				return;

			const char szAuthorize[7][CONST_USERNAME] = { "", "����Ȩ", "����Ȩ", "��ȨȨ", "���Ȩ", "����Ȩ", "����Ȩ" };
            _snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "��ϲ[%s]������[%s]!", destName, szAuthorize[value] );
        }
        break;

    case SQMemberManager::SHELVE:	// ���
        {
            if ( access.Dismiss == 0 || destName == NULL )
                return;

            CallbackForShelve checker;
            checker.copyMember = &member;
            checker.rawTitle = access.Title;

            // ����ʧ�ܺ�ֱ�ӷ���
			if ( ( lockedFindMember( pPlayer->m_Property.m_szTongName, destName, checker ) < 0 ) || !checker.result )
				return;

            _snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "[%s]����Ͳ�ȡ������Ȩ��!", destName );
        }	
        break;

    case SQMemberManager::ABDICATE:	// ��ְ
        {
            if ( access.Title == 0 || access.Title >= 8 )
                return;

            CallbackForShelve checker;
            checker.copyMember = &member;
            checker.rawTitle = -1;

            // ����ʧ�ܺ�ֱ�ӷ���
			if ( ( lockedFindMember( pPlayer->m_Property.m_szTongName, pPlayer->GetName(), checker ) < 0 ) || !checker.result )
				return;

            _snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "[%s]��ѹ�������ȥ������ְ��!", destName );
        }
        break;

    case SQMemberManager::BANISH:	// ����
        {
            if ( access.CastOut == 0 || destName == NULL )
                return ;

            BOOL isUpdatWS = pPlayer->IsInFactionFirst();

            CallbackForShelve checker;
            checker.copyMember = &member;
            checker.rawTitle = -2;

            // ����ʧ�ܺ�ֱ�ӷ���
			if ( ( lockedFindMember( pPlayer->m_Property.m_szTongName, destName, checker ) < 0 ) || !checker.result )
				return;

            if ( isUpdatWS )
            {
                g_Script.SetCondition( NULL, pPlayer, NULL );
                LuaFunctor( g_Script, "OnKCAuthorize" )[ 9 ][ destName ](); // 9��ʾ���Ǳ����𣬹�������Ϊ0
                g_Script.CleanCondition();
            }

           /* if (DelMember( pPlayer->m_Property.m_szTongName, destName, FALSE ))
				_snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "[%s]�Ѿ�������˰���!", destName );
			else
				TalkToDnid( pPlayer->m_ClientIndex, GetGW()->m_FactionManager.m_sErrInfo.c_str());*/
        }
        break;

	case SQMemberManager::QUIT:		// �˰�
		{
			if ( access.Title != 0 )
				return;

			/*if (DelMember( pPlayer->m_Property.m_szTongName, pPlayer->GetName(), TRUE ))
				_snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "[%s]���ĵ��뿪�����ǵİ���!", destName );*/
        }
		break;

	case SQMemberManager::TITLENAME:
		{
			if ( access.Dismiss == 0 || destName == NULL || titleName == NULL )
				return;

			// ֻ�ܰ���ʹ��
			if ( access.Title != 8 )
				return;

            CallbackForUpdateMember checker;
            checker.newTitle = titleName;
            checker.copyMember = &member;

            // ����ʧ�ܺ�ֱ�ӷ���
			if ( lockedFindMember( pPlayer->m_Property.m_szTongName, destName, checker ) < 0 )
				return;

            _snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "���ڰ����谮�м�,Ϊ[%s]����һ����ͷ��!", destName );
		}
		break;

    case SQMemberManager::KCAUTHORIZE:  // �������� ������Ȩ��
        {
            if ( pPlayer == NULL )
                return;

            if ( access.Title != 8 || destName == NULL || pPlayer->m_Property.m_Name == NULL )
                return;

            if ( dwt::strcmp( pPlayer->m_Property.m_Name, destName , sizeof( pPlayer->m_Property.m_Name ) ) == 0 ) // ���������Լ�
                return;

            // ���µ�һ��ı�־�ж�
            if ( !pPlayer->IsInFactionFirst() )
                return;

            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnKCAuthorize" )[ value ][ destName ]();
            g_Script.CleanCondition();

            if ( value > 0 && value <= 6 )
                _snprintf( msgchat.cChatData, sizeof( msgchat.cChatData ), "[%s]����������ְ��[%s]", destName,  szKingCityTitle[value - 1] );
        }
        break;

    case SQMemberManager::KCQUIT:
        {
            if ( pPlayer == NULL )
                return;

            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnKCAuthorize" )[ value ][ destName ]();
            g_Script.CleanCondition();
        }
        break;
	}

    if ( member.szName[0] )
    {
        msg.byType = SAUpdateMemberMsg::E_UPDATE_MEMBER;
        msg.stInfo = member;
        SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::UPDATEMEMBER, &msg, sizeof( msg ) );
    }

    if ( msgchat.cChatData[0] )
    {
        msgchat.wResult = TRUE;
        msgchat.cTalkerName[0] = 0;
        msgchat.cChatData[ sizeof( msgchat.cChatData ) - 1 ] = 0;
        SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::TALK, &msgchat, sizeof( msgchat ) );
    }
}

BOOL CFactionManager::OperFacRes( LPCSTR szFacName, const DWORD dwType, const int nNum )
{
    if ( szFacName == NULL )
        return FALSE;

    std::map< std::string, SFactionData >::iterator iter;
    iter = factionManagerMap.find( szFacName );
    if ( iter == factionManagerMap.end() )
        return FALSE;

    SFactionData& faction = iter->second;

	//DWORD handle = mFactionManager.Locate( szFacName );
	//if ( handle == 0xffffffff )
	//	return FALSE;

	//SFactionData* temp = mFactionManager.Useseg( handle );
	//if ( temp == NULL )
	//	return FALSE;

    //SimFactionData::SimFactionInfo *pFacinfo = &temp->stFaction;

    BOOL result = FALSE;
    LPDWORD data = NULL;

    switch ( dwType )
    {
        case 0: data = &faction.stFaction.iMoney;           break;
        case 1: data = &faction.stFaction.dwWood;           break;
        case 2: data = &faction.stFaction.dwStone;          break;
        case 3: data = &faction.stFaction.dwPaper;          break;
        case 4: data = &faction.stFaction.dwMine;           break;
        case 5: data = &faction.stFaction.dwJade;           break;
        case 6: data = &faction.stFaction.dwFactionProsperity;  break;

        case 7: 
	        if ( nNum < 0 && faction.stFaction.byFactionLevel < ( DWORD )abs( nNum ) )
                goto _err;

	        if ( nNum > 0 && (DWORD)( 0xff - faction.stFaction.byFactionLevel ) < ( DWORD )nNum )
                goto _err;

            faction.stFaction.byFactionLevel += ( BYTE )nNum;
            result = TRUE;
            goto _err;
    }

    if ( data == NULL )
        goto _err;

	if ( nNum < 0 && *data < ( DWORD )abs( nNum ) )
        goto _err;

	if ( nNum > 0 && ( 0xffffffff - *data ) < ( DWORD )nNum )
        goto _err;

    *data += nNum;

    result = TRUE;

_err:
	//mFactionManager.Useend( temp );

	return result;
}

struct callbackForUpdateSimMember
{
	callbackForUpdateSimMember( SimFactionData::SimMemberInfo &tempMember ) : tempMember( tempMember ) {}

	void operator () ( SFactionData &faction, DWORD pos )
	{
		faction.stMember[pos].iDonation = tempMember.iDonation;
		faction.stMember[pos].dwAccomplishment = tempMember.dwAccomplishment;
	}

	SimFactionData::SimMemberInfo &tempMember;
};

BOOL CFactionManager::UpdataMemberInfo( CPlayer *pPlayer )
{
	if( pPlayer == NULL )
		return FALSE;

	if( pPlayer->m_Property.m_szTongName[0] == 0 )
		return FALSE;
	
	if ( lockedFindMember( pPlayer->m_Property.m_szTongName, pPlayer->GetName(), 
		callbackForUpdateSimMember( pPlayer->m_stFacRight ) ) < 0 )
        return FALSE;

	SAUpdateMemberMsg msg;
	msg.byType = SAUpdateMemberMsg::E_UPDATE_MEMBER;
	msg.stInfo = pPlayer->m_stFacRight;
	SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::UPDATEMEMBER, &msg, sizeof( msg ) );

    return TRUE;
}

//struct _GetFactionIds
//{
//    BOOL operator () ( LPCSTR key, SFactionData &data )
//    {
//        factionIds->push_back( data.stFaction.factionId );
//        return true;
//    }
//
//    std::list< WORD > *factionIds;
//};

void CFactionManager::GetFactionIds( std::list< WORD > &ids )
{
    std::map< std::string, SFactionData >::iterator iter;
    for ( iter = factionManagerMap.begin(); iter != factionManagerMap.end(); iter++ )
         ids.push_back( iter->second.stFaction.factionId );

    //_GetFactionIds gfids;
    //gfids.factionIds = &ids;
    //mFactionManager.Traverse( gfids );
}

void CFactionManager::LoadFactionDataList(SAGetFactionDataMsg *pMsg)
{

	//rfalse(2, 1, "LoadFactionDataList %d", pMsg->nFactions);
	if (pMsg == NULL || pMsg->nFactions > MAX_SENDFACTIONDATA)
		return;

	// ���ﲻ�ܵ����������,��ΪSAGetFactionMsg���ܶ���յ�...
	// mFactionManager.Clear();

	if (pMsg->nFactions > 0)
	{
		LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
		const DWORD dwFactionMax = pMsg->nFactions;

		for (DWORD n = 0; n < dwFactionMax; n++)
		{
			SaveFactionData_Lua FactionData;
			memset(&FactionData, 0, sizeof(SaveFactionData_Lua));

			memcpy(&FactionData, streamIter, sizeof(SaveFactionData_Lua));
			streamIter += sizeof(SaveFactionData_Lua);

			lua_State *ls = g_Script.ls;
			if (lua_gettop(ls) == 0)
			{

				lua_settop(ls, 0);
				lua_getglobal(ls, "LoadFactionData");
				if (lua_isnil(ls, -1))
				{
					//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
					lua_pop(ls, 1);
					return;
				}

				lua_newtable(ls);
				g_Script.SetTableNumber(ls, "FactionID", FactionData.FactionID);
				
				
// 				lua_createtable(g_Script.ls, 0, 0);
// 				if (!lua_istable(ls, -1))
// 				{
// 					return;
// 				}
				int ck = luaEx_unserialize(ls, FactionData.bFactionData, FactionData.dfactionlenth);
				if (ck <= 0)
				{
					break;
				}
				lua_rawseti(ls, -2, 1);

				if (lua_pcall(ls, 1, 0, 0) != 0)
				{
					char err[1024];
					sprintf(err, "%s\r\n", lua_tostring(ls, -1));
					rfalse(2, 1, err);
					lua_settop(ls, 0);
					return;
				}
			}
		}
		rfalse(2, 1, "��ȡ�������ݳɹ�,%d������", pMsg->nFactions);
	}
}

// ������������õ�

static DWORD itemAccumulator = 0;
static DWORD prevTimestamp = 0;
static time_t timeSegment = 0;

static BOOL timeAccumulatorInit()
{
    tm timeMaker; 
    ZeroMemory(&timeMaker, sizeof(tm)); 
    timeMaker.tm_year = SItemBase::YEAR_SEGMENT - 1900;
    timeSegment = mktime(&timeMaker);
    return TRUE;
}

static BOOL timeAccumulatorInited = timeAccumulatorInit();

SaveFactionData stFaction[MAX_FACTION_NUMBER];
SQSaveFactionMsg stSaveMsg;
WORD wFactionNum = 0;


SaveFactionData temp;

void CFactionManager::SaveFaction( BOOL bSaveMode )
{
    // ���û���յ����ݿ�İ��ɶ�ȡȷ����Ϣ����ʲôҲ������������
    if ( !factionLoadSuccess )
        return;

    ZeroMemory( &stFaction, sizeof(stFaction) );
    stSaveMsg.nServerID = GetGroupID();
    stSaveMsg.bSaveMode = bSaveMode;
    wFactionNum = 0;
    stSaveMsg.nTime = static_cast< int >( time( NULL ) - timeSegment );

    if ( factionManagerMap.size() == 0 && bSaveMode )
    {
		stSaveMsg.dwSendSize = 0;
		stSaveMsg.nFactions = 0;

        SendToLoginServer( &stSaveMsg, stSaveMsg.dwSendSize + 
			( sizeof( stSaveMsg ) - sizeof( stSaveMsg.streamFaction ) ) );
    }

    std::map< std::string, SFactionData >::iterator iter;
    
    // ׼����ʼ����
    DWORD nFactions = 0;
	LPBYTE streamIter = (LPBYTE)stSaveMsg.streamFaction;
    for ( iter = factionManagerMap.begin(); iter != factionManagerMap.end(); iter++ )
    {
        if ( nFactions > MAX_FACTION_NUMBER || iter->second.stMember[0].szName[0] == 0 )
            continue ;

        // ȡ(��ͷ��Ϣ����Ա��Ϣ)
        size_t size = sizeof( SaveFactionData::SaveFactionInfo ) + iter->second.stFaction.byMemberNum * 
            sizeof( SaveFactionData::SaveMemberInfo );

        if ( stSaveMsg.dwSendSize + size >= sizeof( stSaveMsg.streamFaction ) )
        {
            stSaveMsg.nFactions = nFactions;
            SendToLoginServer( &stSaveMsg, stSaveMsg.dwSendSize + 
                ( sizeof( stSaveMsg ) - sizeof( stSaveMsg.streamFaction ) ) );

            streamIter = (LPBYTE)stSaveMsg.streamFaction;
            nFactions = 0;
            stSaveMsg.dwSendSize = 0;
        }

        stSaveMsg.dwSendSize += (DWORD)size;
        ZeroMemory( &temp, sizeof( temp ) );

        memcpy( &temp, &iter->second.stFaction, sizeof( SimFactionData::SimFactionInfo ) );

        for( int n = 0; n < iter->second.stFaction.byMemberNum; n++ )
            memcpy( &temp.stMember[n], &iter->second.stMember[n], 
                sizeof( SimFactionData::SimMemberInfo ) );

        // д�����ͷ����
        *( SaveFactionData::SaveFactionInfo* )streamIter = temp.stFaction;
        streamIter += sizeof( SaveFactionData::SaveFactionInfo );

        // ��Ա����
        memcpy( streamIter, temp.stMember, sizeof( SaveFactionData::SaveMemberInfo ) * 
            iter->second.stFaction.byMemberNum );

        streamIter += sizeof( SaveFactionData::SaveMemberInfo ) * iter->second.stFaction.byMemberNum;

        nFactions ++;
    }

	if ( nFactions )
	{
		stSaveMsg.nFactions = nFactions;
		SendToLoginServer( &stSaveMsg, stSaveMsg.dwSendSize + 
			( sizeof( stSaveMsg ) - sizeof( stSaveMsg.streamFaction ) ) );
	}
}

void CFactionManager::DeleteFactionRecall(SADeleteFaction *pMsg) //Ҳ����Ҫ���ݿⷵ�غ��ɾ����Ҳ���Բ���
{
	lua_State *ls = g_Script.ls;
	if (lua_gettop(ls) == 0)
	{
		lua_settop(ls, 0);
		lua_getglobal(ls, "onDeleteFactionRecall");
		if (lua_isnil(ls, -1))
		{
			//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
			lua_pop(ls, 1);
			return;
		}

		lua_createtable(g_Script.ls, 0, 0);
		g_Script.SetTableNumber(g_Script.ls, "nServerId", pMsg->nServerId);
		g_Script.SetTableNumber(g_Script.ls, "dFactionID", pMsg->dFactionID);
		g_Script.SetTableNumber(g_Script.ls, "bresult", pMsg->bresult);

		if (lua_pcall(ls, 1, 0, 0) != 0)
		{
			char err[1024];
			sprintf(err, "%s\r\n", lua_tostring(ls, -1));
			rfalse(2, 1, err);
			lua_settop(ls, 0);
			return;
		}
	}
}

void CFactionManager::GetScriptData(SAGetScriptData *pMsg)
{
	BYTE *pBuffer = (BYTE*)&pMsg->wLenth + sizeof(WORD);
	if (pMsg->wLenth > 0)
	{
		lua_State *ls = g_Script.ls;
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, "OnGetScriptData");
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return;
			}
			lua_createtable(g_Script.ls, 0, 0);
			int ck = luaEx_unserialize(ls, pBuffer, pMsg->wLenth);
			if (ck <= 0)
			{
				lua_pushnil(ls);
				return;
			}

			if (lua_pcall(ls, 1, 0, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return;
			}
		}
	}
}

void CFactionManager::GetScriptData(SAScriptData *pMsg)
{
	if (pMsg->wLenth > 0)
	{
		lua_State *ls = g_Script.ls;
		if (lua_gettop(ls) == 0)
		{
			lua_settop(ls, 0);
			lua_getglobal(ls, "OnGetScriptData");
			if (lua_isnil(ls, -1))
			{
				//rfalse(2,1,FormatString("PrepareFunction %s not find \r\n",funcname));
				lua_pop(ls, 1);
				return;
			}
			BYTE  *pDataaddress = (BYTE*)&pMsg->wLenth + sizeof(WORD);
			lua_createtable(g_Script.ls, 0, 0);
			int ck = luaEx_unserialize(ls, pDataaddress, pMsg->wLenth);
			if (ck <= 0)
			{
				lua_pushnil(ls);
				return;
			}
			lua_rawseti(ls, -2, 1);

			if (lua_pcall(ls, 1, 0, 0) != 0)
			{
				char err[1024];
				sprintf(err, "%s\r\n", lua_tostring(ls, -1));
				rfalse(2, 1, err);
				lua_settop(ls, 0);
				return;
			}
		}
	}
}

__int64 now;
__int64 dwSaveFactionTime = _time64(&now);
void CFactionManager::Run()
{

     //������������ʱ��������б�
	//ȱ�ٺ������� ���� _time64 ��ȡʱ���  60*30(30���ӱ���һ��) 2015/5/12 �����
	__int64 now1;
	_time64(&now1);
	if ((now1 - dwSaveFactionTime) > (60 * 30 ))
	{
		dwSaveFactionTime = _time64(&now1);
		SaveFaction( FALSE );
		rfalse("save faction");
		time_t currTime = time( NULL );
		tm *pcurTime = localtime( &currTime );
		if ( pcurTime == NULL )
			return ;

		//if ( ( pcurTime->tm_hour == 8 || pcurTime->tm_hour == 12 ||
		//	pcurTime->tm_hour == 16 || pcurTime->tm_hour == 20 ||
		//	pcurTime->tm_hour == 0 || pcurTime->tm_hour == 4 ) && ( pcurTime->tm_min <= 30 ) )
        // ע��������pcurTime->tm_min�� 0-59
        if ( ( pcurTime->tm_min < 30 ) )
		{
			if ( pcurTime->tm_hour == 0 )
				FactionDepletion( FALSE );
			else
				FactionDepletion( TRUE );
		}
	}
}

void CFactionManager::FactionDepletion( BOOL onlyCheck )
{
    int isCheck = onlyCheck ? 3 : 1;

    std::map< std::string, SFactionData >::iterator iter;
    for ( iter = factionManagerMap.begin(); iter != factionManagerMap.end(); iter++ )
    {
        BOOL isDelFaction = FALSE;
        SFactionData &data = iter->second;

        // ���ɵȼ� * ( 5 + ���ɳ�Ա�� * 1 + NPC * 2 + ������ * 2 )
        // ���Ե����ɵȼ���ά���ѵ�Ӱ��
        // ��Ϊȥ���˽���������NPC���
        DWORD modify = ( 5 + data.stFaction.byMemberNum * 1 + data.stFaction.byNpcNum * 2 + 
            data.stFaction.byNpcNum * 2 );

        LPCSTR strshow = NULL;
        if ( data.stFaction.iMoney <= ( modify * isCheck ) )
        {
            if ( !onlyCheck )
                isDelFaction = TRUE;

            strshow = "ά�����ɵ��ʽ���,�������ٽ�ɢ!";
        }

        // ����
        if ( ( data.stFaction.dwJade <= 2 * (DWORD)isCheck ) ||
            ( data.stFaction.dwMine <= 2 * (DWORD)isCheck ) || 
            ( data.stFaction.dwPaper <= 2 * (DWORD)isCheck ) ||
            ( data.stFaction.dwStone <= 2 * (DWORD)isCheck ) ||
            ( data.stFaction.dwWood <= 2 * (DWORD)isCheck ) )
        {
            if ( !onlyCheck )
                isDelFaction = TRUE;

            strshow = strshow ? "ά�����ɵ���Դ���ʽ���,�������ٽ�ɢ!" : "ά�����ɵ���Դ����,�������ٽ�ɢ!";
        }

        if ( strshow != NULL )
        {
            SAChatFactionMsg msg; 
            msg.wResult = 1;
            dwt::strcpy( msg.cTalkerName, "��ʾ:", sizeof( msg.cTalkerName ) );
            dwt::strcpy( msg.cChatData, strshow, sizeof( msg.cChatData ) );
            SendToAllMember( iter->second, &msg, sizeof( msg ) );
        }

        if ( isDelFaction && ( !onlyCheck ) )
        {
            //DeleteFaction( data.stFaction.szFactionName );
            return ;
        }

        if ( !onlyCheck )
        {
            data.stFaction.iMoney -= modify;
            data.stFaction.dwJade -= 2;
            data.stFaction.dwMine -= 2;
            data.stFaction.dwPaper -= 2;
            data.stFaction.dwStone -= 2;
            data.stFaction.dwWood -= 2;
        }
    }
}

extern LPIObject GetPlayerByDnid(DNID dnidClient);
BOOL CFactionManager::RecvRemoteApplyJoinMsg( SQRemoteApplyJoin *pMsg )
{
//     if ( pMsg == NULL )
//         return FALSE;
// 
//     CPlayer *pApplyPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwApplyGID)->DynamicCast(IID_PLAYER);
//     SFactionData faction;
// 
//     if ( pApplyPlayer )
//     {
// 		double dElapsed_time = abs( difftime(time( NULL ), ( time_t )pApplyPlayer->m_Property.dwLastLeaveFactionTime ));
// 		if ( dElapsed_time / 3600 < (7 * 24) )
// 		{
// 			TalkToDnid(pApplyPlayer->m_ClientIndex, "���˰�ʱ�䲻�����죬�����ٴμ�����ɣ���");
// 			return FALSE;
// 		}
// 		else
// 			pApplyPlayer->m_Property.dwLastLeaveFactionTime = 0;
// 
//         if ( !GetFactionInfo( pMsg->szTongName, faction ) )
//             return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_NOTONG );
// 
//         if ( faction.stFaction.byOpenRApplyJoin == 0 )
//             return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_CLOSE );
// 
//         if ( faction.stMember[0].Title != 8 )
//             return FALSE;
// 
//         if ( faction.stMember[0].IsOnline == FALSE )
//             return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_OUTLINE );
// 
//         if ( pApplyPlayer->m_Property.m_szTongName[0] != 0 )
//             return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_HAVE );
//     }
// 
//     CPlayer *pPlayer = ( CPlayer* ) GetPlayerByDnid( faction.stMember[0].dnidClient )->DynamicCast( IID_PLAYER );
//     if ( pPlayer == NULL )
//         return FALSE;
// 
//     if ( pApplyPlayer == NULL )
//         return SendRApplyJoinRes( pPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_AJNULL );
// 
//     if ( pPlayer->m_Property.m_isRApplyJoin == TRUE )
//         return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_BUSY );
//     
//     pPlayer->m_Property.m_isRApplyJoin = TRUE;
//     SendRApplyJoinRes( pPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_MSGBOX, pMsg->dwApplyGID );

    return TRUE;
}

BOOL CFactionManager::RecvOperApplyJoinMsg( SQRecvOperApplyJoin *pMsg )
{
//     if ( pMsg == NULL )
//         return FALSE;
// 
//     CPlayer *pApplyPlayer = (CPlayer *)GetPlayerByGID(pMsg->dwApplyGID)->DynamicCast(IID_PLAYER);
//     CPlayer *pSrcPlayer = (CPlayer *)GetPlayerByGID( pMsg->dwSrcGID )->DynamicCast(IID_PLAYER);
//     if ( pApplyPlayer == NULL || pSrcPlayer == NULL )
//         return FALSE;
// 
//     pSrcPlayer->m_Property.m_isRApplyJoin = FALSE;
// 
//     if ( pMsg->type == SQRecvOperApplyJoin::SA_RRAJ_REFUSE )
//         return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_REFUSE );
//     else if ( pMsg->type == SQRecvOperApplyJoin::SA_RRAJ_ADOPT )
//     {
//         if ( pSrcPlayer->m_Property.m_szTongName[0] == 0 )
//         {
//             SendRApplyJoinRes( pSrcPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_NOTONG );
//             return SendRApplyJoinRes( pApplyPlayer->m_ClientIndex, SARemoteApplyJoin::SA_AJ_ER_NOTONG );
//         }
// 
// 		double dElapsed_time = abs( difftime(time( NULL ), ( time_t )pApplyPlayer->m_Property.dwLastLeaveFactionTime ));
// 		if ( dElapsed_time / 3600 < (7 * 24) )
// 		{
// 			TalkToDnid(pApplyPlayer->m_ClientIndex, "���˰�ʱ�䲻�����죬�����ٴμ�����ɣ���");
// 			TalkToDnid(pSrcPlayer->m_ClientIndex, "�Է��˰�ʱ�䲻�����죬�����ٴμ�����ɣ���");
// 			return FALSE;
// 		}
// 
// 		// ����ս�ڼ��ֹ���롢�˳�����
// 		if ( GetGW()->m_FactionManager.m_bRefused )
// 		{
// 			TalkToDnid(pApplyPlayer->m_ClientIndex, GetGW()->m_FactionManager.m_sErrInfo.c_str());
// 			TalkToDnid(pSrcPlayer->m_ClientIndex, GetGW()->m_FactionManager.m_sErrInfo.c_str());
// 			return FALSE;
// 		}
// 
//         if( !AddMember( pSrcPlayer->m_Property.m_szTongName, pApplyPlayer ) )
//         {
//             TalkToDnid(pSrcPlayer->m_ClientIndex, "��������г����쳣������");
//             TalkToDnid(pApplyPlayer->m_ClientIndex, "��������г����쳣������");
//             return FALSE;
//         }
// 
//         pSrcPlayer->SendMyState();
// 
//         // �㲥��Ϣ
//         LPCSTR pInfo = FormatString("�����¼�����һ��%s[%s]������", (pApplyPlayer->m_Property.m_Sex == 0) ? "����" : "����",pApplyPlayer->GetName());
//         SAChatFactionMsg msg;
//         msg.wResult = 1;
//         msg.cTalkerName[0] = 0;
//         dwt::strcpy( msg.cChatData, pInfo, sizeof(msg.cChatData) );
//         SendFactionAllServer( pSrcPlayer->m_Property.m_szTongName, SQSendServerMsg::TALK, &msg, sizeof(msg) );
//         TalkToDnid(pApplyPlayer->m_ClientIndex, "�ɹ������ᣡ");
//         return TRUE;
//     }

    return TRUE;
}

BOOL CFactionManager::SendRApplyJoinRes( DNID dClientIndex, WORD wResType, DWORD dwApplyGID )
{
    SARemoteApplyJoin msg;
    msg.type = wResType;
    msg.dwApplyGID = dwApplyGID;

    g_StoreMessage( dClientIndex, &msg, sizeof( msg ) );

    if ( wResType >= SARemoteApplyJoin::SA_AJ_ER_BUSY )
        return FALSE;
    else
        return TRUE;
}





BOOL CFactionManager::DispatchFactionMessage(DNID dnidClient, struct STongBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (dnidClient == 0 ||
		pMsg == NULL ||
		pPlayer == NULL)
		return FALSE;

	switch (pMsg->_protocol)
	{
	case STongBaseMsg::EPRO_TONG_CREATE_FACTION:
	{
		SQCreateFaction* factionMsg = static_cast<SQCreateFaction*>(pMsg);
		if (factionMsg)
		{
			return OnCreateFaction(dnidClient, pPlayer, factionMsg);
		}
	}
	break;

	case STongBaseMsg::EPRO_TONG_FACTION_INFO:
	{
		SQFactioninfo* factionMsg = static_cast<SQFactioninfo*>(pMsg);
		if (factionMsg)
		{
			return OnQuestFactionInfo(dnidClient, pPlayer, factionMsg);
		}
	}
	break;
	case STongBaseMsg::EPRO_TONG_FACTION_LIST:
	{
		SQFactionList* factionMsg = static_cast<SQFactionList*>(pMsg);
		if (factionMsg)
		{
			return OnQuestFactionList(dnidClient, pPlayer, factionMsg);
		}
	}
	break;
	case STongBaseMsg::EPRO_TONG_QUEST_JOINFACTION:
	{
		SQJoinFaction* factionMsg = static_cast<SQJoinFaction*>(pMsg);
		if (factionMsg)
		{
			return OnQuestJoinFaction(dnidClient, pPlayer, factionMsg);
		}
	}  
	break;
	case STongBaseMsg::EPRO_TONG_MANAGER_MEMBER:
	{
		SQManagerFaction* factionMsg = static_cast<SQManagerFaction*>(pMsg);
		if (factionMsg)
		{
			return OnManagerFaction(dnidClient, pPlayer, factionMsg);
		}
	}  
		break;
	case STongBaseMsg::EPRO_TONG_FACTION_DELETE:
	{
		SQDisbandFaction* factionMsg = static_cast<SQDisbandFaction*>(pMsg);
		if (factionMsg)
		{
			return OnDeleteFaction(dnidClient, pPlayer, factionMsg);
		}
	}
	break;
	case STongBaseMsg::EPRO_TONG_SYNFACTION:
	{
		SQSynFaction* factionMsg = static_cast<SQSynFaction*>(pMsg);
		if (factionMsg)
		{
			return OnSynFaction(dnidClient, pPlayer, factionMsg);
		}
	}
	break;
	case STongBaseMsg::EPRO_TONG_SCENEDATA:
		pPlayer->OnRequestFactionScene((SQFactionSceneData*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_SCENEMAPDATA:
		pPlayer->OnRequestFactionInfo((SQFactionSceneMapData*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_SCENECHALLENGE:
		pPlayer->OnChallengeScene((SQChallengeSceneMsg*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_SCENEFINISH:
		pPlayer->OnChallengeFinish((SQSceneFinishMsg*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_SCENEHURTRANK:
		pPlayer->OnRequestFactionSceneRank((SQSceneHurtRank*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_INITFACTIONSALARY:
		pPlayer->OnInitFactionSalaryData((SQInitFactionSalary*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_GETFACTIONSALARY:
		pPlayer->OnGetFactionSalary((SQGetFactionSalary*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_NOTICE:
		pPlayer->OnShowNotice((SQFactionNotice*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_MODIFYNOTICE:
		pPlayer->OnModifyNotice((SQFModifyNotice*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_OPERATELOG:
		pPlayer->OnShowoperateLog((SQFactionOperateLog*)pMsg);
		break;
	case STongBaseMsg::EPRO_TONG_SENDEMAILTOALL:
		pPlayer->OnShowoperateLog((SQFactionOperateLog*)pMsg);
		break;
	default:
		break;
	}
	return TRUE;
}

BOOL CFactionManager::OnCreateFaction(DNID dnidClient, CPlayer* pPlayer, SQCreateFaction *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("CreateFaction"))
	{
		g_Script.PushParameter(pMsg->szFactionName);
		g_Script.PushParameter(pMsg->wFactionIcon);
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;

}

BOOL CFactionManager::OnQuestFactionInfo(DNID dnidClient, CPlayer* pPlayer, SQFactioninfo *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnShowFactioninfo"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CFactionManager::OnQuestFactionList(DNID dnidClient, CPlayer* pPlayer, SQFactionList *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnShowFactionList"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.PushParameter(pMsg->btype);
		g_Script.PushParameter(pMsg->szFactionName);
		g_Script.PushParameter(pMsg->FactionID);
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CFactionManager::OnQuestJoinFaction(DNID dnidClient, CPlayer* pPlayer, SQJoinFaction *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("PlayerAddFaction"))
	{
		g_Script.PushParameter(pMsg->dFactionID);
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CFactionManager::OnManagerFaction(DNID dnidClient, CPlayer* pPlayer, SQManagerFaction *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("PlayerManagerFaction"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.PushParameter(pMsg->bManagerType);
		g_Script.PushParameter(pMsg->bAgree);
		g_Script.PushParameter(pMsg->szPlayerName);
		g_Script.Execute();
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CFactionManager::OnDeleteFaction(DNID dnidClient, CPlayer* pPlayer, SQDisbandFaction *pMsg) //��ɢ����
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("PlayerManagerFaction"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
	}

	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CFactionManager::OnSynFaction(DNID dnidClient, CPlayer* pPlayer, SQSynFaction *pMsg)
{
	if (!pPlayer || !pMsg)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("OnSynFaction"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
	}

	g_Script.CleanPlayer();
	return TRUE;
}

