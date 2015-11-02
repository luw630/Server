#include "StdAfx.h"
#include "DMainApp.h"
#include "networkmodule\logmsgs.h"
#include "networkmodule\datamsgs.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\CenterMsg.h"
#include "pub\rpcop.h"
#include "pub\ConstValue.h"
#include "GameObjects\Player.h"
#include "GameObjects\count.h"
#include "GameObjects\scriptmanager.h"
#include "GameObjects\sendmail.h"
#include "GameObjects\gaminghouse.h"
#include "GameObjects\globalfunctions.h"
#include "pub/GBK_VisibleTest.h"
#include "pub/ComplierConfigure.h"
#include "pub/traceinfo.h"
#include "DirectNet/lzw.hpp"
#include "networkmodule/PlugingameMsg.h"
#include "Environment.h"
#include "networkmodule/SectionMsgs.h"

#include <crtdbg.h>
#include <fstream>
#include <PakList.h>
#include <FilePath.h>
#include <boost/bind.hpp>
#include "GameObjects\TransportConf.h"
#include "GameObjects\SanguoCode\TimerEvent_SG\TimerEvent_SG.h"
#include <time.h>  
#include "GameObjects\SanguoCode\Common\ConfigManager.h"
#include "recharge.h"


#define GAME_FPS                5

char ORBCADDR[256] = "127.0.0.1";

#define easyState(r) (r==SS_NULL?"未连接":(r==SS_SUCCESS?"正常":(r==SS_BREAK?"连接断开":"认证中")))
DWORD g_StartUpZoneServerTime;
extern dwt::stringkey<char [256]> szDirname;
extern HWND hMainWnd;
extern void g_SetMainHWnd(HWND);
extern void SaveAndLogoutAllPlayer();
extern LPCSTR GetStringTime();
extern BOOL isLimitWG;
extern LPCSTR _GetStringTime();  

WORD CDAppMain::m_wServerID = 0;
int GROUP_SERVER_ID = 0;
extern DWORD g_dServerID;

// 用于在数据库服务器1拖多时，区别分组时用的编号数据
DWORD clusterMask = 0, clusterId = 0;

static DWORD dwPingInfoT = 0;
static DWORD dwPingTimesT = 0;
static DWORD dwPingInfo = 0;
static DWORD dwPingTimes = 0;
static int PingMAX = 0;
static int PingMIN = 123456789;
       DWORD dwAvePing = 0;

double Fps = 0.0;
double Fpc[100] ={0.0, };
DWORD frames = 0;

DWORD g_AccOnlineCheck[6] = { 0, 0, 0, 0, 0, 0 };
DWORD g_CurOnlineCheck[6] = { 0, 0, 0, 0, 0, 0 };

dwt::cStringMatchDict mdict;
PakList pak;

std::list<WORD> g_messagelist;  //消息队列

void    g_AddSpendMoney( __int64 iSpendMoney )
{
    GetApp()->AddSpendMoney( iSpendMoney );
}

void ShowVersion(void)
{
    char vermsg[20];
    sprintf(vermsg, "版本号：Build%04x", SERVERVERSION);
    rfalse(2, 1, vermsg);

	char playermsg[40];
	sprintf(playermsg, "玩家数据版本号：Build 0x%x", SFixPlayerDataBuf::GetVersion() );
	rfalse(2, 1, playermsg);
	char playermsg1[40];
	sprintf(playermsg1, "玩家数据大小：Build 0x%x", sizeof(SFixPlayerDataBuf));
	rfalse(2,1,playermsg1);
}

CDAppMain *&GetApp(void)
{
    static class CDAppMain *ptheApp = NULL;
    return ptheApp;
}

BOOL CreateApp(void)
{
    if (GetApp() == NULL)
    {
        g_SetRootPath(NULL);
        g_SetFilePath("");

        pak.AddPakFile("patch.dat");
        pak.AddPakFile("data.dat");

        return (GetApp() = new CDAppMain) != NULL;
    }
    return FALSE;
}

void g_EQVLog(LPCSTR info)
{
	TraceInfo(szDirname + "EQVInfo.log", "[%s]%s", GetStringTime(), info);
}

void g_Consuming(LPCSTR info)
{
	TraceInfo(szDirname + "Consuming.log", "%s\t%s", GetStringTime(), info);
}

void ClearApp(void)
{
    if (GetApp() != NULL)
        delete GetApp();
    GetApp() = NULL;
}

void ModifyNormalPlayerNumber(int iChange)
{
    GetApp()->m_dwNormalPlayerNumber += iChange;
}

void ModifyHangupPlayerNumber(int iChange)
{
    GetApp()->m_dwHangupPlayerNumber += iChange;
}

void ModifyLinkBreakPlayerNumber(int iChange)
{
    GetApp()->m_dwLinkBreakPlayerNumber += iChange;
}

int InitMaskMap()
{
//     dwt::ifstream stream;
//     stream.open("maskfile.h");
//     if (!stream.is_open())
//         return 1;
// 
//     char str[128];
//     str[10] = 0;
//     while (!stream.eof())
//     {
//         stream >> str;
//         if (str[10] != 0)
//             return 0;
// 
//         mdict.SetKeyWord(str, dwt::strlen(str, 128));
//     }

    return 1;
}

BOOL IsValidName(LPCSTR szName)
{
    int len = dwt::strlen(szName, CONST_USERNAME);
    if ( len == 0 )
        return FALSE;

    for (int i=0; i<len; i++)
    {
        if (((BYTE)szName[i]) <= 32)
            return FALSE;

        if (((BYTE)szName[i]) == '\'')
            return FALSE;

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

        if (mdict.Contain(&szName[i], len-i))
            return FALSE;
        
        // 双字节字符不必检测第2个字节
        if (((BYTE)szName[i]) > 128) 
        {
            // 是否为不可见字符
            if (!isVisibleCharacterW(*(wchar_t*)&szName[i]))
                return FALSE;

            i++;

            // 现在需要检测第二个字符是否被截断，不然会出错的！
            if (szName[i] == 0)
                return FALSE;
        }
        else
        {
            // 是否为不可见字符
            if (!isVisibleCharacterA((BYTE)szName[i]))
                return FALSE;
        }
    }

    return TRUE;
}

BYTE tempTransBuff[0xffff];

void SendMessageToPlayer( LPCSTR szName, SMessage *pMsg, DWORD wSize )
{
    if ( wSize > 0xffff )
        return;

	extern LPIObject GetPlayerByName(LPCSTR szName);

	LPIObject obj = GetPlayerByName( szName );
	CPlayer *player = ( CPlayer* )obj->DynamicCast(IID_PLAYER);
	if ( player )
		::g_StoreMessage( player->m_ClientIndex, pMsg, ( WORD )wSize );
}

int g_SendDirect(DNID dnidClient, LPVOID pPackage, DWORD wSize)
{
    if ( wSize > 0xffff )
        return 0;
    return GetApp()->m_Server.SendDirect(dnidClient, pPackage, ( WORD )wSize);
}

int g_StoreMessage(DNID dnidClient, LPVOID pMsg, DWORD wSize)
{
    if (wSize > 0xffff || 0 == dnidClient)
        return 0;

    return GetApp()->m_Server.SendMessage(dnidClient, (LPBYTE)pMsg, (WORD)wSize);
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

LPSTR g_Utf8ToAnsi(const char* Utf8Str)
{
	if (Utf8Str == NULL)
		return NULL;
	wchar_t* pBuf = NULL;
	int WriteByte = 0;
	WriteByte = ::MultiByteToWideChar(CP_UTF8, 0, Utf8Str, -1, NULL, 0);
	pBuf = new wchar_t[WriteByte + 1];
	memset(pBuf, 0, (WriteByte + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, Utf8Str, -1, (LPWSTR)pBuf, WriteByte + 1);

	char* AnsiStr = NULL;
	int AnsiStrByte = 0;
	AnsiStrByte = ::WideCharToMultiByte(CP_ACP, 0, pBuf, -1, NULL, 0, 0, 0);
	AnsiStr = new char[AnsiStrByte + 1];
	::WideCharToMultiByte(CP_ACP, 0, pBuf, WriteByte + 1, AnsiStr, AnsiStrByte + 1, 0, 0);
	if (pBuf != NULL)
		delete[] pBuf;
	return AnsiStr;
}

DWORD g_GetClientCount()
{
    return GetApp()->m_Server.GetClientNum();
}

void g_CutClient(DNID dnidClient)
{
	// 通知被关闭客户端禁止自动重连
	SACheckRebindMsg msg;
	msg.SMessage::_protocol = 100;
	msg.dwResult			= 1;
	msg.dnidClient			= 0;
	msg.gid					= 0;

    int esize = 0;
    try
    {
        esize = (int)lite::Serializer( msg.streamData, sizeof( msg.streamData ) )( "" ).EndEdition();
    }
    catch (lite::Xcpt &)
    {
        return;
    }
    
    g_StoreMessage(dnidClient, &msg, sizeof(msg) - esize);

    GetApp()->m_Server.DelOneClient(dnidClient);
}


void g_CutClient(DNID dnidClient, int cutFlag)
{
	// 通知被关闭客户端禁止自动重连
	SACheckRebindMsg msg;
	msg.SMessage::_protocol = 100;
	msg.dwResult = cutFlag;
	msg.dnidClient = 0;
	msg.gid = 0;

	int esize = 0;
	try
	{
		esize = (int)lite::Serializer(msg.streamData, sizeof(msg.streamData))("").EndEdition();
	}
	catch (lite::Xcpt &)
	{
		return;
	}

	g_StoreMessage(dnidClient, &msg, sizeof(msg) - esize);

	GetApp()->m_Server.DelOneClient(dnidClient);
}

BOOL g_GetAddrByDnid(DNID dnidClient, sockaddr_in *addr, size_t size)
{
    return GetApp()->m_Server.GetDnidAddr(dnidClient, addr, size);
}

void g_SetLogStatus(DNID dnidClient, LINKSTATUS state)
{
    GetApp()->m_Server.SetLinkStatus(dnidClient, state);
}

LINKSTATUS g_GetLogStatus(DNID dnidClient)
{
    return GetApp()->m_Server.GetLinkStatus(dnidClient);
}

DNID g_GetLinkMac( DNID dnidClient )
{
    return GetApp()->m_Server.GetLinkMac( dnidClient );
}


WORD GetServerIDFromName(LPCSTR szServerName)
{
    if (dwt::strcmp("区域", szServerName, 4) == 0)
    {
        if (szServerName[4] == '\0')
            return 0;

        char number[32];
        dwt::strcpy(number, szServerName+5, 5);

        return (WORD)atoi(number);
    }

    return 0;
}

CDAppMain::CDAppMain(void) : 
    m_Server(TRUE)
{
    m_eLoginStatus = SS_NULL;
    m_pGameWorld = NULL;

    m_dwNormalPlayerNumber = 0;
    m_dwHangupPlayerNumber = 0;
    m_dwLinkBreakPlayerNumber = 0;

    m_i64TotalSpendMoney = 0;
}

CDAppMain::~CDAppMain(void)
{
}

BOOL CDAppMain::InitServer()
{
	sTransportConf->load();
	//BlessMoudle *hads = BlessMoudle::getSingleton();
	//CSanguoPlayer playerTest;
	//playerTest.SetSanguoPlayerDNID(0);
	//SFixData testData;
	//playerTest.SetSanguoPlayerData(&testData);

	if (InitMaskMap() == 0)
        return rfalse(0, 0, "InitMaskMap false");

    // 区域服务器上包含了所有的游戏数据对象
    ::GetCurrentDirectory(MAX_PATH-1, m_InitPath);
    m_InitPath[MAX_PATH-1] = 0;

    m_hMainWnd = NULL;
    m_hInst = NULL;
    m_hMenu = NULL;
    m_bQuit = FALSE;
    m_dwPort = -1;
    m_dwIP = -1;
	m_dwIndex = -1;

	m_dwNew = 0;
	m_dwActivity = 0;
	m_dwRush = 0;





    extern dwt::stringkey<char [256]> szIniFilename;

	IniFile IniFile;
    char buffer[256];
	if (!IniFile.Load(szIniFilename)) 
        return rfalse(0, 0, "无法打开配置文件%s", (LPCSTR)szIniFilename);

    IniFile.GetString("SERVER", "ACCOUNT", "", buffer, 16);
    dwt::strcpy(m_szName, buffer, 16);

    IniFile.GetString("SERVER", "IP", "", buffer, 64);
    m_dwIP = inet_addr(buffer);

    IniFile.GetInteger("SERVER", "PORT", -1, (int*)&m_dwPort);

	IniFile.GetInteger( "SERVER", "INDEX", -1, (int*)&m_dwIndex );
	IniFile.GetInteger( "SERVER", "ISNEW", 0, (int*)&m_dwNew );
	IniFile.GetInteger( "SERVER", "ISACTIVITY", 0, (int*)&m_dwActivity );

	// wk 20150613 用作login端口
	//IniFile.GetInteger( "SERVER", "ISRUSH", 0, (int*)&m_dwRush );

	//获取开服时间
	memset(buffer, 0, sizeof(char) * 256);
	IniFile.GetString("SERVER", "OPENTIME", "", buffer, 32);
	string openTimeStr(buffer);
	tm openTm;
	int year, month, day, hour, minute, second;
	sscanf(openTimeStr.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	openTm.tm_year = year - 1900;
	openTm.tm_mon = month - 1;
	openTm.tm_mday = day;
	openTm.tm_hour = hour;
	openTm.tm_min = minute;
	openTm.tm_sec = second;
	m_ServerStartTime = _mktime32(&openTm);

	IniFile.GetString("LOGIN", "IP", "", szLoginIP, 32);
	IniFile.GetString("LOGIN", "PORT", "", szLoginPORT, 32);
	
	

	IniFile.GetString("ORB", "IP", "", szOrbIP, 32);
	IniFile.GetString("ORB", "PORT", "", szOrbPORT, 32);

	IniFile.GetString("ORBCENTER", "IP", "127.0.0.1", ORBCADDR, 32);

	//TODO:Tony Modify [2012-3-6]Comment:[这里得到中心服的IP和PORT]
	IniFile.GetString( "CenterServer" , "IP" , "" , m_szCenterIP , 32 );
	IniFile.GetString( "CenterServer" , "PORT" , "" , m_szCenterPORT , 32 );
	///////////////////////////////////////////////////////////////////
	
	

    IniFile.Clear();

    if ( !IniFile.Load( "../login/loginserver.ini" ) && !IniFile.Load( "../loginserver/loginserver.ini" ) ) 
        return rfalse( 0, 0, "无法打开配置文件LoginServer.ini" );

    IniFile.GetInteger( "INFO", "ID", 1, ( int* )&GROUP_SERVER_ID );
	IniFile.GetInteger("SERVICE", "PlayerPORT", 0, (int*)&m_dwRush);

    IniFile.Clear();

    if ( GROUP_SERVER_ID == 1 )
        return rfalse(0, 0, "取本机组ID失败[GROUP_SERVER_ID]！！！");

	g_dServerID = GROUP_SERVER_ID;
    // 这里可能要使用INI配置，因为在有两个网卡的机器上，这里可能取的内网的地址
    if (m_dwIP == 0)
        return rfalse(0, 0, "没有配置本机服务IP！！！");

    extern WORD g_wServerID;
    g_wServerID = m_wServerID = GetServerIDFromName(m_szName);
    if (m_wServerID == 0)
        return rfalse(0, 0, "取出的服务器ID为0！！！");

    g_SetMainHWnd(hMainWnd);

    // 这里要把自己注册到登陆服务器上去
    if ( !m_LoginServer.Connect( szLoginIP, szLoginPORT ) )
        return rfalse(0, 0, "无法连接登陆服务器！！！");

	//TODO:Tony Modify [2012-3-6]Comment:[这里先连接中心服]
	if ( !m_CenterClient.Connect( m_szCenterIP , m_szCenterPORT ) )
	{
		return rfalse(2, 1, "无法连接中心服!!!!!");
	}

	//沙箱警告
#ifdef MSDKTEST
	rfalse(0, 0, "沙箱充值环境~！！！！！！！");
#endif

	///////////////////////////////////////////////////////////////////
    
    //// 这里要把自己注册到跨场景服务器上去
    //if ( !m_ORBServer.Connect( szOrbIP, szOrbPORT ) )
    //    rfalse(0, 0, "无法连接跨场景服务器！！！");

    // 向服务器注册本机
    m_ServerVersion = SERVER_VERSION;
    SQLoginMsg msg;
    msg.wVersion = MAKEWORD(0, SERVER_VERSION);

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( m_szName )( "123456798" );
        size_t esize = slm.EndEdition();
        m_LoginServer.SendMessage( &msg, (WORD)(sizeof(msg) - esize) );
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

    m_eLoginStatus = SS_LOGIN;

	memset(buffer, 0, sizeof(char) * 256);
	_snprintf(buffer, sizeof buffer, "ZoneServer_%d", GROUP_SERVER_ID % 1000);
	SetWindowText(hMainWnd, buffer);

    rfalse(2, 1, "正在向登陆服务器注册本机");

    //memcpy((LPVOID)0x74FC1834, this, 16);

    // 创建邮件发送线程
    //extern dwt::mtQueue<POSTPARAM> g_PostParamQueue;
    //StartSPThread(g_PostParamQueue);

	//TODO:Tony Modify [2012-3-6]Comment:[向中心服注册]
	SQRegisterServer _msg;
	_msg.dwIP = m_dwIP;
	_msg.dwPort = m_dwPort;
	_msg.index = GROUP_SERVER_ID;
	_msg._dNew = m_dwNew;
	_msg._dActivity = m_dwActivity;
	_msg._dRush = m_dwRush;
	try
	{
		lite::Serializer slm( _msg.streamData, sizeof( _msg.streamData ) );
		slm( m_szName )( "123456798" );
		size_t _esize = slm.EndEdition();
		m_CenterClient.SendToCenterServer( &_msg , static_cast<WORD>( sizeof( _msg ) - _esize ) );
	}
	catch ( lite::Xcpt & )
	{
		return FALSE;
	}
	rfalse(2, 1, "正在向中心服务器注册本机");
	///////////////////////////////////////////////////////////////////

    int InitRcdMap();
    InitRcdMap();
	TimerEvent_SG::FristReg();
    return TRUE;
}

DWORD seg = timeGetTime();
  void AddInfo(LPCSTR);
//---------------------------------------------------------------------------
BOOL CDAppMain::LoopServer()
{

    static DWORD dwPrevTime = timeGetTime(), dwCurFrame = 0;

    if ((int)(timeGetTime()-dwPrevTime) < 1000/GAME_FPS)
    {
        // 还没有经过指定时间间隔，继续等待
        return TRUE;
    }



    dwCurFrame++;					// AI帧数增加
    dwPrevTime += 1000/GAME_FPS;	// 等待时间调整， 为了使时间能够精确一点，使用+=1000/GAME_FPS来减小误差

    // ------------------------------
    // 空闲部分结束，游戏部分开始
    // ------------------------------

    // Adding game process codes at here
    extern double fps_max, cur;
    unsigned __int64 GetCpuSpeed();
    unsigned __int64 CycleSeg();
    unsigned __int64 CycleEnd();

    extern double fps4ls, curls;
    extern double fps4os, curos;
    extern double fps4s, curs;
    extern double fps4g, curg;
	extern double fps4cs , curcs;

    CycleSeg();

    {
        // 处理登录服务器的网络操作
        CycleSeg();
        m_LoginServer.Execution();
        curls = CycleEnd() / (double)GetCpuSpeed();
        if (curls > fps4ls)
            fps4ls = curls;

        // 处理跨场景服务器的网络操作
        CycleSeg();
        //m_ORBServer.Execution();
        curos = CycleEnd() / (double)GetCpuSpeed();
        if (curos > fps4os)
            fps4os = curos;

		//TODO:Tony Modify [2012-3-6]Comment:[处理中心服的网络操作]
		CycleSeg();
		m_CenterClient.Execution();
		curcs = CycleEnd() / ( double )GetCpuSpeed();
		if ( curcs > fps4cs )
		{
			fps4cs = curcs;
		}
		///////////////////////////////////////////////////////////////////

        // 处理本机的AI
        switch (m_eLoginStatus)
        {
        case SS_BREAK: // login fail
            // 因为如果是登陆服务器重起，上面的GID重新累加，所以区域服务器上的GID就失效了，所以必须重启
			//system("cagent_tools alarm '告警内容'");
            ExitServer();
            rfalse(0, 0, "和登陆服务器连接断开！！！");
            m_bQuit = true;
            return FALSE;

        case SS_INIT:

            ShowVersion();

            rfalse(2, 1, "注册本机成功，开始启动本地服务……");
            {
	            LoadEnvironment();

                m_pGameWorld = new CGameWorld;
                if (!m_pGameWorld->Initialize())
                    return rfalse(0, 0, "启动游戏服务失败！");
				CConfigManager::Instance()->m_ServerStartTime = m_ServerStartTime; //设置开服时间
                char sPORT[64];
                // if (!InitializeServer(m_Server, itoa(m_dwPort, sPORT, 10), OnDispatch, this))
                if ( !m_Server.Create( itoa(m_dwPort, sPORT, 10) ) )
                    return rfalse(0, 0, "无法在本机的[%s]端口启动监听", sPORT);

                // 开启定时数据发送脉冲
                DWORD Param[2] = {1, 1};
                m_Server.SetOption( Param );

                // 开启网络底层数据压缩
                Param[0] = 2;
                m_Server.SetOption( Param );

                // 限制一机只能开一个客户端！
                extern DWORD macLimit;
                m_Server.SetMacLimit( macLimit );

                m_eLoginStatus = SS_SUCCESS;

                //// Get current flag
                //int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

                //// Turn on leak-checking bit
                //tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

                //// Set flag to the new value
                //_CrtSetDbgFlag( tmpFlag );
            }
            break;

        case SS_SUCCESS:
            {
                static DWORD dwHalfHourSkipTime = timeGetTime();
                if ( (int)(timeGetTime() - dwHalfHourSkipTime) > 1000 * 60 * 30 )
                {
                    dwHalfHourSkipTime = timeGetTime();

                    // 纪录统计数据到log文件
                    DisplayServerInfo(3);
                }

                SYSTEMTIME s;
                GetLocalTime(&s);
                static DWORD day = s.wDay;
                if ( day != s.wDay )
                {
                    day = s.wDay;
                    extern void SaveItemDropRcdToFile( BOOL );
                    extern void CleanItemDropRcd();
                    SaveItemDropRcdToFile( true );
                    CleanItemDropRcd();
                }

				// 保存帮派的数据 + m_wServerID * 1000,避免在同一时间所有区域都保存
                //static DWORD dwFacSaveTime = timeGetTime();
                //if ( (int)(timeGetTime() - dwFacSaveTime) > 1000 * 60 * 30 + m_wServerID * 1000 )
                //{
                //    dwFacSaveTime = timeGetTime();
                //    GetGW()->m_FactionManager.SaveBuffer();
                //}

                // 钱的记录刷新
//                g_Count.UpdateMoneyDataCount(TIMECOUNT_HOURS);
                // 脚本循环
//				rfalse(2, 1, " g_Script.SetLoop() ");
                g_Script.SetLoop();
                // 房间循环
 //               _theGHouseManager->Notify();

                static DWORD dw10MSkipTime = timeGetTime();
                if ( (int)(timeGetTime() - dw10MSkipTime) > 1000 * 60 * 10 )
                {
                    dw10MSkipTime = timeGetTime();

                    // 断线重连
                    void ReconnectAllServer();
                    ReconnectAllServer();
                }
            }

            CycleSeg();
			
            {
                LARGE_INTEGER bg, ed, fq;
                QueryPerformanceCounter( &bg );
                m_Server.CheckAlive();
				//g_messagelist.clear();
                int count = m_Server.Execution( boost::bind(&CDAppMain::OnDispatch, this, _1, _2, _3, _5 ) );
				//20150522 wk 充值返回余额处理
				CRecharge::getSingleton()->RechargeGetBackDispose();

                QueryPerformanceCounter( &ed );
                QueryPerformanceFrequency( &fq );
                DWORD mtick = ( DWORD )( ( ed.QuadPart - bg.QuadPart ) * 1000 / fq.QuadPart );
                if ( mtick > 100 )
                {
                    rfalse( 2, 1, "发现了超过预订时间的集群处理消息[count = %d][tick = %d]", count, mtick );
// 					std::list<WORD>::iterator iter = g_messagelist.begin();
// 					while (iter != g_messagelist.end())
// 					{
// 						rfalse(2, 1, "消息ID = %d", *iter);
// 						iter++;
// 					}
                }
            }

            curs = CycleEnd() / (double)GetCpuSpeed();
            if (curs > fps4s)
                fps4s = curs;
            // _ASSERTE(_CrtCheckMemory());

            CycleSeg();
            m_pGameWorld->Run();
            {
  //          char temp[256];
  //          sprintf( temp, "MainLoop = %d\r\n", timeGetTime() - seg );
			//AddInfo(temp);
            //OutputDebugString( temp );
            }
            curg = CycleEnd() / (double)GetCpuSpeed();
            if (curg > fps4g)
                fps4g = curg;

            // _ASSERTE(_CrtCheckMemory());
            break;

        default: // wait login server's result
            break;
        }
    }

    cur = CycleEnd() / (double)GetCpuSpeed();
    if (cur > fps_max)
        fps_max = cur;

    // ------------------------------
    // 游戏部分结束，休眠部分开始
    // ------------------------------

    int iUseTime = timeGetTime() - dwPrevTime;

    if (iUseTime > (1000/GAME_FPS))
    {
        // 如果当前桢耗时（以及总时差）远超出实际计时，则直接忽略，重新调整计时器
        if ( iUseTime > ( 10000 ) )
            dwPrevTime = timeGetTime() - ( 1000 / GAME_FPS );

        void AddInfo(LPCSTR);
        static int iSendWarning = 0;
        static char str[32];
        // 等待时间累计每超过一秒，就发出一个警告！
        if (iUseTime > iSendWarning*1000)
        {
            if (iUseTime > (iSendWarning+1)*1000)
            {
                iSendWarning++;
                AddInfo("警告等级[");
                AddInfo(itoa(iSendWarning, str, 10));
                AddInfo("]进入等待时间累计超时，可能是服务器过载，也可能是服务器内部出错\r\n");
            }
        }
        else if (iSendWarning != 0)
        {
            // 等待时间累计每恢复一秒，就解除一个警告！
            AddInfo("等待时间累计超时状态解除\r\n");
            iSendWarning--;
        }
    }
    else
    {
        // 休眠空余时间.
        Sleep(1000/GAME_FPS - iUseTime);
    }

    return TRUE;
}

//---------------------------------------------------------------------------
LRESULT CDAppMain::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    switch (uMsg) 
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // 分析菜单选择：
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

extern BOOL factionLoadSuccess;
extern BOOL unionLoadSuccess;

BOOL CDAppMain::ExitServer()
{
	if (g_Script.PrepareFunction("OnExitServer")) //服务器退出调用1次lua，用于保存一下数据
	{
		g_Script.Execute();
	}
	if (g_Script.PrepareFunction("OnServerDown")) //服务器退出调用
	{
		g_Script.Execute();
	}
	Sleep(10000);//现在主要用于保存军团信息的刷写回数据库


	SaveAndLogoutAllPlayer();
    Sleep(10000);

	//退出服务器保存排行榜数据
	g_Script.CleanCondition();
	
	m_CenterClient.OnLogout();
    m_Server.Destroy();
    m_LoginServer.Destroy();

    if (m_pGameWorld != NULL) 
    {
        m_pGameWorld->Destroy();
        delete m_pGameWorld;
        m_pGameWorld = NULL;
    }

	TransportConf::destroy_instance();
    return TRUE;
}

void SendMacAccLimit( LPCSTR acc, DNID mac, LPCSTR info )
{
    try
    {
		SQGameServerRPCOPMsg rpc;
		rpc.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
		lite::Serializer slm( rpc.streamData, sizeof( rpc.streamData ) );

		// 准备存储过程
		slm [OP_PREPARE_STOREDPROC] ("script.UpdateMacAccLimit")

		// 设定调用参数
		[OP_BEGIN_PARAMS]
		( 1 ) ( mac )
		( 2 ) ( acc )
        ( 3 ) ( ( int )GetGroupID() )
		( 4 ) ( info )
		[OP_END_PARAMS]

		// 调用存储过程
		[OP_CALL_STOREDPROC]
		[OP_RPC_END];

		rpc.dnidClient = INVALID_DNID;
		dwt::strcpy( ( char* )rpc.GetExtendBuffer(), "", 16 );
        SendToLoginServer( &rpc, (long)(sizeof( rpc ) - slm.EndEdition()) );
    }
    catch ( lite::Xcpt & )
    {
    }
}

static void SaveChk( LPBYTE buf, CPlayer *player )
{
	if ( buf == NULL )
		return;

    static char msg[128*1024];
    lzw::lzwDecoder ldc;
    ldc.SetEncoderData( buf + 2, *( LPWORD )buf );
    int ck = (int)ldc.lzw_decode( msg, sizeof( msg ) );
    if ( ck <= 0 )
        return;
    
    msg[ ck ] = 0;
    DNID mac = player ? player->GetMac() : 0;
    char strinfo[300];
    int len = sprintf( strinfo, "%s - %I64x - %s - %d - %d", 
        ( player ? player->GetAccount() : "unknown acc" ), mac, 
		( player ? player->m_Property.m_Name : "unknown name" ), 
		( player ? player->m_Property.m_Level : 0 ), 
		( player ? 0/*player->m_Property.m_dwConsumePoint*/ : 0 ) );

	TraceInfoDirectly_C( "wglimit.txt", strinfo );
    TraceInfoDirectly_C( "wglimit.txt", msg );
    for ( LPSTR it = msg; *it != 0; it ++ ) {
        if ( *it == '\r' || *it == '\n' ) {
            *it = 0;
            break;
        }
    }

    dwt::strcpy( strinfo + len, msg, sizeof( strinfo ) - len );

    if ( player ) {
        player->timeOutChk = 0;
        SendMacAccLimit( player->GetAccount(), mac, strinfo );
    }
}

bool CDAppMain::OnDispatch(DNID dnidClient, LINKSTATUS enumStatus, LPVOID data, LPVOID attachment)
{
    SMessage *pMsg = (SMessage *)data;
    IObject *pObj = (IObject *)attachment;
	if(!pMsg)return false;
//	g_messagelist.push_back(pMsg->_protocol);
	if (pMsg->_protocol == SMessage::ERPO_SECTION_MESSAGE)
	{
		SMessage* srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)pMsg);
		if (srcMsg)
		{
			OnDispatch(dnidClient,enumStatus,(LPVOID)srcMsg,attachment);
			SectionMessageManager::getInstance().popMessage(srcMsg);
		}
	}
    // 这里只处理系统级的消息
    if (!pMsg)
    {
		if (pObj)
        {
            // 说明该连接断开，就需要将相对应的玩家设定吊线处理
            extern void OnPlayerDisconnect(DNID dnidClient);
            OnPlayerDisconnect(dnidClient);

            // 根据ContextProvider的处理规则，在连接断开的时候，清除附加在Dnid上的对象
            RemovePlayerByDnid(dnidClient);
        }

        return false;
    }

    // 判断是否是底层传来的特殊处理消息
    if ( pMsg->_protocol == 0xef )
    {
        LPBYTE opary = ( LPBYTE )data;
        switch ( opary[1] )
        {
        case 0:
            // 这个链接是用了外挂的。。。封掉！
            if ( CPlayer *player = ( CPlayer* )GetPlayerByDnid( dnidClient )->DynamicCast( IID_PLAYER ) )
            {
                if ( player->isErrPlayer == 0 )
                {
                    // 注册非法客户端退出标识！
                    player->isErrPlayer = 100 + rand() % 500;
										
                    // 向帐号服务器通知该mac和帐号存在问题，需要做封号处理！
                    extern BOOL wgLimit;
                    if ( wgLimit & 8 ) // 向帐号服务器提交外挂信息
                    {
						/*
                        player->timeOutChk = timeGetTime() + ( rand() % 0xffff ); // 随机1分钟内超时处理
                        SASynObjectMsg msg;
                        msg.wCheckID = 12;
                        msg.wCurX = player->m_wCurX + 9;
                        msg.wCurY = player->m_wCurY + 14;
                        msg.dwGlobalID = player->GetGID() + 1489;
                        msg.dwExtra = msg.wCheckID + msg.wCurX + msg.wCurY + msg.dwGlobalID;
                        g_StoreMessage( dnidClient, &msg, sizeof( msg ) );
						//*/
                        // SendMacAccLimit( player->GetAccount(), *( DNID* )( opary + 2 ) );
                    }
                }
            }
            break;
        }

        return true;
    }
    else if ( reinterpret_cast< LPDWORD >( data )[0] == 0x04ff1023 )
    {
        // 写入信息到文件！
        CPlayer *player = ( CPlayer* )GetPlayerByDnid( dnidClient )->DynamicCast( IID_PLAYER );
        LPBYTE buf = ( LPBYTE )data;
        SaveChk( buf + 4, player );

        return true;
    }

	// 先判断是否是登陆认证级的消息
    if (pMsg->_protocol == SMessage::EPRO_REBIND_MESSAGE)
    {
        // 帮派数据读取是否成功的标志，如不成功，则玩家不能登录
		// XYD3 帮派系统暂时未做
//         extern BOOL factionLoadSuccess;
//         extern BOOL unionLoadSuccess;
//         if ( !factionLoadSuccess || !unionLoadSuccess )
//         {
//             g_CutClient(dnidClient);
//             return true;
//         }

        // 这里处理连接重定向的认证
        QuestCheckRebind(dnidClient, (SQRebindMsg *)pMsg);
        return true;
    }

    // 其它的消息只能在认证以后的状态下使用，否则视为非法
    if (enumStatus < LinkStatus_Connected)
    {

		if (pMsg->_protocol == SMessage::EPRO_GMTOOL_MSG)
		{
			CPlayer *pPlayer = (CPlayer *)0xffffff;
			m_pGameWorld->OnDispatch(dnidClient, pMsg, pPlayer);
			return true;
		}
		else
		{
			g_CutClient(dnidClient);
			return true;
		}
    }

    CPlayer *pPlayer = NULL;

    {
        pPlayer = (CPlayer *)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);// (CPlayer *)pObj->DynamicCast(IID_PLAYER);
        if (pPlayer == NULL)
        {
            // 这个是因为客户端在发送logout之后又发送了一个关闭仓库的消息。。。
            if (pMsg->_protocol == SMessage::EPRO_ITEM_MESSAGE)
                return true;

            void TraceMemory(void*, int, unsigned int, const char*);
            TraceMemory(pMsg, -128, 300, "InvalidContextMsg.mem");

            // 该连接没有附加Player对象，无效，直接断开！
            g_CutClient(dnidClient);
            return true;
        }
        else
        {
            if ( pObj != pPlayer )
            {
                // 该连接没有附加Player对象和实际连接绑定的对象有误，直接断开！
                rfalse(2, 1, "异常的连接绑定对象！");
                g_CutClient(dnidClient);
                return true;
            }

            //LPIObject obj(pObj);

            //if (obj.Count() < 2) // 引用计数一定大于等于2
            //{
            //    rfalse(2, 1, "异常的对象引用计数！");
            //    g_CutClient(dnidClient);
            //    return true;
            //}
        }
    }
	
    LARGE_INTEGER bg, ed, fq;
    QueryPerformanceCounter( &bg );
    m_pGameWorld->OnDispatch(dnidClient, pMsg, pPlayer);
    QueryPerformanceCounter( &ed );
    QueryPerformanceFrequency( &fq );
    DWORD mtick = ( DWORD )( ( ed.QuadPart - bg.QuadPart ) * 1000 / fq.QuadPart );
    if ( mtick > 20 )
    {
        #define GetByte(index) (buf[index])

        LPBYTE buf = ( LPBYTE )data;
//        rfalse(2, 1, "RTL [%s] [lt = %d] "
//            "[info = <%02x %02x %02x %02x %02x %02x %02x %02x>]", 
//            GetStringTime(), mtick, GetByte(0), GetByte(1), GetByte(2), 
//            GetByte(3), GetByte(4), GetByte(5), GetByte(6), GetByte(7));

        if ( mtick > 200 )    // 10秒
            TraceInfo( "超时记录.txt", "%s [tick = %d] [info = <%02x %02x %02x %02x %02x %02x %02x %02x>]", _GetStringTime(), mtick,
            GetByte(0), GetByte(1), GetByte(2), 
            GetByte(3), GetByte(4), GetByte(5), GetByte(6), GetByte(7));	

        //rfalse( 2, 1, "发现了超过预订时间的单个处理消息[%02x]" );
    }
    return true;
}

int CDAppMain::QuestCheckRebind(DNID dnidClient, SQRebindMsg *pMsg)
{
    // 临时处理，测ping值
    if (pMsg->streamData[0] == 0)
    {
        int ping = (int)pMsg->gid;
        if ((ping > 0) && (ping < 30000))
        {
            if ((dwPingInfo > 0xf0000000) ||
                (dwPingTimes > 0xf0000000))
            {
                dwPingTimes /= 2;
                dwPingInfo /= 2;
            }

            dwPingTimes++;
            dwPingInfo += ping;

            dwAvePing = (DWORD)( dwPingInfo / (float)dwPingTimes );

            if ((dwPingInfoT > 0xf0000000) ||
                (dwPingTimesT > 0xf0000000))
            {
                dwPingTimesT /= 2;
                dwPingInfoT /= 2;
            }

            dwPingTimesT++;
            dwPingInfoT += ping;

            if (ping > PingMAX)
                PingMAX = ping;

            if (ping < PingMIN)
                PingMIN = ping;
        }

        return 1;
    }

	LPCSTR account = 0;
	size_t esize = 0;

	SQCheckRebindMsg msg;
    msg.gid = pMsg->gid;
    msg.dnidClient = dnidClient;

    try
    {
        lite::Serialreader sl(pMsg->streamData);
        account = sl();

        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm(account);
        esize = slm.EndEdition();
    }
    catch (lite::Xcpt &)
    {
        return false;
    }

	// 向登陆服务器请求验证
	if (!m_LoginServer.SendMessage(&msg, (WORD)(sizeof(msg) - esize)))
    {
        rfalse(1, 1, "check : send rebind msg to loginserver fail!!!");
    }

    extern bool g_check;
    if (g_check)
    {
        extern LPCSTR GetStringTime();
        rfalse(1, 1, "check : [%s] send rebind msg to loginserver [%s]!!!", GetStringTime(), account );
    }

    return 1;
}

void CDAppMain::DisplayServerInfo(char i)
{
    rfalse(i, 1, "当前服务器状态：\r\n"
        "登陆服务器[%s]\r\n",
        easyState(m_eLoginStatus));

    if ( m_pGameWorld == NULL )
        return;

    rfalse(i, 1, "当前连接数/底层连接数 ： %d/%d", m_Server.GetClientNum(), m_Server.GetClientNum_Dll());
    rfalse(i, 1, "对象映射数量：%s", m_pGameWorld->GetRelationString());
    m_pGameWorld->DisplayServerInfo(i);

    rfalse(i, 1, "异常/正常/掉线/挂机/自动/DXTC：%d/%d/%d/%d/%d/%d", g_CurOnlineCheck[0], 
        g_CurOnlineCheck[1], g_CurOnlineCheck[2], g_CurOnlineCheck[3], g_CurOnlineCheck[4], g_CurOnlineCheck[5]);

    // rfalse(i, 1, "当前本服务器组的总人数：%d", GetCurrentTatolPlayerNumber());

    rfalse(i, 1, "历史平均ping值：%d，当前平均ping值：%d", 
        (dwPingTimesT == 0)?(0):(dwPingInfoT/dwPingTimesT),
        (dwPingTimes == 0)?(0):(dwPingInfo/dwPingTimes));
    rfalse(i, 1, "当前最高ping值：%d，当前最低ping值：%d", PingMAX, PingMIN);

    if (i != 2)
    {
        dwPingInfo = 0;
        dwPingTimes = 0;
        PingMAX = 0;
        PingMIN = 123456789;
    }
}


LPCSTR GetInitPath()
{
    return GetApp()->m_InitPath;
}

WORD GetServerID()
{
    return CDAppMain::m_wServerID;
}

DWORD GetGroupID()
{
    return ( DWORD )GROUP_SERVER_ID;
}

BYTE GetZoneID()
{
    return ((CDAppMain::m_wServerID == 115) ? 1 : ((CDAppMain::m_wServerID == 1630) ? 2 : 3));
}

DWORD TryInsertClusterID(DWORD timestamp32bit)
{
    if ((clusterMask == 0) || ((~clusterMask) & 0x1fffffff) || (clusterId & 0x1fffffff))
        return timestamp32bit;

    return ((timestamp32bit & clusterMask) | clusterId);
}

int SendToLoginServer(SMessage *data, long size)
{
    return GetApp()->m_LoginServer.SendMsgToLoginSrv(data, size);
}

BOOL SetDnidContext(DNID dnidClient, LPVOID pContext)
{
    return GetApp()->m_Server.SetLinkContext(dnidClient, pContext);
}

LPCSTR GetServerName()
{
    return GetApp()->m_szName;
}

SQGameServerRPCOPMsg LogNotify::msg;

LogNotify::LogNotify( LPCSTR proc, BYTE dbcId ) 
    : proc( proc ), dbcId( dbcId ), index( 1 ), slm( msg.streamData, sizeof( msg.streamData ) ) 
{
	// 准备存储过程 设定调用参数
	slm [OP_PREPARE_STOREDPROC] ( proc ) [OP_BEGIN_PARAMS];
}

LogNotify& LogNotify::operator [] ( const lite::Variant &vt )
{
    slm ( index++ ) ( vt );
    return *this;
}

void LogNotify::operator () ()
{
	slm [OP_END_PARAMS] [OP_CALL_STOREDPROC] [OP_RPC_END];

	msg.dstDatabase = dbcId;
    SendToLoginServer( &msg, (long)(sizeof( msg ) - slm.EndEdition()) );
}

SQGameServerRPCOPMsg SimpleLogNotify::msg;

SimpleLogNotify::SimpleLogNotify( DWORD tableId, BYTE dbcId ) 
    : tableId( tableId ), dbcId( dbcId ), slm( msg.streamData, sizeof( msg.streamData ) ) 
{
	slm [-1] ( 1 ) ( tableId );
}

SimpleLogNotify& SimpleLogNotify::operator () ( int idx, int v )
{
    slm [ idx ] ( v );
    return *this;
}

SimpleLogNotify& SimpleLogNotify::operator () ( int idx, QWORD v )
{
    slm [ idx ] ( v );
    return *this;
}

SimpleLogNotify& SimpleLogNotify::operator () ( int idx, LPCSTR v )
{
    slm [ idx ] ( v );
    return *this;
}

void SimpleLogNotify::operator () ()
{
	msg.dstDatabase = dbcId;
    SendToLoginServer( &msg, (long)(sizeof( msg ) - slm.EndEdition()) );
}

void SaveDestoriedItem( SPackageItem *pi, CPlayer *player )
{
    SPackageItem item = *pi;

    lite::Serializer slm( SimpleLogNotify::msg.streamData, sizeof( SimpleLogNotify::msg.streamData ) );

	try
	{
        // 准备存储过程（查询）
        slm [OP_PREPARE_STOREDPROC] ( "sp_SaveDestoriedItem" ) 

		    // 设定调用参数
		    [OP_BEGIN_PARAMS]
            (1) ( GetGroupID() )                // 服务器ID
            (2) ( player->GetAccount() )        // 帐号
            (3) ( (int)player->GetSID() )            // 角色ID
            (4) ( (int)pi->wIndex )  // 道具详细数据！
            (5) ( pi->uniqueId() )  // 道具详细数据！
            (6) ( pi, sizeof( SPackageItem ) )  // 道具详细数据！
		    [OP_END_PARAMS]

		    // 调用存储过程
		    [OP_CALL_STOREDPROC]

            // 结束
            [OP_RPC_END];

        // 全都准备好后，再发送消息到登陆服务器！并且由登陆服务器再转到账号服务器
        SimpleLogNotify::msg.dstDatabase = 2; // 这个是 rcd100 所在的中央数据库日志服务器
        SendToLoginServer( &SimpleLogNotify::msg, (long)(sizeof( SimpleLogNotify::msg ) - slm.EndEdition()) );
	}
    catch ( lite::Xcpt & )
	{
	}
}

void UpdateImmediately( CPlayer *p1, CPlayer *p2 )
{
    // 这里要处理游客的情况，避免将游客的数据发送到数据库！
    if ( p2 != NULL && memcmp( p2->GetAccount(), "游客", 4 ) == 0 )
        p2 = NULL;

    if ( memcmp( p1->GetAccount(), "游客", 4 ) == 0 )
    {
        if ( p2 == NULL )
            return;

        p1 = p2;
        p2 = NULL;
    }

	SDoubleSavePlayerMsg msg;
	//p1->Backup( msg.data[0] );
	msg.gid[0] = p1->GetGID();
	dwt::strcpy( msg.account[0], p1->GetAccount(), sizeof( msg.account[0] ) );
	msg.gid[1] = 0;
	msg.account[1][0] = 0;
	
	if ( p2 ) // 2号位可能没有数据！
	{
		//p2->Backup( msg.data[1] );
		msg.gid[1] = p2->GetGID();
		dwt::strcpy( msg.account[1], p2->GetAccount(), sizeof( msg.account[1] ) );
	}

	SendToLoginServer( &msg, msg.gid[1] ? sizeof( msg ) : ( sizeof( msg ) - sizeof( msg.data[0] ) ) );
}
SectionMessageManager::SectionMessageManager()
{
	m_id = REGIONSERVER;
}
void SectionMessageManager::sendSectionMessage(DNID dnidClient,SSectionMsg* msg)
{
	g_StoreMessage(dnidClient,msg,sizeof(SSectionMsg));
}

BOOL CDAppMain::SendToCenterServer( void* pMsg, WORD iSize )
{
	return m_CenterClient.SendToCenterServer( pMsg , iSize );
}

