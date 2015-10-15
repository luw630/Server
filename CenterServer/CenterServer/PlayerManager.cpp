#include "StdAfx.h"
#include <boost/bind.hpp>
#include ".\playermanager.h"
#include "../../networkmodule/CenterMsg.h"
#include "../../networkmodule/LogMsgs.h"
#include "liteserializer/lite.h"
#include "DMainApp.h"
extern CDAppMain &GetApp(void);

PlayerManager::PlayerManager(void)
	: m_ServerModule(TRUE), m_dPlayerConnectNum(0)
{
	
}

PlayerManager::~PlayerManager(void)
{
	Destroy();
}

BOOL PlayerManager::Host( LPCSTR port )
{
	return m_ServerModule.Create( port );
}

void PlayerManager::Destroy()
{
	m_ServerModule.Destroy();
}

void PlayerManager::Execution()
{
	m_ServerModule.Execution(boost::bind(&PlayerManager::Entry, this, _1, _2, _3, _4));
}

bool PlayerManager::Entry( DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size )
{
	//这里收到玩家启动客户端的消息，发送服务器状态
	SMessage *pMsg = (SMessage *)data;
	if ( pMsg == NULL )
	{
		return true;
	}

	//给客户端发送服务器状态数据

	if ( pMsg->_protocol == pMsg->EPRO_Center_MESSAGE )
	{
		SCenterBaseMsg* _pMsg = static_cast< SCenterBaseMsg* >( pMsg );
		switch (_pMsg->_protocol)
		{
		case SCenterBaseMsg::EPRO_RequestClientVersionInfo: //返回当前游戏资源更新信息
		{
			SARequestAssetUpdateInfo _msg;
			CDAppMain& appMain = GetApp();
			_msg.streamLength = appMain.m_iAssetBundleInfoLength;
			memcpy(_msg.streamData, appMain.m_AssetbundleVersionInfo, appMain.m_iAssetBundleInfoLength);
			SendToClient(dnidClient, &_msg, sizeof(SARequestAssetUpdateInfo) - (MAX_ASSET_SIZE - appMain.m_iAssetBundleInfoLength));

			//rfalse(2, 1, "");
		}
			break;
		case SCenterBaseMsg::EPRO_RequestServerState:
		{
			//接受请求数据
			SARequestServerState _msg;
			bool b = GetApp().m_ServerManager[GAMESERVER_FLAG].GetGameServerList(&_msg);
			if (b)
			{
				SendToClient(dnidClient, &_msg, _msg.MySize());

				m_dPlayerConnectNum++;
				char string[100] = {};
				sprintf_s(string, "\nPlayeConnectCenterServer: %d  ", m_dPlayerConnectNum);
				GetApp().SaveConnectNumString(string);
			}
		}
			break;
		case SCenterBaseMsg::EPRO_RequestServerDetailInfo:
		{
			SQRequestServeDetailInfoMsg *psrcMsg = static_cast<SQRequestServeDetailInfoMsg*>(pMsg);
			if (NULL == psrcMsg)
				break;
			ServerData *pServerData = NULL;
			GameServer *pGameServer = NULL;
			if (GetApp().m_ServerManager[GAMESERVER_FLAG].FindGameServerByIndex(psrcMsg->dwIndex, &pGameServer))
			{
				if (NULL != pGameServer)
				{
					pServerData = pGameServer->GetServerData();
				}
			}
			if (NULL != pServerData)
			{
				SARequestServeDetailInfoMsg msg;
				msg.dwIndex = pServerData->m_dwIndex;
				lite::Serializer sl(msg.streamData, sizeof(msg.streamData));
				sl(pServerData->m_dwIP)(pServerData->m_dwPort);
				SendToClient(dnidClient, &msg, sizeof(msg) - sl.EndEdition());
			}
		}
			break;
		default:
			break;
		}
	}
	else if (pMsg->_protocol == SMessage::EPRO_SYSTEM_MESSAGE)
	{
		SQLoginMsg* _pMsg = static_cast<SQLoginMsg*>(pMsg);
	}

	return true;
}

void PlayerManager::SendToClient( DNID dnidClient , const void* pMsg, size_t size )
{
	m_ServerModule.SendMessage( dnidClient , pMsg , static_cast<WORD>( size ) );
}


















