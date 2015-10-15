#include "StdAfx.h"
#include <boost/bind.hpp>
#include ".\servermanager.h"
#include "NetWorkModule/logmsgs.h"
#include "networkmodule/CenterMsg.h"
#include "liteserializer/lite.h"
#include "DMainApp.h"
#include "Globals.h"

extern void AddInfo( LPCSTR Info );

ServerManager::ServerManager(void) :m_dZonsServerPlayerNum(0)
{
}

ServerManager::~ServerManager(void)
{
	RemoveAllGameServer();
	Destroy();
}

BOOL ServerManager::Host( LPCSTR port )
{
	return m_ServerModule.Create( port );
}

void ServerManager::Execution()
{
	//处理网络协议
	m_ServerModule.Execution(boost::bind(&ServerManager::Entry, this, _1, _2, _3, _4));
}

void ServerManager::Destroy()
{
	m_ServerModule.Destroy();
}

bool ServerManager::Entry( DNID dnidClient, LINKSTATUS enumStatus, void *data, size_t size )
{
	SMessage *pMsg = (SMessage *)data;
	if (pMsg == NULL)
	{
		// 说明该连接断开
		// 这里处理内部服务器的断线
		for (size_t i = 0; i < MAX_MANAGE_SIZE;i++)
		{
			GameServerMap::iterator it = m_GameServerMap[i].find(dnidClient);
			if (it == m_GameServerMap[i].end())
			{
				// 如果这个联接和游戏服务器无关，就不用管他
				continue;
			} 
			char _temp[512];
			sprintf(_temp, "[%s]connected fail \r\n", it->second.GetServerName().c_str());
			//AddInfo(_temp);
			OnServerconnectedFail(_temp);
			LogoutGameServer(dnidClient,i);

		}
// 		GameServerMap::iterator it = m_GameServerMap.find(dnidClient);
// 		if (it == m_GameServerMap.end())
// 		{
// 			// 如果这个联接和游戏服务器无关，就不用管他
// 			return true;
// 		}

		// 需要将该服务器上的数据清除

		// 移除该服务器联接相关数据
		//m_GameServerMap.erase(dnidClient);
// 		char _temp[512];
// 		sprintf( _temp , "[%s]已经断开连接\r\n" , it->second.GetServerName().c_str() );
// 		AddInfo( _temp );
// 		LogoutGameServer( dnidClient );
		return true;
	}
	switch (enumStatus)
	{
	case LinkStatus_Waiting_Login:
		{
			// 当处于LinkStatus_Waiting_Login等待认证消息的状态中，只能接受认证消息
			// 先判断是否是登陆认证级的消息
			if (pMsg->_protocol != SMessage::EPRO_Center_MESSAGE)
			{
				m_ServerModule.DelOneClient(dnidClient);
				return true;
			}

			SCenterBaseMsg* msg = static_cast<SCenterBaseMsg*>(pMsg);
			switch (msg->_protocol)
			{
				case SCenterBaseMsg::EPRO_RegisterServer:
				{
					// 开始认证，将状态字移近一步
					m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Login);

					// 这里处理服务器的登陆
					LoginGameServer(dnidClient, static_cast<SQRegisterServer*>(pMsg));
				}
				break;
				case SCenterBaseMsg::EPRO_RegisterLoginServer:
				{
					// 开始认证，将状态字移近一步
					m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Login);
					RegisterLoginServer(dnidClient, static_cast<SQRegisterLoginServer*>(pMsg));
					// 这里处理服务器的登陆
				}
				break;
				case SCenterBaseMsg::EPRO_RegisterDBServer:
				{
					// 开始认证，将状态字移近一步
					m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Login);

					// 这里处理服务器的登陆
					RegisterDBServer(dnidClient, static_cast<SQRegisterDBServer*>(pMsg));
				}
				break;
				default:
					break;
			}
		}
		break;

	case LinkStatus_Login:
		{
			// 当处于等待认证的时候，该连接不允许发送任何消息
			// 所以收到消息就断开该连接
			m_ServerModule.DelOneClient( dnidClient );
			LogoutGameServer( dnidClient );
		}
		break;

	case LinkStatus_Connected:
		{
			// 其它的消息只能在认证以后的状态下使用，否则视为非法(连接断开)
			switch ( pMsg->_protocol )
			{
			case SMessage::EPRO_Center_MESSAGE:
				{
					SCenterBaseMsg* msg = static_cast<SCenterBaseMsg*>( pMsg );
					//玩家登陆到区域服，把玩家的简单数据发过来
					if ( msg->_protocol == SCenterBaseMsg::EPRO_LoginPlayer )
					{
						GameServer* server;
						bool b = FindGameServerByDNID( dnidClient , &server );
						if ( b )
						{
							SQLoginPlayer* _msg = static_cast<SQLoginPlayer*>( pMsg );
							PlayerData _data;
							_data.m_dDindClient = _msg->dnidClient;
							lite::Serialreader sl( _msg->streamData, sizeof( _msg->streamData ));
							_data.m_strPlayerAccount = sl();
							server->AddPlayer( _data );
							m_dZonsServerPlayerNum++;
							char string[100] = {};
							sprintf_s(string, "  PlayeConnectZoneServer :%d ", m_dZonsServerPlayerNum);
							GetApp().SaveConnectNumString(string);
						}
					}
					//玩家离开区域服，删除玩家数据
					else if ( msg->_protocol == SCenterBaseMsg::EPRO_LogoutPlayer )
					{
						GameServer* server;
						bool b = FindGameServerByDNID( dnidClient , &server );
						if ( b )
						{
							SQLogoutPlayer* _msg = static_cast< SQLogoutPlayer* >( pMsg );
							server->RemovePlayerByDNID( _msg->dnidClient );
						}
					}
					else if (msg->_protocol == SCenterBaseMsg::EPRO_LogoutServer)
					{
						for (size_t i = 0; i < MAX_MANAGE_SIZE; i++)
						{
							GameServerMap::iterator it = m_GameServerMap[i].find(dnidClient);
							if (it == m_GameServerMap[i].end())
							{
								// 如果这个联接和游戏服务器无关，就不用管他
								continue;
							}
							char _temp[512];
							sprintf(_temp, "[%s]Send On Logout \r\n", it->second.GetServerName().c_str());
							AddInfo(_temp);
							LogoutGameServer(dnidClient, i);
							break;
						}
					}
					else if (msg->_protocol == SCenterBaseMsg::EPRO_CheckWhiteList) //白名单
					{
						SQCheckWhiteList   *pCheckMsg = static_cast< SQCheckWhiteList* >(pMsg);
						if (pCheckMsg)
						{
							try
							{
								lite::Serialreader sl(pCheckMsg->streamData);
								LPCSTR	account = sl();
								LPCSTR	password = sl();
								QWORD	macAddr = 0;

					
								SACheckWhiteList  sawhiteresult;
								memset(sawhiteresult.streamData, 0, MAX_ACCAPASS);
								memcpy(sawhiteresult.streamData, pCheckMsg->streamData, MAX_ACCAPASS);

								std::string str(account);
								sawhiteresult.bresult = GetApp().bCheckWhiteListIn(str);
								sawhiteresult.dnidPlayerClient = pCheckMsg->dnidPlayerClient;
								m_ServerModule.SendMessage(dnidClient, &sawhiteresult, sizeof(SACheckWhiteList));
							}
							catch (lite::Xcpt&)
							{
								// 解析账号密码字符串失败
								return FALSE;
							}
						}
					}
				}
				break;
			default:
				break;
			}

		}
		break;
	default:
		{

		}
		break;
	}

	return true;
}

bool ServerManager::AddGameServer(GameServer& server, BYTE bserverflag)
{
	if (bserverflag < MAX_MANAGE_SIZE)
	{
		GameServerMap::_Pairib b = m_GameServerMap[bserverflag].insert(GameServerMap::value_type(server.GetServerDNID(), server));
		return b.second;
	}
	return false;
}

bool ServerManager::RemoveGameServerByDnid(DNID dnidClient, BYTE bflag)
{
	GameServerMap::iterator it = m_GameServerMap[bflag].find(dnidClient);
	if (it != m_GameServerMap[bflag].end())
	{
		m_GameServerMap[bflag].erase(it);
		return true;
	}
	return false;
}

void ServerManager::RemoveAllGameServer()
{
	for (size_t i = 0; i < MAX_MANAGE_SIZE;i++)
	{
		if (!m_GameServerMap[i].empty())
		{
			m_GameServerMap[i].clear();
		}
	}
}

void ServerManager::BroadcastAllServer( const void* pMsg, size_t size )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while ( it != m_GameServerMap[GAMESERVER_FLAG].end() )
	{
		m_ServerModule.SendMessage( it->first , pMsg , static_cast<WORD>( size ) );
		++it;
	}
}

void ServerManager::BroadcastToServer( DNID id , const void* pMsg, size_t size )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].find(id);
	if (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		m_ServerModule.SendMessage( it->first , pMsg , static_cast<WORD>( size ) );
	}
}

void ServerManager::BroadcastButServer( DNID id , const void* pMsg , size_t size )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		if ( it->first != id )
		{
			m_ServerModule.SendMessage( it->first , pMsg , static_cast<WORD>( size ) );
		}
		++it;
	}
}

GameServerMap::iterator ServerManager::FindGameServerByDNID( DNID id )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].find(id);
	if (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		return it;
	}
	return it;
}

bool ServerManager::FindGameServerByDNID( DNID id , GameServer** server )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].find(id);
	if (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		*server = &(it->second);
		return true;
	}
	return false;
}

GameServerMap::iterator ServerManager::FindGameServerByIndex( DWORD id )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		if ( id == it->second.GetServerIndex() )
		{
			return it;
		}
		++it;
	}
	return it;
}

bool ServerManager::FindGameServerByIndex( DWORD id , GameServer** server )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		if ( id == it->second.GetServerIndex() )
		{
			*server = &( it->second );
			return true;
		}
		++it;
	}
	return false;
}

GameServerMap::iterator ServerManager::FindGameServerByName( std::string& gameServerStr )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		if ( gameServerStr == it->second.GetServerName() )
		{
			return it;
		}
		++it;
	}
	return it;
}

bool ServerManager::FindGameServerByName( std::string& gameServerStr , GameServer** server )
{
	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		if ( gameServerStr == it->second.GetServerName() )
		{
			*server = &( it->second );
			return true;
		}
		++it;
	}
	return false;
}

BOOL ServerManager::LoginGameServer( DNID dnidClient, SQRegisterServer *pMsg )
{
	SARegisterServer _sendMsg;
	_sendMsg.code = SARegisterServer::ERC_Register_Success;

	LPCSTR account = NULL;
	LPCSTR password = NULL;

	try
	{
		lite::Serialreader sl(pMsg->streamData);
		account = sl();
		password = sl();

		if (strlen(account) < 9)
			return FALSE;
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}

	// 需要从登陆账号中解析出服务器的类型，编号一共16个有效数据
	// 头两个汉字“登陆|区域|聊天|网关|扩展”标识服务器类型，然后跟“_”，另外的为数字标号
	char number[32];
	DWORD ID = 0;
	char _temp[128];
	sprintf( _temp , "[%s]连接失败\r\n" , account );

	memcpy(number, account+5, 4);
	number[4] = 0;
	if (dwt::strcmp("区域", account, 4) == 0)
	{
		ID = 0x10000000 | (0x0fffffff & atoi(number));
	}
	else 
	{

		_sendMsg.code = SARegisterServer::ERC_Register_Fail;
		m_ServerModule.SendMessage( dnidClient , &_sendMsg , sizeof( _sendMsg ) );
		m_ServerModule.DelOneClient(dnidClient);
		AddInfo( _temp );
		return FALSE;
	}

	// 服务器在这里通过连接认证？！
	m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Connected);

	m_ServerModule.SendMessage(dnidClient, &_sendMsg, sizeof(_sendMsg));

	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].find(dnidClient);
	if (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		return FALSE;
	}
	else
	{
		ServerData data;
		data.m_strServerName = account;
		data.m_dDindClient = dnidClient;
		data.m_dwGid = ID;
		data.m_dwIndex = pMsg->index;
		data.m_dwNew = pMsg->_dNew;
		data.m_dwActivity = pMsg->_dActivity;
		data.m_dwRush = pMsg->_dRush;

		data.m_dwIP = pMsg->dwIP; // dj + 20121226
		data.m_dwPort = pMsg->dwPort;
		GameServer server(data);
		AddGameServer( server,GAMESERVER_FLAG );
	}
	
	sprintf( _temp , "[%s]连接成功\r\n" , account );
	AddInfo( _temp );
	return TRUE;
}

void ServerManager::LogoutGameServer(DNID dnidClinet, BYTE bflag)
{
	if (bflag < MAX_MANAGE_SIZE)
	{
		RemoveGameServerByDnid( dnidClinet,bflag );
	}
	
}

bool ServerManager::GetGameServerList( struct SARequestServerState* pMsg )
{
	
	if (m_GameServerMap[GAMESERVER_FLAG].empty())
	{
		return false;
	}

	GameServerMap::iterator it = m_GameServerMap[GAMESERVER_FLAG].begin();
	
	//unsigned long num = m_GameServerMap.size();
	
	//ServerState ss;
	pMsg->wSaveCount = 0;
	while (it != m_GameServerMap[GAMESERVER_FLAG].end())
	{
		pMsg->atServerData[pMsg->wSaveCount].dwIndex = it->second.GetServerData()->m_dwIndex;
		pMsg->atServerData[pMsg->wSaveCount].dwTotalPlayers = it->second.GetServerData()->m_wTotalPlayers;
		pMsg->atServerData[pMsg->wSaveCount].dwNew = it->second.GetServerData()->m_dwNew;
		pMsg->atServerData[pMsg->wSaveCount].dwRush = it->second.GetServerData()->m_dwRush;
		pMsg->atServerData[pMsg->wSaveCount].dwActity = it->second.GetServerData()->m_dwActivity;		
		++it;
		++pMsg->wSaveCount;
		if (pMsg->wSaveCount >= SARequestServerState::csiMaxSaveCount)
		{
			AddInfo("需要发送的服务器状态数据太大了");
			break;
		}
	}	
	return true;
}

BOOL ServerManager::RegisterLoginServer(DNID dnidClient, struct SQRegisterLoginServer* pMsg)
{
	SARegisterLoginServer _sendMsg;
	_sendMsg.code = SARegisterLoginServer::ERC_Register_Success;

	LPCSTR account = NULL;
	try
	{
		lite::Serialreader sl(pMsg->streamservername);
		account = sl();

		if (strlen(account) < 1)
			return FALSE;
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}

	// 需要从登陆账号中解析出服务器的类型，编号一共16个有效数据
	// 头两个汉字“登陆|区域|聊天|网关|扩展”标识服务器类型，然后跟“_”，另外的为数字标号
	char number[32];
	DWORD ID = 0;
	char _temp[128];
	sprintf(_temp, "[%s]连接失败\r\n", account);

	memcpy(number, account + 5, 4);
	number[4] = 0;
	if (dwt::strcmp("登陆", account, 4) == 0)
	{
		ID = 0x10000000 | (0x0fffffff & atoi(number));
	}
	else
	{
		_sendMsg.code = SARegisterLoginServer::ERC_Register_Fail;
		m_ServerModule.SendMessage(dnidClient, &_sendMsg, sizeof(_sendMsg));
		m_ServerModule.DelOneClient(dnidClient);
		AddInfo(_temp);
		return FALSE;
	}

	// 服务器在这里通过连接认证？！
	m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Connected);

	m_ServerModule.SendMessage(dnidClient, &_sendMsg, sizeof(_sendMsg));

	GameServerMap::iterator it = m_GameServerMap[LOGINERVER_FLAG].find(dnidClient);
	if (it != m_GameServerMap[LOGINERVER_FLAG].end())
	{
		return FALSE;
	}
	else
	{
		ServerData data;
		data.m_strServerName = account;
		data.m_dDindClient = dnidClient;
		data.m_dwGid = ID;
		data.m_dwIndex = pMsg->index;
		data.m_dwNew = pMsg->_dNew;
		data.m_dwActivity = pMsg->_dActivity;
		data.m_dwRush = pMsg->_dRush;

		
		data.m_dwIP = pMsg->dwIP; // dj + 20121226
		data.m_dwPort = pMsg->dwPort;
		GameServer server(data);
		AddGameServer(server,LOGINERVER_FLAG);
	}

	sprintf(_temp, "[%s]连接成功\r\n", account);
	AddInfo(_temp);
	return TRUE;
}

BOOL ServerManager::RegisterDBServer(DNID dnidClient, struct SQRegisterDBServer* pMsg)
{
	SARegisterDBServer _sendMsg;
	_sendMsg.code = SARegisterDBServer::ERC_Register_Success;

	LPCSTR account = NULL;
	try
	{
		lite::Serialreader sl(pMsg->streamservername);
		account = sl();

		if (strlen(account) < 5)
			return FALSE;
	}
	catch (lite::Xcpt &)
	{
		return FALSE;
	}

	// 需要从登陆账号中解析出服务器的类型，编号一共16个有效数据
	// 头两个汉字“登陆|区域|聊天|网关|扩展”标识服务器类型，然后跟“_”，另外的为数字标号
	char number[32];
	DWORD ID = 0;
	char _temp[128];
	sprintf(_temp, "[%s]连接失败\r\n", account);

	memcpy(number, account + 5, 4);
	number[4] = 0;
	if (dwt::strcmp("数据", account, 4) == 0)
	{
		ID = 0x10000000 | (0x0fffffff & atoi(number));
	}
	else
	{
		_sendMsg.code = SARegisterDBServer::ERC_Register_Fail;
		m_ServerModule.SendMessage(dnidClient, &_sendMsg, sizeof(_sendMsg));
		m_ServerModule.DelOneClient(dnidClient);
		AddInfo(_temp);
		return FALSE;
	}

	// 服务器在这里通过连接认证？！
	m_ServerModule.SetLinkStatus(dnidClient, LinkStatus_Connected);

	m_ServerModule.SendMessage(dnidClient, &_sendMsg, sizeof(_sendMsg));

	GameServerMap::iterator it = m_GameServerMap[DBSERVER_FLAG].find(dnidClient);
	if (it != m_GameServerMap[DBSERVER_FLAG].end())
	{
		return FALSE;
	}
	else
	{
		ServerData data;
		data.m_strServerName = account;
		data.m_dDindClient = dnidClient;
		data.m_dwGid = ID;
		data.m_dwIndex = pMsg->index;
		data.m_dwNew = pMsg->_dNew;
		data.m_dwActivity = pMsg->_dActivity;
		data.m_dwRush = pMsg->_dRush;


		data.m_dwIP = pMsg->dwIP; // dj + 20121226
		data.m_dwPort = pMsg->dwPort;
		GameServer server(data);
		AddGameServer(server, DBSERVER_FLAG);
	}

	sprintf(_temp, "[%s]连接成功\r\n", account);
	AddInfo(_temp);
	return TRUE;
}

int ServerManager::OnServerconnectedFail(const char* strServerAlam)
{
	AddInfo(strServerAlam);
	char stralam[512] = {};
	sprintf_s(stralam, "cagent_tools alarm '%s' ", strServerAlam);
	return system(stralam);
}







