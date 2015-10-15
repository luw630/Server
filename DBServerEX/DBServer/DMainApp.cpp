#include "StdAfx.h"

#include "DMainApp.h"
#include <iostream>
#include <vector> 
#include "boost/bind.hpp"
#include "LITESERIALIZER/Lite.h"
using namespace mysql;
#include "pub/DBProcessor.h"
#include "pub/rpcop.h"

#include "NetworkModule\DataMsgs.h"
#include "NetworkModule\RefreshMsgs.h"
#include "NetworkModule\MailMsg.h"
#include "networkmodule/accountmsgs.h"
#include "NetworkModule\FactionBBSMsgs.h"
#include "networkmodule\CenterMsg.h"

#include "pub\ComplierConfigure.h"
#include "NetDBModuleEx.h"
#include "pub\ConstValue.h"
#include "LITESERIALIZER/variant.h"
#include "NetWorkModule/BuildingTypedef.h"

#include "persistent/dpl.h"
#include "networkmodule/SectionMsgs.h"
#include "mysqldbprocess.h"

#include "区域服务器/区域服务器/CenterModule.h"
#pragma comment(lib, "Ws2_32.lib")


extern void AddInfo( LPCSTR Info );
extern void LogMsg( LPCSTR filename, LPCSTR szLogMessage, int ex = -1 );
#define LogErrorMsg( str ) LogMsg( "debugError.txt", str )

static class CDAppMain *ptheApp = NULL;

void CDAppMain::WriteRole()
{
	_g::_p::_WriteRole();
}
void ShowVersion(void)
{
    static char vermsg[256];
    sprintf( vermsg, "MYSQL_版本号：Build_%d " "[" __DATE__ " " __TIME__ "]\r\n", SERVERVERSION );
    AddInfo( vermsg );

	sprintf( vermsg, "玩家数据_版本号：Build_ 0x%x " "[" __DATE__ " " __TIME__ "]\r\n", 	SFixPlayerDataBuf::GetVersion() );
	AddInfo( vermsg );

	sprintf( vermsg, "玩家数据_版本大小：Build_ 0x%x " "[" __DATE__ " " __TIME__ "]\r\n",sizeof(SFixPlayerDataBuf));
	AddInfo( vermsg );
}

//inline void SendDebug(const char* szOutput, ...)
//{
//	char  out[1024];
//	va_list  va;
//
//	if(szOutput == NULL)
//		OutputDebugString("\r\n");
//	else
//	{
//		va_start(va, szOutput);
//		vsprintf(out, szOutput, va);
//		va_end(va);
//	}
//
//	OutputDebugString(out); 
//}
//using namespace lite;

//printVariant( Variant &t)
//{
//	switch ( t.dataType )
//	{
//		case lite::Variant::VT_INTEGER :
//			SendDebug("Variant::_integer %d", t._integer);
//			break;
//		case Variant::VT_STRING:
//			SendDebug("Variant::_string %s", t._string);
//			break;
//		default:
//			SendDebug("otherType! %d", t.dataType);
//	}
//}

BOOL CreateApp(void)
{
    /*
    static char tempBuffer[10240];
    lite::Serializer slm( tempBuffer, sizeof( tempBuffer ) );
    __int64 t = -64;
    slm((__int16)-16)((__int32)-32)(&t);
    slm.EndEdition();
    lite::Serialreader slr( tempBuffer );
    __int16 a = slr();
    __int32 b = slr();
    __int64 c = slr();
    slr.Rewind();
    __int64 d = slr();
    __int64 e = slr();
    __int64 f = slr();
    */
    if ( ptheApp == NULL )
        return ( ptheApp = new CDAppMain ) != NULL;
    return NULL;
}

void ClearApp(void)
{
    if ( ptheApp )
        delete ptheApp;
    ptheApp = NULL;
}

CDAppMain &GetApp(void)
{
    return *ptheApp;
}

BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize )
{
    if ( wSize & 0xffff0000 )
        return false;

    return GetApp().m_NetProviderModule.SendMessage( dnidClient, lpMsg, (WORD)wSize );
}

CDAppMain::CDAppMain(void)
{
    // 一开始没有创建退出事件，表示正常运行状态，没有退出信号
    quitEvent = NULL;
    atBackup = -1;
}

CDAppMain::~CDAppMain(void)
{
}

std::string title, bakPath = "d:/dbbackupiv";

int CDAppMain::LoadBackupSetting()
{
    IniFiles IniFile;
    if ( !IniFile.open( ( __argc == 1 ) ? "bakset.ini" : __argv[1] ) )
        return rfalse( 2, 1, "注意，没有备份配置文件！" );

    bakPath             = IniFile.read(     "BACKUP", "BAKPATH", "d:/dbbackupiv" );
    int count           = IniFile.read_int( "BACKUP", "COUNT",   0 );

    rfalse( 2, 1, "备份路径：%s/%s", bakPath.c_str(), title.c_str() );

    bakTicks.clear();
    for ( int i=0; i<count; i++ )
    {
        char str[32];
        sprintf( str, "TICK%d", i );
        int tick = IniFile.read_int( "BACKUP", str, 9999 );
        if ( tick != 9999 )
        {
            bakTicks[tick] = 0;
            rfalse( 2, 1, "备份时间点：%d:%d", tick/100, tick%100 );
        }
    }

    return count;
}

BOOL CDAppMain::InitServer()
{
	if (!LoadPlayerInitProperty())
		return FALSE;

   //读取INI配置文件，并连接到数据库
    //启动本地服务器。

    IniFiles IniFile;
    if ( !IniFile.open( ( __argc == 1 ) ? "databaseserver.ini" : __argv[1] ) )
        return rfalse( 0, 0, "Cannot open databaseserver.ini file" );

//    std::string sDBPassword = 
//#ifdef _DEBUG
//    IniFile.read( "DATABASE","PASSWORD","" );
//#else
//    "Jf42fs89";
//#endif

    std::string sDBPassword = IniFile.read( "DATABASE", "PASSWORD",  "" );
	std::string sDBIP       = IniFile.read( "DATABASE", "IP",        "" );
    std::string sDBAccount  = IniFile.read( "DATABASE", "ACCOUNT",   "" );
    std::string sDBDefault  = IniFile.read( "DATABASE", "DEFAULTDB", "" );
    std::string sSRVPort    = IniFile.read( "SERVER",   "PORT",      "" );
                title       = IniFile.read( "SERVER",   "TITLE",     "DBServer_" );

	std::string stCServiceIP = IniFile.read("CENTERSERVICE", "IP", "");
	std::string stCServiceACCOUNT = IniFile.read("CENTERSERVICE", "ACCOUNT", "");
	int stCServicePort = IniFile.read_int("CENTERSERVICE", "PORT", 0);


	std::string stCenterServerIP = IniFile.read("CenterServer", "IP", "");
	std::string stCenterServerPort = IniFile.read("CenterServer", "PORT", "");

    LoadBackupSetting();

	IniFiles IniFile2;
	if (!IniFile.open((__argc == 1) ? "../loginserver/loginserver.ini" : __argv[1]))
		return rfalse(0, 0, "Cannot open loginserver.ini file");
	int GROUP_SERVER_ID = IniFile.read_int("INFO", "ID", 9999);
	char buffer[256];
	_snprintf(buffer, sizeof buffer, "DBServer_%d", GROUP_SERVER_ID % 1000);

    extern HWND hMainWnd;
	::SetWindowText(hMainWnd, buffer);
 
	if ( ! _g::_p::Initialize( sDBAccount.c_str(), sDBPassword.c_str(), sDBIP.c_str(), sDBDefault.c_str() ) )
		return  rfalse( 0, 0, "数据库序列化层初始化失败！" );

	//if ( ! m_RPCDB.Init( sDBAccount.c_str(), sDBPassword.c_str(), sDBIP.c_str(), sDBDefault.c_str() ) )
	//	return rfalse( 0, 0, "初始化数据库RPC连接失败" );

    // 初始网络初始化数据库连接
	if ( !m_NetProviderModule.Create( sSRVPort.c_str() ) )
		return rfalse( 0, 0, "启动网络服务失败！" );

    #define START_DBTHREAD( _proc, _queue, _name ) if ( !StartDBThread( _queue, _proc, \
        sDBAccount.c_str(), sDBPassword.c_str(), sDBIP.c_str(), sDBDefault.c_str(), _name ) ) \
        { MessageBox( 0, "启动（"_name"）数据库连接失败", 0, 0 ); return FALSE; }

    extern void RpcProcedure( mysql::Query &query, std::pair< DNID, void* > &element );
    START_DBTHREAD( RpcProcedure, rpcQueue, "RPC" )

    // 创建网络消息处理线程
    extern void MsgProcedure( mysql::Query &query, std::pair< DNID, void* > &element );
    START_DBTHREAD( MsgProcedure, msgQueue, "MSG" )


		if (!m_CenterClient.Connect(stCenterServerIP.c_str(), stCenterServerPort.c_str()))
		{
			return rfalse(2, 1, "无法连接中心服!!!!!");
		}

	SQRegisterDBServer _msg;
	_msg.dwIP = inet_addr(stCServiceIP.c_str());
	_msg.dwPort = stCServicePort;
	_msg.index = 0;
	_msg._dNew = 0;
	_msg._dActivity = 0;
	_msg._dRush = 0;
	try
	{
		lite::Serializer slm(_msg.streamservername, sizeof(_msg.streamservername));
		slm(stCServiceACCOUNT.c_str());
		size_t _esize = slm.EndEdition();
		m_CenterClient.SendToCenterServer(&_msg, static_cast<WORD>(sizeof(_msg) - _esize));
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}
	rfalse(2, 1, "Registering To CenterServer");

    ShowVersion();
    AddInfo( "Server started\r\n" );

    return TRUE;
}

void CDAppMain::Backup()
{
    // 在主线程中调用，用于启动所有当前角色的保存和激活备份标志
    int ck = _g::_p::Backup( &atBackup, title.c_str(), bakPath.c_str() );
	rfalse( 2, 1, "备份前共提交了%d个保存请求！", ck );
}

//---------------------------------------------------------------------------
BOOL CDAppMain::LoopServer()
{
    if ( atBackup == 1 )
    {
        // 在备份处理过程中，不处理网络消息，但需要检测网络消息队列大小，如果过大，则断开网络。。。
        Sleep( 1 );

        // 检测消息队列大小是否超过限制，并在超过限制后，强行断开网络。。。
        m_NetProviderModule.CheckQueueLimit( 100000, true );
    }
    else if ( atBackup == 0 )
    {
        // 可能网络服务端会由于停止操作过长，而取消服务。。。这里需要尝试恢复。。。
        if ( !m_NetProviderModule.ReactiveHost() )
            MessageBox( 0,"恢复网络服务失败！",0,0 );
        atBackup = -1;
    }
    else
    {
		m_CenterClient.Execution();
        try
        {
            m_NetProviderModule.Execution( boost::bind( &CDAppMain::Entry, this, _1, _2, _3, _4 ) );
        }
        catch ( ... )
        {
            // 这里是一些手动掷出的异常，一般不用考虑
            rfalse( 2, 1, "捕获了一个未处理的异常！" );
        }

        if ( quitEvent == NULL )
        {
            Sleep( 1 );

            // 定时备份时，停止网络消息处理逻辑，将所有缓存数据写入数据库，备份完成后，恢复消息作业。。。
            SYSTEMTIME st;
            GetSystemTime( &st );
            DWORD tick = st.wHour * 100 + st.wMinute;
            static DWORD prevTick = tick;
            if ( ( prevTick != tick ) && ( bakTicks.find( tick ) != bakTicks.end() ) )
                Backup();
        }
	    // 如果已经标记了退出，则当所有数据保存完毕后，关闭退出！
        else if ( WaitForSingleObject( quitEvent, 1 ) == WAIT_OBJECT_0 )
        {
            CloseHandle( quitEvent );
		    PostQuitMessage( 0 );
        }
    }
	
    return TRUE;
}

//---------------------------------------------------------------------------
BOOL CDAppMain::ExitServer()
{
    if ( atBackup != -1 )
        return rfalse( 2, 1, "正处于备份状态中，需要等待操作完成" );

    if ( quitEvent != NULL )
        return rfalse( 2, 1, "已经在退出状态，需要等待退出完成" );

	rfalse( 2, 1, "程序退出中，开始保存所有对象" );

	m_CenterClient.OnLogout();
	m_CenterClient.Destroy();

    m_NetProviderModule.Destroy();

    // 该函数被转移到 Shutdown 里边去了 
    // SaveAll( TRUE );

    int ck = _g::_p::Shutdown( quitEvent = CreateEvent( NULL, 0, 0, NULL ) );
	rfalse( 2, 1, "当前共提交了%d个保存请求！", ck );

	// SetEvent( quitEvent );

    return TRUE;
 
}

bool CDAppMain::Entry( unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, size_t size )
{
    if ( NULL == pMsg )
    {
        // link lost
        return false;
    }

    SMessage *pBaseMsg = (SMessage *)pMsg;

    // RPC操作暂时屏弊，等真正需要的时候再打开------------------------------------
	if ( ( pBaseMsg->_protocol == SMessage::EPRO_SERVER_CONTROL ) && 
		( ( ( SServerCtrlMsg * )pBaseMsg )->_protocol == SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
    {
        try 
        {
            SQGameServerRPCOPMsg *temp = (SQGameServerRPCOPMsg *)pMsg;
            lite::Serialreader ls( temp->streamData );            
        }
        catch ( ... )
        {
            rfalse( 2, 1, "异常的数据大小" );
            return true;
        }

        rpcQueue.mtsafe_push( std::pair< DNID, void* >( qwDnid, pMsg ) );
    }
	else if (pBaseMsg->_protocol == SMessage::ERPO_SECTION_MESSAGE)
	{
		SMessage* srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)pMsg);
		if (srcMsg)
		{
			Entry(qwDnid,enumStatus,srcMsg,((SSectionMsg*)pMsg)->byCount*SSectionMsg::MAX_SIZE);
			SectionMessageManager::getInstance().popMessage(srcMsg);
		}
	}
    else if ( pBaseMsg->_protocol == SMessage::EPRO_DATABASE_MESSAGE )
    {
        // msgQueue.mtsafe_push( std::pair< DNID, void* >( qwDnid, pMsg ) );
        // 目前角色处理部分消息直接进入dpl
		switch ( ( (_SDataBaseMsg *)pBaseMsg )->_protocol ) 
		{
        case _SDataBaseMsg::EPRO_GET_CHARACTER_LIST:
            _g::_p::QueryPlayerList( qwDnid, reinterpret_cast< SQGetCharacterListMsg* >( pBaseMsg ) );
            return true;

        case _SDataBaseMsg::EPRO_CREATE_CHARACTER:
            _g::_p::QueryCreatePlayer( qwDnid, reinterpret_cast< SQCreatePlayerMsg* >( pBaseMsg ) );
            return true;

		//获取角色详细的这个流程登录上应该传一个角色的名字过来，这样检索数据会快很多
		//目前的方式是利用Account+Index来定位，不如用名字来检索快,以后再优化吧。
        case _SDataBaseMsg::EPRO_GET_CHARACTER:
            _g::_p::QuerySelectPlayer( qwDnid, reinterpret_cast< SQGetCharacterMsg* >( pBaseMsg ) );
            return true;

		case _SDataBaseMsg::EPRO_SAVE_CHARACTER:
			_g::_p::SavePlayerData( qwDnid, reinterpret_cast< SQSaveCharacterMsg *>( pBaseMsg ) );
            return true;

		case _SDataBaseMsg::EPRO_DELETE_CHARACTER:
			_g::_p::DeletePlayer( qwDnid, reinterpret_cast< SQDeleteCharacterMsg*>( pBaseMsg ) );
            return true;

        case _SDataBaseMsg::EPRO_DOUBLE_SAVE_PLAYER:
            _g::_p::QueryDoubleSavePlayer( qwDnid, reinterpret_cast< SDoubleSavePlayerMsg* >( pBaseMsg ) );
            return true;

		case _SDataBaseMsg::EPRO_SAVEPLAYER_CONFIG:
			_g::_p::QuerySavePlayerConfig( qwDnid, reinterpret_cast< SQLSavePlayerConfig* >( pBaseMsg ) );
			return true;
		case _SDataBaseMsg::EPRO_DBGET_RankList:
			{
				_g::_p::QueryGetAllPlayerRandList(qwDnid,reinterpret_cast< SQDBGETRankList* >(pBaseMsg) );
			}
			return true;
        // #################################################################################
		case _SDataBaseMsg::EPRO_GET_FACTION:
        case _SDataBaseMsg::EPRO_SAVE_FACTION:
		case _SDataBaseMsg::EPRO_DELETE_FACTION:
		case _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA:
		case _SDataBaseMsg::EPRO_GET_SCRIPTDATA:
            msgQueue.mtsafe_push( std::pair< DNID, void* >( qwDnid, pMsg ) );
			return false;
		case _SDataBaseMsg::EPRO_WRITE_POINT_LOG:
			msgQueue.mtsafe_push(std::pair< DNID, void* >(qwDnid, pMsg));
			return false;
		case _SDataBaseMsg::EPRO_DB_Mail_SANGUO://邮件相关
			//wk 20150609
			msgQueue.mtsafe_push(std::pair< DNID, void* >(qwDnid, pMsg));
			return false;

			/*switch (((SMailMsg *)pBaseMsg)->_protocol)
				{
				case SMailMsg::SendMail:
				_g::_p::SendMail_SanGuo(qwDnid, reinterpret_cast<S2D_SendMail_MSG*>(pBaseMsg));
				return true;
				case SMailMsg::GetMailInfo:
				_g::_p::GetMailInfoFromDB(qwDnid, reinterpret_cast<SQ_GetMailInfo_MSG*>(pBaseMsg));
				return true;
				case SMailMsg::GetMailGoods:
				_g::_p::GetMailAwards_DB_SANGUO(qwDnid, reinterpret_cast<SQ_GetAwards_MSG*>(pBaseMsg));
				return true;
				case SMailMsg::SetMailReadState:
				_g::_p::SetMailState_DB_SANGUO(reinterpret_cast<SQ_SetMailState_MSG*>(pBaseMsg));
				return true;
				case SMailMsg::SendMail_sys:
				_g::_p::SendMailSYS_SanGuo(reinterpret_cast<S2D_SendMailSYS_MSG*>(pBaseMsg));
				return true;

				}*/
        }
    }
    else if ( ( pBaseMsg->_protocol == SMessage::EPRO_MAIL_MESSAGE ) || 
		( pBaseMsg->_protocol == SMessage::EPRO_TONG_MESSAGE ) || 
        ( pBaseMsg->_protocol == SMessage::EPRO_UNION_MESSAGE ) || 
        ( pBaseMsg->_protocol == SMessage::EPRO_BUILDING_MESSAGE ) )
    {
        msgQueue.mtsafe_push( std::pair< DNID, void* >( qwDnid, pMsg ) );
    }
    else if ( ( pBaseMsg->_protocol == SMessage::EPRO_ACCOUNT_MESSAGE ) && 
        ( ( ( SAccountBaseMsg* )pBaseMsg )->_protocol == SAccountBaseMsg::EPRO_REGISTER_LOGINSERVER ) )
    {
        SQRegisterLogsrvMsg *msg = ( SQRegisterLogsrvMsg* )pBaseMsg;

        BYTE lo = ( BYTE )LOWORD( msg->IP ), hi = ( BYTE )HIWORD( msg->IP );
        if ( ( lo != 0 ) && ( ( lo > 8 ) || ( hi >= lo ) || ( ( lo & ~( lo - 1 ) ) != lo ) ) )
        {
            rfalse( 2, 1, "登陆服务器传过来的群集分组基本信息有误！" );
        }
        else
        {
            // 专门用于控制服务器一拖多时的群集分组编号问题！
            static DWORD clusterSlots[8] = { 0,0,0,0, 0,0,0,0 };
            static DWORD clusterMax = lo;

            if ( ( clusterMax != lo ) || ( ( clusterSlots[hi] != 0 ) && ( clusterSlots[hi] != msg->ID ) ) )
                rfalse( 2, 1, "登陆服务器传过来的群集分组信息和之前的设定存在冲突！" );
            else
            {
                clusterSlots[hi] = msg->ID;
                return true;
            }
        }

        m_NetProviderModule.DelOneClient( qwDnid );
        return true;
    }
    else
    {
        // 返回TRUE！需要删除这个消息数据
        return !rfalse( 2, 1, "意外的消息类型" );
    }

    // 标记在该函数结束后不要删除pMsg
    return false;
}

void RpcProcedure( mysql::Query &query, std::pair< DNID, void* > &element )
{
    dwt::AutoDelete< void* > autorelease( element.second );
    SMessage *baseMsg = ( SMessage* )element.second;
    SServerCtrlMsg *ctrlMsg = ( SServerCtrlMsg* )baseMsg;
    SQGameServerRPCOPMsg *rpcMsg = ( SQGameServerRPCOPMsg* )ctrlMsg;

    if ( ( baseMsg->_protocol != SMessage::EPRO_SERVER_CONTROL ) ||
        ( ctrlMsg->_protocol != SServerCtrlMsg::EPRO_GAMESERVER_REQUEST_RPCOP ) )
        return;

    // GetApp().m_RPCDB.KeepAlive();

    // m_RPCDB只可能在 RPC 线程内使用
    // GetApp().m_RPCDB.RpcProcedure( element.first, rpcMsg->streamData, rpcMsg->dnidClient );
    reinterpret_cast< CNetDBModuleEx& >( query ).RpcProcedure( element.first, rpcMsg->streamData, rpcMsg->dnidClient );
}

void MsgProcedure( mysql::Query &query, std::pair< DNID, void* > &element )
{
	dwt::AutoDelete< void* > autorelease( element.second );

	DNID qwDnid = element.first;
	SMessage *pBaseMsg = (SMessage *)element.second;

    // GetApp().m_RPCDB.KeepAlive();

    if ( pBaseMsg->_protocol == SMessage::EPRO_MAIL_MESSAGE )
	{
		//switch ( ( ( SMailBaseMsg* )pBaseMsg )->_protocol )
		//{
			//CASE_CALL( SMailBaseMsg::EPRO_MAIL_SEND,    SendMail,   SQMailSendMsg   );
			//CASE_CALL( SMailBaseMsg::EPRO_MAIL_RECV,    RecvMail,   SQMailRecvMsg   );
			//CASE_CALL( SMailBaseMsg::EPRO_MAIL_DELETE,  DeleteMail, SQMailDeleteMsg );
			//CASE_CALL( SMailBaseMsg::EPRO_MAIL_NEWMAIL, NewMail,    SQNewMailMsg    );
		//}
	}

	else if ( pBaseMsg->_protocol == SMessage::EPRO_TONG_MESSAGE )
	{
		STongBaseMsg *pFacMsg = (STongBaseMsg*)pBaseMsg;
		if( pFacMsg->_protocol == STongBaseMsg::EPRO_FACTIONBBS_MESSAGE )
		{
			switch ( ( ( SFactionBBSMsg* )pFacMsg )->_protocol )
			{
			case SFactionBBSMsg::EPRO_FACTIONBBS_GET:
				//reinterpret_cast< CNetDBModule * >( &query )->GetFacBBS( qwDnid, (SQGetFacBBSMsg*)pBaseMsg );
				break;
			case SFactionBBSMsg::EPRO_FACTIONBBS_SAVE:
				//reinterpret_cast< CNetDBModule * >( &query )->SaveFacBBS( qwDnid, (SQSaveFacBBSMsg*)pBaseMsg );
				break;
			case SFactionBBSMsg::EPRO_FACTIONBBS_DELETE:
				//reinterpret_cast< CNetDBModule * >( &query )->DelFacBBS( qwDnid, (SQDelFacBBSMsg*)pBaseMsg );
				break;
			case SFactionBBSMsg::EPRO_FACTIONBBS_GET_TEXT:
				//reinterpret_cast< CNetDBModule * >( &query )->GetFacBBSText( qwDnid, (SQGetFacBBSTextMsg*)pBaseMsg );
				break;
			case SFactionBBSMsg::EPRO_FACTIONBBS_SET_TOP:
				//reinterpret_cast< CNetDBModule * >( &query )->SetFacBBSTop( qwDnid, (SQSetFacBBSTopMsg*)pBaseMsg );
				break;
			}
		}
	}

	else if ( pBaseMsg->_protocol == SMessage::EPRO_DATABASE_MESSAGE ) 
    {
		switch ( ( (_SDataBaseMsg *)pBaseMsg )->_protocol ) 
		{
		case _SDataBaseMsg::EPRO_GET_FACTION:
            //reinterpret_cast< CNetDBModuleEx& >( query ).GetFactions( qwDnid, ( ( SQGetFactionMsg* )pBaseMsg )->nServerId );
			//reinterpret_cast< CNetDBModuleEx& >(query).GetFactions_New(qwDnid, ((SQGetFactionMsg*)pBaseMsg)->nServerId);
			reinterpret_cast< CNetDBModuleEx& >(query).GetFactions_Data(qwDnid, ((SQGetFactionMsg*)pBaseMsg)->nServerId);
            break;

        case _SDataBaseMsg::EPRO_SAVE_FACTION:
            //reinterpret_cast< CNetDBModuleEx& >( query ).SaveFactions( ( SQSaveFactionMsg* )pBaseMsg );
			reinterpret_cast< CNetDBModuleEx& >(query).SaveFactionsData((SQSaveFactionDataMsg*)pBaseMsg);
            break;
			
		case _SDataBaseMsg::EPRO_DELETE_FACTION:
			reinterpret_cast<CNetDBModuleEx&>(query).DeleteFaction(qwDnid,(SQDeleteFaction*)pBaseMsg);
			break;

		case _SDataBaseMsg::EPRO_SAVE_SCRIPTDATA:
			reinterpret_cast<CNetDBModuleEx&>(query).SaveScriptData((SQScriptData*)pBaseMsg);
			break;

		case _SDataBaseMsg::EPRO_GET_SCRIPTDATA:
			reinterpret_cast<CNetDBModuleEx&>(query).GetScriptData(qwDnid,(SQGetScriptData*)pBaseMsg);
			break;

		case _SDataBaseMsg::EPRO_WRITE_POINT_LOG://20150408 wk
			reinterpret_cast<CNetDBModuleEx&>(query).WritePointLog((SQPointLogMsg*)pBaseMsg); 
			break;
		case _SDataBaseMsg::EPRO_DB_Mail_SANGUO://20150408 wk

			switch (((SMailMsg *)pBaseMsg)->_protocol)
			{
			case SMailMsg::SendMail:
				reinterpret_cast<CNetDBModuleEx&>(query).SendMail( reinterpret_cast<S2D_SendMail_MSG*>(pBaseMsg));
				break;
			case SMailMsg::GetMailInfo:
				reinterpret_cast<CNetDBModuleEx&>(query).GetMailInfo_DB(qwDnid, reinterpret_cast<SQ_GetMailInfo_MSG*>(pBaseMsg));
				break;
			case SMailMsg::GetMailGoods:
				reinterpret_cast<CNetDBModuleEx&>(query).GetMailAwards_DB(qwDnid, reinterpret_cast<SQ_GetAwards_MSG*>(pBaseMsg));
				break;
			case SMailMsg::SetMailReadState:
				reinterpret_cast<CNetDBModuleEx&>(query).SetMailState_DB(reinterpret_cast<SQ_SetMailState_MSG*>(pBaseMsg));
				break;
			case SMailMsg::SendMail_sys:
				reinterpret_cast<CNetDBModuleEx&>(query).SendMail_sys(reinterpret_cast<S2D_SendMailSYS_MSG*>(pBaseMsg));
				break;

			}

			//SMailMsg *_SMailMsg = (SMailMsg *)pBaseMsg;
			//			//GetMailInfoFromDB(qwDnid, reinterpret_cast<SQ_GetMailInfo_MSG* >(pBaseMsg));
			//reinterpret_cast<CNetDBModuleEx&>(query).GetMailInfo_DB(qwDnid, (SQ_GetMailInfo_MSG*)_SMailMsg);
			break;
        }
    }

	//这次建筑消息我就直接从区域上的消息来处理了，以前的做法是通过登录服务器转化成EPRO_DATABASE_MESSAGE
	//感觉这样的操作对我的序列化流来说是没有必要的，所以直接用区域上定义的消息
	else if( pBaseMsg->_protocol == SMessage::EPRO_BUILDING_MESSAGE ) 
	{
		switch ( ( (SBuildingBaseMsg *)pBaseMsg )->_protocol ) 
		{
		case SBuildingBaseMsg::EPRO_BUILDING_SAVE:
			reinterpret_cast< CNetDBModuleEx& >( query ).SaveBuildings( ( SQBuildingSaveMsg* )pBaseMsg );
			break;
			
		case SBuildingBaseMsg::EPRO_BUILDING_GET:
			reinterpret_cast< CNetDBModuleEx& >( query ).GetBuildings( qwDnid, ( SQBuildingGetMsg* )pBaseMsg );
			break;
		}
	}
    else if( pBaseMsg->_protocol == SMessage::EPRO_UNION_MESSAGE ) 
    {
        switch ( ( (SUnionBaseMsg *)pBaseMsg )->_protocol ) 
        {
        case SUnionBaseMsg::EPRO_GET_UNIONDATA:
            reinterpret_cast< CNetDBModuleEx& >( query ).GetUnionData( qwDnid, ( SQGetUnionDataMsg* )pBaseMsg );
            break;
        case SUnionBaseMsg::EPRO_SAVE_UNIONDATA:
             reinterpret_cast< CNetDBModuleEx& >( query ).SaveUnionDatas( ( SQSaveUnionDataMsg* )pBaseMsg );
            break;
        }
    }
	else
	{
		//LogErrorMsg( "异常的消息结构在MsgProcedure队列中." );
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CDAppMain::PrintTimeLog() {}

void CDAppMain::SaveAll( int b ) {}

void CDAppMain::DisplayInfo() 
{
    extern void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    _DisplayInfo( msgQueue.size(), rpcQueue.size(), m_NetProviderModule.GetQueueSize() );
}
void CDAppMain::test(string str)
{
	_g::_p::test(str);
	
	//AddInfo("\n");
}