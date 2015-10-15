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
        return 0;   // ̫��İ��ˣ�

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

std::map<std::string, DWORD> GMTalkMask;      // ��GM���Ե����ӳ���
std::map<std::string, DWORD> KingTalkMask;    // ���������Ե����ӳ���
std::map<std::string, DWORD> MasterTalkMask;  // �������˽��Ե����ӳ���

// ��������-+---------------------
struct TempData
{
	DWORD color;
	char buffer[MAX_CHAT_LEN];
	char name[CONST_USERNAME];
};

#define GLOBAL_MAX_MSG_NUM 30
std::list< TempData >  globalMsg;  // ����������Ϣ

DWORD globalMsgTime;
DWORD golbal_time = 1000 * 10;

// ����ʱ�������Ƶ�ȫ�ֱ���
// ���������ɡ���ᡢ���ŷ��Լ����Ϊ3��
// ���顢˽�ķ��Լ��Ϊ1��,���鵥������
const DWORD DeltaThree_time = 1000 * 3;
const DWORD DeltaOne_time	= 1000 * 1;
const DWORD DeltaThirty_time = 1000 ; 

const DWORD dwRumorNeedMoney = 100;		// ʹ�ô���Ƶ����Ҫ���ĵ������  
const DWORD dwSmallHornIndex = 8001001; // С���ȵ�������

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
		// ������Ϣ...
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

    bool isGMMask     = false;		// GM����
    bool isKingMask   = false;		// ��������
    bool isMasterMask = false;		// �����˽���
	int  ret = 0;

/*
    {
        extern int GetAccountLimit(CPlayer *);
        extern int CheckTemp(CPlayer *pPlayer);

        ret = GetAccountLimit(pPlayer);

        if (ret == 0)
        {
            // û��Ȩ�޵���ͨ��ң�������

            //// ������߲���˵��
            //if (pChatMsg->_protocol != SChatBaseMsg::EPRO_CHAT_PUBLIC)
            //{
            //    if (pPlayer->m_ParentRegion != NULL)
            //    {
            //        if (pPlayer->m_ParentRegion->DynamicCast(IID_PRISON) != NULL ||
            //            pPlayer->m_ParentRegion->DynamicCast(IID_PRISONEX) != NULL )
            //        {
            //            TalkToDnid(dnidClient, "���������ֻ�ܺ͸�������������");
            //            return FALSE;
            //        }
            //    }
            //}

            switch (pChatMsg->_protocol)
            {
            case SChatBaseMsg::EPRO_CHAT_PUBLIC: // ����Ƶ��
                if (!pPlayer->CheckCanTalk(CPlayer::CT_FAST, 2))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;

            case SChatBaseMsg::EPRO_CHAT_FACTION:// ���Ƶ��
                if (!pPlayer->CheckCanTalk(CPlayer::CT_NORMAL, 8))
                {
                    pPlayer->SendErrorMsg(SABackMsg::B_LIMIT_TALK);
                    return FALSE;
                }
                break;

            case SChatBaseMsg::EPRO_CHAT_WHISPER:
                 //�����GM����1���Ӻ����
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
				//	//������һ������������GM̫Ƶ�����Ĵ���
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

            if (memcmp(pPlayer->GetName(), "�ο�", 4) == 0)
            {
                if (pChatMsg->_protocol != SChatBaseMsg::EPRO_CHAT_PUBLIC)
                {
                    TalkToDnid(dnidClient, "�ο�ֻ����ǰ��Ļ˵����");
                    return FALSE;
                }

                if (pPlayer->m_Property.m_Level < 5)
                {
                    TalkToDnid(dnidClient, "�ο���Ϊ����5��������ǰ��Ļ˵����");
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
					TalkToDnid(dnidClient, "������Ϊ����5��������ʹ�ô������ܣ�");
					return FALSE;
                }
            }
            else if (pChatMsg->_protocol == SChatBaseMsg::EPRO_CHAT_GLOBAL)
            {
                if (((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_RUMOUR)
                {
                    if (pPlayer->m_Property.m_Level < 30)
                    {
                        TalkToDnid(dnidClient, "������Ϊ����30�����޷�ɢ������ҥ�ԣ�");
                        return FALSE;
                    }
                }
				else if( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_NORMAL)
				{
					if (pPlayer->m_Property.m_Level < 30)
					{
						TalkToDnid(dnidClient, "������Ϊ����30�����޷�ʹ��ǧ�ﴫ����");
						return FALSE;
					}
				}
                else if ( ( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_TRADE ) || ( ((SQChatGlobalMsg *)pChatMsg)->byType == SAChatGlobalMsg::ECT_SPECIAL + 10 ) )
                {
					if (pPlayer->m_Property.m_Level < 30)
					{
						TalkToDnid(dnidClient, "������Ϊ����30�����޷�ʹ����óƵ����");
						return FALSE;
					}
                }
            }
        }

        // GM����
        std::map<std::string, DWORD>::iterator it = GMTalkMask.find(pPlayer->GetAccount());
        if (it != GMTalkMask.end())
        {
            DWORD MaskTime = it->second;
            if ( (int)(timeGetTime() - MaskTime) < 0 )
                isGMMask = true;
            else
                GMTalkMask.erase(it);
        }

        // ���ǽ���
        it = KingTalkMask.find( pPlayer->GetAccount() );
        if ( it != KingTalkMask.end() )
        {
            DWORD maskTime = it->second;
            if ( ( int )( timeGetTime() - maskTime ) < 0 )
                isKingMask = true;
            else
                KingTalkMask.erase( it );    
        }

        // �����˽���
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
            // ϵͳ�ϲ㴫���Ĺ㲥������ֱ�Ӵ���
            SChatSysMsg * pChatSysMsg;
            pChatSysMsg = (SChatSysMsg*)pChatMsg;

            Broadcast(pChatSysMsg->cSystem);
        }
        break;
*/

	// ����Ƶ��
    case SChatBaseMsg::EPRO_CHAT_FACTION:
		{
            if ( pPlayer->m_Property.m_szTongName[0] == 0 )
				break;
			if ((timeGetTime() - pPlayer->m_dwFactionLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwFactionLimitTime = timeGetTime();

				//�������
				SQChatFactionMsg * pChatQMsg = (SQChatFactionMsg*)pChatMsg;
				SAChatFactionMsg msg;
				memcpy( msg.cChatData, pChatQMsg->cChatData, MAX_CHAT_LEN );
				memcpy( msg.cTalkerName, pChatQMsg->cTalkerName, sizeof(msg.cTalkerName) );
				msg.wResult = TRUE;

				//�����ж�
				if ( !isGMMask && !isKingMask ) // û�б�������GM����ʱ
				{
					if ( pPlayer->IsInFactionFirst() && pPlayer->m_stFacRight.Title == 8 && pPlayer->m_Property.m_Equip[8].details.wIndex == 40207)//���������˻�������
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
			// ֱ��ͨ��ORBת��
			GetGW()->m_TeamManager.OnDispatch(dnidClient, pChatQMsg, pPlayer);
		}
		break;

	// ����Ƶ��
	case SChatBaseMsg::EPRO_CHAT_PUBLIC:
		{
			// ����ڵ�ǰ��Ļ�ڵ���ҵ����죬ֱ�Ӵ���
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

				if (!isGMMask && !isKingMask && !isMasterMask)  // û�б��κ��˽���..
				{
					if (pPlayer->m_ParentArea)
						pPlayer->m_ParentArea->SendAdj(&msg, msg.GetMySize(), -1);

					rfalse(1, 1, "[%s][%s] [��] %s ˵ %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), msg.cCharData);
				}
				else
					g_StoreMessage(dnidClient, &msg, sizeof(msg));
			}
		}
    	break;

	// ˽��Ƶ��
	case SChatBaseMsg::EPRO_CHAT_WHISPER:
        {
			SQChatWisperMsg *pChatWisper = (SQChatWisperMsg*)pChatMsg;

			// һ���ڲ�������˵��
			if ((timeGetTime() - pPlayer->m_dwWhisperLimitTime) > DeltaOne_time)
			{
				pPlayer->m_dwWhisperLimitTime = timeGetTime();

				CPlayer *pDest = (CPlayer*)GetPlayerByName(pChatWisper->szRecvName)->DynamicCast(IID_PLAYER);
				if (!pDest) 
				{
					TalkToDnid(pPlayer->m_ClientIndex, "�Է��ƺ�����");
					break;
				}
	 
				if (CPlayer::OST_HANGUP == pDest->m_OnlineState)
				{
					TalkToDnid(pPlayer->m_ClientIndex, "�Է������ڹһ�״̬");
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

				if (!isGMMask)			// ֻ��GM����Ȩ��������
				{
					dwt::strcpy(wisperMsg.szName, pPlayer->GetName(), CONST_USERNAME);
					wisperMsg.byType = 1;
					g_StoreMessage(pDest->m_ClientIndex, &wisperMsg, sizeof(wisperMsg));

					rfalse(1, 1, "[%s][%s] [��] %s �� %s ˵ %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pDest->GetName(), wisperMsg.cChatData);
				}
			}
        }
    	break;

	// ����Ƶ��
	case SChatBaseMsg::EPRO_CHAT_GLOBAL:
        {
			SQChatGlobalMsg *pChatGlobalMsg = (SQChatGlobalMsg *)pChatMsg;

			SAChatGlobalMsg msg;
			memcpy(msg.cTalkerName, pChatGlobalMsg->cTalkerName, CONST_USERNAME);

			if (pChatGlobalMsg->cChatData[0] == '/'/* && pChatGlobalMsg->cChatData[1] == '>'*/)		//�ͻ��˷��͵���unicode�����ַ���
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
				//g_Script.CallFunc("AnalyzeAndExceteGMCommand", &pChatGlobalMsg->cChatData[1], pPlayer->m_Property.m_GMLevel, pChatGlobalMsg->byType, pChatGlobalMsg->cTalkerName);	//GM�ȼ�ʱ���п���Ȩ��
				g_Script.CallFunc("AnalyzeAndExceteGMCommand", &pChatGlobalMsg->cChatData[1]);	//20150203 wk û��gm��� 
				g_Script.CleanPlayer();
				return false;
            }

			msg.byType = pChatGlobalMsg->byType;
			msg.color = pChatGlobalMsg->color;
			// ���������ַ���������ͬʱҲ���ÿ���������ݱ����±����л�ʱ���������
			memcpy(msg.cHead, pChatGlobalMsg->cHead, MAX_HEAD_LEN);

            dwt::strcpy(msg.cChatData, pChatGlobalMsg->cChatData, MAX_CHAT_LEN);
			//dwt::strcpy(msg.cHead, pChatGlobalMsg->cHead, MAX_HEAD_LEN);

            if ((msg.byType == SAChatGlobalMsg::ECT_TRADE) || (msg.byType == SAChatGlobalMsg::ECT_SPECIAL + 10))
            {
				return TRUE;
            }

			// С����
			if (msg.byType == SAChatGlobalMsg::ECT_SMALL_HORN)
			{
				WORD nislimit = 0;
				if (g_Script.GetWorldTalkLimit(pPlayer,"GetWorldTalkLimit",nislimit) == 1)
				{
					if (nislimit == 1)
					{
						if (!isGMMask && !isKingMask && !isMasterMask)  // 3������Ȩ��
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
					TalkToDnid(dnidClient, "�������������ʱ��δ����");
				}
			}
			else if (msg.byType == SAChatGlobalMsg::ECT_SYSTEM)//ϵͳ��Ϣ
			{
					BroadcastMsg(&msg, msg.GetMySize());
			}
        }
        break;

	// ����Ƶ��
	case SChatBaseMsg::EPRO_CHAT_SCHOOL:
        {
            SQChatSchool *pChatQMsg = (SQChatSchool *)pChatMsg;

            if (0 == pChatQMsg->cChatData[0])
                break;

			// �����ڲ�������˵��
			if ((timeGetTime() - pPlayer->m_dwSchoolLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwSchoolLimitTime = timeGetTime();

				if (!isGMMask && !isMasterMask) // û�б�GM�������˽���ʱ
				{
					extern void BroadcastSchoolChatMsg(CPlayer *pPlayer, SMessage *pMsg, WORD wSize);
					BroadcastSchoolChatMsg(pPlayer, pChatQMsg, pChatQMsg->GetMySize());
					rfalse(1, 1, "[%s][%s] [��] %s ˵ %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pChatQMsg->cChatData);
				}
				else
					g_StoreMessage(dnidClient, pChatQMsg, sizeof(SQChatSchool));
			}
		}
        break;

	// ����Ƶ��
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
                rfalse(1, 1, "[%s][%s] [��] %s ˵ %s", pPlayer->GetAccount(), GetStringTime(), pPlayer->GetName(), pChatQMsg->cChatData);
            }
            else
                g_StoreMessage(dnidClient, pChatQMsg, sizeof(SQAChatRegion));
        }
        break;

	// ����Ƶ��
	case SChatBaseMsg::EPRO_CHAT_RUMOR:
		{
			SAChatRumorMsg *pChatRumorMsg = (SAChatRumorMsg *)pChatMsg;
			SQChatRumorMsg msg;
			
			if (pChatRumorMsg->cChatData[0] == 0)
				break;

			// 3������Ȩ��,����֮�ڲ�������˵��
			if ((timeGetTime() - pPlayer->m_dwRumorLimitTime) > DeltaThree_time)
			{
				pPlayer->m_dwRumorLimitTime = timeGetTime();

				memcpy(msg.cTalkerName, pChatRumorMsg->cTalkerName, CONST_USERNAME);
				dwt::strcpy(msg.cChatData, pChatRumorMsg->cChatData, MAX_CHAT_LEN);
		
				// �ж��������������
				if (!pPlayer->CheckPlayerMoney(XYD_UM_ONLYUNBIND, dwRumorNeedMoney, false))
				{
					TalkToDnid(pPlayer->m_ClientIndex, "���ϵķǰ󶨲���100���޷�ʹ�ô��ţ�");
					break;
				}

				// �����ʾ����,��95%�ļ��ʲ���ʾ
				int tempRate = CRandom::RandRange(1, 100);
				if (tempRate > 5)
					msg.cTalkerName[0] = 0;

				// �㲥��Ϣ
				if (!isGMMask && !isKingMask && !isMasterMask) 
				{
					BroadcastMsg(&msg, msg.GetMySize());
					rfalse(1, 1, "[%s][%s] [ȫ] %s ˵ %s", pPlayer->GetAccount(), GetStringTime(),pPlayer->GetName(), msg.cChatData);
				}
				else
					g_StoreMessage(dnidClient, &msg, msg.GetMySize());
			}
		}
		break;

    default:
        break;
    }

	return !isGMMask; // ����ʧ�ܣ���ʾ������Ϣ��GM������;
}

// ���ܣ�����ϵͳ��Ϣ
// ������   i       ������Ϣ��������
//	        iMsgID  ϵͳ��ϢID
// ���أ� -1 ʧ�� 1 �ɹ�
int CDChatManager::SendSysMsg(DNID dnidClient, int iMsgID)
{
    SChatSysMsg	MsgData;
    MsgData._protocol = SChatSysMsg::EPRO_CHAT_SYSMSG;
    MsgData.btMsgID = iMsgID;

    return g_StoreMessage(dnidClient, &MsgData, sizeof(SChatSysMsg));
}

// ���ܣ�����ϵͳ����
// ������   i       ������Ϣ��������
//          szMsg   ���͵���Ϣ�ı�
// ���أ� -1 ʧ�� 1 �ɹ�
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

// ����������Ϣ����һ���
int CDChatManager::SendToSomeOne(DNID dnidClient, SChatBaseMsg *pMsg)
{
    return 0;
}

// ���ܣ�����������Ϣ���������
// ������pMsg Ҫ���͵���������
// ���أ��ɹ��Ŀͻ�������
int CDChatManager::SendToAll(SChatBaseMsg *pMsg)
{
    return 0;
}

// ����������Ϣ������
int CDChatManager::SendToFriend(SChatBaseMsg *pMsg)
{
    return 0;
};

// ����������Ϣ������
int CDChatManager::SendToGroup(SChatBaseMsg *pMsg)
{
    return 0;
};

// ����������Ϣ������Ա
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

///����˵��
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
