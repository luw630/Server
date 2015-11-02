#include "StdAfx.h"
#include ".\centermodule.h"
#include <boost/bind.hpp>
#include "networkmodule\CenterMsg.h"
#include "networkmodule\SectionMsgs.h"
#ifdef LOGINSERVERMACORS
#include "../../登录服务器/登录服务器/DMainApp.h"
#endif // !ZONESERVER



CenterModule::CenterModule(void)
{
}

CenterModule::~CenterModule(void)
{
}

int CenterModule::Execution()
{
	CNetConsumerModule::Execution( boost::bind(&CenterModule::Entry, this, _1, _2 ) );
	return 1;
}

bool CenterModule::Entry( void* data, size_t size )
{
	SMessage *pMsg = (SMessage *)data;
	if (pMsg)
	{
		if(pMsg->_protocol == SMessage::ERPO_SECTION_MESSAGE)
		{
			SMessage *srcMsg = SectionMessageManager::getInstance().recvMessageWithSection((SSectionMsg*)pMsg);
			if (srcMsg)
			{
				bool ret = Entry(srcMsg,((SSectionMsg*)pMsg)->byCount*SSectionMsg::MAX_SIZE);
				SectionMessageManager::getInstance().popMessage(srcMsg);
				return ret;
			}
		}
	}

	if (pMsg == NULL)
	{
		// 假设为网闪断！
		// 在这里重复几次连接
		//for (int i=0; i<10; i++)
		//{
		//	// 这里要把自己注册到登陆服务器上去
		//	Sleep(1000);

		//	if ( !Connect( GetApp()->szLoginIP, GetApp()->szLoginPORT ) )
		//	{
		//		rfalse(2, 1, "无法连接登陆服务器！！！");
		//		continue;
		//	}

		//	// 向服务器注册本机
		//	SQLoginMsg msg;

		//	try
		//	{
		//		lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
		//		slm( GetApp()->m_szName )( "123456789" );

		//		msg.wVersion = MAKEWORD( 0, GetApp()->m_ServerVersion );
		//		SendMessage( &msg, (WORD)(sizeof(msg) - slm.EndEdition()) );
		//	}
		//	catch ( lite::Xcpt & )
		//	{
		//		return true;
		//	}

		//	return true;
		//}

		//// if check server disconnected this link, get in here
		//GetApp()->m_eLoginStatus = CDAppMain::SS_BREAK;
		return true;
	}

	switch (pMsg->_protocol)
	{
	case SMessage::EPRO_Center_MESSAGE:
		{
			//__asm nop;
			//rfalse(2, 1, "中心服务器注册成功");
			SCenterBaseMsg* msg = static_cast< SCenterBaseMsg* >(pMsg);
			switch ( msg->_protocol )
			{
			case SRegisterServer::EPRO_RegisterServer:
				{
					SARegisterServer* _msg = static_cast< SARegisterServer* >( msg );
					if ( _msg->code == SARegisterServer::ERC_Register_Success )
					{
						rfalse(2, 1, "ZoneServer To CenterServer Register ok");
					}
					else
					{
						rfalse(2, 1, "ZoneServer To CenterServer Register Fail");
					}
				}
				break;
			case SRegisterServer::EPRO_RegisterLoginServer:
			{
				SARegisterLoginServer* _msg = static_cast<SARegisterLoginServer*>(msg);
				if (_msg->code == SARegisterLoginServer::ERC_Register_Success)
				{
					rfalse(2, 1, "LoginServer To CenterServer Register ok");
				}
				else
				{
					rfalse(2, 1, "LoginServer To CenterServer Register Fail");
				}
			}
			break;
			case SRegisterServer::EPRO_RegisterDBServer:
			{
				SARegisterDBServer* _msg = static_cast<SARegisterDBServer*>(msg);
				if (_msg->code == SARegisterDBServer::ERC_Register_Success)
				{
					rfalse(2, 1, "DBServer To CenterServer Register ok");
				}
				else
				{
					rfalse(2, 1, "DBServer To CenterServer Register Fail");
				}
			}
			break;
			case SRegisterServer::EPRO_CheckWhiteList:
			{

				SACheckWhiteList* _msg = static_cast<SACheckWhiteList*>(msg);
				if (_msg)
				{
					OnCheckWhiteList(_msg);
					//GetApp()->m_PlayerManager.OnCheckAccountServer(_msg->dnidPlayerClient, _msg->streamData, _msg->strpassword, _msg->bresult);
				}
				
			}
			break;
			default:
				{

				}
				break;
			}

		}
		break;
	}
	return true;
}

BOOL CenterModule::SendToCenterServer(void* pMsg, WORD iSize)
{
	if (pMsg == NULL) 
		return FALSE;

	return SendMessage(pMsg, iSize);
}

BOOL CenterModule::OnCheckWhiteList(struct SACheckWhiteList *pMsg)
{
#ifdef LOGINSERVERMACORS
extern CDAppMain *&GetApp(void);
	if (pMsg)
	{
		GetApp()->m_PlayerManager.OnCheckAccountServer(pMsg->dnidPlayerClient, pMsg->bresult, pMsg);
	}
#endif
	return TRUE;
}

bool CenterModule::OnLogout()
{
	SQLogoutServer  sqlogout;
	return SendToCenterServer(&sqlogout, sizeof(SQLogoutServer));
}
























