#include "StdAfx.h"
//  2003-09-01
//  �Ѽ�⣺ָ�����������߽硢memcpy��strcpy
//

#include <time.h>
#include <WinInet.h>
#include "gameworld.h"
#include "player.h"
#include "npc.h"
#include "monster.h"
#include "region.h"
#include "fightarena.h"
#include "prison.h"
#include "biguanarea.h"
#include "eventregion.h"
#include "area.h"
#include "item.h"
#include "Pet.h"
#include "scriptmanager.h"
#include "building.h"
#include "Mounts.h"
#include "GlobalFunctions.h"
#include <Psapi.h>

#include "networkmodule\logmsgs.h"
#include "networkmodule\refreshmsgs.h"
#include "networkmodule\teammsgs.h"
#include "networkmodule\orbmsgs.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\itemmsgs.h"
#include "networkmodule\menumsgs.h"
#include "networkmodule\namecardmsgs.h"
#include "networkmodule\gmmsgs.h"
#include "networkmodule\scriptmsgs.h"
#include "networkmodule\GMModuleMsg.h"
#include "networkmodule\TongMsgs.h"
#include "networkmodule\DataTransMsgs.h"
#include "networkmodule\CardPointModifyMsgs.h"
#include "networkmodule\MailMsg.h"
#include "networkmodule\FactionBBSMsgs.h"
#include "networkmodule\UpgradeMsgs.h"
#include "networkmodule\UnionMsgs.h"
#include "networkmodule/PlugingameMsg.h"
#include "networkmodule/BiguanMsgs.h"
#include "networkmodule/CenterMsg.h"
#include "NETWORKMODULE/DailyMsgs.h"
#include "DSkillStudyTable.h"

#include "���������\netloginservermodule.h"
#include "networkmodule\netprovidermodule\netprovidermoduleadv.h"
#include "���������\dmainapp.h"
#include "pubroom.h"

#include "relationextra.h"

#include "UpgradeSys.h"
#include "DFightData.h"
#include "DTelergyData.h"
#include "VenapointData.h"
#include "DSpcialItemData.h"
#include "GoatMutateData.h"
#include "plugingame.h"
//#include "exchangemanager.h"
//#include "exchangemc.h"

#include "SkillDataTable.h"
#include "FlyDataTable.h"
#include "ProtectedDataTable.h"

#include "DSpecialPlayerRcd.h"
#include "../pub/ConstValue.h"
#include "pub/thread.h"
#include "EventMgr.h"
//add by yuntao.liu
#include "BulletManager_s.h"
#include "BulletMgr.h"

//add by ab_yue
#include "BuffBaseData.h"

#include "GlobalSetting.h"
#include "LuckyTimeSystem.h"
#include "TransportConf.h"
#include "GameObjects\CMonsterService.h"
#include "GameObjects\CPlayerService.h"
#include "GameObjects\CItemService.h"
#include "GameObjects\CSkillService.h"
#include "GameObjects\CItemDropService.h"
#include "GameObjects\CJingMaiService.h"
#include "GameObjects\CMountService.h"
#include "GameObjects\CFightPetService.h"
#include "GameObjects\CBuffService.h"
#include "GameObjects\CMakeTeamQuestService.h"
#include "GameObjects\GroundItemWinner.h"
#include "GameObjects\CFriendService.h"
#include "GameObjects\CTiZhiService.h"
#include "GameObjects\\CKylinArmService.h"
#include "GameObjects\NpcSaleMgr.h"
#include "GameObjects\ScriptManager.h"
#include "GameObjects\Random.h"
#include "GameObjects\CMystring.h"
#include "GameObjects\CGloryAndTitleServer.h"
#include "RankList.h"
#include "GameObjects\memory_object.h"
#include "SanguoCode\ArenaMoudel\ArenaMoudel.h"
#include "GameObjects\SanguoCode\MailMoudle\MailMoudle.h"
#include "Networkmodule/PlayerBaseInf.h"
#include "GameObjects\SanguoCode\GmToolMoudle\GmToolMoudle.h"
#include "SanguoCode\BaseDataManager.h"
#pragma comment(lib, "Wininet.lib")

SIZE_T GetMemoryInfo();
extern  CSpecialPlayerRcd    *g_pSpecialPlayerRcd;
extern long GetItemCount();
/*extern SItemData* GetItemData();*/
extern int SendToLoginServer(SMessage *, long size);
extern LPIObject GetPlayerByName(LPCSTR szName);
extern LPIObject GetPlayerByGID(DWORD GID);
extern BOOL MoveSomebody(LPCSTR name, WORD regionid, WORD x, WORD y);
extern size_t GetPlayerNumber();
extern Cmemory_object *g_pmemoryobject;
extern void g_EQVLog(LPCSTR info);

//extern BOOL GetFactionHeaderInfo( LPCSTR szFactionName, SFactionData::SFaction &factionHeader );

extern WORD   TopFactionID[ 5 ];
extern string TopFactionName[ 5 ];
extern DWORD FactionFightInf[2][5];
extern std::map< WORD, std::string > factionIdMap;
extern DWORD g_AccOnlineCheck[6];
extern DWORD g_CurOnlineCheck[6];
extern std::set< DWORD > GJLimitRegionMap;
extern void BroadcastTalkToMsg(const char* pMsg, DWORD wSize);
static bool SortMonsterData(BossDeadData p1, BossDeadData p2);
extern LPIObject GetObjectByGID(DWORD GID);
//#define GAME_FPS                5

ULONGLONG timeTick32 = GetTickCount64();
WORD g_wServerID = 0;

BOOL buyItemBind = 1;
 int MAXPLAYERLEVEL			= 75;			// ��ҵ���ߵȼ�

// �µ�GM���ģ��,����url���ܻ�����ӳ٣����е���Ϊ����һ���߳������Լ���
struct SGMCmdMsg
{
    SGMCmdMsg(){};
    BOOL result;               // ���
    DWORD sid;                 // ���SID
    SQGameMngMsg msg;          // ����GM����Ϣ
};

UGE::mtQueue< SGMCheck > gmCheckQueue;
UGE::mtQueue< SGMCmdMsg > gmCmdqueue;
bool g_bLoadgmCheckURL = true;
char gmCURLFromFile[1024] = "http://tools.dreamwork.cn/gm/gmcard.ashx?game=%d&serverid=%d&rnd=%d&key=%s&account=%s&cmd=%d|%d|%d|%d|%d|%d&ip=%s&plat=0";

BOOL LoadgmCheckURL( )
{
    // ���ļ�
    dwt::ifstream fs( "data/GMCheckURL.txt" );
    if ( fs.is_open() && !fs.eof() )
    {
        fs.getline( gmCURLFromFile, sizeof( gmCURLFromFile ) );
        gmCURLFromFile[ ( sizeof( gmCURLFromFile ) - 1 ) ] = 0;
        rfalse(2, 1, "�ɹ�����GMCheckURL From File!");
    }
    else
        rfalse(2, 1, "����GMCheckURL From Fileʧ�ܣ��Ҳ����ļ�data/GMCheckURL.txt!");
    fs.close();

    return TRUE;
}

class CheckGMEvent
{
public:
    CheckGMEvent( UGE::mtQueue< SGMCheck > &mtUniqueQueue ) : mtUniqueQueue( mtUniqueQueue ) {}

    int operator ()()
    {
        while( !mtUniqueQueue.QuitFlag() )
        {
            if ( mtUniqueQueue.size() == 0 )
                Sleep( 1 );
            else
            {
                SGMCheck check;
                if ( mtUniqueQueue.mtsafe_get_and_pop( check ) )
                {       
                    // �Ƿ����URL
                    if ( check.wGameType == (WORD)(-1) && check.msg.wGMCmd == SQGameMngMsg::GM_LOADGMCHECKURL )
                    {
                        g_bLoadgmCheckURL = true;
                        rfalse(2, 1, "�ɹ���������GMCheckURL From File!");
                        continue;
                    }

                    // LUR��֤
                    // ��ʼ��WinInet ����
                    HINTERNET hInternet = InternetOpen( "CEHTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL );
                 
                    if ( g_bLoadgmCheckURL )
                    {
                        LoadgmCheckURL();
                        g_bLoadgmCheckURL = false;
                    }
		    
					if ( gmCURLFromFile[0] == 0 ) 
						continue;		    

                    static char lpLUR[1024] = {0};
                    _snprintf( lpLUR, ( sizeof( lpLUR ) - 1 ), gmCURLFromFile,check.wGameType, check.dwServerid, check.dwRand, check.cradKey.c_str(), check.account.c_str(), 
                        check.wGMCmd, check.msg.iNum[0], check.msg.iNum[1], check.msg.iNum[2], check.msg.iNum[3], check.msg.iNum[4], check.ip.c_str(), check.mac );
                    lpLUR[ ( sizeof( lpLUR ) - 1 ) ] = 0;

					for (int i = 0; lpLUR[i] != 0; i++)
						if (lpLUR[i] == 31 || lpLUR[i] == 30)
							lpLUR[i] = '_';

                    HINTERNET hInternetRes = InternetOpenUrl( hInternet, lpLUR, NULL, 0, INTERNET_FLAG_RELOAD, 0 ); 

                    DWORD result = 0;
                    DWORD size = 0;
                    if ( InternetReadFile( hInternetRes,(LPVOID)&result, 4, &size ) )
                    {
                        SGMCmdMsg msg;
                        msg.msg = check.msg;
                        msg.result = ( result == '1' );
                        msg.sid = check.sid;
                        gmCmdqueue.mtsafe_push( msg );
                    }

                    InternetCloseHandle( hInternetRes );
                    InternetCloseHandle( hInternet );
                }
            }
        }

        return 0;
    }

public:
    UGE::mtQueue< SGMCheck > &mtUniqueQueue;
};

void SynNpcList( CPlayer *pPlayer, CRegion *pRegion )
{
	std::vector<CRegion::NpcList> npcList;
	pRegion->GetAreaNpcList( npcList );

	extern BYTE hugeBuffer[ 0xffff ];
	SAUpdateNpcListMsg &msg = * new ( hugeBuffer )SAUpdateNpcListMsg;
	msg.num = (WORD)npcList.size();
    msg.option = 0;
    if ( pRegion->m_IsBlock )
        msg.option = msg.option | SAUpdateNpcListMsg::OPTION_REGION_BLOCK;

    if ( GJLimitRegionMap.find( pPlayer->m_Property.m_CurRegionID ) != GJLimitRegionMap.end() )
        msg.option = msg.option | SAUpdateNpcListMsg::OPTION_REGION_GJLIMIT;

	for( int i = 0; i < (int)npcList.size(); i ++ )
		msg.npcArray[i] = npcList[i];

	g_StoreMessage( pPlayer->m_ClientIndex, &msg, ( sizeof( SAUpdateNpcListMsg ) - sizeof( msg.npcArray) ) 
		+ sizeof( SAUpdateNpcListMsg::NpcInfo ) * msg.num );
}

BOOL LoadNewItemData( LPCSTR filename)
{
// 	DWORD crc32[MAX_ITEMSCOPE] ={0};
// 	if (GetItemData() == NULL)
// 		return false;
// 
// 	for (int i = 0; i < GetItemCount(); i++)
// 		crc32[i] = ( CItem::GetRawData( GetItemData()[i].wItemID ) ? CItem::GetRawData( GetItemData()[i].wItemID )->first : -1 );// ���²��ȶ�crc����,�ж��Ƿ����
// 
// 	if ( !CItem::LoadItemData(filename, true, TRUE) )
// 		return false;
// 	
// 	for (int index = 0; index < GetItemCount(); index++)
// 	{
// 		std::pair< DWORD, const SItemData* > *rawData = CItem::GetRawData( GetItemData()[index].wItemID );
// 		if ( ( rawData != NULL ) && ( rawData->first != crc32[index] ) )
// 		{
// 			// ���µ����пͻ���
// 			AnswerUpdateItemDataMsg msg;
// 			msg.index = GetItemData()[index].wItemID;
// 
// 			try
// 			{
// 				lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
// 
// 				if ( rawData->second == NULL )
// 					slm( lite::Variant() );
// 				else
// 					slm( lite::Variant( static_cast< const ClientVisible* >( rawData->second ), sizeof( ClientVisible ) ) );
// 
// 				BroadcastMsg( &msg, static_cast< WORD>( sizeof( msg ) - slm.EndEdition() ) );
// 			}
// 			catch ( lite::Xcpt & )
// 			{
// 			}
// 		}
// 	}
	return true;
}

SIZE_T GetMemoryInfo()
{
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS processcounters;
	if (GetProcessMemoryInfo(handle, &processcounters, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		return processcounters.WorkingSetSize;
	}
	rfalse(2, 1, "GetMemoryInfo ERROR %d", GetLastError());
	return 0;
}

DWORD g_GetGuid(__int32 IID)
{
	static DWORD s_CurGlobalID[6] = {1000, 1000, 1000, 1000, 1000,1000};		// ȫ�ֱ�ʶ��

	if (IID == IID_PLAYER)		// ��ҵ�GID���ڵ�½�������ϴ����ģ���4λ��Ϊ0
        return 0;
	else if (IID == IID_NPC)
		return 0x10000000 | (0x0fffffff&(s_CurGlobalID[0]++));
	else if (IID == IID_MONSTER)
		return 0x20000000 | (0x0fffffff&(s_CurGlobalID[1]++));
	else if (IID == IID_ITEM)
		return 0x30000000 | (0x0fffffff&(s_CurGlobalID[2]++));
	else if (IID == IID_FIGHT_PET)
		return 0x40000000 | (0x0fffffff&(s_CurGlobalID[3]++));
	else if (IID == IID_SINGLEITEM)
		return 0x50000000 | (0x0fffffff&(s_CurGlobalID[4]++));

	return 0x60000000 | (0x0fffffff&(s_CurGlobalID[5]++));
}

CGameWorld *&GetGW()
{
	static CGameWorld *s_pGameWorld;
	return s_pGameWorld;
}

void TalkToDnid(DNID dnidClient, LPCSTR info, bool addPrefix, bool bPickUp, WORD wShowType)
{
	UNREFERENCED_PARAMETER(bPickUp);	
	char* Utf8Str = g_AnsiToUtf8(info);
	SAChatTipsMsg msg;

	msg.bHasPrefix = addPrefix;
	if (addPrefix)
		dwt::strcpy(msg.cPrefix, "[ϵͳ]", sizeof(msg.cPrefix));
	dwt::strcpy(msg.cChatData, (LPCSTR)Utf8Str, sizeof(msg.cChatData));
	msg.wShowType = wShowType;
	g_StoreMessage(dnidClient, &msg, msg.GetMySize());
	if (Utf8Str != NULL)
		delete[] Utf8Str;
}

void TalkToAll(LPCSTR info)
{
    //SQORBChatGlobalMsg msg;
    //msg.dnidClient = 0x1111111111111111;
    //msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
    //dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
    //dwt::strcpy(msg.cChatData, info, sizeof(msg.cChatData));

    //SendToORBServer(&msg, msg.GetMySize());
	char* Utf8Str = g_AnsiToUtf8(info);

    SAChatGlobalMsg globalMsg;
    globalMsg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
	dwt::strcpy(globalMsg.cChatData, Utf8Str, MAX_CHAT_LEN);
	memcpy(globalMsg.cTalkerName, "[GM]", CONST_USERNAME);

    BroadcastMsg( &globalMsg, globalMsg.GetMySize() );

	SAGMCheckChatGMMsg msgchat;

	dwt::strcpy(msgchat.szMessage, Utf8Str, 49);
	// strcpy(msgchat.szMessage,info);
	// strcpy(msgchat.szNameTalker,"");
	msgchat.szNameTalker[0] = '\0';
	msgchat.szMessage[49] = '\0';
	msgchat.wChatType = SChatBaseMsg::EPRO_CHAT_GLOBAL;
	msgchat.wChatPara = SAChatGlobalMsg::ECT_SYSTEM;

	SendToLoginServer(&msgchat, sizeof(SAGMCheckChatGMMsg));
	if (Utf8Str != NULL)
		delete[] Utf8Str;
}

void TalkToAll( LPCSTR info, SRawItemBuffer *item, LPCSTR name )
{
	char* Utf8InfoStr = g_AnsiToUtf8(info);
	char* Utf8NameStr = g_AnsiToUtf8(name);
    static char msgBuffer[612];
    SAChatGlobalMsg *ptr = new ( msgBuffer ) SAChatGlobalMsg();
	ptr->byType = SAChatGlobalMsg::ECT_SPECIAL + CONST_USERNAME;
	dwt::strcpy(ptr->cTalkerName, Utf8NameStr ? Utf8NameStr : "[ϵͳ]", CONST_USERNAME);

    try
    {
        lite::Serializer slm( ptr->cChatData, sizeof( msgBuffer ) - ( sizeof( *ptr ) - sizeof( ptr->cChatData ) ) );
		slm(Utf8InfoStr);
 //       if ( item ) 
 //          slm( item, item->size );
        BroadcastMsg( ptr, (DWORD)(sizeof( msgBuffer ) - slm.EndEdition()) );
    }
    catch ( lite::Xcpt & )
    {
    }
	if (Utf8InfoStr != NULL)
		delete[] Utf8InfoStr;
	if (Utf8NameStr != NULL)
		delete[] Utf8NameStr;
}

void TalkToAllObj(LPCSTR info)
{
	//SQORBChatGlobalMsg msg;
	//msg.dnidClient = 0x1111111111111111;
	//msg.byType = SAChatGlobalMsg::ECT_GAMEMASTER;
	//dwt::strcpy(msg.cTalkerName, "[ϵͳ]", sizeof(msg.cTalkerName));
	//dwt::strcpy(msg.cChatData, info, sizeof(msg.cChatData));

	//SendToORBServer(&msg, msg.GetMySize());

// 	SAChatGlobalMsg globalMsg;
// 	globalMsg.byType = type;
// 	dwt::strcpy( globalMsg.cChatData, info, MAX_CHAT_LEN );
// 	memcpy( globalMsg.cTalkerName, "[GM]", CONST_USERNAME);
	// globalMsg.cTalkerName[0] = 0;

	char* Utf8InfoStr = g_AnsiToUtf8(info);
	BroadcastTalkToMsg(Utf8InfoStr, strlen(Utf8InfoStr) + 1);
	//BroadcastMsg( &globalMsg, globalMsg.GetMySize() );

	SAGMCheckChatGMMsg msgchat;

	dwt::strcpy(msgchat.szMessage, Utf8InfoStr, 49);
	// strcpy(msgchat.szMessage,info);
	// strcpy(msgchat.szNameTalker,"");
	msgchat.szNameTalker[0] = '\0';
	msgchat.szMessage[49] = '\0';
	msgchat.wChatType = SChatBaseMsg::EPRO_CHAT_GLOBAL;
	msgchat.wChatPara = SAChatGlobalMsg::ECT_SYSTEM;

	SendToLoginServer(&msgchat, sizeof(SAGMCheckChatGMMsg));
	if (Utf8InfoStr != NULL)
		delete[] Utf8InfoStr;
}

BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0)
{
	return GetGW()->PutPlayerIntoRegion(pPlayer->self.lock(), wNewRegionID, wStartX, wStartY, dwRegionGID);
}

BOOL ClearClient(DNID dnidClient)
{
    if (dnidClient == 0) 
        return FALSE;

    // ���������ӳ����ϵĶ���
    // RemovePlayerByDnid(dnidClient);

    g_CutClient(dnidClient);

    return TRUE;
}

void OnPlayerDisconnect(DNID dnidClient)
{
    if (dnidClient == 0) 
        return;

	if (CPlayer *pPlayer = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER))
	{
		pPlayer->OnDisconnect();
	}
}

CGameWorld::CGameWorld()
{
	GetGW() = this;
	m_ptrLuckySystem = std::make_shared<CLuckyTimeSystem>();
    m_pUpgradeData  = NULL;
	m_BossupdateTime = 0xffffffff;
	m_activityUpdateTime = 0xffffffff;
	memset(m_BossState,0,128);
	m_BlessOpenClose = 0;
	CArenaMoudel::getSingleton()->InitArenaData();
}

CGameWorld::~CGameWorld(void)
{
	GetGW() = NULL;
}

BOOL CGameWorld::Initialize()
{
	extern void g_SetRootPath(LPSTR);
	extern void g_SetFilePath(LPSTR);
	g_SetRootPath(".");
	g_SetFilePath("");

	CRegionManager::Initialize();
	CPlayerManager::Initialize();
    CDChatManager::Initialize();

#ifdef GMMODULEON
    m_NetGMModule.Init();
#endif
 
    // ����GM���ģ��
//    gmCheckQueue.BeginThread( CheckGMEvent( gmCheckQueue ) );

	// ����INI�ļ�װ�س���
	IniFile IniFile;
    char filename[256];
    extern LPCSTR GetInitPath();
    extern dwt::stringkey<char [256]> szIniFilename;
	if (!IniFile.Load(szIniFilename)) 
        return rfalse(0, 0, "�޷��������ļ�%s", filename);

    // ��DB Server�����ȡ��������
	SQGetFactionMsg msg;
	extern DWORD GetGroupID();
 	msg.nServerId = GetGroupID();
 	SendToLoginServer( &msg, sizeof(msg) );

    // ȡ�ý�������
//	SQGetUnionDataMsg msgUnion;
//	SendToLoginServer( &msgUnion, sizeof( msgUnion ) );

    char szServerName[17];
	IniFile.GetString("SERVER", "ACCOUNT", "", szServerName, 16);
	IniFile.Clear();

	CItemService::GetInstance().Init();
//	CPlayerService::GetInstance().Init();
	//CSkillService::GetInstance().Init();
	//CBuffService::GetInstance().Init();
	//CMonsterService::GetInstance().Init();
	//CItemDropService::GetInstance().Init();
	//CJingMaiService::GetInstance().Init();

	//add by ly 2014/3/17
//	CXinYangService::GetInstance().Init();

//add by ly 2014/3/24
	//CGloryAndTitleServer::GetInstance().Init();

// 	CMountService::GetInstance().Init();
// 	CFightPetService::GetInstance().Init();
// 	CFriendService::GetInstance().Init();
// 	CTiZhiService::GetInstance().Init();
// 	CKylinArmService::GetInstance().Init();
// 	CSingleRank::GetInstance().Initialized();
	CMyString::GetInstance().Init();
	
	// ����ű�
	g_Script.SetPath("Script\\");
	g_Script.RegisterFunc();
	g_Script.DoFile("Init.lua");
	
	
	if (g_Script.load_errs)
		rfalse(2, 1, "���νű�װ�ع����У���%d���ű�װ��ʧ�ܣ�", g_Script.load_errs);

	if (CRegionManager::size() == 0)
		return rfalse(2, 1, "һ���̶�������û��Ŷ��");
	//20150422  ����
	CPlayerManager::RescueCachePlayers();
	m_dwGameTick = 0;
	m_dgwordTick = 0 ;
	//OnReadBlessFile();
//	LoadActivityNotice();//�ӽű��ж�ȡ�ճ��

	//20150820 wk �������������ýű�
	g_Script.CallFunc("OnServerStart");

	return TRUE;
}

void CGameWorld::Run()
{
	DWORD tick = GetTickCount64() - timeTick32;
	timeTick32 = GetTickCount64();

    // �����ʱ������
    memset(g_AccOnlineCheck, 0, sizeof(g_AccOnlineCheck));

	//DWORD tickcount = GetTickCount();


	CPlayerManager::Run();
	//add by ly 2014/4/14
	CPlayerManager::PlayerAttachAction();
	
	//m_dgwordTick = GetTickCount() - tickcount;
	if (GetTickCount()-m_dgwordTick > 10 * 60 * 1000)
	{
		SYSTEMTIME stime;
		GetLocalTime(&stime);
		rfalse(4,1,"��ǰ�������������%d,[%d-%d-%d,%d:%d:%d]",GetPlayerNumber(),stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond);
		m_dgwordTick = GetTickCount();
	}

	BulletManager::GetInstance().Update();

	EventManager::GetInstance().Run(tick);

	CGroundItemWinner::GetInstance().Run();

	// ���µ�ǰ������
	memcpy(g_CurOnlineCheck, g_AccOnlineCheck, sizeof(g_AccOnlineCheck));

	CDChatManager::OnGolbalMsg();
	
	//tickcount = GetTickCount();
	CRegionManager::Run();
// 	tickcount = GetTickCount() - tickcount;
// 	AddInfo(FormatString("CRegionManager ѭ��ִ�� %d",tickcount));
// 	tickcount = GetTickCount();
	
	m_TeamManager.Run();
    
	m_FactionManager.Run();
    
	m_UnionManager.Run();

    static DWORD prev1 = timeGetTime();
    if ( abs( ( int )( timeGetTime() - prev1 ) ) >= 1000  )
    {
        prev1 = timeGetTime();
        m_dwGameTick ++;
        //CPlugInGameManager::GetInstance()->Update(m_dwGameTick);
    }

    // ÿ1Сʱ����һ�νű��־����ݣ�
    static DWORD prev = timeGetTime();
    if ( abs( ( int )( timeGetTime() - prev ) ) > 1000 * 60 * 60 ) {
        g_Script.SaveTaskEx( NULL );
        prev = timeGetTime();
    }

    // GM������
    if ( gmCmdqueue.size() > 0 )
    {
        SGMCmdMsg msg;
        if ( gmCmdqueue.mtsafe_get_and_pop( msg ) ) {
            CPlayer* player = ( CPlayer* )GetPlayerBySID( msg.sid )->DynamicCast( IID_PLAYER );
            if ( player )
            {
                if ( msg.result  )
                    DispatchGameMngMessage( player->m_ClientIndex, &msg.msg, player );
                else
                {
                    rfalse( 4, 1, "�Ƿ�GM����ִ������ACC:[%s] MAC:[%I64]", player->GetAccount(), player->GetMac() );
                }
            }
        }
    }
}

void CGameWorld::Destroy()
{
    m_PlayerContextMap.clear();

	CPlayerManager::Destroy();
	CRegionManager::Destroy();

    // �������а�����
    //GWSaveScores();

    //CPlugInGameManager::FreeInstance();
	
	g_Count.SavePlayerList("CountData\\CountPlayerList.txt");
}

static char tempBufferGmTool[ 0xfff + 0xff + 0xf ];
BOOL RpcControlGMTool( LPVOID stream, DNID clinet )
{
    try
    {
        lite::Serialreader slr( stream );
        int verify = slr();
        lite::Serializer slm( tempBufferGmTool + 4, sizeof( tempBufferGmTool ) - 4 );

        if ( verify == 0xa00200b )      // ���Ʒ����ܿط���
        {
            LPCSTR account = slr();     // �ܿط��˺���
            int cmd = slr();  // ���Ʒ���ָ������[���ݿ�]

            static char temp[1024] = {0};
            if ( cmd == 'lpl' )
            {
                if ( CPlayer *dst = ( CPlayer* )GetPlayerByAccount( account )->DynamicCast( IID_PLAYER ) )
                {
                    lite::Serializer slm_cmd( temp, sizeof( temp ) );
                    slm_cmd( cmd );
                    // ��Ϣͷ��102
                    ( ( LPDWORD )tempBufferGmTool )[ 0 ] = 102;   
                    // ���ش����Ӻź�ָ������һ�����¹���Ϊһ����Ϣ
                    slm( clinet )( temp, ( sizeof( temp ) - slm_cmd.EndEdition() ) );  

                    // ��Ϊ slm.EndEdition() == ( sizeof( tempBuffer ) - 4 - entityData )
                    // ���� sendData = sizeof( tempBuffer ) - slm.EndEdition()
                    return g_StoreMessage( dst->m_ClientIndex, tempBufferGmTool, (DWORD)(sizeof( tempBufferGmTool ) - slm.EndEdition()) ), TRUE;
                }
            }
        }
        else if ( verify == 0xb00200a )             // �ܿط��ظ����Ʒ���
        {
            DNID dst_dnid = slr();                  // ���Ʒ���dnid

            // ע�⣡�����ش�������lvtӦ����һ����������Ϣ���������������Ϳ��Բ��ô���ֱ��ת����
            lite::Variant lvt = slr();              // �ܿط��Ļش�����
            ( ( LPDWORD )tempBufferGmTool )[ 0 ] = 102;   // ��Ϣͷ��102
            slm( dst_dnid )( lvt );  // ���ش����Ӻź�ָ������һ�����¹���Ϊһ����Ϣ
        
            return  GetApp()->m_LoginServer.SendMessage( tempBufferGmTool, WORD(sizeof( tempBufferGmTool ) - slm.EndEdition()) ), TRUE;
        }
        
    }
    catch( lite::Xcpt & )
    {

    }
    return TRUE;
}


static char tempBuffer[ 0xfff + 0xff + 0xf ];

BOOL RpcControl( LPVOID stream, CPlayer *pPlayer )
{
    if ( pPlayer == NULL )
        return FALSE;

    try
    {
        lite::Serialreader slr( stream );
        int verify = slr();
        lite::Serializer slm( tempBuffer + 4, sizeof( tempBuffer ) - 4 );

        if ( verify == 0xa00200b )      // ���Ʒ����ܿط���
        {
            if ( pPlayer->m_Property.m_GMLevel != 5 ) // ���Ʒ�������5��Ȩ�ޣ�
                return FALSE;

            LPCSTR account = slr();     // �ܿط��˺���
            lite::Variant lvt = slr();  // ���Ʒ���ָ������[���ݿ�]

            // ���ݿ��С���� 0xfff �����л����ڲ��޶��ġ�����
            if ( ( lvt.dataType == lite::Variant::VT_POINTER ) && ( lvt.dataSize <= 0xfff ) )
            {
                if ( CPlayer *dst = ( CPlayer* )GetPlayerByAccount( account )->DynamicCast( IID_PLAYER ) )
                {
                    ( ( LPDWORD )tempBuffer )[ 0 ] = 101;   // ��Ϣͷ��101
                    slm( pPlayer->m_ClientIndex )( lvt );  // ���ش����Ӻź�ָ������һ�����¹���Ϊһ����Ϣ

                    // ��Ϊ slm.EndEdition() == ( sizeof( tempBuffer ) - 4 - entityData )
                    // ���� sendData = sizeof( tempBuffer ) - slm.EndEdition()
                    return g_StoreMessage( dst->m_ClientIndex, tempBuffer, DWORD(sizeof( tempBuffer ) - slm.EndEdition()) ), TRUE;
                }
            }
        }
        else if ( verify == 0xb00200a )             // �ܿط��ظ����Ʒ���
        {
            DNID dst_dnid = slr();                  // ���Ʒ���dnid

            // ע�⣡�����ش�������lvtӦ����һ����������Ϣ���������������Ϳ��Բ��ô���ֱ��ת����
            lite::Variant lvt = slr();              // �ܿط��Ļش�����

            // ���ݿ��С���� 0xfff �����л����ڲ��޶��ġ�����
            if ( ( dst_dnid != 0 ) && ( lvt.dataType == lite::Variant::VT_POINTER ) && ( lvt.dataSize <= 0xfff ) )
                return g_StoreMessage( dst_dnid, ( LPVOID )lvt._pointer, lvt.dataSize ), TRUE;
        }
    }
    catch ( lite::Xcpt & )
    {
        rfalse( 2, 1, "�û������������д����쳣��" );
    }

    return FALSE;
}

int CheckTemp(CPlayer *pPlayer)
{
    if (pPlayer == NULL)
        return 0;

    return pPlayer->m_Property.m_GMLevel;

    // ��ʱд�ڳ�������
    /*static char szSuperAccount[][ACCOUNTSIZE] = 
    {
    ""
    };

    int max_number = sizeof(szSuperAccount)/sizeof(szSuperAccount[0]);

    for (int i=0; i<max_number; i++)
    if (dwt::strcmp(szSuperAccount[i], pPlayer->GetAccount(), ACCOUNTSIZE) == 0)
    return 5;

    if (memcmp("GM", pPlayer->GetAccount(), 2) == 0)
    return 1;

    return 0;*/
}

LPCSTR GetIPString( DNID dnid, WORD *port )
{
    if ( dnid == 0 ) 
        return NULL;

    LPCSTR ipString = NULL;
    sockaddr_in addr;
    ZeroMemory( &addr, sizeof( sockaddr_in ) );

    if ( g_GetAddrByDnid( dnid, &addr, sizeof( addr ) ) )
    {
        if ( port ) *port = addr.sin_port;
        ipString = inet_ntoa( addr.sin_addr );
    }

    return ipString;
}

int CGameWorld::GetAccountLimit(CPlayer *pPlayer)
{
    if (pPlayer == NULL)
        return 0;

    if (pPlayer->m_Property.m_GMLevel > 5 || pPlayer->m_Property.m_GMLevel < 0)
    {
        pPlayer->m_Property.m_GMLevel = 0;
        rfalse(1, 1, "�ʺţ�%s,GM�ȼ��쳣", pPlayer->GetAccount());
        rfalse(2, 1, "�ʺţ�%s,GM�ȼ��쳣", pPlayer->GetAccount());
        return 0;
    }

    return pPlayer->m_Property.m_GMLevel;
}

BOOL CGameWorld::OnDispatch(DNID dnidClient, SMessage *pMsg, CPlayer *pPlayer)
{
    if (pMsg == NULL || dnidClient == 0 || pPlayer == NULL) 
        return 0;

	if (pMsg->_protocol != SMessage::EPRO_GMTOOL_MSG)//gm ����Ϊ�������,û������,���˵�
	{
		if (!pPlayer->isValid())
		{
			rfalse(2, 1, " !pPlayer->isValid :%d", pPlayer->GetSID());
			return FALSE;
		}

		if (pPlayer->isErrPlayer &&
			pMsg->_protocol != SMessage::EPRO_SYSTEM_MESSAGE &&
			pMsg->_protocol != SMessage::EPRO_REGION_MESSAGE)
		{
			if (pPlayer->isErrPlayer <= 1) {
				extern BOOL wgLimit;
				if (wgLimit & 2) // ��Ϸ�߼�������
					return 1;
			}
			else {
				pPlayer->isErrPlayer--;
				// ����ǷǷ��ͻ��ˣ����ýű���������
				extern BOOL isLimitWG;
				if ((pPlayer->isErrPlayer == 1) && (isLimitWG & 0x1))
				{
					g_Script.SetCondition(NULL, pPlayer, NULL);
					LuaFunctor(g_Script, "IllegalClient")();
					g_Script.CleanCondition();
				}
			}
		}
	}

    // �����������Ϣ
    switch(pMsg->_protocol)
    {
	case SMessage::EPRO_SYNTIME_MESSAGE:
		DispatchTimeMessage(dnidClient, (STimeBaseMsg*)pMsg, pPlayer);
		break;
    case SMessage::EPRO_MOVE_MESSAGE:
        DispatchMoveMessage(dnidClient, (SMoveBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_SYSTEM_MESSAGE:
        DispatchSysMessage(dnidClient, (SSysBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_FIGHT_MESSAGE:
        DispatchFightMessage(dnidClient, (SFightBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_TEAM_MESSAGE:
        m_TeamManager.OnDispatch(dnidClient, (STeamBaseMsgG*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_CHAT_MESSAGE:
        {
            if (DispatchChatMessage(dnidClient, (SChatBaseMsg*)pMsg, pPlayer))
                m_NetGMModule.GetChatMessage(dnidClient,(SChatBaseMsg*)pMsg, pPlayer);
        }
        break;
    case SMessage::EPRO_REGION_MESSAGE:
        DispatchRegionMessage(dnidClient, (SRegionBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_ITEM_MESSAGE:
        DispatchItemMessage(dnidClient, (SItemBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_SCRIPT_MESSAGE:
        DispatchScriptMessage(dnidClient, (SScriptBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_UPGRADE_MESSAGE:
        DispatchUpgradeMessage(dnidClient, (SUpgradeMsg*)pMsg, pPlayer);
        break;
	//case SMessage::ERPO_SECTION_MESSAGE:	//�ֶ���Ϣ
		//break;
    case SMessage::EPRO_MENU_MESSAGE:
        DispatchMenuMessage(dnidClient,(SQMenuMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_NAMECARD_BASE:
        DispatchNameCardMessage(dnidClient, (SNameCardBaseMsg*)pMsg, pPlayer);
        break;
    case SMessage::EPRO_GMM_MESSAGE:
        DispatchGMModuleMessage(dnidClient, (SGMMMsg *)pMsg, pPlayer,FALSE);
        break;

    case SMessage::EPRO_TONG_MESSAGE:
        DispatchTongMessage(dnidClient, (STongBaseMsg *)pMsg, pPlayer);
        break;

    case SMessage::EPRO_RELATION_MESSAGE:
        DispatchRelationMessage(dnidClient,(SRelationBaseMsg *)pMsg, pPlayer);
        break;

    case SMessage::EPRO_SPORT_MESSAGE:
        //_theRoomContainerCenter->DispatchContainerMsg((SSportsBaseMsg *)pMsg, pPlayer);
        break;

    case SMessage::EPRO_POINTMODIFY_MESSAGE:
        DispatchCardMessage(dnidClient, (SPointModifyMsg *)pMsg, pPlayer);
        break;
    case SMessage::EPRO_MAIL_MESSAGE:
        DispatchMailMessage(dnidClient,(SMailBaseMsg*)pMsg);
        break;
//     case SMessage::EPRO_PHONE_MESSAGE:
//         DispatchPhoneMessage(dnidClient,(SBasePhoneMsg*)pMsg);
//         break;
    case SMessage::EPRO_BUILDING_MESSAGE :
        CBuilding::OnRecvBuildingMsgs( ( SBuildingBaseMsg* )pMsg, pPlayer );
        break;
    case SMessage::EPRO_PLUGINGAME_MESSAGE:
        CPlugInGameManager::GetInstance()->RecMsg(dnidClient, (SPluginGameBaseMsg*)pMsg);
        break;
    case SMessage::EPRO_UNION_MESSAGE:
        {
            SUnionBaseMsg *pmsg = ( SUnionBaseMsg* )pMsg;
            switch( pmsg->_protocol ) 
            {
            case SUnionBaseMsg::EPRO_UNION_CREATE:
                {
                    SQCreateUnionMsg *_pmsg = ( SQCreateUnionMsg* )pMsg;
                    GetGW()->m_UnionManager.CreateUnionAddPlayer( _pmsg->title, _pmsg->suffx, pPlayer->GetSID(), _pmsg->destID );
                }
                break;
            case SUnionBaseMsg::EPRO_UNION_INVITATORY:
                {
                    SQInvitatoryUnionMsg *__pmsg = ( SQInvitatoryUnionMsg* )pMsg;
                    CPlayer* pplayer = ( CPlayer* )GetPlayerBySID( __pmsg->playerID )->DynamicCast( IID_PLAYER );
                    if( pplayer )
                        if( __pmsg->value )
                        {
                            // ����Ѿ��������壬ֱ����ӵ�������
                            if( GetGW()->m_UnionManager.IsUnion( __pmsg->playerID ) )
                            {
                                if( !GetGW()->m_UnionManager.AddPlayerToUnion( 
                                    pPlayer->GetSID(), GetGW()->m_UnionManager.GetUnionNameByPlayerID( __pmsg->playerID ) ) )
                                    TalkToDnid( pplayer->m_ClientIndex, "��ӵ�����ʧ��!" );
                                else
                                    GetGW()->m_UnionManager.UpdateUnionMember( pPlayer->GetSID() );
                            }
                            else
                            {
                                SACreateUnionMsg msg;
                                msg.destID = pPlayer->GetSID();
                                g_StoreMessage( pplayer->m_ClientIndex, &msg, sizeof( SACreateUnionMsg ) );
                            }
                        }
                        else
                            TalkToDnid( pplayer->m_ClientIndex, "�ܱ�Ǹ���Է��ܾ����������..." );
                }
                break;

            case SUnionBaseMsg::EPRO_UNION_GET_EXP:
                break;

            case SUnionBaseMsg::EPRO_UNION_KICK:
                {
                    SQKickUnionMsg* __pmsg = ( SQKickUnionMsg* )pMsg;
                    // ɾ���ֵ�����
                    GetGW()->m_UnionManager.DeletePlayerToUnion( __pmsg->dwPlayerID, dnidClient );
                }
                break;

            default:
                break;
            }
        }
        break;
	case SMessage::EPRO_DATABASE_MESSAGE:	//���ݿ������Ϣ
		{
			_SDataBaseMsg *pmsg = (_SDataBaseMsg*)pMsg;
			switch (pmsg->_protocol)
			{
			case _SDataBaseMsg::EPRO_DB_Mail_SANGUO: //�ʼ�
				{
					MailMoudle::getSingleton()->DispatchMailMsg(dnidClient, (SMailMsg*)pMsg, pPlayer->m_FixData.m_dwStaticID);
				}
			default:
				break;
			}
		}
		break;
	case SMessage::ERPO_MOUNTS_MESSAGE:	//������Ϣ
		if(pPlayer)
			pPlayer->OnMountsMsg((SMountsMsg*)pMsg);
		break;
	case SMessage::ERPO_BIGUAN_MESSAGE:
		if (pPlayer)
		{
			pPlayer->OnRecvBiGuanMsg((SBiguanMsg*)pMsg);
		}
		break;
   case SMessage::EPRO_SCENE_MESSAGE:
		if (pPlayer)
		{
			DispatchSceneMessage(dnidClient,(SSceneMsg*)pMsg,pPlayer);
		}
	   break;
   case SMessage::EPRO_TRANSFORMERS_MESSAGE:
	   if (pPlayer)
	   {
		   DispatchTransformersMessage(dnidClient, (STransformersMsgs*)pMsg, pPlayer);
	   }
	   break;
   case SMessage::EPRO_DYARENA_MESSAGE:
	   if (pPlayer)
	   {
		   //DispatchDyArenaMessage(dnidClient, (SArenaMsg*)pMsg, pPlayer);
		   DispatchDyArenaMessage(dnidClient, (SSGArenaMsg*)pMsg, pPlayer);
	   }
	   break;
   case SMessage::EPRO_DAILY_MESSAGE:
	   if (pPlayer)
	   {
		   pPlayer->DispatchDailyMessage((SDailyMsg*)pMsg);
	   }
	   break;
   case SMessage::EPRO_SHOP_MESSAGE:
	   if (pPlayer)
	   {
		   pPlayer->DispatchShopMessage((SShopMsg*)pMsg);
	   }
	   break;
   case SMessage::EPRO_VIP_MESSAGE:
	   if (pPlayer)
		   pPlayer->DispatchVipMessage((SVIPMsg*)pMsg);
	   break;
   case SMessage::EPRO_FIGHTPET_MESSAGE:
	   if (pPlayer)
		   pPlayer->DispatchPetMessage((SPetMsg*)pMsg);
	   break;
   case SMessage::EPRO_SANGUOPLAYER_MESSAGE:
	   if (pPlayer)
		   pPlayer->DispatchSGPlayer((SSGPlayerMsg*)pMsg);
	   break;
   case SMessage::EPRO_STORAGE_MESSAGE:
	   if (pPlayer)
		   pPlayer->DsiapatchStorage((SSanguoItem*)pMsg);
	   break;

#ifdef  GM_MODE
    case SMessage::EPRO_GAMEMNG_MESSAGE:
        {
            if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
                return FALSE;
            pPlayer->m_GMIDCheckInf.SQGMMsg = *( (SQGameMngMsg*)pMsg );

            //#ifdef _DEBUG
            //            DispatchGameMngMessage(dnidClient, (SQGameMngMsg*)pMsg, pPlayer);
            //            break;
            //#endif
            //
            //            LPCSTR szIP = GetIPString( pPlayer->m_ClientIndex, NULL );
            //            if ( dwt::strcmp( szIP, "192.168.0.", 10) == 0 )
            //            {
            //                DispatchGameMngMessage(dnidClient, (SQGameMngMsg*)pMsg, pPlayer);
            //                break;
            //            }


            BOOL isChekck = FALSE;
            switch ( ( (SQGameMngMsg*)pMsg )->wGMCmd )
            {
            case SQGameMngMsg::GM_ADDGOODS:     
            case SQGameMngMsg::GM_RENAME:       
            case SQGameMngMsg::GM_SETICON:
            case SQGameMngMsg::GM_SETFIGHT:    
            case SQGameMngMsg::GM_CHANGEFIGHT: 
            case SQGameMngMsg::GM_SETMUTATE:
            case SQGameMngMsg::GM_SETSCAPEGOAT:
            case SQGameMngMsg::GM_ADD_GMC:
            case SQGameMngMsg::GM_ADDGOODSEX:
                    isChekck = TRUE;
                    break;
            default:
                break;
            }

            extern BOOL bCheckGMCmd;
            if ( !bCheckGMCmd || !isChekck )
            {
                DispatchGameMngMessage(dnidClient, (SQGameMngMsg*)pMsg, pPlayer);
                break;
            }

            int iAccountLimit = GetAccountLimit(pPlayer);
            if (iAccountLimit != CheckTemp(pPlayer))
            {
                rfalse(1, 1, "�ʺţ�%s,GM��֤�쳣", pPlayer->GetAccount());
                rfalse(2, 1, "�ʺţ�%s,GM��֤�쳣", pPlayer->GetAccount());
                return FALSE;
            }

            if (iAccountLimit > 5 || iAccountLimit < 0)  
            {
                pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
                return FALSE;
            }

            // ��֤����
            //if ( iAccountLimit == 5 )
            {
                // ���ͻ�ȡ��ݿ�����
                pPlayer->SendGetGMIDInf();
            }
        }
        break;
	case SMessage::EPRO_GMTOOL_MSG:
	{
		GmToolMoudle::getSingleton()->DispatchGmToolMsg(dnidClient, (SGMTOOLMsg*)pMsg);
	}
	break;
#endif

    case 101:
        // ��չ�õ���Ϣ�����ڿͻ���Զ�̷�������ƣ�
        RpcControl( (LPBYTE)pMsg + 4, pPlayer );
        break;
    case 102:
    {
        //GetApp()->m_LoginServer.SendMessage( (LPBYTE)pMsg, sizeof(  ) 
        RpcControlGMTool( (LPBYTE)pMsg + 4, 0 );
    }
    break;
    }

    return 1;
}

BOOL CGameWorld::OnDispathDTM(SMessage *pMsg, WORD wMsgSize, CPlayer *pPlayer)
{
    if (pMsg == NULL || wMsgSize == 0 || pPlayer == NULL) 
        return FALSE;

   // if (pMsg->_protocol == SMessage::EPRO_TONG_MESSAGE)
   // {
		// �ⲿ����ORB�ķ�����Ϣ�����Կ���ɾ����
		/*
        STongBaseMsg *pTongMsg = (STongBaseMsg *)pMsg;
        if (pTongMsg->_protocol == STongBaseMsg::EPRO_TONG_DELETE)
        {
            SATongDeleteMsg *pTDMsg = (SAddMemATongDeleteMsg *)pMsg;
            pPlayer->m_Property.m_szTongName[0] = 0;
            pPlayer->SendMyState();

            if (pTDMsg->byType == SATongDeleteMsg::E_DELETE_MEMBER)
                TalkToDnid(pPlayer->m_ClientIndex,  "���������ǽ壬�Ѿ������������ˣ�");
            else if (pTDMsg->byType == SATongDeleteMsg::E_DELETE_FACTION)
                TalkToDnid(pPlayer->m_ClientIndex,  "��İ���Ѿ�����ɢ�ˣ�");
        }
		*/
   // }
//     else if ( pMsg->_protocol == SMessage::EPRO_REGION_MESSAGE )
//     {
//         SRegionBaseMsg *regionMsg = ( SRegionBaseMsg* )pMsg;
//         if ( regionMsg->_protocol == SRegionBaseMsg::EPRO_QUERY_SIMPLAYER )
//         {
//             SAQueryPlayerMsg *tempMsg = ( SAQueryPlayerMsg* )pMsg;
// 
//             // ����Ǵ�̽����, ����Ҫ��ȡ����
//             if ( tempMsg->mode == 2 && !pPlayer->CheckGoods( 6576, 1, false ) ) 
//                 TalkToDnid( pPlayer->m_ClientIndex, "ɾ����̽����ʧ�ܣ���ѯ��ȡ��" );
//             else
//                 g_StoreMessage( pPlayer->m_ClientIndex, pMsg, wMsgSize );
//         }
//     }

    return TRUE;
}

BOOL CGameWorld::OnDispathDTM(SMessage *pMsg, WORD wMsgSize)
{
    if ( pMsg == NULL || wMsgSize == 0 ) 
        return FALSE;

    if ( pMsg->_protocol == SMessage::EPRO_DATATRANS_MESSAGE )
    {
        // �����������Ϣ
        SDataTransMsg *pDTMsg = (SDataTransMsg *)pMsg;
        switch(pDTMsg->_protocol)
        {
        case SDataTransMsg::EPRO_SRESET:
            g_Script.Restore( ( ( SResetScriptMsg* )pMsg )->flag );
            break;

        case SDataTransMsg::EPRO_SCRIPT_BULLETIN:
            {
                SAScriptBulletinMsg *pSBMsg = (SAScriptBulletinMsg *)pDTMsg;
                SAScriptChatGlobalMsg msg;
                ZeroMemory(&msg.cChatData, sizeof(msg.cChatData));

                msg.byType = SAChatGlobalMsg::ECT_SPECIAL;
                dwt::strcpy(msg.cChatData, pSBMsg->cChatData, MAX_SSYSCALL_LEN);

                BroadcastMsg(&msg, msg.GetMySize());
            }
            break;

	    case SDataTransMsg::EPRO_TONGONE:
		    {
			    extern string s_FactionName;
			    extern DWORD s_dwFactionTitle;
    			
			    SQTongOneMsg *pTOMsg = (SQTongOneMsg *)pDTMsg;
			    s_FactionName = pTOMsg->szTongName;
			    s_dwFactionTitle = pTOMsg->dwFactionTitleID;
		    }
		    break;

	    case SDataTransMsg::EPRO_TONGONE + 1:
            {
                SDT_CheckFriendStateMsg *pMsg = ( SDT_CheckFriendStateMsg* )pDTMsg;
                if ( !GetPlayerByName( pMsg->srcName ) ) // �ų��������Ϣ�ķ�����
                {
                    if ( pMsg->msg.wFriendCount == 0 )
                        return FALSE;

                }
            }
            break;

        default:
            return FALSE;
        }
    }
	else if( pMsg->_protocol == SMessage::EPRO_TONG_MESSAGE )
	{
		STongBaseMsg *pTBMsg = (STongBaseMsg *)pMsg;
        if( pTBMsg->_protocol == STongBaseMsg::EPRO_TONG_SEND_SERVER )
			m_FactionManager.ProcessSSMsg( (SQSendServerMsg*)pTBMsg );
	}

    return TRUE;
}

BOOL CGameWorld::DispatchSysMessage(DNID dnidClient, SSysBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (0 == dnidClient || !pMsg || !pPlayer)
        return FALSE;

    switch (pMsg->_protocol)
    {
    case SSysBaseMsg::EPRO_LOGOUT:
		return OnLogout(dnidClient, (SQLogoutMsg*)pMsg, pPlayer);
        break;
    case SSysBaseMsg::EPRO_ENGINE_FLAGE:
//        pPlayer->m_Property.m_byEngineFlage = ((SQEngineFlage*)pMsg)->byEngineFlage;
        break;
	// �ܵ��ͻ�����Ϣ����ʼ���ͻ��񵤵��߱���Ϣ
	case SSysBaseMsg::EPRO_CLIENT_READY:
		pPlayer->OnClinetReady();
		g_Script.SetCondition(0, pPlayer, 0);
		LuaFunctor(g_Script,"GetBlessInfo")();
		g_Script.CleanCondition();
		pPlayer->SendPlayerBlessOpen();
		return CItemService::GetInstance().SendItemInfo(dnidClient);
		break;

	//add by ly 2014/4/28 �������ÿ��ǩ����ȡ������Ϣ
	case SSysBaseMsg::EPRO_GET_SIGNINAWARD:
		return pPlayer->OnHandleGetSignInAward((SQGetSignInAwardMsg*)pMsg);
		break;
    }

	return FALSE;
}

BOOL CGameWorld::DispatchRelationMessage(DNID dnidClient, SRelationBaseMsg * pMsg, CPlayer *pPlayer)
{
    if(pMsg == NULL || dnidClient == INVALID_DNID || pPlayer == NULL)
        return FALSE;
	return pPlayer->ProcessRelationMsg(dnidClient, pMsg, pPlayer);
}

BOOL CGameWorld::DispatchMenuMessage(DNID dnidClient, SQMenuMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer)
        return FALSE;

	if (!pPlayer->m_ParentRegion || !pPlayer->m_ParentArea)
		return FALSE;

	if (0 == pPlayer->m_CurHp)
		return FALSE;

	CPlayer *pDst = 0;

	switch (pMsg->byMenuItem)
    {
	case SQMenuMsg::EMI_EXCHANGE:
			pPlayer->CExchangeGoods::QuestExchangeWith(pMsg->dwDestGID);
        break;

	case SQMenuMsg::EMI_INVITEJOINTEAM:					// Զ���������
		{
			DWORD wIndex = pMsg->dwDestGID;

			// �Լ��޷�ͬ���Լ�
			if (pPlayer->m_SendRequestID == wIndex)
				break;

			// �Ƿ�ͬ������ҹ���
			CPlayer::SyncTeamQuestList::iterator viewIt = pPlayer->m_LastSynTeamRequestIDList.find(wIndex);
			if (viewIt == pPlayer->m_LastSynTeamRequestIDList.end())
				break;

			// ����һ����ȫ���б��У�
			TRWithIndex *questInfo = CMakeTeamQuestService::GetInstance().GetQuestDetail(wIndex);
			if (!questInfo)
				break;
			
			CPlayer *pDst = (CPlayer *)GetPlayerByGID(questInfo->dwGID)->DynamicCast(IID_PLAYER);
			if (!pDst || !pDst->m_ParentRegion)
				break;

			pMsg->dwDestGID = questInfo->dwGID;
        }
	case SQMenuMsg::EMI_CREATETEAM:						// �����������
	case SQMenuMsg::EMI_JOINTEAM:						// �����������
		m_TeamManager.OnRecvTeamMessage(dnidClient, pMsg->dwDestGID, 
							((SQMenuMsg::EMI_CREATETEAM == pMsg->byMenuItem) || (SQMenuMsg::EMI_INVITEJOINTEAM == pMsg->byMenuItem)) ? true : false, pPlayer, pDst);
		break;

    case SQMenuMsg::EMI_JOINFACTION:
        QuestJoinFaction(dnidClient, pMsg->dwDestGID, pPlayer);
        break;

    case SQMenuMsg::EMI_OTHEREQUIPMENT:
        LookOtherPlayerEquipment(pPlayer, (CPlayer*)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER));
        break;

    case SQMenuMsg::EMI_OTHEREQUIPMENTSID:
        LookOtherPlayerEquipment(pPlayer, (CPlayer*)GetPlayerBySID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER));
        break;

// zeb 2009.11.19
	//case SQMenuMsg::EMI_JOKINGWITHOTHER:
	//	{
	//		CPlayer* pp = (CPlayer*)GetPlayerByGID(pMsg->dwDestGID)->DynamicCast(IID_PLAYER);
	//		SQJokingWitheOtherMsg* pJMsg = (SQJokingWitheOtherMsg*)pMsg;

	//		if ( NULL != pp && NULL != pPlayer && pp->m_ParentRegion )
	//		{
	//			if ( pp->m_ParentRegion->m_dwPKAvailable != CRegion::PAT_NULL )
	//			{
	//				TalkToDnid(pPlayer->m_ClientIndex, "�����ƺ��������ƣ�");
	//				break;
	//			}

	//			const SItemData *pData = CItem::GetItemData(pJMsg->wItemIdx);
	//			if(pData == NULL || pData->wEffect < 40000)
	//			{
	//				// ��Ʒ����ȷ?
	//				break;
	//			}

	//			SPackageItem *pItem = pPlayer->FindItemByIndex(pJMsg->wItemIdx);
	//			if (pItem == NULL)
	//			{
	//				// ֪ͨ���û�����Ƶ���
	//				TalkToDnid(pPlayer->m_ClientIndex, "û�ж�Ӧ�����Ƶ��ߣ�");
	//				break;
	//			}

	//			if(NULL != pData && (pData->wSellPrice != 0 || pData->wScorePrice != 0))
	//			{
	//				char consumingmsg[256] = {0};

	//				_snprintf(consumingmsg, 255, "%s\t%s\t%u\t%s",pPlayer->GetAccount(),pPlayer->GetName(), 1, pData->szName);

	//				extern void g_Consuming(LPCSTR info);
	//				g_Consuming(consumingmsg);
	//			}

	//			pPlayer->DelItem( *pItem, "����" );

	//			pp->SetMutate(4, (pData->wEffect-40000));

	//			char szOkMsg[128] = {0};

	//			_snprintf(szOkMsg, 127, "��ѽ�����ã��㱻%s���ˣ�����Ѿ�ģ�", pPlayer->GetName());
	//			TalkToDnid(pp->m_ClientIndex, szOkMsg);
	//		}
	//	}
	//	break;
// zeb 2009.11.19
	case SQMenuMsg::EMI_WIFE_REMIT:
        {
			pPlayer->OnWifeRemitMsg();
        }
        break;

    case SQMenuMsg::EMI_LAKELOG:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnOpenLakeLog" )();
            g_Script.CleanCondition();
        }
        break;

    case SQMenuMsg::EMI_BOSSINFO:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnOpenBossInfo" )();
            g_Script.CleanCondition();
        }
        break;
    case SQMenuMsg::EMI_ASSOCIATION:
        {
			CPlayer *pplayer = ( CPlayer* )GetPlayerByGID( pMsg->dwDestGID )->DynamicCast(IID_PLAYER);
			if ( pplayer )
			{
				g_Script.SetCondition( NULL, pPlayer, NULL );
				LuaFunctor( g_Script, "OnAssociation" )[ pplayer->GetSID()]();
				g_Script.CleanCondition();
			}
        }
        break;

    case SQMenuMsg::EMI_UNIONRECVEXP:
        {
            GetGW()->m_UnionManager.ReceiveExp( pPlayer->GetSID() );
        }
        break;

    case SQMenuMsg::EMI_XYBAPJIAN:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnOpenXYBaoJian" )();
            g_Script.CleanCondition();
        }
        break;

    case SQMenuMsg::EMI_ASK_ADDDOCTOR:
        {
//             if ( pPlayer->m_dwExtraState & SHOWEXTRASTATE_ADDDOCTORBUFF )  // ���ڸ���BUFF��
//             {
//                 TalkToDnid( pPlayer->m_ClientIndex, "����ͬʱǿ��������" );
//                 break;
//             }
// 
//             CPlayer *pplayer = ( CPlayer* )GetPlayerByGID( pMsg->dwDestGID )->DynamicCast( IID_PLAYER );
//             if ( pplayer )
//             {
//                 if ( pplayer->m_dwExtraState & SHOWEXTRASTATE_BEADDDOCTORBUFF )  // ��ʾ���ڱ���BUFF
//                 {
//                     TalkToDnid( pPlayer->m_ClientIndex, "�Է�����ǿ����" );
//                     break;
//                 }
// 
//                 SAAddDoctorBUFFMsg msg;
//                 if ( pplayer->m_wDoctorDefBuff || pplayer->m_wDoctorDamBuff || pplayer->m_wDoctorPowBuff || pplayer->m_wDoctorAgiBuff ) // ����Լ���BUFF���͵�ѯ���Լ��Ƿ�ͬ��
//                 {
//                     msg.byType = SABackMsg::B_ASK_DOCTORBUFFADD;
//                     msg.dwGID = pPlayer->GetGID(); // ҽ��ID
//                     pplayer->SendMsg( &msg, sizeof( msg ) );
//                     TalkToDnid( pPlayer->m_ClientIndex, "�Է���ǿ�����ȴ��Է�ȷ�ϣ��ظ����ӻḲ�ǵ�ǰЧ��" );
//                 }
//                 else
//                 {
//                     msg.byType = SABackMsg::B_AGREE_ADDDOCTORBUFF;
//                     msg.dwGID = pplayer->GetGID(); // ��ҽ��ID
//                     pPlayer->SendMsg( &msg, sizeof( msg ) ); 
//                 }
//             }
        }
        break;

    case SQMenuMsg::EMI_AGREE_ADD_DOCTORBUFF:
        {
            SAAddDoctorBUFFMsg msg;
            msg.dwGID = pPlayer->GetGID();  // ��ҽ��ID
            msg.byType = SABackMsg::B_AGREE_ADDDOCTORBUFF;
            CPlayer *pDoctor = ( CPlayer* )GetPlayerByGID( pMsg->dwDestGID )->DynamicCast( IID_PLAYER );
            if ( pDoctor ) pDoctor->SendMsg( &msg, sizeof( msg ) );  
        }
        break;

    case SQMenuMsg::EMI_REFUSE_ADD_DOCTORBUFF:
        {
            CPlayer *pDoctor = ( CPlayer* )GetPlayerByGID( pMsg->dwDestGID )->DynamicCast( IID_PLAYER );
            if ( pDoctor ) TalkToDnid( pDoctor->m_ClientIndex, "�Է��ܾ�ǿ��" );
        }
        break;

    case SQMenuMsg::EMI_DOCTORADDBUFF:  // ҽ��BUFF
        {
            CPlayer *pplayer = ( CPlayer* )GetPlayerByGID( pMsg->dwDestGID )->DynamicCast( IID_PLAYER );
            if ( pplayer )
            {
                pplayer->m_dwDoctorGID = pPlayer->GetGID();  // ����ҽ����GID
                pplayer->OnRevDoctorAddBuffMsg( ( SQDoctorAddBuffMsg* )pMsg, pPlayer );
            }
        }
        break;
    case SQMenuMsg::EMI_TASKINF:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnTaskPanel" )();
            g_Script.CleanCondition();
        }
        break;
    case SQMenuMsg::EMI_NEWS:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "NewVersion" )();
            g_Script.CleanCondition();
        }
        break;
    case SQMenuMsg::EMI_REQUESTDATA:
        {
            g_Script.SetCondition( NULL, pPlayer, NULL );
            g_Script.CallFunc( "ClientRequestGetData" );
            g_Script.CleanCondition();
        }
        break;
    case SQMenuMsg::EMI_FLOWER:
        {
            CPlayer* pDstPlayer = NULL;
            SQPresentFlowerMsg* pFlowerMsg = ( SQPresentFlowerMsg* )pMsg;
            if ( pMsg->dwDestGID > 0 )
                pDstPlayer = ( CPlayer* )( GetPlayerByGID( pMsg->dwDestGID )->DynamicCast( IID_PLAYER ) );
            else if ( pFlowerMsg->szDestName[ 0 ] != 0 )
                pDstPlayer = ( CPlayer* )( GetPlayerByName( pFlowerMsg->szDestName )->DynamicCast( IID_PLAYER ) );

            if ( pDstPlayer )        
            {
                g_Script.SetCondition( NULL, pPlayer, NULL );
                g_Script.CallFunc( "PresentFlower", pDstPlayer->GetSID() );
                g_Script.CleanCondition();
            }
        }
        break;
    case SQMenuMsg::EMI_CLICKOTHER:
        break;
    case SQMenuMsg::EMI_WULINRENMING:
        {
            if ( !pPlayer->IsWuLinChief() )
            {
                TalkToDnid( pPlayer->m_ClientIndex, "ֻ��������������Ȩ��������" );
                break;
            }

            if ( pMsg->dwDestGID < 1 || pMsg->dwDestGID > 4 )
                break;

            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnWuLinRM" )[pMsg->dwDestGID]();
            g_Script.CleanCondition();
        }
        break;
    case SQMenuMsg::EMI_WULINJIEPING:
        {
            if ( !pPlayer->IsWuLinChief() )
            {
                TalkToDnid( pPlayer->m_ClientIndex, "ֻ��������������Ȩ��Ƹ��" );
                break;
            }

            if ( pMsg->dwDestGID < 1 || pMsg->dwDestGID > 4 )
                break;

            g_Script.SetCondition( NULL, pPlayer, NULL );
            LuaFunctor( g_Script, "OnWuLinJP" )[pMsg->dwDestGID]();
            g_Script.CleanCondition();
        }
        break;
    }

    return TRUE;
};

extern DWORD qa, qp, qn, qm, qi, qb, qsi;

BOOL CGameWorld::DispatchRegionMessage(DNID dnidClient, struct SRegionBaseMsg *pMsg, CPlayer *pPlayer)
{
	if (!pMsg || 0 == dnidClient || !pPlayer)
		return FALSE;
    
    switch (pMsg->_protocol)
    {
    case SRegionBaseMsg::EPRO_OBJECT_INFO:
        {
            qa++;
            if (pPlayer->m_ParentRegion == NULL) 
                return FALSE;

            if (pPlayer->m_ParentArea == NULL) 
                return FALSE;

            LPIObject pDest = pPlayer->m_ParentRegion->SearchObjectListInAreas(((SQObjectInfoMsg*)pMsg)->dwGlobalID, 
                pPlayer->m_ParentArea->m_X, pPlayer->m_ParentArea->m_Y);

            if (!pDest)
                return FALSE;

            switch(pDest->GetIID())
            {
            case IID_PLAYER:
                if (CPlayer *pObj = (CPlayer *)pDest->DynamicCast(IID_PLAYER))
                {
                    if(((SQObjectInfoMsg*)pMsg)->wInfoType == SQObjectInfoMsg::GIT_ALL) 
                    {
                        qp++;
                        g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SASynPlayerMsg));
                    }
                    else if(((SQObjectInfoMsg*)pMsg)->wInfoType == SQObjectInfoMsg::GIT_SALENAME)
                    {
                        qsi++;
                        if (CPlayer *pObj = (CPlayer *)pDest->DynamicCast(IID_PLAYER))
                        {
							SASendSaleNameMsg msg;
							msg.dwGlobalID = pObj->GetGID();
							dwt::strcpy(msg.szStallName, pObj->szStallName, MAX_STALLNAME);
							g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SASendSaleNameMsg));
                        }
                    }
                }
                break;

            case IID_NPC:
                qn++;
                if (CNpc *pObj = (CNpc *)pDest->DynamicCast(IID_NPC))
				{
					g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SASynNpcMsg));
				}
                break;

            case IID_MONSTER:
                qm++;
                if (CMonster *pObj = (CMonster *)pDest->DynamicCast(IID_MONSTER))
					g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SASynMonsterMsg));
                break;

            case IID_ITEM:
                qi++;
                if (CItem *pObj = (CItem *)pDest->DynamicCast(IID_ITEM))
                    g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SAAddGroundItemMsg));
                break;
			case IID_SINGLEITEM:
				qi++;
				if (CSingleItem *pObj = (CSingleItem *)pDest->DynamicCast(IID_SINGLEITEM))
					g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SAAddGroundItemMsg));
				break;

            case IID_BUILDING:
                qb++;
                if (CBuilding *pObj = (CBuilding *)pDest->DynamicCast(IID_BUILDING))
                {
                    WORD size = sizeof( tempBuffer );
                    pObj->GetStateMsg( tempBuffer, size );
                    g_StoreMessage( pPlayer->m_ClientIndex, tempBuffer, size );
                }
                break;

			case IID_FIGHT_PET:
				qb++;
				if (CFightPet *pObj = (CFightPet *)pDest->DynamicCast(IID_FIGHT_PET))
					g_StoreMessage(pPlayer->m_ClientIndex, pObj->GetStateMsg(), sizeof(SASynFightPetMsg));
				break;
            }
        }
        break;
	case SRegionBaseMsg::EPRO_SETPKRULE:
		{
			if( pPlayer->m_Property.m_Level < 30 )
			{
				TalkToDnid( pPlayer->m_ClientIndex, "��ĵȼ�С��30�������ܸ���PK״̬" );
				break;
			}

            DWORD lastTime = timeGetTime() - pPlayer->m_nChagePkRuleTime;
            if ( lastTime < 60 * 1000 )
			{
				TalkToDnid( pPlayer->m_ClientIndex, "����ȴ�60������л�PK״̬" );
				break;
			}
            
            pPlayer->m_nChagePkRuleTime = timeGetTime();

			SAPKRule msg;
			if (pPlayer)
			{
				if (pPlayer->m_nPkRule!=((SQPKRule*)pMsg)->byPKRule)
				{
					pPlayer->m_nPkRule = ((SQPKRule*)pMsg)->byPKRule;
					msg.byPKRule = ((SAPKRule*)pMsg)->byPKRule;
					std::string pkmod;
					if (msg.byPKRule==0)
					{
						pkmod="��ƽģʽ";
					}
					else if (msg.byPKRule==1)
					{
						pkmod="���ɹ���ģʽ";
					}
					else if (msg.byPKRule==2)
					{
						pkmod="���鹥��ģʽ";
					}
					else if (msg.byPKRule==3)
					{
						pkmod="���ɹ���ģʽ";
					}
					else if (msg.byPKRule==4)
					{
						pkmod="ȫ�幥��ģʽ";
					}
					char strpk[256];
					sprintf(strpk,"PKģʽ�Ѿ��л�Ϊ%s",pkmod.c_str());
					TalkToDnid( pPlayer->m_ClientIndex, strpk );
					g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAPKRule));
				}
			}
		}
		break;
	///��ѯ���/������Ϣ
	case SRegionBaseMsg::EPRO_QUERY_SIMPLAYER:
        {
             SQQueryPlayerMsg *tempMsg = ( SQQueryPlayerMsg* )pMsg;
				
            if ( tempMsg->mode != 0 ) break;

			SAQueryPlayerMsg msg;
            // ���ҵ���Ҿ��ڸ÷��������ҵ���ֱ�ӻش��ͻ���
		    if ( CPlayer *pDestPlayer = ( CPlayer* )GetPlayerByName( tempMsg->szName )->DynamicCast( IID_PLAYER ) )
		    {
			    dwt::strcpy(msg.szName, pDestPlayer->m_Property.m_Name, sizeof(msg.szName));
			    msg.bySchool = pDestPlayer->m_Property.m_School;
			    msg.bSex     = pDestPlayer->m_Property.m_Sex;
				msg.wLevle = pDestPlayer->m_Property.m_Level;
				msg.wRegion =pDestPlayer->m_Property.m_CurRegionID;
				msg.wServerID =GetServerID();
				msg.dnidClient =pDestPlayer->m_ClientIndex;
		    }
			else
			{
				msg.wServerID = 0;
			}
			    g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(SAQueryPlayerMsg)); 
		}
		break;

	// standding on region switch point
	case SRegionBaseMsg::EPRO_SET_REGION:
		{
			// check msg
			SQSetRegionMsg *qmsg = (SQSetRegionMsg*)(pMsg);
			if (!qmsg || !qmsg->dwGlobalID || !qmsg->wConfigSerialID)
			{
				rfalse ("Notice, msg(SQSetRegionMsg) err...");
				break;
			}
			
			const TransportInfo *tr_info = sTransportConf->get(qmsg->wConfigSerialID);
			if (!tr_info) break;

			// info error ?
			int   cur_rgn_id = tr_info->m_iSrcMapID;
			float cur_x      = tr_info->m_SrcCenterPos.fPosx;
			float cur_y      = tr_info->m_SrcCenterPos.fPosy;

			if(!pPlayer->m_ParentRegion)
			{
				//StackWalker sw;
				//sw.ShowCallstack();
				break;
			}

			if (pPlayer->m_ParentRegion->m_wMapID != cur_rgn_id ||
				abs (pPlayer->m_curX - cur_x) > tr_info->m_iSrcRange ||
				abs (pPlayer->m_curY - cur_y) > tr_info->m_iSrcRange)
				break;
			
			int   dest_rgn_id = tr_info->m_iDesMapID;
			float dest_x      = tr_info->m_DesPos.fPosx;
			float dest_y      = tr_info->m_DesPos.fPosy;
			CRegion * pRegion= (CRegion*)FindRegionByID(dest_rgn_id)->DynamicCast(IID_REGION);
			if (pRegion)
			{
				if (pPlayer->m_Property.m_Level >= pRegion->m_dwNeedLevel)
				{
					PutPlayerIntoDestRegion(pPlayer, dest_rgn_id, dest_x, dest_y);
				} 
				else
				{
					TalkToDnid(pPlayer->m_ClientIndex, 
						FormatString("��ĵȼ�����!!!,��ͼ�ȼ�Ϊ�� %d ��", 
						pRegion->m_dwNeedLevel
						)
						);
				}
			}
		}
		break;
    }

    return TRUE;
}

BOOL CGameWorld::DispatchChatMessage(DNID dnidClient, struct SChatBaseMsg *pMsg, CPlayer *pPlayer) 
{
    if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
        return FALSE;

    return CDChatManager::ProcessChatMsg(dnidClient, pMsg, pPlayer);
}

BOOL CGameWorld::SetPlayerData(DNID dnidClient, DWORD GID, SFixData *Data, SPlayerTempData *pTempData)
{
	SIZE_T smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "SetPlayerData  %d", smemsize);

	if (!dnidClient || !GID || !Data)
	{
		rfalse(4, 1, "!dnidClient || !GID || !Data");
		return FALSE;
	} 
	
	CPlayer *pPlayer = (CPlayer *)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
	if (!pPlayer)
	{
		rfalse(4, 1, "GetPlayerByDnid NULL %d",dnidClient);
		//�����ڵ�¼���ر�����ҽ϶�ͬʱս���л�����޷�ȡ����ҵ����������˵Ӧ�ò�����ֵģ����г���ʹ������ķ�������������ķ����Ƚ���
		pPlayer = (CPlayer*)GetObjectByGID(GID)->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			rfalse(4, 1, "GetObjectByGID NULL %d", GID);
			return FALSE;
		}
	}
		

	LPIObject obj = GetPlayerByName(Data->m_Name);
	if (obj&&obj->DynamicCast(IID_PLAYER))
	{
		rfalse(4, 1, "��SetPlayerData���ظ�����Ҷ���%s", Data->m_Name);
		return FALSE;
	}
	
	pPlayer->m_Property.m_Name[0] = 0;

	// ˵���������Ѿ�ȡ����ͨ���ֲ�ʽ��ҹ�������ȷ����ҵ�ǰ�Ƿ�Ψһ�������
	// ��Ŀǰ�Ĺ����£����������Ҫ�յ�SetPlayerData�ض���һϵ��ǰ��������
	// SetPlayerData <-- �������������SSetPlayerDataMsg <-- ��½������SendPlayerData
	// <-- ��½����������SGetPlayerDataMsg <-- ���������RecvCheckRebind(dwResult == 1)
	// <-- �������������SACheckRebindMsg <-- ��½������DoCheckRebind(find account and gid)

	// �ָ��ڴ�����ɫʱ���ݵ����ӱ��
	pPlayer->m_ClientIndex = dnidClient;

	
	// ���ӵ�һ������(Ҫ�����������֣��Ժ���)
	//pPlayer->m_PlayerOpenClose = m_BlessOpenClose;
	pPlayer->SetFixData(Data);
    smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "SetFixData  %d", smemsize);

	pPlayer->FirstLoginInit();
	smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "FirstLoginInit  %d", smemsize);

	pPlayer->m_ServerRelationName = GetApp()->m_szName;



	pPlayer->LoginChecks();

	
	// ���뻺�汸��
//     void InsertIntoCache(LPCSTR account, CPlayer *player);
//     InsertIntoCache(pPlayer->GetAccount(), pPlayer);   

	smemsize = GetMemoryInfo() / 1024;
	rfalse(2, 1, "SetPlayerDataFinish  %d", smemsize);

	return TRUE;
}

const SQSynSimPlayerMsg *CGameWorld::GetRegisterPlayerToORBMsg(DWORD dwGID, WORD wServerID)
{
    static SQSynSimPlayerMsg msg;

    CPlayer *pPlayer = (CPlayer*)CPlayerManager::LocateObject(dwGID)->DynamicCast(IID_PLAYER);
    if (pPlayer == NULL)
        return NULL;

    msg.dwStaticID = pPlayer->m_Property.m_dwStaticID;
    msg.dnidClient = pPlayer->m_ClientIndex;
    msg.dwClientGID = pPlayer->GetGID();
    msg.wCurRegionID = pPlayer->m_Property.m_CurRegionID;
    msg.wServerID = wServerID;

    memcpy(msg.szName, pPlayer->m_Property.m_Name, 10); // ���˽ض�
    msg.szName[10] = 0;
    // msg.dwGMLevel = pPlayer->m_Property.m_GMLevel;

    return &msg;
}

BOOL CGameWorld::CreateNewPlayer(DNID dnidClient, LPCSTR acc16, DWORD GID, DWORD limitedState, DWORD online, DWORD offline, QWORD puid)
{
	if (0 == dnidClient || 0 == GID || !acc16) 
    {
		rfalse(1, 1, "0 == dnidClient || 0 == GID || !acc16 in CreateNewPlayer");
        return 0;
    }


	// ��ҵĴ�������
	CPlayer::SParameter Param;
	Param.dnidClient = dnidClient;
    Param.dwVersion  = 1;

    LPIObject pObj;

	// ��Ҫ�ж��½�Ŀ���Ƿ����ظ�
	pObj = CPlayerManager::LocateObject(GID);

	if (!pObj)
    {
		extern LPIObject GetPlayerByAccount(LPCSTR szAccount);
        pObj = GetPlayerByAccount(acc16);
        if (pObj)
            rfalse(2, 1, "����ظ���----->[%s]", acc16);
    }

	if (pObj)
    {
		// Ŀ���Ѿ����ڣ������½���ֱ��ʹ��
		CPlayer *pPlayer = (CPlayer *)pObj->DynamicCast(IID_PLAYER);
		if (!pPlayer)
		{
			MY_ASSERT(0);
			return 0;
		}

        // GID��ͬ�����Ӧ������Ϊ��½�������Ǳ��Ѿ�û�����������
        // ���Խ���ǰ�Ķ���logout��ͬʱΪ�˱������ݲ�ͬ����ɸ��ƣ�ȡ����ǰ��½
		if (pPlayer->GetGID() != GID)
        {
			pPlayer->Logout();
			return 0;
        }

		// �ڲ�ѯӳ����ڱ������
        if (!BindPlayerByDnid(dnidClient, pObj))
        {
            rfalse(1, 1, "check : BindPlayerByDnid fail in CreateNewPlayer");
            return 0;
        }

        if (reinterpret_cast<DNID&>(pPlayer->m_aClickParam[4]) == dnidClient)
        {
            // ������һ����������������˵GID��ͬ����ǰ�ǲ����������ֱ����Ϊ���°󶨵�
            // ������������ظ��յ�����Ϣ����ᵼ���ظ��յ�SetPlayerData����Ϣ���������ֱ��
            return 0;
        }

		// ͬʱ��Ҫ����һ����Ч���ӶϿ�
		if (pPlayer->m_ClientIndex != 0 && pPlayer->m_ClientIndex != dnidClient)
        {
            // ������Ҫ�������һ��������������Context���ݣ���Ȼ�ᵼ��Player-Logout!!!
			if (!RemovePlayerByDnid(pPlayer->m_ClientIndex))
            {
				rfalse(1, 1, "check : RemovePlayerByDnid fail in CreateNewPlayer");
                return 0;
            }

            {
                WORD port1 = 0, port2 = 0;
                LPCSTR tempIP = GetIPString(pPlayer->m_ClientIndex, &port1);
                std::string ip1 = tempIP ? tempIP : "";
                tempIP = GetIPString(dnidClient, &port2);
                std::string ip2 = tempIP ? tempIP : "";

                pPlayer->m_Property.m_Name[CONST_USERNAME - 1] = 0;

                rfalse(1, 1, "��ɫ[%s][%s]����[%s:%d]��[%s][%s:%d]�滻������", 
                    pPlayer->m_szAccount.c_str(), pPlayer->m_Property.m_Name, 
                    ip1.c_str(), port1, acc16, ip2.c_str(), port2);

                // send to old player : ���˺����ӱ𴦵�½��IPΪxxx.xxx.xxx.xxx
                // send to new player : ���˺��Ѵӱ𴦵�½��IPΪxxx.xxx.xxx.xxx
                if (( port1 != 0) && (port2 != 0 ))
                {
                    //TalkToDnid(pPlayer->m_ClientIndex, FormatString("���˺����ӱ𴦵�½��IPΪ[%s]", ip2.c_str()));
                    //TalkToDnid(dnidClient, FormatString("���˺��Ѵӱ𴦵�½��IPΪ[%s]", ip1.c_str()));
					TalkToDnid(dnidClient, "���ӳɹ�");
                }
            }

            g_CutClient(pPlayer->m_ClientIndex, 3); //3��ʾ���������������
        }

        pPlayer->m_ClientIndex = dnidClient;

		//20150403 wk ��������bug��־
		
		CSanguoPlayer *pSGplayer = (CSanguoPlayer*)pPlayer->DynamicCast(IID_SANGUOPLAYER);
		if (pSGplayer)
		{
			if (pSGplayer->GetBaseDataMgr() == nullptr)
			{
				StackWalker wp;
				wp.ShowCallstack();
				rfalse(4, 1, " pPlayer->GetBaseDataMgr()==nullptr  ShowCallstack ");
				rfalse(1, 1, "pPlayer->GetBaseDataMgr()==nullptr  ShowCallstack  ");
				return 1;
			}
		}

		pSGplayer->GetBaseDataMgr()->SetDNID(dnidClient);

        pPlayer->m_DisWaitTime = -1;

        // �ý�ɫû�г�ʼ�������Ҫ�������½��������ȡʣ������
        if (!pPlayer->m_bInit)
        {
            rfalse(1, 1, "check : !pPlayer->m_bInit in CreateNewPlayer");
            return 1;
        }

        pPlayer->SetPUID(puid);
        pPlayer->InitTimeLimit(limitedState, online, offline);
		pPlayer->m_Property.m_bStopTime = 1;			//�������������


		pPlayer->SendData(SARefreshPlayerMsg::ONLY_BACKUP);
		pPlayer->HangupLoginInit();
		pPlayer->LoginChecks();
        pPlayer->SendMyState();

		/// �������ʱ,���ýű�
		{
			CScriptState s(NULL, pPlayer, NULL, NULL);
			LuaFunctor(g_Script, "OnPlayerReLogin")();
		}
		
		///���ߺ������Ҿ�����Ϣ
	//	m_FactionManager.MemberLogin(pPlayer);

		if (pPlayer->m_ParentArea)
			pPlayer->m_ParentArea->SendAdjState(dnidClient, dnidClient);

        return 2;
    }

    // �½���ɫ��Ӧ�ñ�֤�ͻ�����ȵ������˽�ɫ��ż���������������Ϣ
    // Ҳ���ǿͻ����ڵ�½��ĵ�һ����ϢӦ��,�ض���SAChDataMsg!!!
	pObj = CPlayerManager::GenerateObject(IID_PLAYER, GID, (LPARAM)&Param);
	if (pObj)
	{
		CPlayer *pPlayer = (CPlayer *)pObj->DynamicCast(IID_PLAYER);
        if (!pPlayer) 
        {
            rfalse(1, 1, "GenerateObject fail in CreateNewPlayer");
            return 0;
        }
		
        pPlayer->SetPUID(puid);
        pPlayer->InitTimeLimit(limitedState, online, offline);
		pPlayer->SetAccount(acc16);

        // ���������dnid����֤�ͻ������յ�SAChDataMsgǰ�����յ�����������Ϣ
        pPlayer->m_ClientIndex = 0;
        reinterpret_cast<DNID&>(pPlayer->m_aClickParam[4]) = dnidClient;

		// �ڲ�ѯӳ����ڱ������
        if (!BindPlayerByDnid(dnidClient, pObj))
        {
            rfalse(1, 1, "BindPlayerByDnid(new) fail in CreateNewPlayer");
            return 0;
        }

        // �ڲ�ѯӳ����ڱ������
        if (!BindAccountRelation(acc16, pObj))
        {
            rfalse(1, 1, "BindAccountRelation(new) fail in CreateNewPlayer");
            return 0;
        }



        return 1;
	}

    rfalse(1, 1, "check : end return fail in CreateNewPlayer");
    return 0;
}

extern BOOL g_bHangup;

BOOL CGameWorld::OnLogout(DNID dnidClient, SQLogoutMsg *pMsg, CPlayer *pPlayer)
{
	if (0 == dnidClient || !pMsg || !pPlayer)
		return FALSE;
	//������Ŀ global IDû���ô����ʰѴ˶δ�������
	//if (pPlayer->GetGID() != pMsg->dwGlobalID) 
    //    return FALSE;
	
	if (g_pmemoryobject)
	{
		g_pmemoryobject->OnPlayerLogout(pPlayer->GetSID());
	}
	pPlayer->Logout(true);
	//TODO:Tony Modify [2012-3-8]Comment:[�����������Ͽ����ӣ�������Ϣ�����ķ�]
	SQLogoutPlayer _msg;
	_msg.dnidClient = dnidClient;
	GetApp()->SendToCenterServer( &_msg , (WORD)( sizeof(_msg) ) );

	//������Ұ�����Ϣ
	//m_FactionManager.MemberLogout(pPlayer);
	///////////////////////////////////////////////////////////////////

	// smemsize = GetMemoryInfo() / 1024;
	//rfalse(2, 1, "CGameWorld::OnLogout   Finish%d", smemsize);
	return TRUE;
}

BOOL CGameWorld::PutPlayerIntoRegion(LPIObject pP, WORD wRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID, bool bStoreMessage)
{
	CPlayer *pPlayer = (CPlayer*)pP->DynamicCast(IID_PLAYER);
	if (!pPlayer || 0 == pPlayer->m_CurHp)
		return FALSE;

	// ��̬��ͼ�Ĵ��� ��dwRegionGID != 0�ľ��Ƕ�̬������
	// ˵����Ҫ��GID��Ϊ������ţ���Ϊ��̬��ͼ����̬�����޷��Ե�ͼIDΪ��������Ϊ�ж����
	if (0 != dwRegionGID)
    {
		//��̬��ͼ��˲��
		if (pPlayer->m_ParentRegion && pPlayer->m_ParentRegion->GetGID() == dwRegionGID)
        {
			if (pPlayer->m_ParentArea)
            {
				SADelObjectMsg msg;
				msg.dwGlobalID = pPlayer->GetGID();
				pPlayer->m_ParentArea->SendAdj(&msg, sizeof(msg), pPlayer->m_ClientIndex);
            }
			//������������Ϳ�����
			pPlayer->m_curTileX = wStartX;
			pPlayer->m_curTileY = wStartY;
			pPlayer->m_curX		= float(wStartX << TILE_BITW);
			pPlayer->m_curY		= float(wStartY << TILE_BITH);
			pPlayer->m_DynamicRegionState = 0;
			if (0 == pPlayer->m_ParentRegion->Move2Area(pP, wStartX, wStartY, true))
                return rfalse(2, 1, "PutPlayerToRegion: Move2Area Fail.");
	
            pPlayer->Stand();

			if (bStoreMessage)
			{
				g_StoreMessage(pPlayer->m_ClientIndex, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
			}

            return TRUE;
        }

		//�л�����̬��ͼ
		extern LPIObject FindRegionByGID(DWORD GID);
		if (CDynamicRegion *pRegion = (CDynamicRegion *)FindRegionByGID(dwRegionGID)->DynamicCast(IID_DYNAMICREGION))
        {
			///���붯̬��ͼǰ����
			if (pPlayer->m_RidingStatus!=0)
			{
				pPlayer->DownMounts();
			}
			// �糡���ƶ�����������
			//pPlayer->m_Property.m_CurRegionID = dwRegionGID;
			pPlayer->m_PartprevRegID = pPlayer->m_Property.m_CurRegionID;
			pPlayer->m_PartprevX = pPlayer->m_curTileX;
			pPlayer->m_PartprevY = pPlayer->m_curTileY;
			pRegion->SetPlayerID(pPlayer->m_Property.m_dwStaticID);
			pRegion->SetPlayerRInfo(pPlayer->m_Property.m_CurRegionID, pPlayer->m_curTileX, pPlayer->m_curTileY);
			pPlayer->m_curTileX = wStartX;
			pPlayer->m_curTileY = wStartY;
			pPlayer->m_curX		= float(wStartX << TILE_BITW);
			pPlayer->m_curY		= float(wStartY << TILE_BITH);
			pPlayer->m_DynamicRegionState = 1;
// 			lite::Variant ret;
// 			CScriptManager::TempCondition temp;
// 			g_Script.PushCondition(temp);
// 			g_Script.SetCondition(0,pPlayer,0);
// 			LuaFunctor(g_Script,"PushPlayerTemplate")[pRegion->m_DynamicIndex](&ret);
// 			g_Script.CleanCondition();
// 			g_Script.PopCondition(temp);
			pPlayer->m_CountTA = 0;
			
			//////////////sdfsadfsadf
            if (0 == pRegion->AddObject(pP))
                return rfalse(2, 1, "PutPlayerToRegion: AddObject Fail.");

            pPlayer->Stand();

			if (bStoreMessage)
			{
				g_StoreMessage(pPlayer->m_ClientIndex, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
			}
		   

            return TRUE;
        }

        return FALSE;
    }

	//��ͨ����
	if (pPlayer->m_ParentRegion)
    {
		// ��ͨ������˲��
		if (!pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION) && pPlayer->m_ParentRegion->m_wRegionID == wRegionID)
        {
			WORD oriX = wStartX;
			WORD oriY = wStartY;

			oriX = GetCurArea(oriX, _AreaW);
			oriY = GetCurArea(oriY, _AreaH);
			
			CArea *pDestArea = (CArea*)pPlayer->m_ParentRegion->GetArea(oriX, oriY)->DynamicCast(IID_AREA);
			bool bDelObject = true;
            if (pPlayer->m_ParentArea&&pDestArea&&pPlayer->m_ParentArea == pDestArea){
				bDelObject = false;
            }
			
			if (bDelObject)
			{
				SADelObjectMsg msg;
				msg.dwGlobalID = pPlayer->GetGID();
				pPlayer->m_ParentArea->SendAdj(&msg, sizeof(msg), pPlayer->m_ClientIndex);
			}


			pPlayer->m_curTileX = wStartX;
			pPlayer->m_curTileY = wStartY;
			pPlayer->m_curX		= float(wStartX << TILE_BITW);
			pPlayer->m_curY		= float(wStartY << TILE_BITH);
			pPlayer->m_DynamicRegionState = 0;
			
			if (bDelObject){
				if (pPlayer->m_ParentRegion->Move2Area(pP, wStartX, wStartY, true) == 0)
					return rfalse(2, 1, "PutPlayerToRegion: AddObject Fail.");
			}
			pPlayer->Stand();
			if (bStoreMessage)
			{
				g_StoreMessage(pPlayer->m_ClientIndex, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
			}
           

            return TRUE;
        }
    }

	//��ͨ�����Ŀ糡���ƶ�
	if (CRegion *pRegion = (CRegion*)FindRegionByID(wRegionID)->DynamicCast(IID_REGION))
    {
		
// 		CDynamicRegion *pDynamicR = (CDynamicRegion*)pPlayer->m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
// 		if (pDynamicR)
// 		{
// 			//˵���ǴӶ�̬������������
// 			if (g_Script.m_pPlayer && g_Script.m_pPlayer->GetGID() == pPlayer->GetGID())
// 			{
// 				lite::Variant ret;
// 				LuaFunctor(g_Script,"PopPlayerTemplate")[pDynamicR->m_DynamicIndex](&ret);
// 			}
// 			else
// 			{
// 				g_Script.SetCondition(0,pPlayer,0);
// 				lite::Variant ret;
// 				LuaFunctor(g_Script,"PopPlayerTemplate")[pDynamicR->m_DynamicIndex](&ret);
// 				g_Script.CleanCondition();
// 			}
// 		}
		// �糡���ƶ�����������
		pPlayer->m_Property.m_CurRegionID = wRegionID;
		pPlayer->m_curTileX = wStartX;
		pPlayer->m_curTileY = wStartY;
		pPlayer->m_curX		= float(wStartX << TILE_BITW);
		pPlayer->m_curY		= float(wStartY << TILE_BITH);
		pPlayer->m_DynamicRegionState = 0;
		if (pRegion->AddObject(pP) == 0)
            return rfalse(2, 1, "PutPlayerToRegion: AddObject Fail.");

		pPlayer->Stand();

		if (bStoreMessage)
		{
			g_StoreMessage(pPlayer->m_ClientIndex, pPlayer->GetStateMsg(), sizeof(SASynPlayerMsg));
		}
		

		return TRUE;
	}

    return TRUE;
}

BOOL CGameWorld::RefreshPlayerCardPoint(SARefreshCardPointMsg *pMsg)
{
//    CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pMsg->dwGID)->DynamicCast(IID_PLAYER);
  //  if (pPlayer == NULL)
        return FALSE;

//    pPlayer->m_Property.m_dwXYDPoint = pMsg->dwCardPoint;

//    g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SARefreshCardPointMsg));

    return true;
}

BOOL CGameWorld::RefreshPlayerMoneyPoint(SARefreshMoneyPointMsg *pMsg)
{
	if(pMsg == NULL)
		return FALSE;

    CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pMsg->dwGID)->DynamicCast(IID_PLAYER);
    if (pPlayer == NULL)
        return FALSE;
//
//    pPlayer->m_Property.m_dwMoneyPoint = pMsg->dwMoneyPoint;
//	if(pPlayer->m_Property.m_dwMoneyPoint < pMsg->mpn.dwMoneyPoint)
//		pMsg->dwRet = SARefreshMoneyPointMsg::R_NOTENOUGH_POINT;

	DWORD dwSubMoney = pMsg->mpn.dwMoneyPoint;

	switch(pMsg->dwRet)
	{
	case SARefreshMoneyPointMsg::R_INVALID_ACCOUNT:
		{
			TalkToDnid(pPlayer->m_ClientIndex, " ��Ч�Ļ����˺ţ�");
		}
		break;

	case SARefreshMoneyPointMsg::R_NOTENOUGH_POINT:
		{
			TalkToDnid(pPlayer->m_ClientIndex, " �˺Ż��ֲ��㣡");
		}
		break;

    //case SARefreshMoneyPointMsg::R_OK:
    //    {
    //        switch(pMsg->mpn.dwOprate)
    //        {
    //        case SMoneyPointNotify::OPT_EXCHANGEITEM:
    //            if(pPlayer->AddItemWithIndex(pMsg->mpn.ATT.ITEMS.ItemID[0]))
    //            {
    //                pPlayer->m_Property.m_dwMoneyPoint -= dwSubMoney;
    //                NotifyMoneyPointToLogin(pPlayer->GetAccount(), 
    //                    SQNotifyMoneyPointMsg::RET_OK, 
    //                    dwSubMoney/*pPlayer->m_Property.m_dwMoneyPoint*/);
    //            }
    //            break;

    //        case SMoneyPointNotify::OPT_EXCHANGETELE:
    //            if(pPlayer->SetTelergyProficM(pMsg->mpn.ATT.TELE.dwTeleID, 
    //                pMsg->mpn.ATT.TELE.dwTeleNum))
    //            {
    //                pPlayer->m_Property.m_dwMoneyPoint -= dwSubMoney;
    //                NotifyMoneyPointToLogin(pPlayer->GetAccount(), 
    //                    SQNotifyMoneyPointMsg::RET_OK, 
    //                    dwSubMoney/*pPlayer->m_Property.m_dwMoneyPoint*/);
    //            }
    //            break;

    //        case SMoneyPointNotify::OPT_EXBOTH:
    //            if(pPlayer->AddItemWithIndex(pMsg->mpn.ATT.EXBOTH.dwItemID))
    //            {
    //                pPlayer->SetTelergyProficM(pMsg->mpn.ATT.EXBOTH.dwTeleID, pMsg->mpn.ATT.EXBOTH.dwTeleNum);
    //                pPlayer->m_Property.m_dwMoneyPoint -= dwSubMoney;
    //                NotifyMoneyPointToLogin(pPlayer->GetAccount(), 
    //                    SQNotifyMoneyPointMsg::RET_OK, 
    //                    dwSubMoney/*pPlayer->m_Property.m_dwMoneyPoint*/);
    //            }
    //            break;

    //        default:
    //            return FALSE;

    //        }
    //    }
    //    break;

	case SARefreshMoneyPointMsg::R_TIMEIN:
		{
			TalkToDnid(pPlayer->m_ClientIndex, " ���ڴ���һ����������");
		}
		break;

	case SARefreshMoneyPointMsg::R_TIMEOUT:
		{
			TalkToDnid(pPlayer->m_ClientIndex, " �����������ʱ�ˣ�");
		}
		break;

	default:
		return FALSE;

	}
    //g_StoreMessage(pPlayer->m_ClientIndex, pMsg, sizeof(SARefreshCardPointMsg));

	return TRUE;
}

BOOL CGameWorld::NotifyMCInfo(SExchangeMCMsg *pMsg)
{
	if(pMsg == NULL)
		return FALSE;

	// _EMCWorkInstance->Entry(pMsg, NULL);
	return TRUE;

}

BOOL CGameWorld::NotifyEQVPoint(SEquivalentModifyMsg *pMsg)
{
	switch(pMsg->_protocol)
	{
	case SEquivalentModifyMsg::EPRO_GET_EQUIVALENT:
		{
// 			SAGetEQVPointMsg *pEQVMsg = (SAGetEQVPointMsg*)pMsg;
// 
//             LPCSTR account = NULL;
//             try
//             {
//                 lite::Serialreader sl( pEQVMsg->streamData );
//                 account = sl();
//             }
//             catch ( lite::Xcpt & )
//             {
//                 return FALSE;
//             }
// 
// 			LPIObject pObj = GetPlayerByAccount( account );
// 			CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
// 
// 			if(NULL == pPlayer)
// 				return FALSE;
// 
// 			if (pEQVMsg->type == EEQ_XYDPOINT)
// 			{
// 				pPlayer->m_Property.m_dwXYDPoint = pEQVMsg->dwEQVPoint;
// 			}
// 			else if (pEQVMsg->type == EEQ_GIFTCOUPON_NEWXYD)
// 			{
//  				pPlayer->m_Property.m_dwGiftCoupon = pEQVMsg->dwEQVPoint;
// 			}
		}
		break;
	case SEquivalentModifyMsg::EPRO_DEC_EQUIVALENT:
		{
// 			SADecEQVPointMsg *pEQVMsg = (SADecEQVPointMsg*)pMsg;
// 			char einfo[256];
// 
//             LPCSTR account = NULL;
//             try
//             {
//                 lite::Serialreader sl( pEQVMsg->streamData );
//                 account = sl();
//             }
//             catch ( lite::Xcpt & )
//             {
//                 return FALSE;
//             }
// 
// 			LPIObject pObj = GetPlayerByAccount(account);
// 			CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
// 
// 			if(NULL == pPlayer)
// 			{
// 				_snprintf(einfo, 255, "���δ�ҵ���%s", account);
// 				g_EQVLog(einfo);
// 
// 				return FALSE;
// 			}
// 
// 			if(pEQVMsg->type == EEQ_XYDPOINT)
// 			{
// 				pPlayer->m_Property.m_dwXYDPoint = pEQVMsg->dwEQVPoint;
// 			}
// 			else if(pEQVMsg->type == EEQ_GIFTCOUPON_NEWXYD)
// 			{ 
// 				pPlayer->m_Property.m_dwGiftCoupon = pEQVMsg->dwEQVPoint; 
// 			}
// 			else
// 			{
// 				_snprintf(einfo, 255, "���������쳣����������Ϣ����ȷ���˺ţ�%s", account);
// 				g_EQVLog(einfo);
//                 return FALSE;
// 			}
// 
//             pPlayer->m_bWaitBuyResult = false;
//             pPlayer->ATM_temporaryVerifier = 0;
// 
// 			if(ECT_BUYITEM == pEQVMsg->consumable)
// 			{
//                 if ( pEQVMsg->type == EEQ_XYDPOINT )
//                 {
//                     // ͳ���̵�������Ԫ��
//                     DWORD dwBuyPoint = pEQVMsg->dwDecPoint;
//                     if ( ( __int64 )CItemUser::shopBuyPoint + dwBuyPoint < 0xffff0000 )
//                         CItemUser::shopBuyPoint += dwBuyPoint;
//                 }
// 
// 			    //_snprintf(einfo, 255, "�˺�Ϊ%s��ɫΪ%s����ҵ�ǰԪ����%u��������%u��",
// 			    //	account, pPlayer->GetName(), pPlayer->m_Property.m_dwXYDPoint, pPlayer->m_Property.m_dwGiftCoupon);
//                 _snprintf(einfo, 255, "��ϣ�acc=%s, name=%s, item=%u, count=%d, use=%d, last=%d", 
//                     account, pPlayer->GetName(), pEQVMsg->nIndex, pEQVMsg->nNumber, pEQVMsg->dwDecPoint, pEQVMsg->dwEQVPoint);
// 			    g_EQVLog(einfo);
// 
// 				// ����������ߣ��Ȱ�
// 				bool bind = false;
// 				if( buyItemBind && pEQVMsg->type == EEQ_GIFTCOUPON_NEWXYD )
// 					bind = true;
// 
//                 //_snprintf(einfo, 255, "�˺�Ϊ%s��ɫΪ%s����ҹ�����ϣ�׼�������Ʒ��%d[%d]��", 
//                 //    account, pPlayer->GetName(), pEQVMsg->nIndex, pEQVMsg->nNumber);
//                 //g_EQVLog(einfo);
// 
// 				if ( (WORD)pEQVMsg->nIndex && GenerateNewItem( pPlayer, 
//                     GenItemParams( (WORD)pEQVMsg->nIndex, pEQVMsg->nNumber ), LogInfo( 4, "��������" ), bind ) )
// 				{
// 					// succeed!
// 					// ��¼��������
// 					g_pSpecialItemData->OnCreatedItem((WORD)pEQVMsg->nIndex, pEQVMsg->nNumber);
// 
// // 					SABuy msg;
// // 					msg.nRet = 1;
// // 					msg.nMoney = pPlayer->m_Property.m_Money;
// // 					g_StoreMessage(pPlayer->m_ClientIndex , &msg, sizeof(SABuy));
// 					pPlayer->SetPlayerConsumePoint(pEQVMsg->dwDecPoint);         
// 					//_snprintf(einfo, 255, "�˺�Ϊ%s����������Ʒ�ѳɹ������ѻ��� %u", account,(pEQVMsg->dwDecPoint));
// 					//g_EQVLog(einfo);
// 				}
// 				else if( (WORD)pEQVMsg->nIndex )
// 				{
// 					// some error occured
// 					_snprintf(einfo, 255, "�˺�Ϊ%s����������Ʒ%d[%d]ʧ�ܣ������Ǳ������ˣ��ѿ�Ǯ��", account, pEQVMsg->nIndex, pEQVMsg->nNumber);
// 					g_EQVLog(einfo);
// 				}
// 			}
// 			else if(ECT_BUYTASKID == pEQVMsg->consumable)
// 			{
//                 if ( pEQVMsg->type == EEQ_XYDPOINT )
//                 {
//                     // ͳ�ƽű�������Ԫ��
//                     DWORD dwBuyPoint = pEQVMsg->dwDecPoint;
//                     if ( ( __int64 )CItemUser::scriptBuyPoint + dwBuyPoint < 0xffff0000 )
//                         CItemUser::scriptBuyPoint += dwBuyPoint;
//                 }
// 
// 				//_snprintf(einfo, 255, "�˺�Ϊ%s��ɫΪ%s����ҹ�����ϣ�׼���������%u�����ѻ��֣�%u", account, pPlayer->GetName(), pEQVMsg->nIndex,pEQVMsg->dwDecPoint);
//                 _snprintf(einfo, 255, "��ϣ�acc=%s, name=%s, task=%u, use=%u, last=%d", 
//                     account, pPlayer->GetName(), pEQVMsg->nIndex, pEQVMsg->dwDecPoint, pEQVMsg->dwEQVPoint);
// 				pPlayer->SetPlayerConsumePoint(pEQVMsg->dwDecPoint);
// 				g_EQVLog(einfo);
// 
// 				SRoleTask task;
// 				task.wTaskID = (pEQVMsg->nIndex >> 16);
// 				task.byNeedNum = 0;
// 				task.byFinished = 0;
// 				task.byGoodsID = 0;
// 				task.wComplete = (WORD)pEQVMsg->nIndex;
// 				task.wNextGoods = 0;
// 
// 				for (int i = 0; i < 15; i++)
// 					task.wForgeGoods[i] = 0;
// 				task.wForgeGoods[0] = 0;     // ��Ʒ����
// 				task.bSave = 1;
// 				memset(task.m_szName, 0, sizeof(char)*CONST_USERNAME);
// 
// 				pPlayer->AddTask(task);
// 			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

BOOL CGameWorld::RebindPlayerToNewRegionServer(SAResolutionDestRegionMsg *pMsg)
{
    //if (pMsg->dnidClient == 0)
    //    goto __rebindfail;

//     if (pMsg->dwGID == 0)
//         goto __rebindfail;
// 
//     if (pMsg->dwGID == -1)
//         goto __cutlink;
// 
// //    CPlayer *pPlayer = (CPlayer*)GetPlayerByDnid(pMsg->dnidClient)->DynamicCast(IID_PLAYER);
//     CPlayer *pPlayer = (CPlayer*)GetPlayerByGID(pMsg->dwGID)->DynamicCast(IID_PLAYER);
//     if (pPlayer == NULL)
//         goto __cutlink;
// 
//     //if (pPlayer->GetGID() != pMsg->dwGID)
//     //    goto __cutlink;
// 
//     // ������������ƶ�����������
//     pPlayer->m_Property.m_CurRegionID = pMsg->wRegionID;
// 
//     pPlayer->m_wCurX = pMsg->wX;
//     pPlayer->m_wCurY = pMsg->wY;
// 
// 	// add by yuntao.liu
// 	pPlayer->mCurShowX = float(pPlayer->m_wCurX << TILE_BITW);
// 	pPlayer->mCurShowY = float(pPlayer->m_wCurY << TILE_BITH);
// 	
//     pPlayer->SendData(SARefreshPlayerMsg::ONLY_BACKUP);
// 
// 	// ���Լ��ӵ�ǰ���������
//     if (pPlayer->m_ParentRegion)
//     {
//         LPIObject PTemp = pPlayer->self.lock();
//     	pPlayer->m_ParentRegion->DelObject(PTemp);
//     }
// 
//     //// send set region message to client
//     //{
//     //    SASetRegionMsg msg;
//     //    msg.wMapID = pMsg->wMapID;
//     //    msg.wCurRegionID = pMsg->wRegionID;
//     //    msg.x = pMsg->wX;
//     //    msg.y = pMsg->wY;
//     //    g_StoreMessage(pMsg->dnidClient, &msg, sizeof(SASetRegionMsg));
//     //}
// 
//     // send rebind message to client
//     {
//         SRebindRegionServerMsg msg;
//         msg.ip = pMsg->dwip;
//         msg.port = pMsg->dwport;
//         msg.gid = pMsg->dwGID;
//         
//         try
//         {
//             lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
//             slm( pPlayer->GetAccount() );
//             g_StoreMessage( pMsg->dnidClient, &msg, DWORD(sizeof(msg) - slm.EndEdition()) );
//         }
//         catch ( lite::Xcpt & ) 
//         {
//             return FALSE;
//         }
//     }
// 
//     // del player
//     pPlayer->Logout();
// 
//     return TRUE;
// 
// __cutlink:
//     g_CutClient(pMsg->dnidClient);
//     return FALSE;
// 
// __rebindfail:
// 	if (pMsg->dnidClient != 0)
//     {
//         if ( pMsg->wMapID == 0xffff )
//         {
//             // ����˵��û���ҵ�Ŀ�곡��
//             // Ŀǰ�����쳣����
//         }
//         else
//         {
// 		    // ����֪ͨ�ͻ��˵ȼ����󲻹�
//             SABackMsg msg;
//             msg.byType = SABackMsg::SYS_LEVEL_NOT_ENOUGH;
//             g_StoreMessage(pMsg->dnidClient, &msg, sizeof(SABackMsg));
//         }
//     }

    return FALSE;
}

void CGameWorld::DisplayServerInfo(char i)
{
    rfalse(i, 1, "���������%d", CPlayerManager::size());
    if (i == 2) 
        return;

    extern BOOL TraceAllRegionInfo(char i);
    TraceAllRegionInfo(i);
}

BOOL CGameWorld::DispatchFacBBSMessage( DNID dnidClient, SFactionBBSMsg *pMsg, CPlayer *pPlayer )
{
	if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL )
        return FALSE;

	if( pPlayer->m_Property.m_szTongName[0] == 0 )
		return	FALSE;

	switch (pMsg->_protocol)
	{
	case SFactionBBSMsg::EPRO_FACTIONBBS_SAVE:
		{
			SQSaveFacBBSMsg *pSaveFacBBSMsg = (SQSaveFacBBSMsg*)pMsg;
			dwt::strcpy( pSaveFacBBSMsg->szFaction, pPlayer->m_Property.m_szTongName, sizeof( pSaveFacBBSMsg->szFaction ) );
			dwt::strcpy( pSaveFacBBSMsg->szName, pPlayer->GetName(), sizeof( pSaveFacBBSMsg->szName ) );
			SendToLoginServer( pSaveFacBBSMsg, long(sizeof( SQSaveFacBBSMsg ) - pSaveFacBBSMsg->nFreeSize) );
			break;
		}
	case SFactionBBSMsg::EPRO_FACTIONBBS_SET_TOP:
		{
			SQSetFacBBSTopMsg *pSetFacBBSMsg = (SQSetFacBBSTopMsg*)pMsg;

			// ��֤Ȩ��,Ȩ�޲���
			SASetFacBBSTopMsg msg;
			if( !pPlayer->m_stFacRight.Editioner )
			{
				msg.wIndex = 0xffff;
				msg.byTop = 0;
			}
			else
			{
				msg.wIndex = pSetFacBBSMsg->wIndex;
				msg.byTop = pSetFacBBSMsg->byTop;
				SendToLoginServer( pSetFacBBSMsg, sizeof( SQSetFacBBSTopMsg ) );
			}

			g_StoreMessage( pPlayer->m_ClientIndex, &msg, sizeof( msg ) );
			break;
		}
	case SFactionBBSMsg::EPRO_FACTIONBBS_DELETE:
		{
			SQDelFacBBSMsg *pDelFacBBSMsg = (SQDelFacBBSMsg*)pMsg;
			
			// ������.......
			if( pDelFacBBSMsg->wIndex == 0 )
				break;

			// ��֤Ȩ��
			SADelFacBBSMsg msg;
			msg.byResult = pPlayer->m_stFacRight.Editioner;
			msg.wIndex   = pDelFacBBSMsg->wIndex;
			g_StoreMessage( pPlayer->m_ClientIndex, &msg, sizeof( msg ) );

			if( pPlayer->m_stFacRight.Editioner )
				SendToLoginServer( pDelFacBBSMsg, sizeof( SQDelFacBBSMsg ) );

			break;
		}
	case SFactionBBSMsg::EPRO_FACTIONBBS_GET:
		{
			SQGetFacBBSMsg *pGetFacBBSMsg = (SQGetFacBBSMsg*)pMsg;
			dwt::strcpy( pGetFacBBSMsg->szFaction, pPlayer->m_Property.m_szTongName, sizeof( pGetFacBBSMsg->szFaction ) );
			dwt::strcpy( pGetFacBBSMsg->szName, pPlayer->GetName(), sizeof( pGetFacBBSMsg->szName ) );
			SendToLoginServer( pGetFacBBSMsg, sizeof( SQGetFacBBSMsg ) );
			break;
		}
	case SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT:
		{
			SQGetFacBBSTextMsg *pGetFacBBSTextMsg = (SQGetFacBBSTextMsg*)pMsg;
			dwt::strcpy( pGetFacBBSTextMsg->szName, pPlayer->GetName(), sizeof( pGetFacBBSTextMsg->szName ) );
			SendToLoginServer( pGetFacBBSTextMsg, sizeof( SQGetFacBBSTextMsg ) );
			break;
		}
	}

	return TRUE;
}

BOOL CGameWorld::DispatchMailMessage(DNID dnidClient,SMailBaseMsg *pMsg)
{
	if (dnidClient == 0 || pMsg == NULL )
	{
        return FALSE;
	}

	switch (pMsg->_protocol)
	{
	case SMailBaseMsg::EPRO_MAIL_SEND:
		{
			SQMailSendMsg *pQSendMsg = (SQMailSendMsg*)pMsg;

			SendToLoginServer(pQSendMsg,sizeof(SQMailSendMsg));
		}
		break;
	case SMailBaseMsg::EPRO_MAIL_RECV:
		{
			SQMailRecvMsg *pQRecvMsg = (SQMailRecvMsg*)pMsg;

			SendToLoginServer(pQRecvMsg,sizeof(SQMailRecvMsg));
		}
		break;
	case SMailBaseMsg::EPRO_MAIL_DELETE:
		{
			SQMailDeleteMsg *pDeleteMsg = (SQMailDeleteMsg*)pMsg;

			SendToLoginServer(pDeleteMsg,sizeof(SQMailDeleteMsg));
		}
		break;
	}

	return 1;
}

//BOOL CGameWorld::DispatchPhoneMessage(DNID dnidClient, struct SBasePhoneMsg *pMsg)
//{
//	if (dnidClient == 0 || pMsg == NULL )
//	{
//        return FALSE;
//	}
//
//	switch (pMsg->_protocol)
//	{
//	case SBasePhoneMsg::EPRO_PHONE_CALL:
//		{
//			SQPhoneCallMsg *pQCallMsg = (SQPhoneCallMsg*)pMsg;
//			
//			SendToLoginServer(pQCallMsg,sizeof(SQPhoneCallMsg));
//		}
//		break;
//	case SBasePhoneMsg::EPRO_PHONE_ACCEPT:
//		{
//			SAPhoneAcceptMsg *AAcceptMsg = (SAPhoneAcceptMsg*)pMsg;
//
//			SendToLoginServer(AAcceptMsg,sizeof(SAPhoneAcceptMsg));
//		}
//		break;
//	}
//
//	return 1;
//}

#ifdef GMMODULEON
BOOL CGameWorld::DispatchGMModuleMessage(DNID dnidClient,SGMMMsg * pMsg,CPlayer *pPlayer,BOOL bDistribute)
{
    if(bDistribute == FALSE)
    {
        if(dnidClient == 0 || pMsg == NULL,pPlayer == NULL)
            return FALSE;
    }
    else
    {
        if(pMsg == NULL)
        return FALSE;
    }
    return m_NetGMModule.OnGMMessage(dnidClient,pMsg,bDistribute);
}

#endif

BOOL CGameWorld::DispatchGameMngMessage(DNID dnidClient, struct SQGameMngMsg *pMsg, CPlayer *pPlayer)
{
#ifdef  GM_MODE

    if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
        return FALSE;

    int iAccountLimit = GetAccountLimit(pPlayer);
    //int iAccountLimit = 5;
    if (iAccountLimit != CheckTemp(pPlayer))
    {
        rfalse(1, 1, "�ʺţ�%s,GM��֤�쳣", pPlayer->GetAccount());
        rfalse(2, 1, "�ʺţ�%s,GM��֤�쳣", pPlayer->GetAccount());
        return FALSE;
    }

    if (iAccountLimit > 5 || iAccountLimit < 0)  
    {
        pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
        return FALSE;
    }
#ifdef _NET_TEST
	iAccountLimit =pPlayer->_L_GetLuaValue("GetPlayerLimit");
	if (iAccountLimit < 4 || iAccountLimit == 0xffffffff)
	{
		pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
		return FALSE;
	}
#endif

#ifdef _DEBUG
	iAccountLimit = 5;
#endif

    switch (pMsg->wGMCmd)
    {
        /*
    case SQGameMngMsg::GM_DELMEMBER:
	    {
		    if ( pPlayer->m_Property.m_szTongName[0] == 0 )
			    return FALSE;

		    SQGameMngStrMsg *pGMStrMsg = (SQGameMngStrMsg *)pMsg;
		    char strFaction[CONST_USERNAME];
		    dwt::strcpy( strFaction, pPlayer->m_Property.m_szTongName, CONST_USERNAME );

		    // ֻ�е��Լ��Ƕӳ�����ɾ���������Լ���ʱ�򣬲Ž�ɢ����
		    if( dwt::strcmp( pPlayer->GetName(), pGMStrMsg->szName, CONST_USERNAME ) == 0 )
		    {
			    TalkToDnid(dnidClient, "���������Լ������Լ�������");
			    // GetGW()->m_FactionManager.DeleteFaction( pMsg->szUserpass, pPlayer );
			    return TRUE;
		    }
		    else if( !GetGW()->m_FactionManager.DelMember( pPlayer->m_Property.m_szTongName, pGMStrMsg->szName, FALSE ) )
		    {
			    TalkToDnid(dnidClient, "������Աʧ�ܣ�����");
			    return FALSE;
		    }

		    //LPCSTR pInfo = FormatString("[%s]�Ѿ������������ˡ���", pGMStrMsg->szName);

		    //SAChatWisperMsg msg;
		    //msg.byType = 1;
		    //dwt::strcpy(msg.szName, "[����ϵͳ]", sizeof(msg.szName));
		    //dwt::strcpy(msg.cChatData, pInfo, sizeof(msg.cChatData));
		    //GetGW()->m_FactionManager.SendFactionAllMember( strFaction, SQSendServerMsg::TALK, &msg, sizeof(msg) );

		    TalkToDnid(dnidClient, "������Ա�ɹ���");
	    }
        break;
        */

    case SQGameMngMsg::GM_OTHERMOVETO:       // 5
        {
            if (iAccountLimit < 1)  
            {
                pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
                return FALSE;
            }

            SQGameMngStrMsg *pGMStrMsg = (SQGameMngStrMsg *)pMsg;
            MoveSomebody(pGMStrMsg->szName, pGMStrMsg->iNum[0], pGMStrMsg->iNum[1], pGMStrMsg->iNum[2]);
        }
        break;

    case SQGameMngMsg::GM_ONLINECHECK:
        {
            if (iAccountLimit < 1)
            {
                pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
                return FALSE;
            }

            SAChatGlobalMsg msg;
            msg.byType = SAChatGlobalMsg::ECT_NORMAL;
            memcpy(msg.cTalkerName, "ϵͳ", 5);
            LPCSTR GetServerName();
            sprintf(msg.cChatData, "λ�ڵ�ǰ������[%s]��������%d", GetServerName(), g_GetClientCount());
            g_StoreMessage(dnidClient, &msg, msg.GetMySize());
            //sprintf(msg.cChatData, "��ǰ���������������%d", GetCurrentTatolPlayerNumber());
            //g_StoreMessage(dnidClient, &msg, msg.GetMySize());
        }
        break;

	case SQGameMngMsg::GM_MOVETO:       // 5
        //if (iAccountLimit < 1)
        //{
        //    pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
        //    return FALSE;
        //}

        PutPlayerIntoDestRegion(pPlayer, pMsg->iNum[0], pMsg->iNum[1], pMsg->iNum[2]);
        break;

    case SQGameMngMsg::GM_SETHPMPSP:    // 5
        break;

	case SQGameMngMsg::GM_ADDGOODS:     // 3
//         if (iAccountLimit < 5)
//         {
//             pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
//             return FALSE;
//         }
		if (!pMsg)
		{
			return FALSE;
		}
		
		if (pMsg->iNum[0] == 1)
		{
			if (pPlayer->m_Blessvec.size() != 0)
			{
				pPlayer->m_Blessvec.clear();
			}
			g_Script.SetCondition(0, pPlayer, 0);
			LuaFunctor(g_Script,"OnProcessGetBless")[pMsg->iNum[1]]();
			g_Script.CleanCondition();
		}
		else if (pMsg->iNum[0] == 2)
		{
			if (pPlayer->m_Property.m_BlessOnce == 1)
			{
				pPlayer->OnAllNotice();
				pPlayer->LockItemCell(pPlayer->m_Property.m_BlessSPackageitem.wCellPos,false);
				pPlayer->m_Property.m_BlessOnce = 0;
				//SABlessQuestAddItems msg;
				//msg.Item = pPlayer->m_Property.m_BlessSPackageitem;
				//g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SABlessQuestAddItems));
			}
		}
		else if (pMsg->iNum[0] == 3)
		{
			pPlayer->ChangeMoney(1,pMsg->iNum[1]);
		}
		else if (pMsg->iNum[0] == 4)
		{
			pPlayer->ChangeMoney(2,pMsg->iNum[1]);
		}
		else if (pMsg->iNum[0] == 9)
		{
			//������¼������
			SQRefreshRanklist sMsg;
			memcpy(sMsg.name, pPlayer->m_Property.m_Name, sizeof(char) * CONST_USERNAME);
			
			sMsg.level = pMsg->iNum[1];
			SendToLoginServer(&sMsg,sizeof(SQRefreshRanklist));
		}
		else
		{
			pPlayer->RecvAddItem(pMsg->iNum[0], pMsg->iNum[1]);
		}
		break;

    case SQGameMngMsg::GM_RENAME:       // 5
        {
            if (iAccountLimit != 5)  
            {
                pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
                return FALSE;
            }

            SQGameMngStr2Msg *pGMStrMsg = (SQGameMngStr2Msg *)pMsg;
            CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName)->DynamicCast(IID_PLAYER);
            if (pDestPlayer == NULL)
            {
                TalkToDnid(dnidClient, "�Ҳ�����ָ������ң�����");
                return FALSE;
            }

            pDestPlayer->m_Property.m_GMLevel = atoi(pGMStrMsg->szName2);

            if (pDestPlayer->m_Property.m_GMLevel > 5)
                pDestPlayer->m_Property.m_GMLevel = 5;

            TalkToDnid(dnidClient, 
                FormatString("���Ѿ��ɹ��Ľ�[%s]��GMȨ����Ϊ��[%d]������", 
                pGMStrMsg->szName, pDestPlayer->m_Property.m_GMLevel));

            TalkToDnid(pDestPlayer->m_ClientIndex, 
                FormatString("[%s]�����GMȨ����Ϊ��[%d]������", 
                pPlayer->GetName(), pDestPlayer->m_Property.m_GMLevel));

/*
            SQGameMngStr2Msg *pGMStrMsg = (SQGameMngStr2Msg *)pMsg;
            CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName2)->DynamicCast(IID_PLAYER);
            if (pDestPlayer != NULL)
            {
                TalkToDnid(dnidClient, "��ǰ��������У��Ѿ����˽���������ˣ�����");
                return FALSE;
            }

            pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName)->DynamicCast(IID_PLAYER);
            if (pDestPlayer == NULL)
            {
                TalkToDnid(dnidClient, "�Ҳ�����ָ������ң�����");
                return FALSE;
            }

            memcpy(pDestPlayer->m_Property.m_Name, pGMStrMsg->szName2, CONST_USERNAME);

            pDestPlayer->SendMyState();

            TalkToDnid(pDestPlayer->m_ClientIndex, "GM������������֣�����");
            TalkToDnid(dnidClient, "�����ɹ�������");
*/
        }
        break;

	case SQGameMngMsg::GM_RECOVERPLAYER:
        {
// 			if (iAccountLimit < 3)
// 			{
// 				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
// 				return FALSE;
// 			}
// 
// 			SQGameMngStr2Msg *pGMStrMsg = (SQGameMngStr2Msg *)pMsg;
// 			CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName)->DynamicCast(IID_PLAYER);
// 			if (pDestPlayer == NULL)
// 			{
// 				TalkToDnid(dnidClient, "�Ҳ�����ָ������ң�����");
// 				return FALSE;
// 			}
// 
// 			pDestPlayer->m_CurHp = pDestPlayer->m_Property.m_dwBaseHP;
// 			pDestPlayer->m_wMP = pDestPlayer->m_Property.m_wBaseMP;
// 			pDestPlayer->m_wSP = pDestPlayer->m_Property.m_wBaseSP;
// 
// 			TalkToDnid(dnidClient, 
// 				FormatString("���Ѿ��ɹ��Ľ�[%s]��HP,MP,SP�ָ�������", pGMStrMsg->szName));
        }
		break;

    case SQGameMngMsg::GM_SETICON:
        {
            //if (iAccountLimit < 2)  
            //{
            //    pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            //    return FALSE;
            //}

            //SQGameMngStr2Msg *pGMStrMsg = (SQGameMngStr2Msg *)pMsg;
            //CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName)->DynamicCast(IID_PLAYER);
            //if (pDestPlayer == NULL)
            //{
            //    TalkToDnid(dnidClient, "�Ҳ�����ָ������ң�����");
            //    return FALSE;
            //}

            //pDestPlayer->m_Property.m_dwSysTitle[1] = atoi(pGMStrMsg->szName2);
            //pDestPlayer->SendMyState();

            //TalkToDnid(dnidClient, 
            //    FormatString("���Ѿ��ɹ��Ľ�[%s]��ͼ����Ϊ��[%d]������", 
            //    pGMStrMsg->szName, pDestPlayer->m_Property.m_dwSysTitle[1]));

			// for tom version
// 			if (iAccountLimit < 3)  
//             {
//                 pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
//                 return FALSE;
//             }
// 
//             SQGameMngStr2Msg *pGMStrMsg = (SQGameMngStr2Msg *)pMsg;
//             CPlayer *pDestPlayer = (CPlayer *)GetPlayerByName(pGMStrMsg->szName)->DynamicCast(IID_PLAYER);
//             if (pDestPlayer == NULL)
//             {
//                 TalkToDnid(dnidClient, "�Ҳ�����ָ������ң�����");
//                 return FALSE;
//             }
// 
// 			pDestPlayer->m_Property.m_dwSysTitle[0] = pDestPlayer->m_Property.m_dwSysTitle[1] = atoi(pGMStrMsg->szName2);
//             pDestPlayer->SendMyState();
// 
//             TalkToDnid(dnidClient, 
//                 FormatString("���Ѿ��ɹ��Ľ�[%s]��ͷ����Ϊ��[%d]������", 
//                 pGMStrMsg->szName, pDestPlayer->m_Property.m_dwSysTitle[1]));
        }
        break;

    case SQGameMngMsg::GM_SETFIGHT:     // 3
//         if (iAccountLimit < 5)  
//         {
//             pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
//             return FALSE;
//         }
// 
//         if (pMsg->iNum[0] > 0)
//         {
//             if ( pPlayer->GetSkillLevel( (WORD)pMsg->iNum[0] - 1 ) == 0 )
//                 pPlayer->ActivaSkill( (WORD)pMsg->iNum[0] - 1, "ָ�����" );
//             pPlayer->SetSkillLevel((WORD)pMsg->iNum[0] - 1, pMsg->iNum[1], "ָ�����");//SetSkillProficiency
//         }
        break;

	case SQGameMngMsg::GM_ADDEXP:
		{
			//if (iAccountLimit < 5)  
			//{
			//	pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
			//	return FALSE;
			//}

			//if (pMsg->iNum[0] > 0)
            pPlayer->SendAddPlayerExp(pMsg->iNum[0], SAExpChangeMsg::GM, "GMָ�����");
		}
		break;

	case SQGameMngMsg::GM_ADDTELERGY:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if (pMsg->iNum[0] > 0)
				pPlayer->AddTelergy(pMsg->iNum[0], "GMָ�����");
		}
		break;
		
	case SQGameMngMsg::GM_UPDATETELERGY:
		{
		}
		break;

	case SQGameMngMsg::GM_SETFLY:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if ( pMsg->iNum[0]>=0 )
				pPlayer->ActiveFly(pMsg->iNum[0], "GMָ�����", true);
		}
		break;
	case SQGameMngMsg::GM_SETPROTECT:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if ( pMsg->iNum[0]>=0 )
				pPlayer->ActiveProtected(pMsg->iNum[0], "GMָ�����", true);
		}
		break;
	case SQGameMngMsg::GM_ADDFLYPROF:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if ( pMsg->iNum[0]>=0 && pMsg->iNum[1]>0)
				pPlayer->AddFly( pMsg->iNum[0], pMsg->iNum[1], "GMָ�����");  
		}
		break;
	case SQGameMngMsg::GM_ADDPROTECTPROF:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if ( pMsg->iNum[0]>=0 && pMsg->iNum[1]>0)
				pPlayer->AddProtected( pMsg->iNum[0], pMsg->iNum[1], "GMָ�����");
		}
		break;
	case SQGameMngMsg::GM_ADDSKILLPROF:
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}

			if (pMsg->iNum[0] > 0)
				pPlayer->AddSkillProficiency((WORD)pMsg->iNum[0] - 1, pMsg->iNum[1], "GMָ�����");
		}
		break;

    case SQGameMngMsg::GM_SRESET:    // 5
        if (iAccountLimit < 3)  
        {
            pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            return FALSE;
        }
        else
        {
            g_Script.Restore( ( BYTE )pMsg->iNum[0] );
            TalkToDnid(dnidClient, pMsg->iNum[0] ? "�Ѿ��㲥����ȫ���õ���Ϣ��" : "�Ѿ��㲥�����ýű�����Ϣ��" );
        }
        break;
/*
    case SQGameMngMsg::GM_UPLOAD:     // 3
        if (iAccountLimit < 5) 
        {
            pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            return FALSE;
        }
        else
        {
            SQGMUploadFileMsg *pUMsg = (SQGMUploadFileMsg *)pMsg;
            extern void UploadFile(DNID dnidClient, DWORD dwVerinfo, WORD wTransDataSize, BYTE byTransFlag, DWORD dwOffset, BYTE byDestFilenameLen, LPBYTE pDataEntry);
            UploadFile(dnidClient, pUMsg->dwVerinfo, pUMsg->wTransDataSize, pUMsg->byTransFlag, pUMsg->dwOffset, pUMsg->byDestFilenameLen, pUMsg->pDataEntry);
        }
        break;
*/
    case SQGameMngMsg::GM_CHANGEFIGHT:     // 3
        break;

	case SQGameMngMsg::GM_SETMUTATE:     // 3
        if (iAccountLimit < 5)  
        {
            pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            return FALSE;
        }

        if (pMsg->iNum[0] >= 0 && pMsg->iNum[1] >= 0)
        {
            pPlayer->SetMutate( pMsg->iNum[0], pMsg->iNum[1] );
        }
        break;

    case SQGameMngMsg::GM_SETSCAPEGOAT:     // 3
        if (iAccountLimit < 5)  
        {
            pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            return FALSE;
        }

        if (pMsg->iNum[0] >= 0 )
        {
            pPlayer->SetScapegoat( pMsg->iNum[0], pMsg->iNum[1] );
        }
        break;
    case SQGameMngMsg::GM_ADD_GMC:     // 3
        break;
    case SQGameMngMsg::GM_TEST:         // 7
        if (iAccountLimit <=2 )  
        {
            pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
            return FALSE;
        }

/*        pPlayer->m_fActFrameDec = pMsg->iNum[0] / 1000.0f;*/
        break;
	//case SQGameMngMsg::GM_RLOADMC:
	//	{
	//		_EMCWorkInstance->LoadMCInfo();
	//	}
		break;

	case SQGameMngMsg::GM_MOVEBUILDTO:
		{
			if (iAccountLimit <= 1 )
				return FALSE;

			SQGameMngStrMsg *pGMStrMsg = (SQGameMngStrMsg *)pMsg;
			CBuilding::MoveBuilding( pGMStrMsg->szName, pGMStrMsg->iNum[0], pGMStrMsg->iNum[1], pGMStrMsg->iNum[2], TRUE );
		}
		break;
	case SQGameMngMsg::GM_MOVEBUILDIDTO:
		{
			if (iAccountLimit <= 1 )
				return FALSE;

			SQGameMngStrMsg *pGMStrMsg = (SQGameMngStrMsg *)pMsg;
			if ( pGMStrMsg->szName == NULL )
				break;

			WORD *temp = ( WORD* )( &pGMStrMsg->iNum[3] );
			CBuilding::MoveBuilding( pGMStrMsg->iNum[0], *( QWORD* )( &pGMStrMsg->iNum[1] ), 
				temp[0] ,temp[1], temp[2], TRUE );
		}
		break;
	case SQGameMngMsg::GM_SET_PLAYER_PROPERTY:
		{
			if (iAccountLimit < 5)
				return FALSE;

			SQGameMngStrMsg* pGMMsg = (SQGameMngStrMsg*)pMsg;
			pPlayer->_SetPropertyTemp(pGMMsg->iNum[0], pGMMsg->iNum[1]);
		}
		break;
    case SQGameMngMsg::GM_ADDGOODSEX:     // 3�ƶ��ȼ�
//         if (iAccountLimit < 5)  
//         {
//             pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
//             return FALSE;
//         }
// 
		pPlayer->RecvAddItem(pMsg->iNum[0], pMsg->iNum[1], pMsg->iNum[2]);
        break;

	case SQGameMngMsg::GM_ADDMOUNTS:	// ������
		{
			//pPlayer->CreateMounts(pMsg->iNum[0], pMsg->iNum[1]);
		}
		break;
	case SQGameMngMsg::GM_ADDPETS:		// �ӳ���
		{
			// [2012-7-24 12-06 gw: +�򿪳��﹦��]
			pPlayer->CreatePets(pMsg->iNum[0], pMsg->iNum[1]);
		}
		break;
	case SQGameMngMsg::GM_ADDBUFF:		//��BUFF
		{
// 			if (iAccountLimit < 5)  
// 			{
// 				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
// 				return FALSE;
// 			}
// 
// 			WORD lv = pMsg->iNum[1];
// 			if (lv < 0 || lv > 1000)lv = 0;
// 				
// 			pPlayer->buffManager.AddBuff(pMsg->iNum[0],0,lv,1);
		}
		break;
	case SQGameMngMsg::GM_ADDMOUNT_PROPERTY:	// �޸���������
		{
			if (iAccountLimit < 5)  
			{
				pPlayer->SendErrorMsg(SABackMsg::ERR_GMLIMIT);
				return FALSE;
			}
			//g_Script.SetCondition( NULL, pPlayer, NULL );
			//g_Script.AddMountsProperty( pMsg->iNum[0], pMsg->iNum[1], pMsg->iNum[2] );
			//g_Script.SetCondition( NULL, NULL, NULL );			
			int  index = pMsg->iNum[0];
			if( index<0 )
				index = pPlayer->GetEquipMount();
			int  type = pMsg->iNum[1];
			int		v = pMsg->iNum[2];

			//SPlayerMounts::Mounts* pM = pPlayer->GetMounts(index);
			//if( !pM ) return false;
			g_Script.SetCondition(0,pPlayer,0);
 			g_Script.AddMountsProperty(index,type,v,Mounts::MaxLevel);
			g_Script.SetCondition(0,0,0);
			//{//������
			//	pPlayer->SendStatePanelDataMsg();
			//}
			//SASetMountExp expMsg;
			//expMsg.index = index;
			//expMsg.exp = pM->Exp;
			//expMsg.level = pM->Level;

			//g_StoreMessage(pPlayer->m_ClientIndex,&expMsg,sizeof(expMsg));
		}
		break;
	case SQGameMngMsg::GM_OPENVENA:
		//��ͨѨ��
		{
			//g_Script.SetCondition( NULL, pPlayer, NULL );
			//g_Script.AddMountsProperty( pMsg->iNum[0], pMsg->iNum[1], pMsg->iNum[2] );
			//g_Script.SetCondition( NULL, NULL, NULL );			
			//int  index = pMsg->iNum[0];
			//if( index<=0 )
			if(pPlayer)
				pPlayer->GmOpenVena(pMsg->iNum[0],pMsg->iNum[1]);
		}
		break;
	case SQGameMngMsg::GM_SendMsg:
		{
			if (pPlayer)
			{
				SAChatBroadcastMsg aMsg;
				memcpy(aMsg.BroadcasInfo,pMsg->m_Message,256);
				extern void BroadcastSysNoticeMsg(SMessage *aMsg);
				BroadcastSysNoticeMsg(&aMsg);
			}
		}
		break;
    default:
        break;
    }
#endif    

    return  TRUE;
}

int GetAccountLimit(CPlayer *pPlayer)
{
    return GetGW()->GetAccountLimit(pPlayer);
}

LPCSTR GetScoresList(LPCSTR szType, BYTE bySchool)
{
    return NULL;
}

BOOL MoveSomebody(LPCSTR name, WORD regionid, WORD x, WORD y)
{
    CPlayer *pPlayer = (CPlayer *)GetPlayerByName(name)->DynamicCast(IID_PLAYER);

    if (pPlayer == NULL)
        return false;

    return PutPlayerIntoDestRegion(pPlayer, regionid, x, y);
}

BOOL TypeSomebody(LPCSTR name)
{
//     CPlayer *pPlayer = (CPlayer *)GetPlayerByName(name)->DynamicCast(IID_PLAYER);
// 
//     if (pPlayer == NULL)
//         return false;
// 
//     pPlayer->m_Property.m_Name[ CONST_USERNAME - 1 ] = 0;
// 
//     WORD agi1 = pPlayer->GetAGI();
//     DWORD s1 = 0, s2 = 0, s3 = 0, s6 = 0;
//     int s4 = 0, s5 = 0;
//     WORD agi2 = pPlayer->GetAGIEX( s1, s2, s3, s4, s5, s6 );
// 
//     rfalse(2, 1, "����[%s]\r\n"
//                 "�����˺�[%s]\r\n"
//                 "�ȼ�[%d]\r\n"
//                 "��Ǯ[%d]\r\n"
//                 "��ǰ����λ��[%d][%d/%d]\r\n"
//                 "AGI:[%d][%d]\r\n"
//                 "[%u][%u][%u][%d][%d][%u]\r\n",
//                 pPlayer->m_Property.m_Name,
//                 pPlayer->m_szAccount.c_str(),
//                 pPlayer->m_Property.m_Level,
//                 pPlayer->m_Property.m_Money,
//                 pPlayer->m_Property.m_CurRegionID, pPlayer->m_wCurX, pPlayer->m_wCurY,
//                 agi1, agi2, s1, s2, s3, s4, s5, s6 );

    return true;
}

BOOL CGameWorld::QuestJoinFaction(DNID dnidClient, DWORD GID, CPlayer *pPlayer)
{
    if (dnidClient == 0 ||
        GID == 0 ||
        pPlayer == NULL)
        return FALSE;

	// ���ǰ�����(�˴��ж��������󣬸��ж�����ֻ���ж��������Ƿ��а��ɣ������ж��������ǲ��ǰ���)
    if( pPlayer->m_Property.m_szTongName[0] == 0 )
	{
		TalkToDnid(dnidClient, "����δ�����κΰ��ɣ��޷�������");
        return FALSE;
	}

	if( pPlayer->m_stFacRight.Accept == 0)//û�н���Ȩ
	{
		TalkToDnid(dnidClient, "����û�н���Ȩ���޷�������");
		return FALSE;
	}

    CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(GID)->DynamicCast(IID_PLAYER);
    if (pDestPlayer == NULL)
    {
        TalkToDnid(dnidClient, "�Ҳ���Ŀ����ҡ�����ʧ�ܣ�");
        return FALSE;
    }

	// ��������������
    if (pDestPlayer->m_Property.m_szTongName[0] != 0)
    {
        TalkToDnid(dnidClient, "Ŀ������Ѿ��ǰ��ڡ�����ʧ�ܣ�");
        return FALSE;
    }

    TalkToDnid(dnidClient, "�Ѿ���Ŀ�귢�����롣��ȴ���Ӧ��");

    SAQuestJoinMsg msg;
    msg.dwSrcGID = pPlayer->GetGID();
    msg.dwCheckID = msg.dwSrcGID;

    g_StoreMessage(pDestPlayer->m_ClientIndex, &msg, sizeof(msg));

    return TRUE;
}

BOOL CGameWorld::DispatchTongMessage(DNID dnidClient, struct STongBaseMsg *pMsg, CPlayer *pPlayer)
{
    if (dnidClient == 0 ||
        pMsg == NULL ||
        pPlayer == NULL)
        return FALSE;
	
	return m_FactionManager.DispatchFactionMessage(dnidClient, pMsg, pPlayer);
}

BOOL CGameWorld::LookOtherPlayerEquipment(CPlayer *pSrcPlayer, CPlayer *pDestPlayer, bool b_notice)
{
     if ( pSrcPlayer == NULL )
         return FALSE;
 
     if ( pDestPlayer == NULL )
     {
         TalkToDnid( pSrcPlayer->m_ClientIndex, " ��鿴����Ҳ����߻��߲����ڣ�" );
         return FALSE;
     }
 
     if ( pSrcPlayer->GetGID() == pDestPlayer->GetGID() )
     {
         TalkToDnid( pSrcPlayer->m_ClientIndex, " �鿴�Լ���װ��������ô�鷳��" );
         return FALSE;
     }
 
     SAEquipmentListMsg msg;
	 memcpy(msg.szName, pDestPlayer->m_Property.m_Name, CONST_USERNAME);
     memcpy( msg.Equip, pDestPlayer->m_Property.m_Equip, sizeof(msg.Equip) );

	 msg.dwGlobal		= pDestPlayer->GetGID();
	 msg.wOtherLevel	= pDestPlayer->m_Level;
	 msg.bySchool		= pDestPlayer->m_Property.m_School;
	 msg.bySex			= pDestPlayer->m_Property.m_Sex;

	 msg.m_OtherMaxHP	= pDestPlayer->m_MaxHp;
	 msg.m_OtherMaxMP	= pDestPlayer->m_MaxMp;
	 msg.m_OtherMaxTP	= pDestPlayer->m_MaxTp;

									
	 msg.m_OtherHP		= pDestPlayer->m_CurHp;
	 msg.m_OtherMP		=pDestPlayer->m_CurMp;
	 msg.m_OtherTP		= pDestPlayer->m_CurTp;

	 msg.m_OtherGongJi		= pDestPlayer->m_GongJi;
	 msg.m_OtherFangYu		= pDestPlayer->m_FangYu;
	 msg.m_OtherBaoJi		= pDestPlayer->m_BaoJi;
	 msg.m_OtherShanBi		= pDestPlayer->m_ShanBi;
	 msg.m_OtherAtkSpeed	= PLAYER_BASIC_ATKSPEED + (pDestPlayer->m_AtkSpeed - PLAYER_BASIC_ATKSPEED) / 10;
	 //msg.m_OtherAtkSpeed	= pDestPlayer->m_CurAtkSpeed;
	 msg.m_OtherMoveSpeed	= pDestPlayer->m_OriSpeed / 1000;

	 msg.m_fPositionX	= pDestPlayer->m_Property.m_X;
	 msg.m_fPositionY	= pDestPlayer->m_Property.m_Y;
	 msg.m_fDirection	= pDestPlayer->m_Property.m_dir;

	 msg.bFaceID		= pDestPlayer->m_Property.m_FaceID;
	 msg.bHairID		= pDestPlayer->m_Property.m_HairID; 

	 for (int i = 0; i < sizeof(ShowEquipPos) / sizeof(int); i++)
	 {
		msg.m_3DEquipID[i] = pDestPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex;
		rfalse("3D  %u", pDestPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex);
	 }

	WORD *Other = &msg.m_OtherGongjiPt;
   *Other = pDestPlayer->m_Property.m_JingGong;

   Other ++;
	*Other = pDestPlayer->m_Property.m_FangYu;

	Other ++;
	*Other = pDestPlayer->m_Property.m_QingShen;
	
	Other ++;
	*Other = pDestPlayer->m_Property.m_JianShen;

	msg.m_OtherCurExp = pDestPlayer->m_Property.m_Exp;
	msg.m_OtherMaxExp = pDestPlayer->m_MaxExp;

	Other = &msg.m_Otherbingshang;
	*Other = pDestPlayer->m_IceDamage;

	Other ++;
	*Other = pDestPlayer->m_IceDefence;

	Other ++;
	*Other = pDestPlayer->m_FireDamage;

	Other ++;
	*Other = pDestPlayer->m_FireDefence;

	Other ++;
	*Other = pDestPlayer->m_XuanDamage;

	Other ++;
	*Other = pDestPlayer->m_XuanDefence;

	Other ++;
	*Other = pDestPlayer->m_PoisonDamage;

	Other ++;
	*Other = pDestPlayer->m_PoisonDefence;

	Other ++;
	*Other = pDestPlayer->m_ReduceDamage;

	Other ++;
	*Other = pDestPlayer->m_AbsDamage;

	Other ++;
	*Other = pDestPlayer->m_NonFangyu;

	Other ++;
	*Other = pDestPlayer->m_MultiBaoJi;

	msg.m_OtherJP = pDestPlayer->m_Property.m_CurJp;
	msg.m_OtherMaxJP = pDestPlayer->m_MaxJp;

	msg.m_OtherShaLu = pDestPlayer->m_Property.m_PKValue;
	msg.m_OtherXiaYi = pDestPlayer->m_Property.m_XYValue;
	msg.m_XwzValue = pDestPlayer->m_XwzValue;

	 g_StoreMessage(pSrcPlayer->m_ClientIndex, &msg, sizeof(msg));
	 
//	memcpy( msg.szName, pDestPlayer->m_Property.m_Name, CONST_USERNAME );         // ����
// 	memcpy( msg.szTitle, pDestPlayer->m_Property.m_szTitle, CONST_USERNAME );       // �ƺ�	
// 	memcpy( msg.szFaction, pDestPlayer->m_Property.m_szTongName, CONST_USERNAME );  // ����
// 	memcpy( msg.szSpouse, pDestPlayer->m_Property.m_szMateName, CONST_USERNAME );   // ��ż
//  memcpy( msg.szName, pDestPlayer->m_Property.m_Name, CONST_USERNAME );         // ����
// 
//     msg.dwGID          = pDestPlayer->GetGID();
//      msg.wkillValue     = pDestPlayer->m_wPKValue;           // ɱ��ֵ
//      msg.nValue         = pDestPlayer->m_Property.m_XYValue;             // ����ֵ
// 	 msg.nJpValue         = pDestPlayer->m_Property.m_CurJp //����
// 	msg.nExpValue         = pDestPlayer->m_Property.m_Exp//����
//     msg.dwExtraState   = pDestPlayer->m_dwExtraState;                   // ״̬
//     msg.wMedicalEthics = pDestPlayer->m_Property.wMedicalEthics;        // ҽ��ֵ
//     msg.nState         = pDestPlayer->GetCurActionID() == EA_ZAZEN ? EA_ZAZEN :EA_STAND;
//     msg.bySex          = pDestPlayer->m_Property.m_Sex;
//     msg.byBRON         = pDestPlayer->m_Property.m_byBRON;
//     msg.bySchool       = pDestPlayer->m_Property.m_School;
//     msg.wWeaponIndex   = pDestPlayer->m_Property.m_Equip[15].wIndex;
//     msg.wEquipIndex    = pDestPlayer->m_Property.m_Equip[13].wIndex;
//     msg.wMutateID      = pDestPlayer->GetMutateID();
//     msg.wOtherLevel    = pDestPlayer->m_Property.m_Level;
// 
//     msg.newMark = 0xAF;
//     msg.infoLength = 0;
//     
//     // [����չ�Ĺ���]�ű����ɵĸ�����Ϣ�鿴��ַ��
//     DWORD nullSize = sizeof( msg.infoString );
//     lite::Variant lvt;
//     g_Script.SetCondition(NULL, pDestPlayer, NULL);
//     LuaFunctor(g_Script, "LookInfoString")[pSrcPlayer->GetGID()][pSrcPlayer->m_Property.puid]( &lvt );
//     g_Script.CleanCondition();
//     if ( lvt.dataType == lite::Variant::VT_STRING && lvt.dataSize < sizeof( msg.infoString ) ) {
//         if ( lvt.dataSize == ( BYTE )lvt.dataSize ) { // ����߼���Ϊ�˷�ֹ��������תBYTEʱ���ضϣ���infoString���鳤�ȴ���256ʱ����֣�
//             msg.infoLength = ( BYTE )lvt.dataSize;
//             memcpy( msg.infoString, lvt._string, lvt.dataSize );
//             nullSize -= lvt.dataSize;
//         }
//     }
// 
//     g_StoreMessage(pSrcPlayer->m_ClientIndex, &msg, sizeof(msg));
// 
// 	if (b_notice)
// 	{
// 		LPCSTR info = FormatString("��λ%s���ڴ�����", (pSrcPlayer->m_Property.m_Sex == 1) ? "��ʿ" : "Ů��");
// 			//(pSrcPlayer->m_Property.m_Sex == pDestPlayer->m_Property.m_Sex) ?
// 			//"���" : "����ĬĬ");
// 
// 		TalkToDnid(pDestPlayer->m_ClientIndex, info);
// 	}
    return TRUE;
}

// �����3�鿴���а�װ����Ϣ
BOOL CGameWorld::LookRankListEquipInfo(CPlayer *pSrcPlayer, CPlayer *pDestPlayer)
{
	if ( !pSrcPlayer)
		return FALSE;

	if ( !pDestPlayer)
	{
		TalkToDnid( pSrcPlayer->m_ClientIndex, " ��鿴����Ҳ����ߣ�" );
		return FALSE;
	}

	SAankEquipInfoMsg msg;
	memcpy(msg.szName, pDestPlayer->m_Property.m_Name, CONST_USERNAME);
	memcpy( msg.Equip, pDestPlayer->m_Property.m_Equip, sizeof(msg.Equip) );

	msg.dwGlobal		= pDestPlayer->GetGID();
	msg.wOtherLevel	= pDestPlayer->m_Property.m_Level;
	msg.bySchool		= pDestPlayer->m_Property.m_School;
	msg.bySex			= pDestPlayer->m_Property.m_Sex;

	msg.m_fPositionX	= pDestPlayer->m_Property.m_X;
	msg.m_fPositionY	= pDestPlayer->m_Property.m_Y;
	msg.m_fDirection	= pDestPlayer->m_Property.m_dir;

	msg.bFaceID		= pDestPlayer->m_Property.m_FaceID;
	msg.bHairID		= pDestPlayer->m_Property.m_HairID;

	for (int i = 0; i < sizeof(ShowEquipPos) / sizeof(int); i++)
	{
		msg.m_3DEquipID[i] = pDestPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex;
		rfalse("3D  %u", pDestPlayer->m_Property.m_Equip[ShowEquipPos[i]].wIndex);
	}

	g_StoreMessage(pSrcPlayer->m_ClientIndex, &msg, sizeof(SAankEquipInfoMsg));

	return TRUE;
}

BOOL CGameWorld::UpdatePlayerData(SFixBaseData * pPlayerBaseData)
{
//     CPlayer * pPlayer;
//     pPlayer = (CPlayer *)GetPlayerByName(pPlayerBaseData->m_Name)->DynamicCast(IID_PLAYER);    
//     {
//         if(pPlayer == NULL)
//         {
//             return FALSE;
//         }
//         else
//         {
//             pPlayer->UpdatePlayerProperty(pPlayerBaseData);
//         }
//     }
    return TRUE;
}

BOOL CGameWorld::RecvPrepareExchangeResult(SAPrepareExchangeMsg *pMsg)
{
 //   SExchangeFullInfo fi;
 //   if (PopExchangeInfo(pMsg->vi, fi))
	//{
 //       CPlayer *pPlayer = (CPlayer *)GetPlayerByDnid(fi.m_SrcDnid)->DynamicCast(IID_PLAYER);
 //       if (pPlayer == NULL)
 //           return FALSE;

 //       pPlayer->EndPrepareExchange(&fi, pMsg->result);
 //       return TRUE;
 //   }

    return FALSE;
}

BOOL CGameWorld::DispatchCardMessage(DNID dnidClient, SPointModifyMsg *pMsg, CPlayer *pPlayer)
{
	//if(dnidClient == 0 ||  pMsg == NULL || pPlayer == NULL)
	//	return FALSE;

	//switch(pMsg->_protocol)
	//{
	//case SPointModifyMsg::EPRO_EXCHANGEMC_BASE:
	//	{
	//		SExchangeMCMsg *pMCM = (SExchangeMCMsg *)pMsg;
	//		_EMCWorkInstance->Entry(pMCM, pPlayer);

	//	}
	//	case SPointModifyMsg::EPRO_ENCOURAGEEMENT: //�齱
	//		{
	//			SendToLoginServer(pMsg,sizeof(SQEncouragement));
	//		}
	//	
	//default:
	//	return FALSE;
	//}

	return TRUE;
}

void ReconnectAllServer()
{
    GetGW()->CPlayerManager::Reconnect();
}

BOOL CGameWorld::DispatchPhoneMessage(DNID dnidClient,SBasePhoneMsg *pMsg)
{
    return TRUE;
}

BOOL  CGameWorld::NotifyEncouragement(struct SAEncouragement *pMsg)
{
    //�������ID\֪ͨ��ҽ��
    CPlayer * pPlayer =NULL;
    pPlayer = (CPlayer *)GetPlayerByName(pMsg->m_szName)->DynamicCast(IID_PLAYER);

    if ( pPlayer ) 
    {
//         SRoleTask * retTask = pPlayer->FindTask(1204);
//         if (retTask)
//         {
//             retTask->wComplete = (WORD)pMsg->m_iRet;
//             switch (pMsg->m_iRet)
//             {
//             case SAEncouragement::RET_INVALIDCARD :
//                 {
//                     TalkToDnid(pPlayer->m_ClientIndex, "���Ǳ�Ǹ���������������Ч�ҽ����롣");
//                     pPlayer->DelTask(retTask);
// 
//                 };
//                 break;
//             case SAEncouragement::RET_ISUSEED :
//                 {
//                     TalkToDnid(pPlayer->m_ClientIndex, "���Ǳ�Ǹ�����µĶҽ������Ѿ���ȡ�������ˡ�");
//                     pPlayer->DelTask(retTask);
//                 };
//                 break;
//             case SAEncouragement::RET_WEEKOK :
//                 {
//                     TalkToDnid(pPlayer->m_ClientIndex, "��֤�ɹ�����ȥ�콱�ɡ�");
//                 }//����OK
//                 break;
//             case SAEncouragement::RET_MONTH:
//                 {
//                     TalkToDnid(pPlayer->m_ClientIndex, "��֤�ɹ�����ȥ�콱�ɡ�");
//                 }//����OK
//                 break;
//             }
//         }	

    };
    return TRUE;
}

BOOL CGameWorld::DispatchGMIDCheckMsg(DNID dnidClient, struct SQCheckGMInfMsg *pMsg, CPlayer *pPlayer)
{
    if ( pMsg == NULL || pPlayer == NULL )
        return FALSE;

    try
    {
        extern DWORD GetGroupID();
        lite::Serialreader slr( pMsg->streamData );
        LPCSTR key = slr();
        if ( key == NULL )
            return FALSE;

        SGMCheck msg;
        msg.wGameType = 3;
        msg.dwServerid = GetGroupID();
        msg.mac = pPlayer->GetMac();
        msg.sid = pPlayer->GetSID();
        msg.wGMCmd = pPlayer->m_GMIDCheckInf.SQGMMsg.wGMCmd;
        msg.dwRand = pPlayer->m_GMIDCheckInf.dwRand;
        msg.cradKey = key;
        msg.account = pPlayer->GetAccount();
        msg.msg = pPlayer->m_GMIDCheckInf.SQGMMsg;
        
        LPCSTR ip = GetIPString( pPlayer->m_ClientIndex, NULL );
        msg.ip = ip ? ip : "";
        gmCheckQueue.mtsafe_push( msg );

    }
    catch ( lite::Xcpt &)
    {   	
    }

    return TRUE;
}


// ��ǰ�ɵ����а�XYD3����
SScoreTable s_Scores;
SScoreTable s_ScoresRecord;  // ���ڱȽ�
/*
static void OperateScoreFile( BOOL bOperate, const char *filename, SScoreTable::SScoreRecode *arrayScore, DWORD dwMaxRecode )
{
    if (bOperate) 
    {
        std::ofstream fout( filename );
        if ( !fout.is_open() )
            return;

        for( DWORD i = 0; i < dwMaxRecode; ++i ) 
        {
            if ( arrayScore[i].Name[0] != 0 )
                fout << arrayScore[i].Name << std::endl << arrayScore[i].dwValue << std::endl;
        }

        fout.close();
    }
    else 
    {
        std::ifstream fin( filename );
        if ( !fin.is_open() ) 
        {
            memset( arrayScore, 0, sizeof(SScoreTable::SScoreRecode) * dwMaxRecode );
            return;
        }
        //std::strstream::

        DWORD dwNum = 0;
        char szBuf[128] = {0};
        while( dwNum < dwMaxRecode ) 
        {
            if (fin.eof()) 
                break;

            fin >> szBuf >> arrayScore[dwNum].dwValue;
            dwt::strcpy( arrayScore[dwNum].Name, szBuf, CONST_USERNAME );
            dwNum++;
        }

        fin.close();
    }
}

static void OperateScoreFileEx( BOOL bOperate, const char *filename, SScoreTable::SScoreRecodeEx *arrayScore, DWORD dwMaxRecode )
{
    if (bOperate) 
    {
        std::ofstream fout( filename );
        if ( !fout.is_open() )
            return;

        for( DWORD i = 0; i < dwMaxRecode; ++i ) 
        {
            if ( arrayScore[i].Name[0] != 0 )
            {
                fout << arrayScore[i].Name << "\t" << arrayScore[i].dwValue;
                fout << "\t" << arrayScore[i].level << "\t" << arrayScore[i].sid;
                fout << std::endl;
            }
        }

        fout.close();
    }
    else 
    {
        std::ifstream fin( filename );
        if ( !fin.is_open() ) 
        {
            memset( arrayScore, 0, sizeof( SScoreTable::SScoreRecodeEx ) * dwMaxRecode );
            return;
        }

        #define CHECKEOF() { if ( fin.eof() ) break; }

        DWORD dwNum = 0;
        char szBuf[128] = {0};
        while( dwNum < dwMaxRecode ) 
        {
            CHECKEOF();
            fin >> szBuf;
            CHECKEOF();
            fin >> arrayScore[dwNum].dwValue;
            CHECKEOF();
            fin >> arrayScore[dwNum].level;
            CHECKEOF();
            fin >> arrayScore[dwNum].sid;

            dwt::strcpy( arrayScore[dwNum].Name, szBuf, CONST_USERNAME );
            dwNum++;
        }

        fin.close();
    }
}

static BOOL LoadScores()
{
    // �ӱ����ļ���߻�ȡ��������  
	memset(&s_Scores, 0, sizeof(s_Scores));
	memset(&s_ScoresRecord, 0, sizeof(s_ScoresRecord));
    OperateScoreFile( 0, "NewXYDScores\\taxis_Alevel.txt", s_Scores.ALevel,  MAX_SCORE_NUMBER );
    OperateScoreFile( 0, "NewXYDScores\\taxis_Amoney.txt", s_Scores.AMoney,  MAX_SCORE_NUMBER );
    OperateScoreFile( 0, "NewXYDScores\\taxis_Fame1.txt",  s_Scores.FAME[0], MAX_SCORE_NUMBER );
    OperateScoreFile( 0, "NewXYDScores\\taxis_Fame2.txt",  s_Scores.FAME[1], MAX_SCORE_NUMBER );
    OperateScoreFile( 0, "NewXYDScores\\taxis_MedicalEthics.txt",  s_Scores.MedicalEthics, MAX_SCORE_NUMBER );

    OperateScoreFile( 0, "NewXYDScores\\taxis_PKValue.txt",   s_Scores.PKValue,   MAX_SCORE_NUMBER );

    OperateScoreFileEx( 0, "NewXYDScores\\taxis_RoseNum.txt",   s_Scores.RoseNum,   MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 0, "NewXYDScores\\taxis_BaoTu.txt",     s_Scores.BaoTu,     MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 0, "NewXYDScores\\taxis_ZhenFa.txt",    s_Scores.ZhenFa,    MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 0, "NewXYDScores\\taxis_HuntScore.txt", s_Scores.HuntScore, MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 0, "NewXYDScores\\taxis_MazeScore.txt", s_Scores.MazeScore, MAX_FIVESCORE_NUMBER );

    char szBuf[256] = { 0 };
    for( int i = 0; i < MAX_SCHOOL_NUMBER; i++ ) 
    {
        sprintf(szBuf, "NewXYDScores\\taxis_Slevel%d.txt",i);
        OperateScoreFile(0, szBuf,  s_Scores.Level[i], MAX_SCORE_NUMBER);
    }

    memcpy( &s_ScoresRecord, &s_Scores, sizeof(SScoreTable) );

    return TRUE;
}

static BOOL SaveScores()
{
    if( memcmp(&s_ScoresRecord, &s_Scores, sizeof(SScoreTable)) == 0 )
        return TRUE;

    DWORD dwAttr = GetFileAttributes( "NewXYDScores" );  // ��ֹ������û������ļ���
    if( dwAttr == 0xffffffff )
    {
        if ( !CreateDirectory( "NewXYDScores", NULL ) )
            rfalse( 2, 1, "����[NewXYDScores]�ļ���ʧ�ܣ���Ҫ�ֹ�������" );
    }

    // �˳��������а��ļ�
    OperateScoreFile( 1, "NewXYDScores\\taxis_Alevel.txt", s_Scores.ALevel,  MAX_SCORE_NUMBER );
    OperateScoreFile( 1, "NewXYDScores\\taxis_Amoney.txt", s_Scores.AMoney,  MAX_SCORE_NUMBER );
    OperateScoreFile( 1, "NewXYDScores\\taxis_Fame1.txt",  s_Scores.FAME[0], MAX_SCORE_NUMBER );
    OperateScoreFile( 1, "NewXYDScores\\taxis_Fame2.txt",  s_Scores.FAME[1], MAX_SCORE_NUMBER );
    OperateScoreFile( 1, "NewXYDScores\\taxis_MedicalEthics.txt",  s_Scores.MedicalEthics,   MAX_SCORE_NUMBER );

    OperateScoreFile( 1, "NewXYDScores\\taxis_PKValue.txt",   s_Scores.PKValue,   MAX_SCORE_NUMBER );

    OperateScoreFileEx( 1, "NewXYDScores\\taxis_RoseNum.txt",   s_Scores.RoseNum,   MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 1, "NewXYDScores\\taxis_BaoTu.txt",     s_Scores.BaoTu,     MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 1, "NewXYDScores\\taxis_ZhenFa.txt",    s_Scores.ZhenFa,    MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 1, "NewXYDScores\\taxis_HuntScore.txt", s_Scores.HuntScore, MAX_FIVESCORE_NUMBER );
    OperateScoreFileEx( 1, "NewXYDScores\\taxis_MazeScore.txt", s_Scores.MazeScore, MAX_FIVESCORE_NUMBER );

    char szBuf[256] = { 0 };
    for(int i = 0; i < MAX_SCHOOL_NUMBER; i++) 
    {
        sprintf(szBuf, "NewXYDScores\\taxis_Slevel%d.txt",i);
        OperateScoreFile(1, szBuf,  s_Scores.Level[i], MAX_SCORE_NUMBER);
    }

    memcpy( &s_ScoresRecord, &s_Scores, sizeof(SScoreTable) );

    return TRUE;
}

static void UpdateScores( SScoreTable::SScoreRecode &theScore, SScoreTable::SScoreRecode *arrayScore, 
                          DWORD dwMaxRecode, WORD wScoreType, BYTE byOrder = 1 )
{
//     if ( arrayScore == NULL )
//         return;
// 
//     if ( dwMaxRecode < 1 )
//         return;
// 
//     for ( DWORD i = 0; i < dwMaxRecode; ++i )
//     {
//         if ( dwt::strcmp( arrayScore[i].Name, theScore.Name, sizeof( theScore.Name ) ) == 0 )
//         {
//             memmove( &arrayScore[i], &arrayScore[i+1], (dwMaxRecode-i-1) * sizeof( SScoreTable::SScoreRecode ) );
//             arrayScore[dwMaxRecode - 1].dwValue = 0;
//             arrayScore[dwMaxRecode - 1].Name[0] = 0;
//             break;
//         }
//     }
// 
//     BOOL bThing = 1;
// 
//     for ( i = 0; i < dwMaxRecode; ++i )
//     {     
//         if ( wScoreType == SScoreTable::ST_MEDICALETHICS || wScoreType == SScoreTable::ST_KILLVAL )
//         {
//             WORD warrayValue =  (WORD)arrayScore[i].dwValue & 0xffff; //ҽ��ֵ
//             WORD wtheValue = (WORD)theScore.dwValue & 0xffff;
//             if ( ( warrayValue == wtheValue ) && (theScore.dwValue >> 16) > 0 ) //ҽ��ֵ����жϵȼ�
//                 bThing = ( (arrayScore[i].dwValue >> 16) < (theScore.dwValue >> 16));
//             else
//                 bThing = (byOrder == 1) ? (warrayValue < wtheValue) : (warrayValue > wtheValue);
//         }
//         else if ( wScoreType == SScoreTable::ST_LFAME )
//             bThing = (byOrder == 1) ? ( (int)arrayScore[i].dwValue < (int)theScore.dwValue ) :( (int)arrayScore[i].dwValue > (int)theScore.dwValue );
//         else if ( wScoreType == SScoreTable::ST_ALEVEL)
//             bThing = (byOrder == 1) ? ( (arrayScore[i].dwValue & 0xffff) < (theScore.dwValue & 0xffff) ) : ( (arrayScore[i].dwValue & 0xffff) > (theScore.dwValue & 0xffff) );
//         else
//             bThing = (byOrder == 1) ? ( arrayScore[i].dwValue < theScore.dwValue ) : ( arrayScore[i].dwValue > theScore.dwValue );
// 
//         if ( bThing )
//         {
//             // �������ͷ��
//             if ( wScoreType >= SScoreTable::ST_ALEVEL && wScoreType <= SScoreTable::ST_MEDICALETHICS )
//             {
//                 DWORD byState = 0;
//                 if ( wScoreType == SScoreTable::ST_ALEVEL )
//                     byState = SHOWEXTRASTATE_SCORES_LEVEL;
//                 else if ( wScoreType == SScoreTable::ST_AMONEY )
//                     byState = SHOWEXTRASTATE_SCORES_MONEY;
//                 else if ( wScoreType == SScoreTable::ST_RFAME )
//                     byState = SHOWEXTRASTATE_SCORES_FAMEG;
//                 else if ( wScoreType == SScoreTable::ST_LFAME )
//                     byState = SHOWEXTRASTATE_SCORES_FAMEX;
//                 else if ( wScoreType == SScoreTable::ST_MEDICALETHICS )
//                     byState = SHOWEXTRASTATE_SCORES_MEDICALETHICS;
// 
//                 CPlayer *pPlayer = NULL ;
//                 if ( arrayScore[ dwMaxRecode - 1 ].Name[0] != '\0' )
//                 {
//                     if ( pPlayer = (CPlayer *)GetPlayerByName(arrayScore[ dwMaxRecode - 1 ].Name)->DynamicCast(IID_PLAYER) )                
//                         pPlayer->SetScoresState( 0, byState );
//                 }
// 
//                 if ( pPlayer = (CPlayer *)GetPlayerByName(theScore.Name)->DynamicCast(IID_PLAYER) )
//                     pPlayer->SetScoresState( byState, 0 );
//             }
// 
//             if ( i < dwMaxRecode - 1 )
//                 memmove( &arrayScore[ i + 1 ], &arrayScore[ i ], ( dwMaxRecode - i - 1 ) * sizeof( SScoreTable::SScoreRecode ) );
//             else
//             {
//                 arrayScore[dwMaxRecode - 1].dwValue = 0;
//                 arrayScore[dwMaxRecode - 1].Name[0] = 0;
//             }
//             arrayScore[i] = theScore;
//             break;
//         }
//     }
}

static void UpdateScoresEx( SScoreTable::SScoreRecodeEx &theScore, SScoreTable::SScoreRecodeEx *arrayScore, 
                         DWORD dwMaxRecode, WORD wScoreType, BYTE byOrder = 1 )
{
    if ( arrayScore == NULL )
        return;

    if ( dwMaxRecode < 1 )
        return;

    for ( DWORD i = 0; i < dwMaxRecode; ++i )
    {
        if ( dwt::strcmp( arrayScore[i].Name, theScore.Name, sizeof( theScore.Name ) ) == 0 )
        {
            memmove( &arrayScore[i], &arrayScore[i+1], (dwMaxRecode-i-1) * sizeof( SScoreTable::SScoreRecodeEx ) );
            arrayScore[dwMaxRecode - 1].dwValue = 0;
            arrayScore[dwMaxRecode - 1].Name[0] = 0;
            arrayScore[dwMaxRecode - 1].level   = 0;
            arrayScore[dwMaxRecode - 1].sid     = 0;
            break;
        }
    }

    BOOL bThing = FALSE;

    for ( i = 0; i < dwMaxRecode; ++i )
    {     
        if ( wScoreType == SScoreTable::ST_ROSENUM || 
            wScoreType == SScoreTable::ST_BAOTU  || 
            wScoreType == SScoreTable::ST_ZHENFA || 
            wScoreType == SScoreTable::ST_HUNTER || 
            wScoreType == SScoreTable::ST_MIGONG )
        {
            // ��������������������У��Ȳ���order��
            BOOL isTrue = FALSE;
            if ( wScoreType == SScoreTable::ST_ROSENUM )
                isTrue = ( ( arrayScore[i].dwValue & 0x7fffffff ) < ( theScore.dwValue & 0x7fffffff ) );
            else    
                isTrue = ( arrayScore[i].dwValue < theScore.dwValue );

            BOOL isSubTrue = ( ( wScoreType != SScoreTable::ST_ROSENUM ) ? ( arrayScore[i].dwValue == theScore.dwValue ) : 
                               ( ( arrayScore[i].dwValue & 0x7fffffff ) == ( theScore.dwValue & 0x7fffffff ) ) );

            if ( isTrue )
                bThing = TRUE;
            else if ( isSubTrue )
            {
                if ( arrayScore[i].level < theScore.level )
                    bThing = TRUE;
                else if ( arrayScore[i].level == theScore.level )
                {
                    if ( arrayScore[i].sid < theScore.sid )
                        bThing = TRUE;
                }
            }
            else
                bThing = FALSE;

        }

        if ( bThing )
        {
            if ( i < dwMaxRecode - 1 )
                memmove( &arrayScore[ i + 1 ], &arrayScore[ i ], ( dwMaxRecode - i - 1 ) * sizeof( SScoreTable::SScoreRecodeEx ) );
            else
            {
                arrayScore[dwMaxRecode - 1].dwValue = 0;
                arrayScore[dwMaxRecode - 1].Name[0] = 0;
                arrayScore[dwMaxRecode - 1].level   = 0;
                arrayScore[dwMaxRecode - 1].sid     = 0;
            }
            arrayScore[i] = theScore;
            break;
        }
    }
}
*/


void CGameWorld::UpdateNewXYDScore( SFixProperty *p )
{
//     SScoreTable::SScoreRecode   theScore;
//     SScoreTable::SScoreRecodeEx theScoreEx;
//     ZeroMemory( &theScore, sizeof( theScore ) );
//     ZeroMemory( &theScoreEx, sizeof( theScoreEx ) );
// 
//     if ( p->m_bySchool >= 5 )
//         return;
// 
//     if ( p->m_GMLevel != 0 )
//         return;
// 
//     dwt::strcpy(theScore.Name, p->m_szName, CONST_USERNAME);
//     dwt::strcpy(theScoreEx.Name, p->m_szName, CONST_USERNAME);
//     theScore.Name[10] = 0;
// 
//     theScore.dwValue = p->m_byLevel;	
//     UpdateScores( theScore, s_Scores.Level[p->m_bySchool], MAX_SCORE_NUMBER, SScoreTable::ST_SLEVEL );
// 
//     theScore.dwValue = ( p->m_bySchool << 16 ) | ( p->m_byLevel & 0xffff );
//     UpdateScores( theScore, s_Scores.ALevel, MAX_SCORE_NUMBER, SScoreTable::ST_ALEVEL );
// 
//     theScore.dwValue = p->m_sXValue;
//     UpdateScores( theScore, s_Scores.XValue[p->m_bySchool], MAX_SCORE_NUMBER, SScoreTable::ST_SXVALUE );
// 
//     if ( p->m_sXValue >= 0 )
//         UpdateScores( theScore, s_Scores.FAME[0], MAX_SCORE_NUMBER, SScoreTable::ST_RFAME );
//     else
//         UpdateScores( theScore, s_Scores.FAME[1], MAX_SCORE_NUMBER, SScoreTable::ST_LFAME, 0 );
// 
//     theScore.dwValue = p->m_dwMoney + p->m_dwStoreMoney;
//     UpdateScores( theScore, s_Scores.Money[p->m_bySchool], MAX_SCORE_NUMBER, SScoreTable::ST_SMONEY );
//     UpdateScores( theScore, s_Scores.AMoney, MAX_SCORE_NUMBER, SScoreTable::ST_AMONEY );
// 
//     theScore.dwValue = ( p->m_byLevel << 16 ) | ( p->wMedicalEthics & 0xffff );
//     UpdateScores( theScore, s_Scores.MedicalEthics, MAX_SCORE_NUMBER, SScoreTable::ST_MEDICALETHICS );
// 
//     theScore.dwValue = ( p->m_byLevel << 16 ) | ( p->m_byPKValue & 0xffff );
//     UpdateScores( theScore, s_Scores.PKValue, MAX_SCORE_NUMBER, SScoreTable::ST_KILLVAL );
// 
// 
//     // �����
//     theScoreEx.level = p->m_byLevel;
//     theScoreEx.sid     = p->m_dwStaticID;
//     theScoreEx.dwValue = ( p->m_bySex << 31 ) | ( ( p->dwTotalCharmValue ) & 0x7fffffff );
//     UpdateScoresEx( theScoreEx, s_Scores.RoseNum, MAX_FIVESCORE_NUMBER, SScoreTable::ST_ROSENUM );
// 
//     theScoreEx.dwValue = p->dwBaoTuScore;
//     UpdateScoresEx( theScoreEx, s_Scores.BaoTu, MAX_FIVESCORE_NUMBER, SScoreTable::ST_BAOTU );
// 
//     theScoreEx.dwValue = p->dwZhenFaPassScore;
//     UpdateScoresEx( theScoreEx, s_Scores.ZhenFa, MAX_FIVESCORE_NUMBER, SScoreTable::ST_ZHENFA );
// 
//     theScoreEx.dwValue = p->dwHuntScore;
//     UpdateScoresEx( theScoreEx, s_Scores.HuntScore, MAX_FIVESCORE_NUMBER, SScoreTable::ST_HUNTER );
// 
//     theScoreEx.dwValue = p->dwMazePassScore;
//     UpdateScoresEx( theScoreEx, s_Scores.MazeScore, MAX_FIVESCORE_NUMBER, SScoreTable::ST_MIGONG );

    // �������ͷ��
    //UpdatePlayerScores( &s_Scores );
}

BOOL CGameWorld::GetLuckySystemPtr(CLuckyTimeSystem** ptr)
{
	if (m_ptrLuckySystem == NULL)
		return FALSE;

	*ptr = m_ptrLuckySystem.get();

	return TRUE;
}

BOOL CGameWorld::GWSaveScores()
{
	// return SaveScores();
	return FALSE;
}

BOOL CGameWorld::GWLoadScores()
{
	// return LoadScores();
	return FALSE;
}
void CGameWorld::AddGlobalBlessInfo(std::string name,std::string ItemName)
{
	GLoblessInfoSTR Info;
	Info.name = name;
	Info.ItemName = ItemName;
	if (m_GlobalInfo.size() >= 10)
	{
		m_GlobalInfo.pop_front();
		m_GlobalInfo.push_back(Info);
	}
	else
	{
		m_GlobalInfo.push_back(Info);
	}
}
void CGameWorld::OnReadBlessFile()
{
	FILE* fr;
	fr = fopen("LOGS\\BlessInfo.txt","a+");//���ļ��ķ�ʽ��û�оʹ������в��ᱻ����
	if (fr != NULL)
	{
		char temp[100];
		memset(temp,0,100);
		int i = 0;
		std::string name;
		std::string ItemName;
		while (fgets(temp,100,fr))
		{
			if (i == 0)
			{
				name = temp;
				size_t size = name.size();
				name = name.substr(0,name.size() - 1);
				size = name.size();
				i = 1;
			}
			else if (i == 1)
			{
				ItemName = temp;
				ItemName = ItemName.substr(0,ItemName.size() - 1);
				AddGlobalBlessInfo(name,ItemName);
				i = 0;
			}
			memset(temp,0,100);

		}
		fclose(fr);
	}
	else
	{
		//rfalse(2,1,"�������ļ�ʧ��!!!");
	}
}
void CGameWorld::OnWriteArenaRankFile()
{
	CArenaMoudel::getSingleton()->Release();
}

void CGameWorld::OnWriteBlessFile()
{
	FILE * fw = NULL;
	fw = fopen("LOGS\\BlessInfo.txt","w+");//�����ļ���ǰ���������
	if (fw)
	{
		GLOBALBLESSINFO::iterator iter = m_GlobalInfo.begin();
		for (; iter != m_GlobalInfo.end(); ++iter)
		{

			GLoblessInfoSTR temp;
			temp.name = iter->name;
			temp.name.append(1,'\n');
			temp.ItemName = iter->ItemName;
			temp.ItemName.append(1,'\n');
			fputs(temp.name.c_str(),fw);
			fputs(temp.ItemName.c_str(),fw);
		}
		fclose(fw);
	}
	else
	{
		int i = 0;
	}
}
void CGameWorld::ProcessBlessInfo(DNID Nid,BYTE flag)
{
	SABlessExcellenceDataMsg pMsg;
	pMsg.m_Num = m_GlobalInfo.size();
	pMsg.m_BlessFlag = flag;
	if (pMsg.m_Num != 0)
	{
		int i = 0;
		GLOBALBLESSINFO::iterator iter = m_GlobalInfo.begin();
		for (;iter != m_GlobalInfo.end(); ++iter,++i)
		{
			SPackageItem item;
			item.flags			=0;	// �����ߵĵ������Ա�־
			item.overlap				=0;	// ��ǰ��������
			item.usetimes			=0;	// ����ʹ�õĴ����������һ���Եģ���Ϊ1��ʹ�����ɾ����
			item.details.uniqueTimestamp		=0;	// [ʱ���      ] 32bit����30��Ϊ��λ���м�¼
			item.details.uniqueCreateIndex	=0;	// [30�����ۼ�ֵ] 30bit��ÿ30���ڵ��������Ͽ�����1073741823������
			item.details.uniqueServerId		=0;	// [���������  ] 2 bit����¼���ߵĲ���λ�ã��Ա����ڲ�ͬ�������ϵı���ظ�
			memcpy(pMsg.m_ExData[i].Name,iter->name.c_str(),iter->name.size());
			item.wIndex = atol(iter->ItemName.c_str());
			pMsg.m_ExData[i].Item = item;
			//memcpy(pMsg.m_ExData[i].ItemIDName,iter->ItemName.c_str(),iter->ItemName.size());
		}
	}
	g_StoreMessage(Nid,&pMsg,sizeof(SABlessExcellenceDataMsg));
}

static bool SortMonsterData(BossDeadData p1, BossDeadData p2)
{
	return p1.m_UpdateTime > p2.m_UpdateTime;
}

// void CGameWorld::UpdateBossData( WORD id,BossDeadData &monsterdata )
// {
// 	//������������
// 	 std::hash_map<WORD, BossDeadData>::iterator bossiter = m_BossDeadData.find(id);
// 	if(bossiter != m_BossDeadData.end()){
// 		bossiter->second = monsterdata;
// 		m_BossupdateTime = timeGetTime();
// 		return;
// 	}
// 	m_BossDeadData[id] = monsterdata;
// 	m_BossupdateTime = timeGetTime();
// 	return;
// }

void CGameWorld::UpdateBossData( BossDeadData &monsterdata )
{
	std::vector<BossDeadData>::iterator bossiter = m_BossDeadData.begin();
	while(bossiter != m_BossDeadData.end())
	{
		BossDeadData Tempdata = *bossiter;
		//������������
		if (Tempdata.m_ID == monsterdata.m_ID)
		{
			m_BossDeadData.erase(bossiter);
			m_BossDeadData.push_back(monsterdata);
			 return;
		}
		bossiter++;
	}
	m_BossDeadData.push_back(monsterdata);
	std::sort(m_BossDeadData.begin(),m_BossDeadData.end(),SortMonsterData);
	return;
}

void LoadFromBuff(BYTE **pBuff,WORD &datavalue)
{
	memcpy(&datavalue,*pBuff,sizeof(WORD));
	*pBuff += sizeof(WORD);
}

void CGameWorld::RecvUpdateBossData( SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer )
{
	if (!m_BossDeadData.size())return;
	if (pmsg->nBossState > m_BossDeadData.back().m_UpdateTime)return;
	
	std::vector<BossDeadData>::iterator bossiter = m_BossDeadData.begin();
	SABossDeadUpdatemsg DeadUpdatemsg ;
	BYTE *pBuff = DeadUpdatemsg.Buff;
	memset(pBuff,0,MAX_BOSSUPDATE_SIZE);

	SABossDeadUpdatemsg sendDeadUpdatemsg ;
	memset(sendDeadUpdatemsg.Buff,0,MAX_BOSSUPDATE_SIZE);

	WORD Updatenum = 0;
	while(bossiter != m_BossDeadData.end())
	{
		BossDeadData Tempdata = *bossiter;
		if (Tempdata.m_UpdateTime > pmsg->nBossState)
		{
			AddToBuff(&pBuff,Tempdata.m_ID);
			AddToBuff(&pBuff,Tempdata.m_level);
			AddToBuff(&pBuff,Tempdata.reginid);
			AddToBuff(&pBuff,Tempdata.wX);
			AddToBuff(&pBuff,Tempdata.wY);
			AddToBuff(&pBuff,Tempdata.wHour);
			AddToBuff(&pBuff,Tempdata.wMinute);
			AddToBuff(&pBuff,Tempdata.wState);
			memcpy(pBuff,Tempdata.m_KillMyName,MAX_NAMELEN);
			pBuff += MAX_NAMELEN;
			Updatenum ++;
		}
		bossiter ++;
	}
	if ( !Updatenum)return;

	int Buffsize = (int)(pBuff - DeadUpdatemsg.Buff) + sizeof(WORD);
	if (Buffsize > MAX_BOSSUPDATE_SIZE)
	{
		AddInfo("���͵Ĺ�������̫����");
		return;
	}

	sendDeadUpdatemsg.nBossState = timeGetTime();
	BYTE *psendDead = sendDeadUpdatemsg.Buff;
	memcpy(psendDead,&Updatenum,sizeof(WORD));
	psendDead += sizeof(WORD);

	memcpy(psendDead,DeadUpdatemsg.Buff,MAX_BOSSUPDATE_SIZE-sizeof(WORD));

	sendDeadUpdatemsg.bType = SABossDeadUpdatemsg::UPDATE_BOSSMSG;
	WORD msgsize = sizeof(SABossDeadUpdatemsg) - (MAX_BOSSUPDATE_SIZE - Buffsize);
	g_StoreMessage(pPlayer->m_ClientIndex,&sendDeadUpdatemsg,msgsize);
}

void CGameWorld::AddToBuff( BYTE **pBuff,WORD datavalue )
{
	memcpy(*pBuff,&datavalue,sizeof(WORD));
	*pBuff += sizeof(WORD);
}

void CGameWorld::LoadActivityNotice()
{
	m_activityTable.clear();
	g_Script.CallFunc("LoadActivityNotice");
}

void CGameWorld::SetActivityNotice( ActivityNotice &aNotice )
{
	m_activityTable.push_back(aNotice);
	m_activityUpdateTime = timeGetTime();
}

void CGameWorld::RecvActivityNotice( SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer )
{
	if (pmsg->bType != SQBossDeadUpdatemsg::UPDATE_ACTIVITYMSG)return;
	if (pmsg->nBossState >= m_activityUpdateTime)return;

	std::vector<ActivityNotice>::iterator bossiter = m_activityTable.begin();

	SABossDeadUpdatemsg ActivityNoticemsg ;
	BYTE *pBuff = ActivityNoticemsg.Buff;
	memset(pBuff,0,MAX_BOSSUPDATE_SIZE);

	WORD Updatenum = m_activityTable.size();
	if (!Updatenum)return;

	memcpy(pBuff,&Updatenum,sizeof(WORD));
	pBuff += sizeof(WORD);
	
	ActivityNoticemsg.nBossState = timeGetTime();

	while(bossiter != m_activityTable.end())
	{
		ActivityNotice acnotice = *bossiter;
		memcpy(pBuff,&acnotice,sizeof(ActivityNotice));
		pBuff += sizeof(ActivityNotice);
		bossiter ++;
	}

	int Buffsize = (int)(pBuff - ActivityNoticemsg.Buff) + sizeof(WORD);
	if (Buffsize > MAX_BOSSUPDATE_SIZE)
	{
		AddInfo("���͵Ĺ�������̫����");
		return;
	}

	ActivityNoticemsg.bType = SABossDeadUpdatemsg::UPDATE_ACTIVITYMSG;
	WORD msgsize = sizeof(SABossDeadUpdatemsg) - (MAX_BOSSUPDATE_SIZE - Buffsize);
	g_StoreMessage(pPlayer->m_ClientIndex,&ActivityNoticemsg,msgsize);
	
}
bool CGameWorld::LoadBlessOpenClose()
{
	bool Flag = false;
	g_Script.CallFunc("GetBlessOpenClose");
	if (g_Script.GetRet(0).GetType() == CScriptValue::VT_NIL)
	{
		rfalse("����GetBlessOpenCloseʧ��");
		return Flag;
	}
	DWORD k = (DWORD)g_Script.GetRet(0);
	if (k != m_BlessOpenClose)
	{
		Flag = true;
	}
	m_BlessOpenClose = k;
	return Flag;
}
void CGameWorld::SendBlessOpenClose()
{
	SABLESSOpenClose pMsgAll;
	pMsgAll.Flag = GetGW()->m_BlessOpenClose;
	extern void BroadcastBlessOpenClose(SMessage *pMsgAll);
	BroadcastBlessOpenClose(&pMsgAll);
}

BOOL CGameWorld::DispatchSceneMessage( DNID dnidClient, struct SSceneMsg *pMsg, CPlayer *pPlayer )
{
	if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
		return FALSE;

	return CSceneMap::GetInstance().ProcessSceneMsg(dnidClient,pMsg,pPlayer);
}

BOOL CGameWorld::DispatchTransformersMessage(DNID dnidClient, struct STransformersMsgs *pMsg, CPlayer *pPlayer)
{
	if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
		return FALSE;
	return TRUE;
//	return pPlayer->ProcessTransformersMsg(pMsg);
}

void CGameWorld::syneffects(DWORD dwgid, WORD weffectsindex, WORD weffectsPosX, WORD weffectsPosY)
{
// 	CActiveObject *pobject = (CActiveObject*)GetObjectByGID(dwgid)->DynamicCast(IID_ACTIVEOBJECT);
// 	if (pobject)
// 	{
// 		pobject->syneffects(weffectsindex,weffectsPosX,weffectsPosY);
// 	}
}

BOOL CGameWorld::DispatchDyArenaMessage(DNID dnidClient, struct SArenaMsg *pMsg, CPlayer *pPlayer)
{
	if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
		return FALSE;

	return CArenaMap::GetInstance().ProcessDyArenaMsg(dnidClient, pMsg, pPlayer);
}

BOOL CGameWorld::DispatchDyArenaMessage(DNID dnidClient, SSGArenaMsg *pMsg, CPlayer *pPlayer)
{
	if (dnidClient == 0 || pMsg == NULL || pPlayer == NULL)
		return FALSE;

	CArenaMoudel::getSingleton()->OnDispatchArenaMsg(dnidClient, pMsg, pPlayer);
	return true;
}

BOOL CGameWorld::SendMsgToLoginSrv(struct SMessage *pMsg, int iSize)
{
	return GetApp()->m_LoginServer.SendMsgToLoginSrv(pMsg, iSize); 
}
