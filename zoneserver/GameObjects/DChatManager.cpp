#include "StdAfx.h"
#include "dchatmanager.h"
#include "Area.h"
#include "Player.h"
#include "prisonex.h"
#include "Random.h"

#include "playermanager.h"
#include "networkmodule/orbmsgs.h"
#include "networkmodule/chatmsgs.h"
#include "networkmodule/scriptmsgs.h"
#include "gameobjects/factionmanager.h"
#include "Gameworld.h"
#include "ScriptManager.h"
#include "Prison.h"
#include "SanguoCode/BaseDataManager.h"
#include "../pub/ConstValue.h"
#include <list>
#include <time.h>
using namespace std;
#define UNDISABLE_ORB
extern CGameWorld *&GetGW();
extern LPIObject GetPlayerByName(LPCSTR szName);

static WORD MakeBroadcastMsg( LPVOID msgBuffer, DWORD msgSize, LPCVOID transData, WORD transSize )
{
    if ( ( transSize > 0xf000 ) || ( DWORD( transSize + 100 ) > msgSize ) )
        return 0;   // 太大的包了！

    struct DummyMsg : public SORBDataTransMsg
    {
        WORD m_wTransType;
        WORD m_wTransSize;
		char m_szTransName[CONST_USERNAME];
        char m_pTransData[1];
    };

    DummyMsg *tempMsg = new ( msgBuffer ) DummyMsg();
    tempMsg->dnidClient = -1;
    tempMsg->m_wTransType = SQAORBDataTransMsg::TRANSDATATO_ALLSERVER;
    tempMsg->m_wTransSize = transSize;
    *( LPWORD )tempMsg->m_szTransName = 1;

    memcpy( tempMsg->m_pTransData, transData, transSize );

    return sizeof( SQAORBDataTransMsg ) + transSize;
}

std::map<std::string, DWORD> GMTalkMask;      // 被GM禁言的玩家映射表
std::map<std::string, DWORD> KingTalkMask;    // 被城主禁言的玩家映射表
std::map<std::string, DWORD> MasterTalkMask;  // 被掌门人禁言的玩家映射表

// 世界聊天-+---------------------
struct TempData
{
	DWORD color;
	char buffer[MAX_CHAT_LEN];
	char name[CONST_USERNAME];
};

#define GLOBAL_MAX_MSG_NUM 30
std::list< TempData >  globalMsg;  // 世界聊天消息

DWORD globalMsgTime;
DWORD golbal_time = 1000 * 10;

// 聊天时间间隔限制的全局变量
// 附近、门派、帮会、传闻发言间隔皆为3秒
// 队伍、私聊发言间隔为1秒,队伍单独处理
const DWORD DeltaThree_time = 1000 * 3;
const DWORD DeltaOne_time	= 1000 * 1;
const DWORD DeltaThirty_time = 1000 ; 

const DWORD dwRumorNeedMoney = 100;		// 使用传闻频道需要消耗的侠义币  
const DWORD dwSmallHornIndex = 8001001; // 小喇叭道具索引

CDChatManager::CDChatManager(void)
{
}

CDChatManager::~CDChatManager(void)
{
	globalMsg.clear();
}

void CDChatManager::Initialize()
{
    wChatCount = 0;
	globalMsgTime = timeGetTime();

	globalMsg.clear();
}

LPCSTR GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf(buffer, "%d-%d-%d %d:%d:%d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);

    return buffer;
}

LPCSTR _GetStringTime()
{
    static char buffer[1024];

    SYSTEMTIME s;
    GetLocalTime(&s);

    sprintf( buffer, "[%d-%d-%d %d:%d:%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond );

    return buffer;
}

void CDChatManager::OnGolbalMsg()
{
	if( ( timeGetTime() - globalMsgTime ) > golbal_time )
	{
		globalMsgTime = timeGetTime();
		// 发送消息...
		std::list< TempData >::iterator it = globalMsg.begin();
		if( globalMsg.size() > 0)
		{
			CGameWorld *tempWorld = static_cast<CGameWorld*>(this);
			if( !tempWorld )
				return;

			SAChatGlobalMsg msg;

			msg.byType = SAChatGlobalMsg::ECT_SMALL_HORN;
			memset(msg.cTalkerName, 0, CONST_USERNAME);
			memset( msg.cChatData, 0, MAX_CHAT_LEN );
			memcpy(msg.cTalkerName, it->name, CONST_USERNAME);
			memcpy( msg.cChatData, it->buffer, MAX_CHAT_LEN );
			msg.color = it->color;
			BroadcastMsg( &msg, msg.GetMySize() );
 
			globalMsg.erase(it);
		}
	}
}

BOOL CDChatManager::ProcessChatMsg(DNID dnidClient, SChatBaseMsg *pChatMsg, CPlayer *pPlayer)
{
	if (!pChatMsg || !pPlayer) 
        return FALSE;

    bool isGMMask     = false;		// GM禁言
    bool isKingMask   = false;		// 城主禁言
    bool isMasterMask = false;		// 掌门人禁言
	int  ret = 0;

/*
    {
        extern int GetAccountLimit(CPlayer *);
        extern int CheckTemp(CPlayer *pPlayer);

        ret = GetAccountLimit(pPlayer);

        if (ret == 0)
        {
            // 没有权限的普通玩家，处理部分

            //// 监狱里边不能说话
            //if (pChatMsg->_protocol != SChatBaseMsg::EPRO_CHAT_PUBLIC)
            //{
            //    if (pPlayer->m_ParentRegion != NULL)
            //    {
            //        if (pPlayer->m_ParentRegion->DynamicCast(IID_PRISON) != NULL ||
            //            pPlayer->m_ParentRegion->DynamicCast(IID_PRISONEX) != NULL )
            //        {
            //            TalkToDnid(dnidClient, "阁下身犯重罪，只能和附近的牢友聊天");
            //            return FALSE;
            //        }
            //    }
            //}

            switch (pChatMsg->_protocol)
            {
            case SChatBaseMsg::EPRO_CHAT_PUBLIC: // 附近频道
                if (!pPlayer->CheckCanTalk(CPlayer::CT_FAST, 2))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;

            case SChatBaseMsg::EPRO_CHAT_FACTION:// 帮会频道
                if (!pPlayer->CheckCanTalk(CPlayer::CT_NORMAL, 8))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;

            case SChatBaseMsg::EPRO_CHAT_WHISPER:
                 //玩家密GM必须1分钟后才行
				//SQChatWisperMsg *pTmpChatMsg;
				//pTmpChatMsg = static_cast<SQChatWisperMsg *> (pChatMsg);
				//if (pTmpChatMsg)
				//{
				//	if  (0 == dwt::strcmp(pTmpChatMsg->szRecvName,"GM",2))
				//	{
				//		if (!pPlayer->CheckCanTalk(CPlayer::CT_GMVEERYSLOW, 60))
				//		{
				//			pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
				//			return FALSE;
				//		}
				//	}
				//	//上面这一块是针对玩家密GM太频繁做的处理
				//	else
				//	{
				//		if (!pPlayer->CheckCanTalk(CPlayer::CT_FAST, 2))
				//		{
				//			pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
				//			return FALSE;
				//		}
				//	}


				//} 
              if (!pPlayer->CheckCanTalk(CPlayer::CT_FAST, 2))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;

            case SChatBaseMsg::EPRO_CHAT_GLOBAL:
                {
                    WORD TimeLimit = 0;
                    if (((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_RUMOUR)
                        TimeLimit = 45;
                    else
                        TimeLimit = 20;

                    if (!pPlayer->CheckCanTalk(CPlayer::CT_SLOW, TimeLimit))
                    {
                        pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                        return FALSE;
                    }
                }
                break;

            default:
                if (!pPlayer->CheckCanTalk(CPlayer::CT_NORMAL, 10))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;
            }

            if (memcmp(pPlayer->GetName(), "游客", 4) == 0)
            {
                if (pChatMsg->_protocol != SChatBaseMsg::EPRO_CHAT_PUBLIC)
                {
                    TalkToDnid(dnidClient, "游客只能向当前屏幕说话！");
                    return FALSE;
                }

                if (pPlayer->m_Property.m_Level < 5)
                {
                    TalkToDnid(dnidClient, "游客修为到了5级才能向当前屏幕说话！");
                    return FALSE;
                }
            }

			if (pChatMsg->_protocol == SChatBaseMsg::EPRO_CHAT_WHISPER)
			{
				SQChatWisperMsg *pMsg = (SQChatWisperMsg*)pChatMsg;

				if (dwt::strcmp(pMsg->szRecvName,"gm",2) == 0
					|| dwt::strcmp(pMsg->szRecvName,"GM",2) == 0)
                {
                   pPlayer->m_dwChatSkipTime[CPlayer::CT_FAST] += 30000;//15000
                }
				else if (pPlayer->m_Property.m_Level < 5)
				{
					TalkToDnid(dnidClient, "阁下修为不到5级，不能使用传音入密！");
					return FALSE;
                }
            }
            else if (pChatMsg->_protocol == SChatBaseMsg::EPRO_CHAT_GLOBAL)
            {
                if (((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_RUMOUR)
                {
                    if (pPlayer->m_Property.m_Level < 30)
                    {
                        TalkToDnid(dnidClient, "阁下修为不到30级，无法散布江湖谣言！");
                        return FALSE;
                    }
                }
				else if( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_NORMAL)
				{
					if (pPlayer->m_Property.m_Level < 30)
					{
						TalkToDnid(dnidClient, "阁下修为不到30级，无法使用千里传音！");
						return FALSE;
					}
				}
                else if ( ( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_TRADE ) || ( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_SPECIAL + 10 ) )
                {
					if (pPlayer->m_Property.m_Level < 30)
					{
						TalkToDnid(dnidClient, "阁下修为不到30级，无法使用商贸频道！");
						return FALSE;
					}
                }
            }
        }

        // GM禁言
        std::map<std::string, DWORD>::iterator it = GMTalkMask.find(pPlayer->GetAccount());
        if (it != GMTalkMask.end())
        {
            DWORD MaskTime = it->second;
            if ( (int)(timeGetTime() - MaskTime) < 0 )
                isGMMask = true;
            else
                GMTalkMask.erase(it);
        }

        // 王城禁言
        it = KingTalkMask.find( pPlayer->GetAccount() );
        if ( it != KingTalkMask.end() )
        {
            DWORD maskTime = it->second;
            if ( ( int )( timeGetTime() - maskTime ) < 0 )
                isKingMask = true;
            else
                KingTalkMask.erase( it );    
        }

        // 掌门人禁言
        it = MasterTalkMask.find( pPlayer->GetAccount() );
        if ( it != MasterTalkMask.end() )
        {
            DWORD maskTime = it->second;
            if ( ( int )( timeGetTime() - maskTime ) < 0 )
                isMasterMask = true;
            else
                MasterTalkMask.erase( it );    
        }
    }
*/

	switch (pChatMsg->_protocol) 
    {
/*
        case SChatBaseMsg::EPRO_CHAT_SYSMSG:
        {
            // 系统上层传来的广播操作，直接处理！
            SChatSysMsg * pChatSysMsg;
            pChatSysMsg = (SChatSysMsg*)pChatMsg;

            Broadcast(pChatSysMsg->cSystem);
        }
        break;
*/

	// 帮派频道
    case SChatBaseMsg::EPRO_CHAT_FACTION:
		{
            if ( pPlayer->m_Property.m_szTongName[0] == 0 )
				break;
			if ((timeGetTime() - pPlayer->m_dwFactionLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwFactionLimitTime = timeGetTime();

				//多服务器
				SQChatFactionMsg * pChatQMsg = (SQChatFactionMsg*)pChatMsg;
				SAChatFactionMsg msg;
				memcpy( msg.cChatData, pChatQMsg->cChatData, MAX_CHAT_LEN );
				memcpy( msg.cTalkerName, pChatQMsg->cTalkerName, sizeof(msg.cTalkerName) );
				msg.wResult = TRUE;

				//城主判断
				if ( !isGMMask && !isKingMask ) // 没有被城主和GM禁言时
				{
					if ( pPlayer->IsInFactionFirst() && pPlayer->m_stFacRight.Title == 8 && pPlayer->m_Property.m_Equip[8].details.wIndex == 40207)//城主并带了虎符（左）
						GetGW()->m_FactionManager.SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::CITYOWENTALK, &msg, sizeof( msg ) );
					else
						GetGW()->m_FactionManager.SendFactionAllServer( pPlayer->m_Property.m_szTongName, SQSendServerMsg::TALK, &msg, sizeof( msg ) );
				}
				else
					g_StoreMessage( dnidClient, &msg, sizeof( msg ) );
	       
			}
		}
        break;

	case SChatBaseMsg::EPRO_CHAT_TEAM:
		{
			SQChatTeam *pChatQMsg = (SQChatTeam*)pChatMsg;
			// 直接通过ORB转发
			GetGW()->m_TeamManager.OnDispatch(dnidClient, pChatQMsg, pPlayer);
		}
		break;

	// 附近频道
	case SChatBaseMsg::EPRO_CHAT_PUBLIC:
		{
			// 针对于当前屏幕内的玩家的聊天，直接处理
			SQChatPublic *pChatQMsg = (SQChatPublic*)pChatMsg;
            SAChatPublic msg;
            
			if (pChatQMsg->wLength >= MAX_CHAT_LEN)
                break;

            if (pChatQMsg->cCharData[0] == 0)
                break;

			if ((timeGetTime() - pPlayer->m_dwPublicLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwPublicLimitTime = timeGetTime();

				dwt::strcpy(msg.cCharData, pChatQMsg->cCharData, MAX_CHAT_LEN);
				msg.wSoundId = pChatQMsg->wSoundId;
				msg.dwGID    = pPlayer->GetGID();

				if (!isGMMask && !isKingMask && !isMasterMask)  // 没有被任何人禁言..
				{
					if (pPlayer->m_ParentArea)
						pPlayer->m_ParentArea->SendAdj(&msg, msg.GetMySize(), -1);

					rfalse(1, 1, "[%s][%s] [屏] %s 说 %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), msg.cCharData);
				}
				else
					g_StoreMessage(dnidClient, &msg, sizeof(msg));
			}
		}
    	break;

	// 私聊频道
	case SChatBaseMsg::EPRO_CHAT_WHISPER:
        {
			SQChatWisperMsg *pChatWisper = (SQChatWisperMsg*)pChatMsg;

			// 一秒内不能连续说话
			if ((timeGetTime() - pPlayer->m_dwWhisperLimitTime) > DeltaOne_time)
			{
				pPlayer->m_dwWhisperLimitTime = timeGetTime();

				CPlayer *pDest = (CPlayer*)GetPlayerByName(pChatWisper->szRecvName)->DynamicCast(IID_PLAYER);
				if (!pDest) 
				{
					TalkToDnid(pPlayer->m_ClientIndex, "对方似乎不在");
					break;
				}
	 
				if (CPlayer::OST_HANGUP == pDest->m_OnlineState)
				{
					TalkToDnid(pPlayer->m_ClientIndex, "对方正处于挂机状态");
					break;
				}
				
				SAChatWisperMsg wisperMsg;
				dwt::strcpy(wisperMsg.szName, pDest->GetName(), CONST_USERNAME);
				dwt::strcpy(wisperMsg.cChatData, pChatWisper->cChatData, MAX_CHAT_LEN);
				wisperMsg.byType = 2;

				CBaseDataManager* baseDataMgr = pDest->GetSanguoBaseData();
				if (baseDataMgr == nullptr)
					return FALSE;
				wisperMsg.wHeadIcon = baseDataMgr->GetIconID();
				wisperMsg.wLevel = pDest->m_FixData.m_dwLevel;
				
				g_StoreMessage(pPlayer->m_ClientIndex, &wisperMsg, sizeof(wisperMsg));

				if (!isGMMask)			// 只有GM才有权利禁密聊
				{
					dwt::strcpy(wisperMsg.szName, pPlayer->GetName(), CONST_USERNAME);
					wisperMsg.byType = 1;
					g_StoreMessage(pDest->m_ClientIndex, &wisperMsg, sizeof(wisperMsg));

					rfalse(1, 1, "[%s][%s] [密] %s 对 %s 说 %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pDest->GetName(), wisperMsg.cChatData);
				}
			}
        }
    	break;

	// 世界频道
	case SChatBaseMsg::EPRO_CHAT_GLOBAL:
        {
			SQChatGlobalMsg *pChatGlobalMsg = (SQChatGlobalMsg *)pChatMsg;

			SAChatGlobalMsg msg;
			memcpy(msg.cTalkerName, pChatGlobalMsg->cTalkerName, CONST_USERNAME);

			if (pChatGlobalMsg->cChatData[0] == '/'/* && pChatGlobalMsg->cChatData[1] == '>'*/)		//客户端发送的是unicode编码字符串
            {
                //if (ret > 0)
                //{        
                //    msg.byType = SAChatGlobalMsg::ECT_SYSTEM;
                //    dwt::strcpy(msg.cChatData, &pChatGlobalMsg->cChatData[2], MAX_CHAT_LEN);
                //    msg.cTalkerName[0] = 0;

                //    BroadcastMsg(&msg, msg.GetMySize());
                //}
                //
                //return true;
				/*g_Script.SetCondition(NULL, pPlayer, NULL);
				LuaFunctor(g_Script, FormatString("AnalyzeAndExceteGMCommand"))[&pChatGlobalMsg->cChatData[2]]();
				g_Script.CleanCondition();*/
				g_Script.SetPlayer(pPlayer);
				//g_Script.CallFunc("AnalyzeAndExceteGMCommand", &pChatGlobalMsg->cChatData[1], pPlayer->m_Property.m_GMLevel, pChatGlobalMsg->byType, pChatGlobalMsg->cTalkerName);	//GM等级时才有控制权力
				g_Script.CallFunc("AnalyzeAndExceteGMCommand", &pChatGlobalMsg->cChatData[1]);	//20150203 wk 没有gm检测 
				g_Script.CleanPlayer();
				return false;
            }

			msg.byType = pChatGlobalMsg->byType;
			msg.color = pChatGlobalMsg->color;
			// 在这里做字符串拷贝，同时也利用拷贝后的数据避免下边序列化时数据溢出！
			memcpy(msg.cHead, pChatGlobalMsg->cHead, MAX_HEAD_LEN);

            dwt::strcpy(msg.cChatData, pChatGlobalMsg->cChatData, MAX_CHAT_LEN);
			//dwt::strcpy(msg.cHead, pChatGlobalMsg->cHead, MAX_HEAD_LEN);

            if ((msg.byType == SAChatGlobalMsg::ECT_TRADE) || (msg.byType == SAChatGlobalMsg::ECT_SPECIAL + 10))
            {
				return TRUE;
            }

			// 小喇叭
			if (msg.byType == SAChatGlobalMsg::ECT_SMALL_HORN)
			{
				WORD nislimit = 0;
				if (g_Script.GetWorldTalkLimit(pPlayer,"GetWorldTalkLimit",nislimit) == 1)
				{
					if (nislimit == 1)
					{
						if (!isGMMask && !isKingMask && !isMasterMask)  // 3个都有权限
						{
							BroadcastMsg(&msg, msg.GetMySize());
						}
						else
							g_StoreMessage(dnidClient, &msg, msg.GetMySize());
					}
				}
				
			}
			else if (msg.byType == SAChatGlobalMsg::ECT_NORMAL)
			{
				if (pPlayer == NULL)
				{
					return FALSE;
				}
				if ((timeGetTime() - pPlayer->m_dwGlobalLimitTime) > DeltaThirty_time)
				{
					BroadcastMsg(&msg, msg.GetMySize());
					pPlayer->m_dwGlobalLimitTime = timeGetTime();
				}
				else 
				{
					TalkToDnid(dnidClient, "世界聊天的限制时间未到！");
				}
			}
			else if (msg.byType == SAChatGlobalMsg::ECT_SYSTEM)//系统消息
			{
					BroadcastMsg(&msg, msg.GetMySize());
			}
        }
        break;

	// 门派频道
	case SChatBaseMsg::EPRO_CHAT_SCHOOL:
        {
            SQChatSchool *pChatQMsg = (SQChatSchool *)pChatMsg;

            if (0 == pChatQMsg->cChatData[0])
                break;

			// 三秒内不能连续说话
			if ((timeGetTime() - pPlayer->m_dwSchoolLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwSchoolLimitTime = timeGetTime();

				if (!isGMMask && !isMasterMask) // 没有被GM和掌门人禁言时
				{
					extern void BroadcastSchoolChatMsg(CPlayer *pPlayer, SMessage *pMsg, WORD wSize);
					BroadcastSchoolChatMsg(pPlayer, pChatQMsg, pChatQMsg->GetMySize());
					rfalse(1, 1, "[%s][%s] [门] %s 说 %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pChatQMsg->cChatData);
				}
				else
					g_StoreMessage(dnidClient, pChatQMsg, sizeof(SQChatSchool));
			}
		}
        break;

	// 区域频道
	case SChatBaseMsg::EPRO_CHAT_REGION:
        {
			SQAChatRegion *pChatQMsg = ( SQAChatRegion* )pChatMsg;

			if (0 == pChatQMsg->cChatData[0])
                break;

			if (!pPlayer->m_ParentRegion)
				break;

            if (!isGMMask && !isKingMask && !isMasterMask)
            {
                pPlayer->m_ParentRegion->Broadcast(pChatQMsg, pChatQMsg->GetMySize(), 0);
                rfalse(1, 1, "[%s][%s] [场] %s 说 %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pChatQMsg->cChatData);
            }
            else
                g_StoreMessage(dnidClient, pChatQMsg, sizeof(SQAChatRegion));
        }
        break;

	// 传闻频道
	case SChatBaseMsg::EPRO_CHAT_RUMOR:
		{
			SAChatRumorMsg *pChatRumorMsg = (SAChatRumorMsg *)pChatMsg;
			SQChatRumorMsg msg;
			
			if (pChatRumorMsg->cChatData[0] == 0)
				break;

			// 3个都有权限,三秒之内不能连续说话
			if ((timeGetTime() - pPlayer->m_dwRumorLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwRumorLimitTime = timeGetTime();

				memcpy(msg.cTalkerName, pChatRumorMsg->cTalkerName, CONST_USERNAME);
				dwt::strcpy(msg.cChatData, pChatRumorMsg->cChatData, MAX_CHAT_LEN);
		
				// 判断身上侠义币数量
				if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, dwRumorNeedMoney, false))
				{
					TalkToDnid(pPlayer->m_ClientIndex, "身上的非绑定不足100，无法使用传闻！");
					break;
				}

				// 随机显示名称,有95%的几率不显示
				int tempRate = CRandom::RandRange(1, 100);
				if (tempRate > 5)
					msg.cTalkerName[0] = 0;

				// 广播消息
				if (!isGMMask && !isKingMask && !isMasterMask) 
				{
					BroadcastMsg(&msg, msg.GetMySize());
					rfalse(1, 1, "[%s][%s] [全] %s 说 %s", pPlayer->GetAccount(), GetStringTime(),pPlayer->GetName(), msg.cChatData);
				}
				else
					g_StoreMessage(dnidClient, &msg, msg.GetMySize());
			}
		}
		break;

    default:
        break;
    }

	return !isGMMask; // 返回失败，表示不发消息到GM工具上;
}

// 功能：发送系统信息
// 参数：   i       接受消息的玩家序号
//	        iMsgID  系统消息ID
// 返回： -1 失败 1 成功
int CDChatManager::SendSysMsg(DNID dnidClient, int iMsgID)
{
    SChatSysMsg	MsgData;
    MsgData._protocol = SChatSysMsg::EPRO_CHAT_SYSMSG;
    MsgData.btMsgID = iMsgID;

    return g_StoreMessage(dnidClient, &MsgData, sizeof(SChatSysMsg));
}

// 功能：发送系统公告
// 参数：   i       接受消息的玩家序号
//          szMsg   发送的消息文本
// 返回： -1 失败 1 成功
int CDChatManager::SendSysCall(DNID dnidClient, char *szMsg)
{
    if (szMsg == NULL) 
        return 0;

    if (szMsg[0] == NULL) 
        return 0;

    SChatToAllMsg msg;
    msg.wSoundId = 0;

    msg.wChatLen = (WORD)dwt::strlen(szMsg, MAX_CHAT_LEN);
    memcpy(msg.cChat, szMsg, msg.wChatLen);

    return g_StoreMessage(dnidClient, &msg, msg.GetMySize());
}

// 发送聊天信息给单一玩家
int CDChatManager::SendToSomeOne(DNID dnidClient, SChatBaseMsg *pMsg)
{
    return 0;
}

// 功能：发送聊天信息给所有玩家
// 参数：pMsg 要发送的聊天数据
// 返回：成功的客户端数量
int CDChatManager::SendToAll(SChatBaseMsg *pMsg)
{
    return 0;
}

// 发送聊天信息给好友
int CDChatManager::SendToFriend(SChatBaseMsg *pMsg)
{
    return 0;
};

// 发送聊天信息给队友
int CDChatManager::SendToGroup(SChatBaseMsg *pMsg)
{
    return 0;
};

// 发送聊天信息给帮会成员
int	CDChatManager::SendToFaction(SChatBaseMsg *pMsg)
{
    return 0;
};

//
//BOOL CALLBACK SendBroadcastChannelMsg(LPIObject &Player, SForChannelBroadCast *pChannelBroadCast)
//{
//    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
//        g_StoreMessage(pPlayer->m_ClientIndex, pChannelBroadCast->pChannelBaseMsg,pChannelBroadCast->iSize);
//
//    return true;
//}
//
//BOOL CALLBACK SendBroadcastGlobalTalkMsg(LPIObject &Player, SForGlobalTalkBroadCast *pGlobalTalkBroadCast)
//{
//    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
//        g_StoreMessage(pPlayer->m_ClientIndex, pGlobalTalkBroadCast->pChatBaseGlobalMsg ,pGlobalTalkBroadCast->iSize);
//
//    return true;
//}
//
//void Broadcast(DNID dnidClient, SChannelBaseMsg *pMsg, int iSize)
//{
//    SForChannelBroadCast ForChannelBroadCast;
//
//    ForChannelBroadCast.pChannelBaseMsg = pMsg;
//    ForChannelBroadCast.iSize = iSize;
//
//    TraversalPlayers((TRAVERSALFUNC)SendBroadcastChannelMsg, (LPARAM)&ForChannelBroadCast);
//}
//
//BOOL CALLBACK SendBroadcastMsg(LPIObject &Player, SAChatPublic *pMsg)
//{
//    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
//        g_StoreMessage(pPlayer->m_ClientIndex, pMsg, pMsg->GetMySize());
//
//    return true;
//}
//
//void BroadcastSystemInfo(LPCSTR info)
//{
//    SAChatPublic msg;
//
//    msg.wSoundId = 0;
//    msg.dwGID = 0xffffffff;
//    msg.wLength = dwt::strlen(info, MAX_CHAT_LEN);
//
//    memcpy(msg.cCharData, info, msg.wLength);
//
//    msg.cCharData[msg.wLength] = 0;
//
//    TraversalPlayers((TRAVERSALFUNC)SendBroadcastMsg, (LPARAM)&msg);
//}
//
//void BroadCastGlobalTalk(DNID dnidClient, SAChatGlobalMsg * pMsg, int iSize)
//{
//    SForGlobalTalkBroadCast ForGlobalTalkBroadCast;
//
//    ForGlobalTalkBroadCast.pChatBaseGlobalMsg = pMsg;
//    ForGlobalTalkBroadCast.iSize = iSize;
//
//    TraversalPlayers((TRAVERSALFUNC)SendBroadcastGlobalTalkMsg, (LPARAM)&ForGlobalTalkBroadCast);
//}

void BroadcastSystemInfo(LPCSTR info)
{
    SAChatPublic msg;

    msg.wSoundId = 0;
    msg.dwGID = 0xffffffff;
    msg.wLength = dwt::strlen(info, MAX_CHAT_LEN);
    dwt::strcpy(msg.cCharData, info, MAX_CHAT_LEN);

    BroadcastMsg(&msg, msg.GetMySize());
}

void BroadcastNearInfo(CActiveObject *activeobj,LPCSTR info)
{
	if(!activeobj)return;

	SAChatPublic msg;

	msg.wSoundId = 0;
	msg.dwGID = activeobj->GetGID();
	msg.wLength = dwt::strlen(info, MAX_CHAT_LEN);
	dwt::strcpy(msg.cCharData, info, MAX_CHAT_LEN);

	if (activeobj->m_ParentArea){
		activeobj->m_ParentArea->SendAdj(&msg, msg.GetMySize(), -1);
	}
}

///怪物说话
void MonsterTalkInfo(CMonster *pmonster,LPCSTR info)
{
	if(!pmonster)return;

	SAChatPublic msg;
	char *Utf8Inf = g_AnsiToUtf8(info);
	msg.wSoundId = 0;
	msg.dwGID = pmonster->GetGID();
	msg.wLength = dwt::strlen(Utf8Inf, MAX_CHAT_LEN);
	dwt::strcpy(msg.cCharData, Utf8Inf, MAX_CHAT_LEN);
	if (Utf8Inf != NULL)
		delete[] Utf8Inf;

	if (pmonster->m_ParentArea){
		pmonster->m_ParentArea->SendAdj(&msg, msg.GetMySize(), -1);
	}
}

void BroadCastAnnounce(LPCSTR info, int iSize)
{
#ifdef DISABLE_ORB
    SAChatGlobalMsg AChatGlobalMsg;
    AChatGlobalMsg.byType = SAChatGlobalMsg::ECT_SYSTEM;
    AChatGlobalMsg.cTalkerName[0] = 0;

    dwt::strcpy(AChatGlobalMsg.cChatData, info, MAX_CHAT_LEN);

    BroadCastGlobalTalk(-1, &AChatGlobalMsg, sizeof(SAChatGlobalMsg));
#else
    //SQORBChatGlobalMsg msg;

    //msg.byType = SAChatGlobalMsg::ECT_SYSTEM;
    //msg.cTalkerName[0] = 0;

    //dwt::strcpy(msg.cChatData, info, min(iSize, (unsigned int)sizeof(msg.cChatData)));

    //msg.dnidClient = 0x1111111111111111;

    //SendToORBServer(&msg, msg.GetMySize());
#endif
}

/*
struct SBroadcastMsgStruct
{
    SMessage *pMsg;
    WORD wSize;
    DNID dnidExcept;
};

BOOL CALLBACK BroadcastMsgCallback(LPIObject &Player, SBroadcastMsgStruct *pSBMS)
{
    if (CPlayer *pPlayer = (CPlayer*)Player->DynamicCast(IID_PLAYER))
        if (pPlayer->m_ClientIndex != pSBMS->dnidExcept)
            g_StoreMessage(pPlayer->m_ClientIndex, pSBMS->pMsg, pSBMS->wSize);

    return true;
}

void BroadcastMsg(SMessage *pMsg, WORD wSize, DNID dnidExcept)
{
    SBroadcastMsgStruct param;
    param.pMsg = pMsg;
    param.wSize = wSize;
    param.dnidExcept = dnidExcept;

    PushMsg

    TraversalPlayers((TRAVERSALFUNC)BroadcastMsgCallback, (LPARAM)&param);
}
*/
