#include "StdAfx.h"

#include <IniFile.h>
#include <fstream>
#include "LITESERIALIZER/lite.h"

#include "Globals.h"
#include "pub/GBK_VisibleTest.h"
#include "DMainApp.h"
#include "DBCenterBaseModule.h"


#include "NetWorkModule/logmsgs.h"
#include "NetWorkModule/datamsgs.h"
#include "NetWorkModule/accountmsgs.h"
#include "NetWorkModule/refreshmsgs.h"
#include "networkmodule/MailMsg.h"
#include "NetWorkModule/BuildingTypedef.h"
#include "directnet/lzw.hpp"

extern size_t max_player_limit;

int authorityFlag = 0;
int clusterMax    = 0;
int clusterId     = 0;
BOOL netLimited   = 0;
BOOL yloginlimit  = 0; // Ϊ1ʱ�οͲ��ܵ�¼��

static __int32  serverid            = 0;
static __int32  chargemode          = 0;

static char     szName              [64]    = "";
static char     szAccountSrvIP      [32]    = "";
static char     szAccountSrvPort    [32]    = "";
static char     szDBServerIP        [32]    = "";
static char     szDBServerPort      [32]    = "";
static char     szDBCenterIP        [32]    = "";
static char     szDBCenterPort      [32]    = "";
static char     szGMServerPORT		[32] = "";
static char     szCollectPORT			[32] = "";

//static char     szPlayerPort        [32]    = "Player4c";
//static char     szServerPort        [32]    = "Server4c";
static char     szPlayerPort[32] = "";
static char     szServerPort[32] = "";

static char     m_szCenterIP[32] = "";
static char     m_szCenterPORT[32] = "";
static char     m_szServerAccount[32] = "";
static char     m_szServerIP[32] = "";
DWORD	m_dServerPORT;

std::string     upversion;

DWORD spanUerId = 0;
static BOOL LoadConfigInfo()
{
    IniFile IniFile;
    if ( !IniFile.Load( "LoginServer.ini" ) ) 
        return rfalse( 0, 0, "�޷��������ļ�LoginServer.ini" );

    IniFile.GetInteger( "ENVIRONMENT",   "MaxClients", MAX_CLIENT_NUMBER, (int*)&max_player_limit );
    IniFile.GetInteger( "ENVIRONMENT",   "CHARGEMODE", 0,                 (int*)&chargemode       );
    IniFile.GetInteger( "INFO",          "ID",         1,                 (int*)&serverid         );
    IniFile.GetInteger( "AUTHORITIES",   "FLAGS",      0,                 (int*)&authorityFlag    );
    IniFile.GetInteger( "CLUSTER",       "MAXIMUM",    0,                 (int*)&clusterMax       );
    IniFile.GetInteger( "CLUSTER",       "ID",         0,                 (int*)&clusterId        );
    IniFile.GetInteger( "ENVIRONMENT",   "NETLIMITED", 0,                 (int*)&netLimited       );
    IniFile.GetInteger( "ENVIRONMENT",   "YLOGINLIMIT",1,                 (int*)&yloginlimit      );
    rfalse( 2, 1, "[��ʾ]����������״̬��%s��", netLimited ? "�ѿ���" : "δ����" );

    if ( clusterMax != 0 )
    {
        if ( clusterMax > 8 )                                     return rfalse( 0, 0, "���ݿ�Ⱥ���������ֵ����8"    );
        if ( clusterId >= clusterMax )                            return rfalse( 0, 0, "���ݿ�Ⱥ�������ų������ֵ" );
        if ( ( clusterMax & ~( clusterMax - 1 ) ) != clusterMax ) return rfalse( 0, 0, "���ݿ�Ⱥ�������Ų���ƽ����" );
        rfalse( 2, 1, "[��ʾ]�÷��������趨Ϊ���ݿ�һ�϶�Ⱥ�����飡[%d/%d]", clusterId, clusterMax );
    }
    else
    {
        rfalse( 2, 1, "[��ʾ]�÷�������δ�������ݿ�һ�϶�Ⱥ���У�" );
    }

    IniFile.GetString(  "INFO",          "GroupName", "", szName,           64 );
    IniFile.GetString(  "ACCOUNTSERVER", "IP",        "", szAccountSrvIP,   32 );
    IniFile.GetString(  "ACCOUNTSERVER", "PORT",      "", szAccountSrvPort, 32 );
    IniFile.GetString(  "DATABSESERVER", "IP",        "", szDBServerIP,     32 );
    IniFile.GetString(  "DATABSESERVER", "PORT",      "", szDBServerPort,   32 );
    IniFile.GetString(  "DATABSECENTER", "IP",        "", szDBCenterIP,     32 );
    IniFile.GetString(  "DATABSECENTER", "PORT",      "", szDBCenterPort,   32 );

	/*IniFile.GetString("SERVICE", "PlayerPORT", "Player4c", szPlayerPort, 32);
	IniFile.GetString("SERVICE", "ServerPORT", "Server4c", szServerPort, 32);*/
	IniFile.GetString("SERVICE", "PlayerPORT", "", szPlayerPort, 32);
	IniFile.GetString("SERVICE", "ServerPORT", "", szServerPort, 32);
	IniFile.GetString("CenterServer", "IP", "", m_szCenterIP, 32);
	IniFile.GetString("CenterServer", "PORT", "", m_szCenterPORT, 32);

	IniFile.GetString("SERVER", "ACCOUNT", "", m_szServerAccount, 64);
	IniFile.GetString("SERVER", "IP", "", m_szServerIP, 32); 
	IniFile.GetInteger("SERVER", "PORT", -1, (int*)&m_dServerPORT);

	IniFile.GetString("SERVICE", "GMServerPORT", "", szGMServerPORT, 32);
	IniFile.GetString("SERVICE", "CollectPORT", "", szCollectPORT, 32);
    char upSpecific[64];
    IniFile.GetString(  "ENVIRONMENT", "UpdateVersion", "null", upSpecific, 32 );
    upversion = upSpecific;
    BOOL notSpecific = ( upversion.empty() || upversion == "null" || upversion == "download" );
    rfalse( 2, 1, "[��ʾ]��ǰ���°汾·��Ϊ : %s ������", notSpecific ? "Ĭ��·��" : upversion.c_str() );

    // ������ʹ��ID
    IniFile.GetInteger(  "SPANSERVER", "ID", 0, (int*)&spanUerId );

    IniFile.Clear();

    // ����������ˢ����Ҫ�����ӵ����ӵ㣨�ھɵ�������Ч������£�
    if ( !GetApp()->m_dbCenterClient.client.IsConnected() ) 
    {
        GetApp()->m_dbCenterClient.currentIP = szDBCenterIP;
        GetApp()->m_dbCenterClient.currentPort = szDBCenterPort;
    }

	char buffer[256];
	_snprintf(buffer, sizeof buffer, "LoginServer_%d", serverid % 1000);
	extern HWND hMainWnd;
	::SetWindowText(hMainWnd, buffer);

    return TRUE;
}

static Globals::SBornPos bornPos[5];

static BOOL LoadBornPostion()
{
    IniFile ini;
    if ( !ini.Load( "create.ini" ) )
        return rfalse( 0, 0, "�޷��������ļ�create.ini" );

	/** �����������ɶ���
	enum SCHOOL_DEFINE
	{
		SD_SHAOLIN = 0,		// ����
		SD_EMEI,						// ����
		SD_WUDANG,				// �䵱
		SD_SHENGHUO,			// ʥ��
		SD_XINGXIU,				// ����

		SD_MAX,
	}; */

    static LPCSTR name[5] = { "����", "��ɽ", "����", "�Ⱥ�", "����" };

    for ( int i=0; i<5; i++ )
    {
        ini.GetInteger( name[i], "BRID", 1,     (int*)&bornPos[i].wRegionID );
        ini.GetInteger( name[i], "BX",   1 + i, (int*)&bornPos[i].wPosX     );
        ini.GetInteger( name[i], "BY",   1 + i, (int*)&bornPos[i].wPosY     );
		ini.GetInteger( name[i], "DIR", 0, (int*)&bornPos[i].byDir );
		ini.GetInteger( name[i], "RADIUS",   1 + i, (int*)&bornPos[i].wRadius);		
		if (bornPos[i].wRadius <= 0) bornPos[i].wRadius = 1;
    }

    return TRUE;
}

//SScoreTable s_Scores;
//
//static void OperateScoreFile( BOOL bOperate, const char *filename, SScoreTable::SScoreRecode *arrayScore, DWORD dwMaxRecode )
//{
//    if (bOperate) 
//    {
//        std::ofstream fout;
//        fout.open(filename);
//
//        for(DWORD i = 0; i < dwMaxRecode; i++) 
//        {
//            if (arrayScore[i].Name[0] != 0)
//				fout << arrayScore[i].Name << std::endl << arrayScore[i].dwValue << std::endl;
//        }
//        
//        fout.close();
//    }
//    else 
//    {
//        std::ifstream fin(filename);
//        if (!fin.is_open()) 
//        {
//            memset(arrayScore, 0, sizeof(SScoreTable::SScoreRecode) * dwMaxRecode);
//            return;
//        }
//        //std::strstream::
//
//        DWORD dwNum = 0;
//        char szBuf[128] = {0};
//        while(dwNum != (dwMaxRecode - 1)) 
//        {
//            if (fin.eof()) 
//                break;
//
//            fin >> szBuf >> arrayScore[dwNum].dwValue;
//            dwt::strcpy(arrayScore[dwNum].Name, szBuf, CONST_USERNAME);
//            dwNum++;
//        }
//
//        fin.close();
//    }
//}
//
//static BOOL LoadScores()
//{
//    memset(&s_Scores, 0, sizeof(s_Scores));
//
//    // �ӱ����ļ���߻�ȡ��������
//    OperateScoreFile( 0, "taxis_Alevel.txt", s_Scores.ALevel,  MAX_SCORE_NUMBER );
//    OperateScoreFile( 0, "taxis_Amoney.txt", s_Scores.AMoney,  MAX_SCORE_NUMBER );
//    OperateScoreFile( 0, "taxis_Fame1.txt",  s_Scores.FAME[0], MAX_SCORE_NUMBER );
//    OperateScoreFile( 0, "taxis_Fame2.txt",  s_Scores.FAME[1], MAX_SCORE_NUMBER );
//	OperateScoreFile( 0, "taxis_MedicalEthics.txt",  s_Scores.MedicalEthics,   MAX_SCORE_NUMBER );
//
//    char szBuf[256] = { 0 };
//    for(int i = 0; i < MAX_SCHOOL_NUMBER; i++) 
//    {
//        sprintf(szBuf, "taxis_Slevel%d.txt",i);
//        OperateScoreFile(0, szBuf,  s_Scores.Level[i], MAX_SCORE_NUMBER);
//    }
//
//    return TRUE;
//}

static dwt::cStringMatchDict mdict;

static int LoadMaskMap()
{
    dwt::ifstream stream;
    stream.open("maskfile.h");
    if (!stream.is_open())
        return 1;

    char str[128];
    str[10] = 0;
    while (!stream.eof())
    {
        stream >> str;
        if (str[10] != 0)
            return 0;

        mdict.SetKeyWord(str, dwt::strlen(str, 128));
    }

    return 1;
}

static std::map< DWORD, DBCenterBaseModule* > dbcMap;  // ר���������ݲɼ���ͳ�ơ�ͳһ������������ݿ��б�

void Globals::DbcExec()
{
    if ( dbcMap.empty() )
        return;

    for ( std::map< DWORD, DBCenterBaseModule* >::iterator it = dbcMap.begin(); it != dbcMap.end(); it ++ )
        it->second->Execution();
}

void Globals::DbcDestroy()
{
    if ( dbcMap.empty() )
        return;

    for ( std::map< DWORD, DBCenterBaseModule* >::iterator it = dbcMap.begin(); it != dbcMap.end(); it ++ )
    {
        it->second->Destroy();
        delete it->second;
    }

    dbcMap.clear();
}

void Globals::DbcInfo()
{
    if ( dbcMap.empty() )
        return;

    for ( std::map< DWORD, DBCenterBaseModule* >::iterator it = dbcMap.begin(); it != dbcMap.end(); it ++ )
    {
        rfalse( 2, 1, "�������ݿ������EX[%d][%s:%s][%s]", it->second->dbcId,
            it->second->currentIP.c_str(), it->second->currentPort.c_str(), 
            it->second->client.IsConnected() ? "����" : "û����" );
    }
}

BOOL Globals::LoadDbcList( BOOL reset )
{
    std::ifstream stream( "dbcList.ini" );
    if ( !stream.is_open() )
        return rfalse( 2, 1, "�޷����������ݿ������б��ļ�dbcList.ini" );

    if ( reset )
        DbcDestroy();

    while ( !stream.eof() )
    {
        DWORD dbcId;
        std::string ip, port;

        stream >> dbcId >> ip >> port;
        if ( ip.empty() || port.empty() )
            break;

        if ( dbcId < 100 || dbcId > 255 )
        {
            rfalse( 2, 1, "�����˴�����������ݿ�����[%d][%s:%s]", dbcId, ip.c_str(), port.c_str() );
            break;
        }

        if ( dbcMap.find( dbcId ) != dbcMap.end() )
            continue;

        DBCenterBaseModule *dbc = new DBCenterBaseModule( dbcId );
        dbcMap[ dbcId ] = dbc;

        // ��û��������ʱҲ��Ҫ���ָ�dbc����Ч�ԣ�Ԥ���ڽ�����ĳһʱ�̳ɹ��������ӣ�
        if ( !dbc->Connect( ip.c_str(), port.c_str() ) )
            rfalse( 2, 1, "�������ݿ������EX[%d][%s:%s][û����]", dbcId, ip.c_str(), port.c_str() );
    }

    stream.close();

    return TRUE;
}

BOOL Globals::LoadConfigure()
{
    g_SetRootPath(NULL);
    g_SetFilePath("\\");

    static PakList pak;
    pak.AddPakFile( "data.dat" );

    if ( !LoadConfigInfo() )
        return rfalse( 0, 0, "LoadConfigInfo fail" );

    if ( !LoadBornPostion() )
        return rfalse( 0, 0, "LoadBornPostion fail" );

    /*if ( !LoadScores() )
        return rfalse( 0, 0, "LoadScores fail" );*/

    if ( !LoadMaskMap() )
        return rfalse( 0, 0, "LoadMaskMap fail" );

    return TRUE;
}

//static BOOL SaveScores()
//{
//    // �˳��������а��ļ�
//    OperateScoreFile( 1, "taxis_Alevel.txt", s_Scores.ALevel,  MAX_SCORE_NUMBER );
//    OperateScoreFile( 1, "taxis_Amoney.txt", s_Scores.AMoney,  MAX_SCORE_NUMBER );
//    OperateScoreFile( 1, "taxis_Fame1.txt",  s_Scores.FAME[0], MAX_SCORE_NUMBER );
//    OperateScoreFile( 1, "taxis_Fame2.txt",  s_Scores.FAME[1], MAX_SCORE_NUMBER );
//	OperateScoreFile( 1, "taxis_MedicalEthics.txt",  s_Scores.MedicalEthics,   MAX_SCORE_NUMBER );
//
//    char szBuf[256] = { 0 };
//    for(int i = 0; i < MAX_SCHOOL_NUMBER; i++) 
//    {
//        sprintf(szBuf, "taxis_Slevel%d.txt",i);
//        OperateScoreFile(1, szBuf,  s_Scores.Level[i], MAX_SCORE_NUMBER);
//    }
//
//    return TRUE;
//}

BOOL Globals::SaveConfigure()
{
    //return SaveScores();

    return TRUE;
}

BOOL Globals::GetChargeMode()
{
    return chargemode;
}

void Globals::GetDatabaseIPPORT( LPCSTR &ip, LPCSTR &port )
{
    ip = szDBServerIP;
    port = szDBServerPort;
}

void Globals::GetDBCenterIPPORT( LPCSTR &ip, LPCSTR &port )
{
    ip = szDBCenterIP;
    port = szDBCenterPort;
}

void Globals::GetAccountIPPORT( LPCSTR &ip, LPCSTR &port )
{
    ip = szAccountSrvIP;
    port = szAccountSrvPort;
}

void Globals::GetGameServerPort( LPCSTR &port )
{
    port = szServerPort;
}

void Globals::GetPlayerServerPort( LPCSTR &port )
{
    port = szPlayerPort;
}

void Globals::GetGMServerPort(LPCSTR &port)
{
	port = szGMServerPORT;
}

void Globals::GetCollectPort(LPCSTR &port)
{
	port = szCollectPORT;
}

BOOL Globals::SetPlayerLinkContext( DNID dnid, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, int gmlevel, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid )
{
    return GetApp()->m_PlayerManager.RegisterContext( dnid, account, password, userpass, limitedState, gmlevel, idkey, online, offline, puid );
}

BOOL Globals::ActivateLinkContextWithRebind( DNID dnid )
{
    return GetApp()->m_PlayerManager.ActivateLinkContextWithRebind( dnid );
}

BOOL Globals::SetPlayerLinkForceTimeOutTime( DNID dnid, DWORD dwTime )
{
    return GetApp()->m_PlayerManager.server.SetLinkForceTimeOutTime( dnid, dwTime );
}

BOOL Globals::SetPlayerLinkStatus( DNID dnid, int state )
{
    return GetApp()->m_PlayerManager.server.SetLinkStatus( dnid, (LINKSTATUS)state );
}

BOOL Globals::CutPlayerLink( DNID dnid )
{
    return GetApp()->m_PlayerManager.server.DelOneClient( dnid );
}

BOOL Globals::IsValidPlayerDnid( DNID dnid )
{
    return GetApp()->m_PlayerManager.server.IsValidDnid( dnid );
}

//BOOL Globals::SendToAccountServer( void *msg, size_t size )
//{
//    if ( size &0xffff0000 )
//        return FALSE;
//
//    return GetApp()->m_AccountClient.client.SendMessage( msg, (WORD)size );
//}

// �� server == -1 ʱ˵��������˺ŷ���������Ϣ��Ҫ�㲥
BOOL Globals::SendToAccountServer( void *msg, size_t size, LPCSTR server )
{
	if ( server == reinterpret_cast< LPCSTR >( -1 ) )
		return GetApp()->m_AccountModuleManage.DispatchAll( msg, size);

	return GetApp()->m_AccountModuleManage.Dispatch( msg, size, server );
}

BOOL Globals::SendToDBCenter( void *msg, size_t size )
{
    if ( size &0xffff0000 )
        return FALSE;

    return GetApp()->m_dbCenterClient.client.SendMessage( msg, (WORD)size );
}


BOOL Globals::SendToDBCenterEx( DWORD dbcId, void *msg, size_t size )
{
    if ( size &0xffff0000 )
        return FALSE;

    std::map< DWORD, DBCenterBaseModule* >::iterator it = dbcMap.find( dbcId );
    if ( it == dbcMap.end() )
        return FALSE;

    return it->second->client.SendMessage( msg, (WORD)size );
}

BOOL Globals::SendToDBServer( void *msg, size_t size )
{
    if ( size &0xffff0000 )
        return FALSE;

    return GetApp()->m_DatabaseClient.client.SendMessage( msg, (WORD)size );
}

BOOL Globals::SendToGameServer( DNID dnid, LPCVOID msg, size_t size )
{
    if ( size &0xffff0000 )
        return FALSE;

    return GetApp()->m_ServerManager.server.SendMessage( dnid, msg, (WORD)size );
}

BOOL Globals::SendToPlayer( DNID dnid, void *msg, size_t size )
{
    if ( size &0xffff0000 )
        return FALSE;

    return GetApp()->m_PlayerManager.server.SendMessage( dnid, msg, (WORD)size );
}

BOOL Globals::Reconnect()
{
    if ( !GetApp()->m_AccountModuleManage.Reconnect() )
    {
        rfalse(2, 1, "�ʺŷ��������������ϰ�������-____-bb");        
    }
    else
    {
        rfalse(2, 1, "�ʺŷ��������Ͽ�-____-bb");
    }

    if ( !GetApp()->m_DatabaseClient.client.IsConnected() )
    {
        if ( !GetApp()->m_DatabaseClient.Connect( szDBServerIP, szDBServerPort ) )
        {
            rfalse(2, 1, "���ݿ�����������ϰ�������-____-bb");
        }
        else
        {
            rfalse(2, 1, "���ݿ�������������Ͽ�^^");
        }
    }

    if ( !GetApp()->m_dbCenterClient.client.IsConnected() )
    {
        if ( !GetApp()->m_dbCenterClient.Connect( 
            GetApp()->m_dbCenterClient.currentIP.c_str(), 
            GetApp()->m_dbCenterClient.currentPort.c_str() ) )
        {
            rfalse(2, 1, "�������ݿ�����ʧ��");
        }
        else
        {
            rfalse(2, 1, "�������ݿ������ɹ�");
        }
    }

    return TRUE;
}

DWORD Globals::GetServerId()
{
    return serverid;
}

DWORD Globals::GetGuid(bool bIsPlayer)
{
    static DWORD s_CurGlobalID[2] = {1000, 1000};		// ȫ�ֱ�ʶ��
    if (bIsPlayer)
        return 0x00000000 | (0x0fffffff&(s_CurGlobalID[0]++));

    return 0xf0000000 | (0x0fffffff&(s_CurGlobalID[1]++));
}

Globals::SBornPos *Globals::GetBornPos( int index )
{
    if ( index < 0 || index > 4 )
        return NULL;

    return &bornPos[index];
}

BOOL Globals::IsValidName( LPCSTR szName )
{
    if (dwt::IsBadStringPtr(szName, CONST_USERNAME))
        return false;

    int len = dwt::strlen(szName, CONST_USERNAME);

    for (int i=0; i<len; i++)
    {   // �ո�֮ǰ�Ķ�����
        if (((BYTE)szName[i]) <= 32)
            return FALSE;
		//����ע���а���#�ţ���Ϊ�����������#�����ֵ�
		if (((BYTE)szName[i]) == '#')
			return FALSE;
        // �����е����ţ����ݿ�������������
        if (((BYTE)szName[i]) == '\'')
            return FALSE;

        // �����е����ţ����ݿ�������������
        if (((BYTE)szName[i]) == '?')
            return FALSE;

        if (((BYTE)szName[i]) == '%')
            return FALSE;

        if (((BYTE)szName[i]) == ':')
            return FALSE;

        if (((BYTE)szName[i]) == '\\')
            return FALSE;

        if (((BYTE)szName[i]) == '/')
            return FALSE;

//         if (mdict.Contain(&szName[i], len-i))
//             return FALSE;
        
        // ˫�ֽ��ַ����ؼ���2���ֽ�
        if (((BYTE)szName[i]) >= 128) 
        {
            //// �Ƿ�Ϊ���ɼ��ַ�
            //if (!isVisibleCharacterW(*(wchar_t*)&szName[i]))
            //    return FALSE;

            i++;

            //// ������Ҫ���ڶ����ַ��Ƿ񱻽ضϣ���Ȼ�����ģ�
            //if (szName[i] == 0)
            //    return FALSE;

			//update by ly 2014/7/17 �ͻ��˴��ݵ�Ϊutf8�����ַ���������ΪӢ���ַ�ʱ��ÿ���ֽڶ������128�����������������ֽ�
			continue;
        }
        else
        {
            // �Ƿ�Ϊ���ɼ��ַ�
            if (!isVisibleCharacterA((BYTE)szName[i]))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL Globals::IsValidSqlString( LPCSTR szString, size_t size )
{
    if ( dwt::IsBadStringPtr( szString, ( DWORD )size ) )
        return false;

    int len = dwt::strlen( szString, ( DWORD )size );

    for (int i=0; i<len; i++)
    {
        //�ո�֮ǰ�ĳ���30��31�����Ķ�����
        if ( ( ( BYTE )szString[i] < 30 ) || ( ( BYTE )szString[i] == 32 ) )
            return FALSE;

        // ������' \ �ţ����ݿ�����
		if (((BYTE)szString[i]) == '\'' || ((BYTE)szString[i]) == '\\')
            return FALSE;

        // ������? %�ţ����ݿ�����
        if ( (((BYTE)szString[i]) == '?') || (((BYTE)szString[i]) == '%'))
            return FALSE;

        // ˫�ֽ��ַ����ؼ���2���ֽ�
        if (((BYTE)szString[i]) > 128) 
        {
            // �Ƿ�Ϊ���ɼ��ַ�
            if (!isVisibleCharacterW(*(wchar_t*)&szString[i]))
                return FALSE;

            i++;

            // ������Ҫ���ڶ����ַ��Ƿ񱻽ضϣ���Ȼ�����ģ�
            if (szString[i] == 0)
                return FALSE;
        }
        else
        {
            //���⴦�������з���Ϊ�˺�ǰ׺ // �Ƿ�Ϊ���ɼ��ַ�			
            if ( ((BYTE) szString[i] == 30 ) || ( (BYTE)szString[i] == 31 ) || (isVisibleCharacterA((BYTE)szString[i]) ) )
            {
            }
            else
                return FALSE;
        }
    }

    return TRUE;
}

LPCSTR Globals::GetStringTime()
{
	static char buffer[1024] = {0};

    SYSTEMTIME s;
    GetLocalTime(&s);

    _snprintf(buffer, 1023, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

void Globals::LogText( LPCSTR info, LPCSTR name )
{
    if ( name == NULL )
        name = "debug.txt";

	SYSTEMTIME s;
    char szLogTime[128];

    GetLocalTime(&s);

	sprintf(szLogTime, "[%d-%d-%d %d:%d:%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    HANDLE hFile = ::CreateFile(name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL );
    if ( hFile == INVALID_HANDLE_VALUE )
        return;

    DWORD sizeA = (DWORD)strlen(szLogTime);
    DWORD sizeB = (DWORD)strlen(info);

    SetFilePointer( hFile, 0, 0, FILE_END );
    WriteFile( hFile, szLogTime, sizeA, &sizeA, 0 );
    WriteFile( hFile, info, sizeB, &sizeB, 0 );
    WriteFile( hFile, "\r\n", 2, &sizeA, 0 );
    CloseHandle( hFile );
}

void Globals::TraceMemory( LPCVOID entry, int offset, unsigned int size, LPCSTR filename )
{
    if ( filename == NULL )
        filename = "DumpMemory.Bin";

    try
    {
        HANDLE hFile = ::CreateFile( filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return;

        DWORD dwSize = 0;
        SetFilePointer( hFile, 0, 0, FILE_END );
        WriteFile( hFile, "{[(Head)", sizeof("{[(Head)")-1, &dwSize, 0 );

        LPBYTE pEntry = ((LPBYTE)entry) + offset;
        if ( !IsBadReadPtr( pEntry, size ) )
        {
            WriteFile( hFile, pEntry, size, &dwSize, 0 );
        }
        else
        {
            WriteFile( hFile, "IsBadReadPtr", sizeof("IsBadReadPtr")-1, &dwSize, 0 );
        }

        WriteFile( hFile, "(Tail)]}", sizeof("(Tail)]}")-1, &dwSize, 0 );

		CloseHandle( hFile );
    }
    catch ( ... )
    {
    }
}

void Globals::UpdateServerStatus( eServerStatus ss, int iPlayerNumber, int iRealPlayerNumber, int iMaxPlayerNumber )
{
    static HANDLE hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, 
        PAGE_READWRITE, 0, 4, "Global\\FileMapping4ServerStatus" );
    if ( hMapFile == NULL ) 
        return;

    if ( !GetApp()->m_PlayerManager.m_bCanLogin )
        ss = SS_REJECT;

    LPVOID lpMapAddress = MapViewOfFile( hMapFile, FILE_MAP_WRITE, 0, 0, 0 );
    if ( lpMapAddress == NULL )
        return;

    *(LPDWORD)lpMapAddress = (ss << 24) | 
        ( ((iPlayerNumber >= iMaxPlayerNumber) ? 100 : (iPlayerNumber * 100 / iMaxPlayerNumber))&0x000000ff ) |
        ( ( iRealPlayerNumber << 8 ) & 0x00ffff00 );

    if ( !UnmapViewOfFile( lpMapAddress ) )
        return;

    // �ȴ����̽������Զ�ע����ǰ�����
    // CloseHandle(hMapFile);
}

BOOL Globals::PutIntoRegion( DNID dnidClient, LPCSTR account, DWORD gid, DWORD regionId, DWORD &serverKey )
{
	return GetApp()->m_ServerManager.PutIntoRegion(dnidClient, account, gid, 5, /*regionId,*/serverKey);
}

BOOL Globals::SetPlayerServerKey( LPCSTR account, DWORD ID )
{
    return GetApp()->m_PlayerManager.SetPlayerServerKey( account, ID );
}

//static void UpdateScores( SScoreTable::SScoreRecode &theScore, 
//    SScoreTable::SScoreRecode *arrayScore, DWORD dwMaxRecode, BYTE byOrder = 1, bool bMedicalEthics = false )
//{
//    if (arrayScore == NULL)
//        return;
//
//    if (dwMaxRecode == 0)
//        return;
//
//    for (DWORD i=0; i<dwMaxRecode; i++)
//    {
//        if (dwt::strcmp(arrayScore[i].Name, theScore.Name, sizeof(theScore.Name)) == 0)
//        {
//            memmove(&arrayScore[i], &arrayScore[i+1], (MAX_SCORE_NUMBER-i-1)*sizeof(SScoreTable::SScoreRecode));
//            arrayScore[MAX_SCORE_NUMBER - 1].dwValue = 0;
//            arrayScore[MAX_SCORE_NUMBER - 1].Name[0] = 0;
//            break;
//        }
//    }
//
//	BOOL bThing = 1;
//	
//	for (i=0; i<dwMaxRecode; i++)
//	{     
//		if (bMedicalEthics)
//		{
//			WORD warrayValue =  arrayScore[i].dwValue & 0xffff; //ҽ��ֵ
//			WORD wtheValue = theScore.dwValue & 0xffff;
//			if ( ( warrayValue == wtheValue ) && (theScore.dwValue >> 16) > 0 ) //ҽ��ֵ����жϵȼ�
//				bThing = ( (arrayScore[i].dwValue >> 16) < (theScore.dwValue >> 16));
//			else
//				bThing = (byOrder == 1) ? (warrayValue < wtheValue) : (warrayValue > wtheValue);
//		}
//		else
//			bThing = (byOrder == 1) ? ( arrayScore[i].dwValue < theScore.dwValue ) : ( arrayScore[i].dwValue > theScore.dwValue );
//
//		if (bThing)
//		{
//			memmove(&arrayScore[i+1], &arrayScore[i], (MAX_SCORE_NUMBER-i-1)*sizeof(SScoreTable::SScoreRecode));
//			arrayScore[i] = theScore;
//			break;
//		}
//	}
//}
//
//void Globals::UpdatePlayerScore( SFixProperty *p )
//{
//    SScoreTable::SScoreRecode theScore;
//    if ( p->m_bySchool >= 5 )
//        return;
//
//    if ( p->m_wGMLevel != 0 )
//        return;
//
//	dwt::strcpy(theScore.Name, p->m_szName, CONST_USERNAME);
//    theScore.Name[10] = 0;
//
//    theScore.dwValue = p->m_byLevel;	
//    UpdateScores(theScore, s_Scores.Level[p->m_bySchool], MAX_SCORE_NUMBER);
//    UpdateScores(theScore, s_Scores.ALevel, MAX_SCORE_NUMBER);
//
//    theScore.dwValue = p->m_sXValue;
//    UpdateScores(theScore, s_Scores.XValue[p->m_bySchool], MAX_SCORE_NUMBER);
//
//    UpdateScores(theScore, s_Scores.FAME[0], MAX_SCORE_NUMBER);
//    UpdateScores(theScore, s_Scores.FAME[1], MAX_SCORE_NUMBER, 0);
//
//    theScore.dwValue = p->m_dwMoney + p->m_dwStoreMoney;
//    UpdateScores(theScore, s_Scores.Money[p->m_bySchool], MAX_SCORE_NUMBER);
//    UpdateScores(theScore, s_Scores.AMoney, MAX_SCORE_NUMBER);
//
//	theScore.dwValue =  p->m_byLevel << 16 | (p->wMedicalEthics & 0xffff);
//	UpdateScores(theScore, s_Scores.MedicalEthics, MAX_SCORE_NUMBER, true, true);
//}

BOOL Globals::UpdatePlayerData( SARefreshPlayerMsg *pMsg )
{
    return GetApp()->m_PlayerManager.UpdatePlayerData( pMsg );
}

BOOL Globals::DoubleUpdatePlayerData( SDoubleSavePlayerMsg *pMsg )
{
    return GetApp()->m_PlayerManager.DoubleUpdatePlayerData( pMsg );
}

BOOL Globals::DoCheckRebindMsg( DNID dnid, SQCheckRebindMsg *pMsg )
{
    return GetApp()->m_PlayerManager.DoCheckRebind( dnid, pMsg );
}

//BOOL Globals::GetPlayerData( LPCSTR account, DWORD gid, SFixProperty *data, SPlayerTempData *tdata )
//{
//}

BOOL Globals::AssignPlayer(DNID dnid, SFixData *data)
{
    return GetApp()->m_PlayerManager.AssignPlayer( dnid, data );
}

BOOL Globals::RebindAccount( DNID dnid, LPCSTR account, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline )
{
    return GetApp()->m_PlayerManager.RebindAccount( dnid, account, limitedState, idkey, online, offline );
}

BOOL Globals::GetServerInfo( DWORD ID, DNID &dnid, DWORD &ip, DWORD &port )
{
    return GetApp()->m_ServerManager.GetServerInfo( ID, dnid, ip, port );
}

BOOL Globals::AccountExist( LPCSTR account )
{
    return GetApp()->m_PlayerManager.AccountExist( account );
}

BOOL Globals::GetDnidContext( DNID dnid, LPCSTR &account, LPCSTR &password, LPCSTR &userpass )
{
    return GetApp()->m_PlayerManager.GetDnidContext( dnid, account, password, userpass );
}

BOOL Globals::GetRegionInfo( DWORD ID, WORD &mapid, WORD &level )
{
    return GetApp()->m_ServerManager.GetRegionInfo( ID, mapid, level );
}

BOOL Globals::GetPlayerData(LPCSTR account, DWORD gid, SFixData *data, SPlayerTempData *tdata)
{
    return GetApp()->m_PlayerManager.GetPlayerData( account, gid, data, tdata );
}

void Globals::RecvGMLoginMsg( SAGMCheckAccountMsg *pMsg )
{
    GetApp()->m_NetGMModule.DoGMLogin( pMsg );
}

void Globals::OnRecvGMMsgFromGameServer( DNID dnidClient, SGMMMsg *pGMMMsg, size_t size )
{
    GetApp()->m_NetGMModule.OnRecvGMMsgFromGameServer( dnidClient, pGMMMsg, size );
}

namespace Globals
{
	void SendUpdateTimeLimit( SPlayer *player, int isLogout )
	{
		if ( player == NULL )
			return;

		try 
		{
            LPCSTR prefix = NULL;
            LPCSTR abs_account = Globals::SplitAccount( player->account.c_str(), prefix );

			SQUpdateTimeLimit msg;

			lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
			//slm( isLogout )( player->idkey.c_str() )( player->account.c_str() );
			slm( isLogout )( player->idkey.c_str() )( abs_account );
			Globals::SendToAccountServer( &msg, sizeof( msg ) - slm.EndEdition(), prefix );
		}
		catch ( lite::Xcpt& )
		{
		}
	}

	void SendUpdateTimeLimit( SQUpdateTimeLimit *pMsg )
	{
		try 
		{
			lite::Serialreader slr( pMsg->streamBuffer );
			LPCSTR account = slr();

			CDAppMain *&GetApp(void);
			SPlayer *player = GetApp()->m_PlayerManager.FindPlayer( account );
			SendUpdateTimeLimit( player, 0 );
		}
		catch ( lite::Xcpt& )
		{
		}
	}

    void UpdateTimeLimit( SQUpdateTimeLimit *data, size_t size )
    {
		try 
		{
			lite::Serialreader slr( data->streamBuffer );
			LPCSTR account = slr();
			DWORD online = slr();

			CDAppMain *&GetApp(void);
			SPlayer *player = GetApp()->m_PlayerManager.FindPlayer( account );
            if ( player != NULL )
                player->online = online;
		}
		catch ( lite::Xcpt& )
		{
		}

		SendToAllGameServer( data, size );
    }

	void GetCenterServerIPPORT(LPCSTR &ip, LPCSTR &port)
	{
		ip = m_szCenterIP;
		port = m_szCenterPORT;
	}

	void GetServerIPPORT(LPCSTR &ip, int &nport)
	{
		ip = m_szServerIP;
		nport = m_dServerPORT;
	}

	void GetServerAccount(LPCSTR &account)
	{
		account = m_szServerAccount;
	}

} // Global

void Globals::SendToAllGameServer(const void *msg, size_t size)
{
	GetApp()->m_ServerManager.Broadcast( msg, size );
}

void Globals::OnRecvMailMsg(SMailBaseMsg *pMsg)
{

	switch (pMsg->_protocol)
	{
	case SMailBaseMsg::EPRO_MAIL_SEND:
		{
			SQMailSendMsg *pQSendMsg = (SQMailSendMsg*)pMsg;

			SendToDBServer(pQSendMsg,sizeof(SQMailSendMsg));
		}
		break;
	case SMailBaseMsg::EPRO_MAIL_RECV:
		{
			SQMailRecvMsg *pQRecvMsg = (SQMailRecvMsg*)pMsg;

			SendToDBServer(pQRecvMsg,sizeof(SQMailRecvMsg));
		}
		break;
	case SMailBaseMsg::EPRO_MAIL_DELETE:
		{
			SQMailDeleteMsg *pDeleteMsg = (SQMailDeleteMsg*)pMsg;

			SendToDBServer(pDeleteMsg,sizeof(SQMailDeleteMsg));
		}
		break;
	case SMailBaseMsg::EPRO_MAIL_NEWMAIL:
		{
			SQNewMailMsg *pNewMailMsg = (SQNewMailMsg*)pMsg;

			SendToDBServer(pNewMailMsg,sizeof(SQNewMailMsg));
		}
		break;
	}
}

SCollect_LoginServerData *Globals::GetCollectData()
{
	return &GetApp()->m_DataCollect.m_Data;
}

//void Globals::ProcessAccountPrefix( std::string &account)
//{
//	int pos = account.find('\n');
//	if ( pos != string::npos )
//	{
//		account = account.substr(pos+1, account.size() );
//	}
//}

BOOL Globals::GetPlayData(DNID dnid)
{
	SQSelPlayerMsg selMsg;
	selMsg.byIndex = 1;
	return GetApp()->m_PlayerManager.SelectCharacter(dnid, &selMsg);
}

LPCSTR Globals::SplitAccount( LPCSTR account, LPCSTR &prefix )
{
    static char prefixBuffer[32];
    prefix = NULL;
    LPCSTR iter = account;
    // �����и�����Լ����ǰ׺���ܳ���16���ֽڣ�
    for ( int i = 0; i < 17; i ++, iter ++ ) 
    {
        if ( *iter < 0 )        // �Ƿ�Ϊ˫�ֽ��ַ���
            iter ++;

        if ( *iter == 0 )       // �Ƿ��Ѿ������ַ���ĩβ��
            break;

        if ( *iter == 30 )    // �ҵ��ָ����������ݲ���Χ��
        {
            memcpy( prefixBuffer, account, iter - account );
            prefixBuffer[ iter - account ] = 0;
            prefix = prefixBuffer;
            return iter + 1;
        }
    }

    // û�зָ����ֱ�ӷ���ԭʼ�ʺţ�
    return account;
}

BYTE tmpBuffer[ 0x100000 ];

static size_t LzwEncoding( const void * src_data, size_t src_size, void * dst_data, size_t dst_size )
{
    lzw::lite_lzwEncoder lencoder;
	lencoder.SetEncoderData( dst_data, dst_size );
    return lencoder.lzw_encode( ( void* )src_data, src_size ) ? lencoder.size() : 0;
}

static BOOL TryEncoding( const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size )
{
    // ���ݹ�С������ѹ������
    if ( src_size < 32 )
    {
        dst_size = src_size;
        dst_data = ( void* )src_data;
        return FALSE;
    }

	dst_size = LzwEncoding( src_data, src_size, ( char* )dst_data + 8, dst_size - 8 );
	if ( dst_size <= 0 || dst_size >= ( src_size - 16 ) )
    {
        // ѹ��ʧ�ܣ�����ѹ����������������󣩣�ֱ�ӷ���ԭʼ����
        dst_data = ( void* )src_data;
        dst_size = src_size;
        return FALSE;
	}

    // ѹ���ɹ������ѹ����ǣ�
    * reinterpret_cast< QWORD* >( dst_data ) = 0xefcdab9036587214;
    dst_size += 8;
    return TRUE;
}





BOOL TryEncoding_NilBuffer( const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size )
{
    assert( dst_data == NULL && dst_size == 0 );
    dst_data = tmpBuffer;
    dst_size = sizeof( tmpBuffer );
    return TryEncoding( src_data, src_size, dst_data, dst_size );
}

#if 0


// Get IP address of the player from dnid
BOOL g_GetAddrByDnid(DNID dnidClient, char *szIP , DWORD size)
{
	if (size < 17)
		return FALSE;

	if ( IsBadWritePtr( szIP, size ) )
		return FALSE;

	sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if(TRUE == GetApp()->m_PServer.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in))) // Get the IP in ULong format
    {
        char * pChar;
        pChar = inet_ntoa(addr.sin_addr); // convert to readable format
        if(pChar == NULL) 
        {
            return FALSE;
        }
        else
        {
            memcpy(szIP,inet_ntoa(addr.sin_addr), 17);// assgin to output value
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL CPlayer::DoCheckRebindMsg(DNID dnidClient, SQCheckRebindMsg* pMsg)
{
    if (pMsg == NULL)
        return FALSE;

    pMsg->key[ACCOUNTSIZE-1] = 0;

    extern bool g_check;
    if (g_check)
    {
        extern LPCSTR GetStringTime();
        rfalse(1, 1, "[%s] DoCheckRebindMsg [%s]!!!", GetStringTime(), pMsg->key);
    }

    SACheckRebindMsg msg;

    msg.dnidClient = pMsg->dnidClient;
    msg.gid = pMsg->gid;
	dwt::strcpy(msg.key, pMsg->key, ACCOUNTSIZE);
    //msg.key[16] = 0;

    msg.dwResult = 1;

    char key[ACCOUNTSIZE];
	dwt::strcpy(key, pMsg->key, ACCOUNTSIZE);
    //key[16] = 0;
    SPlayer *pPlayer = m_PlayerList.GetData(key);
    if (pPlayer == NULL)
    {
        // ����Ҳ����������˵����֤ʧ��
        msg.dwResult = 0;
    }
    else
    {
        if (pPlayer->dwGlobalID != pMsg->gid)
        {
            // �����ȫ��IDУ���������
            msg.dwResult = 0;
        }
        else
        {
            // ���ڶ�������ʱ�������ܿ��ܻ���֪���ͻ����Ѿ��Ͽ��ˣ�����û�н�byIsRebinding״̬�趨
            // ������ʱע�����״̬
            //else if (!pPlayer->byIsRebinding)
            //{
            //    // ����Ǹý�ɫ����û�н������°�״̬
            //    msg.dwResult = 0;
            //}

            SServer *pS = m_ServerList.GetData(pPlayer->dwRegionServerKey);
            if (pS == NULL)
            {
                // �Ҳ���Ŀ�곡��!!!
                msg.dwResult = 0;
            }
            else if (pS->idControl != dnidClient)
            {
                // ָ��Ŀ�곡����������ʵ��Ҫȥ�ĳ�����������ͬ��
                // ����ܿ����Ƕ�����������ɵ����⣡
                // ��ô������Ҫ�����ٴ��ض���
                msg.dwResult = 5147;
                msg.ip = pS->dwIP;
                msg.port = pS->dwPort;
                msg.wMapID = 0;
                msg.wRegionID = 0;
                msg.wCurX = 0;
                msg.wCurY = 0;

                // �����byIsRebinding��־��˵���Ѿ����������ݱ���ģ����Կ���ʹ�ó�������
                if (pPlayer->byIsRebinding)
                {
                    std::map<WORD, SRegion>::iterator it = pS->Regions.find(pPlayer->Data.m_CurRegionID);
                    if (it != pS->Regions.end())
                    {
                        msg.wMapID = it->second.MapID;
                        msg.wRegionID = pPlayer->Data.m_CurRegionID;
                        msg.wCurX = pPlayer->Data.m_wSegX;
                        msg.wCurY = pPlayer->Data.m_wSegY;
                    }
                }
            }
        }
    }

    if (g_StoreSMessage(dnidClient, &msg, sizeof(SACheckRebindMsg)) == 0)
    {
        rfalse(1, 1, "send SACheckRebindMsg fail!!!");
    }

    if (msg.dwResult == 1)
    {
        // �󶨳ɹ�
        pPlayer->byIsLostLink = false;
        pPlayer->byIsRebinding = false;

        if(pPlayer->dwNextCheckExtra != 0)
        {
            pPlayer->dwNextCheckTime = timeGetTime() + REFRESHTIME_ONLINE;
            pPlayer->dwNextCheckExtra = 0;
            pPlayer->dwRebindSegTime = 0;
        }

        //SAChangeRegionMsg msg2;
        //msg2.dnidClient = pMsg->dnidClient;             // ��������У����
        //g_StoreSMessage(dnidClient, &msg2, sizeof(SAChangeRegionMsg));
    }

    return (msg.dwResult == 1);
}


#endif