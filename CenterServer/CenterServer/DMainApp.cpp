#include "StdAfx.h"
#include "liteserializer/lite.h"

#include "engine.h"

#include "DMainApp.h"

#include "networkmodule/AccountMsgs.h"
#include "networkmodule/DbopMsgs.h"
#include "networkmodule/refreshMsgs.h"
//#include "dbserverex/dbserver/mysqldbprocess.h"
//#include "dbserverex/dbserver/netdbmoduleex.h"
//using namespace mysql;
//#include "pub/DBProcessor.h"
#include "pub/traceinfo.h"
#include "pub/dwt.h"
#include "tinyxml2.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

extern bool InitGameVersionInfo();
extern void AddInfo( LPCSTR Info );
extern LPCSTR GetStringTime();

static class CDAppMain *ptheApp = NULL;
static int nRPCThreadCount      = 5;
DWORD dwNetRunLimit     = 100;
boost::filesystem::ofstream g_ofstr("connect.txt", std::ofstream::app);
#define LogErrorMsg( str ) TraceInfoDirectly( "debugCenterServer.txt", str )

BOOL CreateApp()
{
    if ( ptheApp != NULL ) return FALSE;
    return ( ( ptheApp = new CDAppMain ) != NULL );
}

void ClearApp(void)
{
    if ( ptheApp != NULL ) delete ptheApp; 
    ptheApp = NULL;
}

CDAppMain &GetApp(void)
{
    if ( ptheApp == NULL ) _asm int 3;
    return *ptheApp;
}

BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize )
{
    //if ( wSize & 0xffff0000 )
    //    return false;

    //return GetApp().m_NetProviderModule.SendMessage( dnidClient, lpMsg, (WORD)wSize );
	return false;
}

//DWORD GetServerIDByDNID(DNID dnid)
//{
//    return GetApp().m_Servers.GetServerIDByDNID( dnid );
//}

void ShowVersion(void)
{
    char VerMsg[1024] = "";
    sprintf( VerMsg, "版本号\t：Build%04x\r\n", 1 );
	AddInfo( VerMsg );
}

//static const char stid[36+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

CDAppMain::CDAppMain(void) : m_bQuit( FALSE )
{
}

CDAppMain::~CDAppMain(void)
{
}

BOOL CDAppMain::InitServer()
{
    // 启动本地服务器。读取INI配置文件
    g_SetRootPath( NULL );
    g_SetFilePath( "\\" );

    IniFile IniFile;
    if ( !IniFile.Load( "CenterCfg.ini" ) )
        return MessageBox( 0, "Cannot open ini file", "CenterCfg.ini", 0 ), FALSE;

	LoadWhiteList();
    //char sDBIP[16];
    //char sDBAccount[32];
    //char sDBPassword[32];
    //char sDBDefault[32];
    //char sLocalListenPort[32];
    char sTitle[32];

	//IniFile.GetInteger( "SERVER",  "RPCTHREADCOUNT", 5,         &nRPCThreadCount     );
	  IniFile.GetString( "SERVER",   "TITLE",     "中心服务器", sTitle,      32 );
 //   IniFile.GetString( "SERVER",   "PORT",      "DBCenterPort", sLocalListenPort, 32 );
 //   IniFile.GetString( "DATABASE", "IP",        "127.0.0.1",    sDBIP,            16 );
 //   IniFile.GetString( "DATABASE", "ACCOUNT",   "DBCenter",     sDBAccount,       32 );
 //   IniFile.GetString( "DATABASE", "DEFAULTDB", "DBCenter",     sDBDefault,       32 );
 //   IniFile.GetString( "DATABASE", "PASSWORD",  "DBCenter",     sDBPassword,      32 );

	int _id = 0;
	char sCenterPort1[32] = "CenterToGameServer4c";
	char sCenterPort2[32] = "CenterToLoginServer4c";
	char sCenterPort3[32] = "CenterToDbServer4c";
	char sCenterPort4[32] = "CenterToClient4c";


	IniFile.GetInteger( "CenterBase" , "ID" , 2000 , &_id );
	IniFile.GetString( "CenterBase" , "PORT1" , "CenterToGameServer4c" , sCenterPort1 , 32 );
	IniFile.GetString( "CenterBase" , "PORT2" , "CenterToLoginServer4c" , sCenterPort2 , 32 );
	IniFile.GetString("CenterBase", "PORT3", "CenterToDbServer4c", sCenterPort3, 32);
	IniFile.GetString("CenterBase", "PORT4", "CenterToClient4c", sCenterPort4, 32);

    extern HWND hMainWin;
    SetWindowText( hMainWin, sTitle ); 

    IniFile.Clear();

	//创建主机
	BOOL b = m_ServerManager[GAMESERVER_FLAG].Host(sCenterPort1);
	if (!b)
	{
		return MessageBox(0, "创建游戏服管理器失败", "错误", 0), FALSE;
	}

	b = m_ServerManager[LOGINERVER_FLAG].Host(sCenterPort2);
	if (!b)
	{
		return MessageBox(0, "创建登陆服管理器失败", "错误", 0), FALSE;
	}

	b = m_ServerManager[DBSERVER_FLAG].Host(sCenterPort3);
	if (!b)
	{
		return MessageBox(0, "创建数据库服管理器失败", "错误", 0), FALSE;
	}


	b = m_PlayerManager.Host(sCenterPort4);
	if ( b )
	{
		AddInfo( "创建玩家管理器成功\r\n" );
	}
	else
	{
		return MessageBox( 0, "创建玩家管理器失败", "错误", 0 ), FALSE;
	}

	ShowVersion();
	AddInfo( "Server started\r\n" );
	if (InitGameVersionInfo() == false)
		return FALSE;
    return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::LoopServer()
{

	//m_ServerManager.Execution();
	for (size_t i = 0; i < MAX_MANAGE_SIZE; i++)
	{
		m_ServerManager[i].Execution();
	}
	m_PlayerManager.Execution();

    Sleep(1);

    return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::ExitServer()
{
	g_ofstr.close();
    return TRUE;
}

BOOL CDAppMain::bCheckWhiteListIn(std::string& account)
{
	if (m_uwhitelist.size() > 0 )
	{
		usetstring::iterator iterset = m_uwhitelist.find(account.c_str());
		if (iterset != m_uwhitelist.end())
		{
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

void CDAppMain::LoadWhiteList()
{
	m_uwhitelist.clear();
	char Buffer[1024] = {};
	boost::filesystem::ifstream ifstr("whitelist.txt");
	if (ifstr.is_open())
	{
		while (!ifstr.eof())
		{
			ifstr.getline(Buffer, 1024);
			if (0 == Buffer[0] || '//' == (*(LPWORD)Buffer))
				continue;

			std::pair<usetstring::iterator, bool> bresult;
			bresult = m_uwhitelist.insert(Buffer);
			if (!bresult.second)
			{
				char str[100] = {};
				sprintf_s(str, "insert accounts %s  Faile \n", Buffer);
				AddInfo(str);
			}

		}
		ifstr.close();
	}
	
}

void CDAppMain::SaveConnectNumString(const char *strConnect)
{

	if (g_ofstr.is_open())
	{
		g_ofstr << strConnect;
		g_ofstr.flush();
	}
}

LPCSTR GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

bool InitGameVersionInfo()
{
	std::string assetbundleInfo = "";
	//tinyxml2::XMLDocument doc;
	//if (doc.LoadFile("Announcement.xml") != tinyxml2::XMLError::XML_SUCCESS) //加载文件
	//	return false;

	/*ifstream file;
	file.open("Announcement.xml");
	
	while (!file.eof())
	{
		file.read(const_cast<char *>(assetbundleInfo.data()),1);
		
	}*/
	File fi; fi.Open("Announcement.xml");
	CDAppMain& appMain = GetApp();
	appMain.m_iAssetBundleInfoLength = fi.Size();
	fi.Read(appMain.m_AssetbundleVersionInfo, fi.Size());
	fi.Close();
	//while (!file.eof())
	//{
	//	file >> assetbundleInfo;
	//}
 //
	//memcpy(GetApp().m_AssetbundleVersionInfo, assetbundleInfo.c_str(), assetbundleInfo.size());
	//tinyxml2::XMLElement *root;
	//root = doc.RootElement();
	//tinyxml2::XMLElement *curNode;
	//curNode = root->FirstChildElement("Assetbundle"); //第一个资源包节点
	//assetbundleInfo.append(curNode->Attribute("ip"));//资源包的ip
	//assetbundleInfo.append(";");

	//tinyxml2::XMLElement *subNode = curNode->FirstChildElement("Asset"); //获取第一个资源节点
	//while (subNode != nullptr)
	//{
	//	//std::string name = subNode->Attribute("name");
	//	//int version = subNode->IntAttribute("version");
	//	//int size = subNode->IntAttribute("size");
	//	assetbundleInfo.append(subNode->Attribute("name"));
	//	assetbundleInfo.append(",");
	//	assetbundleInfo.append(subNode->Attribute("version"));
	//	assetbundleInfo.append(",");
	//	assetbundleInfo.append(subNode->Attribute("size"));
	//	assetbundleInfo.append(";");
	//	subNode = subNode->NextSiblingElement();
	//}

	//doc.Clear();
	//memset(GetApp().m_AssetbundleVersionInfo, 0, MAX_ASSET_SIZE);
	//memcpy(GetApp().m_AssetbundleVersionInfo, assetbundleInfo.c_str(), assetbundleInfo.size());
	//assetbundleInfo = "";
	//assetbundleInfo.append(GetApp().m_AssetbundleVersionInfo);



	return true;
}

LPSTR g_AnsiToUtf8(const char* AnsiStr)
{
	if (AnsiStr == NULL)
		return NULL;
	wchar_t* pBuf = NULL;
	int WriteByte = 0;
	WriteByte = ::MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, NULL, 0);
	pBuf = new wchar_t[WriteByte + 1];
	memset(pBuf, 0, (WriteByte + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, (LPWSTR)pBuf, WriteByte + 1);

	char* Utf8Str = NULL;
	int Utf8StrByte = 0;
	Utf8StrByte = ::WideCharToMultiByte(CP_UTF8, 0, pBuf, -1, NULL, 0, 0, 0);
	Utf8Str = new char[Utf8StrByte + 1];
	::WideCharToMultiByte(CP_UTF8, 0, pBuf, WriteByte + 1, Utf8Str, Utf8StrByte + 1, 0, 0);
	if (pBuf != NULL)
		delete[] pBuf;
	return Utf8Str;
}