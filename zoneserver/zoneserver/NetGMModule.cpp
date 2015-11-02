#include "StdAfx.h"
#include "DMainApp.h"
#include "NetGMModule.h"
#include "GameObjects\Player.h"
#include "networkmodule\ChatMsgs.h"
#include "GameObjects\dchatmanager.h"

#ifdef GMMODULEON

#include "GameObjects\Region.h"

extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern BOOL MoveSomebody(LPCSTR name, WORD regionid, WORD x, WORD y);
extern LPIObject GetPlayerByName(LPCSTR szName);

CGMLog::CGMLog()
{

}

CGMLog::~CGMLog()
{

}

void CGMLog::ClearOprList()
{
    m_GMOperationList.clear();
}
void CGMLog::AddOprList(SGMOperation &GMOperation)
{
    m_GMOperationList.push_back(GMOperation);
}

static unsigned int __stdcall ReadFileFunc(void *pParameter)
{
    SStructForMultThread * pThreadParameter = (SStructForMultThread *)pParameter;
    CGMLog * pTheGMLog = pThreadParameter->pGMLog;
    DWORD dwByteRead;
    HANDLE hFile = ::CreateFile(pThreadParameter->szFileName,GENERIC_WRITE, 0, NULL,OPEN_ALWAYS, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return 0;
    if(ReadFile(hFile,pThreadParameter->cReadBuf,1024,&dwByteRead,NULL) == TRUE)
    {
        pTheGMLog->ClearOprList();
        SGMOperation GMOperation;
        for(DWORD iLooper = 0;iLooper < dwByteRead/sizeof(SGMOperation);iLooper++)
        {
            memcpy(&GMOperation,pThreadParameter->cReadBuf,sizeof(SGMOperation));
            pTheGMLog->AddOprList(GMOperation);
        }
    }
    CloseHandle(hFile);
    return 1;
}
void CGMLog::Log(LPTSTR Name,GMLEVEL Level,SGMOperation &GMOperation,DWORD dwSize)
{
    std::string strFileName;
    strFileName = Name;
    strFileName += "_GMLog.txt";
    HANDLE hFile = ::CreateFile(strFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return;
    SetFilePointer(hFile, 0, 0, FILE_END);
    WriteFile(hFile, &GMOperation, dwSize-1, &dwSize, 0);
    CloseHandle(hFile);
}



BOOL CGMLog::ReadLog(LPTSTR Name)
{
    std::string strFileName;
    strFileName = Name;
    strFileName +="_GMLog.txt";

    HANDLE hFile = ::CreateFile(strFileName.c_str(),GENERIC_WRITE, 0, NULL,OPEN_ALWAYS, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;
    CloseHandle(hFile);

    return TRUE;
}

static int m_GMAction[0xff];

CGMLevelChecker::CGMLevelChecker()
{
    m_GMAction[GM_ACT_TALKMASK] = 1;
    m_GMAction[GM_ACT_MOVEPLAYER] = 1;
    m_GMAction[GM_ACT_CHECKPLAYER] = 1;
    m_GMAction[GM_ACT_CHANGEPLAYER] = 1;
}

CGMLevelChecker::~CGMLevelChecker()
{
}

int CGMLevelChecker::CheckLevel(BYTE Action, WORD Level)
{
    if (Level > 5 || Level < 0)
        return 0;

    if (m_GMAction[Action] > Level)
        return 0;

    return 1;
}

CNetGMModule::CNetGMModule()
{

}
CNetGMModule::~CNetGMModule()
{
    if (m_pLevelChecker != NULL)
    {
        delete m_pLevelChecker;
        m_pLevelChecker = NULL;
    }

    if (m_pGMLog != NULL)
    {
        delete m_pGMLog;
        m_pGMLog = NULL;
    }
}

void CNetGMModule::Init()
{
    m_pLevelChecker = new CGMLevelChecker();
    m_pGMLog        = new CGMLog();
    m_bSendChatCheck  = FALSE;
    m_wChatCheckNumber = 0;
}
BOOL CNetGMModule::OnGMMessage(DNID dnidClient,SGMMMsg * pGMMMsg,BOOL bDistribute)
{
    switch(pGMMMsg->_protocol)
    {
    case SGMMMsg::EPRO_GM_CHECKCHAT:
        {
            SQGMCheckChatGMMsg * pQGMCheckChatGMMsg;
            pQGMCheckChatGMMsg = (SQGMCheckChatGMMsg *)pGMMMsg;
            if(pQGMCheckChatGMMsg->byOpr == 0)
            {
                if(m_wChatCheckNumber > 0)
                {
                    m_wChatCheckNumber--;
                }
            }
            else
            {
                m_wChatCheckNumber++;
            }
            if(m_wChatCheckNumber==0)
                m_bSendChatCheck = FALSE;
            else
                m_bSendChatCheck = TRUE;
        }
        break;
    case SGMMMsg::EPRO_GM_SENDTALK:
        {
            SQGMTalkMsg * pQGMTalkMsg;
            pQGMTalkMsg = (SQGMTalkMsg *)pGMMMsg;

            SAChatGlobalMsg AChatGlobalMsg;
            AChatGlobalMsg.byType = SAChatGlobalMsg::ECT_SYSTEM;
            AChatGlobalMsg.cTalkerName[0] = 0;

            dwt::strcpy(AChatGlobalMsg.cChatData, pQGMTalkMsg->szTalkMsg, MAX_CHAT_LEN);

            BroadcastMsg(&AChatGlobalMsg, AChatGlobalMsg.GetMySize());

            //void BroadCastGlobalTalk(DNID dnidClient, SAChatGlobalMsg *pMsg, int iSize);
            //BroadCastGlobalTalk(-1, &AChatGlobalMsg, sizeof(SAChatGlobalMsg));
        }
        break;
	case SGMMMsg::EPRO_GM_MOVETOSAFEPOINT:
		{
			SQMoveToSafePointGMMsg *pMsg = (SQMoveToSafePointGMMsg*)pGMMMsg;

			SAMoveToSafePointGMMsg	aMoveToSafePointMsg;

			aMoveToSafePointMsg.dnidClient = pMsg->dnidClient;
			dwt::strcpy(aMoveToSafePointMsg.szName, pMsg->szName, CONST_USERNAME);

			if (MoveSomebody(pMsg->szName,pMsg->iRegion,pMsg->iX,pMsg->iY))
			{
				aMoveToSafePointMsg.wRet = SAMoveToSafePointGMMsg::RET_SUCCESS;
			}
			else
			{
				aMoveToSafePointMsg.wRet = SAMoveToSafePointGMMsg::RET_FAIL;
			}

			GetApp()->m_LoginServer.SendMsgToLoginSrv(&aMoveToSafePointMsg,sizeof(SAMoveToSafePointGMMsg));
		}
		break;
    case SGMMMsg::EPRO_GM_MOVETOBORN:
        {
            SAMovetoBornMsg AMovetoBornMsg;
            WORD wGMLevel;
            SQMovetoBornMsg * pQMovetoBornMsg = (SQMovetoBornMsg*)pGMMMsg;
			memcpy(AMovetoBornMsg.szName, pQMovetoBornMsg->szName, CONST_USERNAME);
            AMovetoBornMsg.dnidClient = pQMovetoBornMsg->dnidClient;
            if(bDistribute == FALSE)
            {

            }
            else
            {
                wGMLevel = pGMMMsg->wGMLevel;
            }
            CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pQMovetoBornMsg->szName)->DynamicCast(IID_PLAYER);
            if (pPlayer == NULL)
                return FALSE;

            if (m_pLevelChecker->CheckLevel(GM_ACT_CHANGEPLAYER, wGMLevel))
            {
                if (pPlayer->m_ParentRegion == NULL) 
                    break;

                if (PutPlayerIntoDestRegion(pPlayer, 
					pPlayer->m_ParentRegion->m_wReLiveRegionID, 
                    (WORD)pPlayer->m_ParentRegion->m_ptReLivePoint.x,
					(WORD)pPlayer->m_ParentRegion->m_ptReLivePoint.y))
                    AMovetoBornMsg.byResult = 1;
                else
                    AMovetoBornMsg.byResult = 0;
            }

            GetApp()->m_LoginServer.SendMsgToLoginSrv(&AMovetoBornMsg,sizeof(SAMovetoBornMsg));
            
        }
        break;
	case SGMMMsg::EPRO_GM_TALKMASKSRV:
		{
			BOOL bFlag=FALSE;

			SATalkMaskGMMsg ATalkMaskGMMsg;

			SQTalkMaskSrvGMMsg *pMsg=(SQTalkMaskSrvGMMsg*)pGMMMsg;

			CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szName)->DynamicCast(IID_PLAYER);

            if (pPlayer != NULL)
			{
				bFlag=TRUE;
			}

			extern std::map< std::string, DWORD > GMTalkMask;

            LPCSTR account = NULL;
            try
            {
                lite::Serialreader sl( pMsg->streamData );
                account = sl();
            }
            catch ( lite::Xcpt & )
            {
            }

			if (pMsg->dwTime != 0)
			{
				GMTalkMask[account] = timeGetTime() + pMsg->dwTime*60000;

				if (bFlag)
				{
					void TalkToAll(LPCSTR info);
					LPCSTR FormatString(LPCSTR, ...);
					//TalkToAll(FormatString("[%s]被封了哑穴", pMsg->szName, pMsg->dwTime));
				}

				ATalkMaskGMMsg.wResult = 1;
			}
			else
			{
				GMTalkMask.erase(account);
				ATalkMaskGMMsg.wResult = 4;	//解除禁言
			}
			
			dwt::strcpy(ATalkMaskGMMsg.szName, pMsg->szName, CONST_USERNAME);
			ATalkMaskGMMsg.dnidClient=pMsg->dnidClient;
			ATalkMaskGMMsg.dwTime=pMsg->dwTime;
			if (bFlag)
			{
				GetApp()->m_LoginServer.SendMsgToLoginSrv(&ATalkMaskGMMsg,sizeof(SATalkMaskGMMsg));
			}
		}
		break;
    case SGMMMsg::EPRO_GM_TALKMASK:
        {
            SATalkMaskGMMsg ATalkMaskGMMsg;
            WORD wGMLevel;
            SQTalkMaskGMMsg *pQTalkMaskGMMsg = (SQTalkMaskGMMsg *)pGMMMsg;
            if (bDistribute == FALSE)
            {
                CPlayer *pPlayer = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
                if (pPlayer == NULL) 
                {

                    ATalkMaskGMMsg.dwTime = 0;
					memcpy(ATalkMaskGMMsg.szName, pQTalkMaskGMMsg->szName, CONST_USERNAME);
                    ATalkMaskGMMsg.wResult = 0;
                    GetApp()->m_LoginServer.SendMsgToLoginSrv(&ATalkMaskGMMsg,sizeof(SATalkMaskGMMsg));
                    return FALSE;
                }

                wGMLevel = (WORD)pPlayer->m_Property.m_GMLevel;
            }
            else
            {
                wGMLevel = pGMMMsg->wGMLevel;
            }

            CPlayer *pPlayerII = (CPlayer*)GetPlayerByName(pQTalkMaskGMMsg->szName)->DynamicCast(IID_PLAYER);
            if (pPlayerII == NULL)
            {
                if (bDistribute)
                {
                    extern void TalkToDnid(DNID, LPCSTR,bool addPrefix = true,bool bPickUp = false, WORD wShowType=1);
                    TalkToDnid(dnidClient, "找不到禁言的目标玩家！");
                }

                return FALSE;
            }
            
            if (m_pLevelChecker->CheckLevel(GM_ACT_TALKMASK,wGMLevel))
            {
                extern std::map<std::string, DWORD> GMTalkMask; // 禁言映射表
                if (pQTalkMaskGMMsg->dwTime != 0)
                {
                    GMTalkMask[pPlayerII->GetAccount()] = timeGetTime() + pQTalkMaskGMMsg->dwTime*60000;

                    void TalkToAll(LPCSTR info);
                    LPCSTR FormatString(LPCSTR, ...);
                    //TalkToAll(FormatString("[%s]被封了哑穴", pQTalkMaskGMMsg->szName));
					ATalkMaskGMMsg.wResult = 1;
                }
                else
				{
                    GMTalkMask.erase(pPlayerII->GetAccount());
					ATalkMaskGMMsg.wResult = 4;	//解除禁言
				}
                
            }
            else
            {
                ATalkMaskGMMsg.wResult = 3; //权限不够
            }

            pQTalkMaskGMMsg->szName[10] = 0;

			memcpy(ATalkMaskGMMsg.szName, pQTalkMaskGMMsg->szName, CONST_USERNAME);
            ATalkMaskGMMsg.dwTime = pQTalkMaskGMMsg->dwTime;
            if(bDistribute == TRUE)
            {
                ATalkMaskGMMsg.dnidClient = pQTalkMaskGMMsg->dnidClient;
                GetApp()->m_LoginServer.SendMsgToLoginSrv(&ATalkMaskGMMsg,sizeof(SATalkMaskGMMsg));
            }
            else
            {
                g_StoreMessage(dnidClient, &ATalkMaskGMMsg, sizeof(SATalkMaskGMMsg));
            }
        }
        break;

    case SGMMMsg::EPRO_GM_MOVETOHIM:
        {
//             CPlayer * pPlayer = NULL;
//             CPlayer * pPlayerMe = NULL;
//             SQMoveTohimMsg * pQMoveTohimMsg;
//             pQMoveTohimMsg = (SQMoveTohimMsg *)pGMMMsg;
//             SAMoveTohimMsg AMoveTohimMsg;
// 
//             char szName[CONST_USERNAME];
//             memcpy(szName,pQMoveTohimMsg->szName,CONST_USERNAME);
//             pPlayer = (CPlayer*)GetPlayerByName(szName)->DynamicCast(IID_PLAYER);
//             pPlayerMe = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
// 
//             if(pPlayerMe == NULL || pPlayer == NULL)
//             {
//                 AMoveTohimMsg.wResult = 2; // 没这个玩家啊
//             }
//             else
//             {
//                 memcpy(szName,pPlayerMe->m_Property.m_Name,CONST_USERNAME);
//                 if (m_pLevelChecker->CheckLevel(GM_ACT_MOVEPLAYER, (WORD)pPlayer->m_Property.m_GMLevel))
//                 {
//                     if(!MoveSomebody(szName,pPlayer->m_Property.m_CurRegionID,pPlayer->m_wCurX,pPlayer->m_wCurY))
//                     {
//                         AMoveTohimMsg.wResult = 0;// 失败
//                     }
//                     else
//                     {
//                         AMoveTohimMsg.wResult = 1;// 成功       
//                     }
//                 }
//                 else
//                 {
//                     AMoveTohimMsg.wResult = 3; // 权限不够
//                 }
//             }
//             g_StoreMessage(dnidClient,&AMoveTohimMsg,sizeof(SAMoveTohimMsg));
        }
        break;
    case SGMMMsg::EPRO_GM_MOVEBYME:
        {
//             SQMovebyMeGMMsg * pQMovebyMeGMMsg;
//             pQMovebyMeGMMsg = (SQMovebyMeGMMsg *)pGMMMsg;
//             SAMovebyMeGMMsg AMovebyMeGMMsg;
//             char szName[CONST_USERNAME];
//             memcpy(szName,pQMovebyMeGMMsg->szName,CONST_USERNAME);
// 
//             CPlayer *pPlayer = NULL;
//             pPlayer = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
//             if(pPlayer==NULL)
//             {
//                 AMovebyMeGMMsg.wResult = 2;
//             }
// 
//             WORD regionid = pPlayer->m_Property.m_CurRegionID;
//             WORD x = pPlayer->m_wCurX;
//             WORD y = pPlayer->m_wCurY;
// 
//             if (!MoveSomebody(szName,regionid,x,y))
//             {
//                 rfalse(2, 1, "当前服务器上找不到这个玩家");
//                 AMovebyMeGMMsg.wResult = 0; //没这个玩家啊
//             }
//             else
//             {
//                 rfalse(2, 1, "将[%s]移动到[%d][%d/%d]", szName, regionid, x, y);
//                 AMovebyMeGMMsg.wResult = 1; //成功
//             }
//             g_StoreMessage(dnidClient,&AMovebyMeGMMsg,sizeof(SQMovebyMeGMMsg));
        }
        break;
    case SGMMMsg::EPRO_GM_ACCOUNTMASK:
        {
            
        }
        break;
    case SGMMMsg::EPRO_GM_ACCOUNTUNMASK:
        {

        }
        break;
    case SGMMMsg::EPRO_GM_CHANGENAME:
        {
            
        }
        break;
    case SGMMMsg::EPRO_GM_CHANGEMORE:
        {
        }
        break;
    case SGMMMsg::EPRO_CHECK_SB_INFO: //查某人的资料（包括等级，地图位置）
        {
            SQCheckSBInfoGMMsg * pQCheckSBInfoGMMsg;
            pQCheckSBInfoGMMsg = (SQCheckSBInfoGMMsg *)pGMMMsg;
            SACheckSBInfoGMMsg ACheckSBInfoGMMsg;
            CPlayer *pPlayer ;
            CPlayer *pMe;
            pMe = (CPlayer*)GetPlayerByDnid(dnidClient)->DynamicCast(IID_PLAYER);
            if(pMe == NULL)
            {

            }
            else
            {

                if (m_pLevelChecker->CheckLevel(GM_ACT_CHECKPLAYER, (WORD)pMe->m_Property.m_GMLevel))
                {
                    pPlayer = (CPlayer*)GetPlayerByName(pQCheckSBInfoGMMsg->szName)->DynamicCast(IID_PLAYER);
                    if(pPlayer == NULL)
                    {
                        ACheckSBInfoGMMsg.wResult = 0;
                    }
                    else
                    { 
                        ACheckSBInfoGMMsg.wResult = 1;
                        memcpy(&(ACheckSBInfoGMMsg.PlayerData),(SFixBaseData*)&(pPlayer->m_Property),sizeof(SFixBaseData));
                        //ACheckSBInfoGMMsg.PlayerData.m_wSegX = pPlayer->m_wCurX;
                        //ACheckSBInfoGMMsg.PlayerData.m_wSegY = pPlayer->m_wCurY;
                        memcpy(&(ACheckSBInfoGMMsg.PlayerPackage),(SFixPackage*)&(pPlayer->m_Property),sizeof(SFixPackage));
                        memcpy(&(ACheckSBInfoGMMsg.PlayerTasks),(SPlayerTasks*)&(pPlayer->m_Property),sizeof(SPlayerTasks));
                    }
                }
                else
                {
                    ACheckSBInfoGMMsg.wResult = 3;
                }
            }
            g_StoreMessage(dnidClient,&ACheckSBInfoGMMsg,sizeof(SACheckSBInfoGMMsg));
        }  
        break;
    case SGMMMsg::EPRO_UPDATE_SB_INFO:
        {
//             SQUpdateSBInfoMsg * pQUpdateSBInfoMsg;
//             pQUpdateSBInfoMsg = (SQUpdateSBInfoMsg *)pGMMMsg;
//             SAUpdateSBInfoMsg AUpdateSBInfoMsg;
// 			
// 			AUpdateSBInfoMsg.dnidClient=pQUpdateSBInfoMsg->dnidClient;
// 
// 			dwt::strcpy(AUpdateSBInfoMsg.szName,pQUpdateSBInfoMsg->PlayerData.m_Name,CONST_USERNAME);
// 
//             AUpdateSBInfoMsg.wResult = GetApp()->m_pGameWorld->UpdatePlayerData(&(pQUpdateSBInfoMsg->PlayerData));
//             if(AUpdateSBInfoMsg.wResult == 1)
// 			{
//                 SQUpdateSBInfoMsg QUpdateSBInfoMsg;
//                 memcpy(&QUpdateSBInfoMsg,pQUpdateSBInfoMsg,sizeof(SQUpdateSBInfoMsg));
// 				//QUpdateSBInfoMsg.PlayerData.m_dwYuanBao = 0;
// 				//QUpdateSBInfoMsg.PlayerData.m_dwZenBao = 0;
//                 g_StoreMessage(dnidClient,&QUpdateSBInfoMsg,sizeof(SQUpdateSBInfoMsg));
//             }
//             GetApp()->m_LoginServer.SendMsgToLoginSrv(&AUpdateSBInfoMsg,sizeof(SAUpdateSBInfoMsg));
        }
        break;
	//-------------------------------------------------------
	//Add By Lovelonely
	case SGMMMsg::EPRO_GM_GETMASKNAME:
		{
			extern std::map<std::string, DWORD> GMTalkMask;

			SQGetMaskNameGMMsg *pMsg=(SQGetMaskNameGMMsg*)pGMMMsg;

			SAGetMaskNameGMMsg aGetMaskMsg;
			
			aGetMaskMsg.dnidClient=pMsg->dnidClient;

			aGetMaskMsg.szName[0]=NULL;
			aGetMaskMsg.wPara=SAGetMaskNameGMMsg::PA_BEGIN;
			aGetMaskMsg.wResult=SAGetMaskNameGMMsg::RT_TALKMASK;
			
			GetApp()->m_LoginServer.SendMsgToLoginSrv(&aGetMaskMsg,sizeof(SAGetMaskNameGMMsg));
			
			aGetMaskMsg.wPara=SAGetMaskNameGMMsg::PA_SEND;

			std::map<std::string, DWORD>::iterator it=GMTalkMask.begin();

			while (it != GMTalkMask.end())
			{
				dwt::strcpy(aGetMaskMsg.szName,it->first.c_str(),20);

				GetApp()->m_LoginServer.SendMsgToLoginSrv(&aGetMaskMsg,sizeof(SAGetMaskNameGMMsg));

				it++;
			}
			
			aGetMaskMsg.szName[0]=NULL;
			aGetMaskMsg.wPara=SAGetMaskNameGMMsg::PA_END;

			GetApp()->m_LoginServer.SendMsgToLoginSrv(&aGetMaskMsg,sizeof(SAGetMaskNameGMMsg));
		}
		break;
	case SGMMMsg::EPRO_GM_GETPING:
		{

			SQGetPingGMMsg *pMsg=(SQGetPingGMMsg*)pGMMMsg;

            if (dnidClient != INVALID_DNID)
            if (pMsg->iPing == 1234567890)
            {
                rfalse(2, 1, "客户端直接返回了ping消息！！！");

				sockaddr_in addr;
				ZeroMemory(&addr,sizeof(sockaddr_in));
				if (GetApp()->m_Server.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in)))
				{
                    rfalse(2, 1, "IP = %s", ::inet_ntoa(addr.sin_addr));
				}

                return FALSE;
            }

			if (pMsg->wPara == SQGetPingGMMsg::PA_GET)
			{
                if ((dnidClient != INVALID_DNID) && (pMsg->iPing != 0xabcdef00))
                {
                    rfalse(2, 1, "客户端传来 SQGetPingGMMsg::PA_GET ！！！");
                    return FALSE;
                }

				SQGetPingGMMsg qGetPingMsg;

				qGetPingMsg.dnidClient=pMsg->dnidClient;
                qGetPingMsg.wPara=SQGetPingGMMsg::PA_RETURN;
				qGetPingMsg.iPing=1234567890;
				qGetPingMsg.ip32=0;
                qGetPingMsg.mac64=0;

				BroadcastMsg(&qGetPingMsg,sizeof(SQGetPingGMMsg));

				return TRUE;
			}
			if (pMsg->wPara == SQGetPingGMMsg::PA_RETURN)
			{
				SQGetPingGMMsg qGetPingMsg;

				qGetPingMsg.dnidClient=pMsg->dnidClient;

				sockaddr_in addr;

				ZeroMemory(&addr,sizeof(sockaddr_in));

				if(GetApp()->m_Server.GetDnidAddr(dnidClient, &addr, sizeof(sockaddr_in)))
				{
                    /*
					if ( char * pChar = inet_ntoa( addr.sin_addr ) ) 
						memcpy(qGetPingMsg.szIP,inet_ntoa(addr.sin_addr), 17);
					else
						return FALSE;
                    */

                    // 16进制IP地址。。。
                    qGetPingMsg.ip32 = addr.sin_addr.S_un.S_addr;
                    qGetPingMsg.mac64 = pMsg->mac64;
				}

				qGetPingMsg.iPing=pMsg->iPing;
				qGetPingMsg.wPara=pMsg->wPara;

				GetApp()->m_LoginServer.SendMsgToLoginSrv(&qGetPingMsg,sizeof(SQGetPingGMMsg));

				return TRUE;
			}
		}
		break;
	case SGMMMsg::EPRO_GM_CHECKONLINEGM:
		{
			SQCheckOnlineGMGMMsg *pMsg=(SQCheckOnlineGMGMMsg*)pGMMMsg;

			if (pMsg->wPara == SQCheckOnlineGMGMMsg::PA_GET)
			{
				SQCheckOnlineGMGMMsg msg;
				
				memcpy(&msg,pMsg,sizeof(SQCheckOnlineGMGMMsg));

				GetApp()->m_LoginServer.SendMsgToLoginSrv(&msg,sizeof(SQCheckOnlineGMGMMsg));
				
				return TRUE;
			}

			if (pMsg->wPara == SQCheckOnlineGMGMMsg::PA_RETURN)
			{
				CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szName)->DynamicCast(IID_PLAYER);
				if (pPlayer == NULL)
				{
					return FALSE;
				}
				if (pMsg->szGM[0][0] == NULL)
				{
					TalkToDnid(pPlayer->m_ClientIndex,"当前无在线GM");

					return TRUE;
				}
				for (int i=0 ; i < 10; i++)
				{
					if (pMsg->szGM[i][0] != NULL)
					{
						char szBuff[64];
						sprintf(szBuff,"当前在线GM:%s",pMsg->szGM[i]);

						TalkToDnid(pPlayer->m_ClientIndex,szBuff);

					}
				}
				return TRUE;
			}
			
		}
		break;
	case SGMMMsg::EPRO_GM_SAYGLOBAL:
		{
			SQSayGlobalGMMsg *pMsg=(SQSayGlobalGMMsg*)pGMMMsg;
            void TalkToAll( LPCSTR info, SRawItemBuffer *item, LPCSTR name );
            TalkToAll( pMsg->_szSay, NULL, pMsg->szName );

            SASayGlobalGMMsg msg;
            memcpy( msg._szSay, pMsg->_szSay, sizeof( msg._szSay ) );
            memcpy( msg.szName, pMsg->szName, sizeof( msg.szName ) );
            msg.wResult = SASayGlobalGMMsg::RT_SUC;
			GetApp()->m_LoginServer.SendMsgToLoginSrv(&msg,sizeof(msg));
		}
	    break;
	case SGMMMsg::EPRO_GM_WISPER:
		{
			SAWisperGMMsg *pMsg=(SAWisperGMMsg*)pGMMMsg;
			CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szPlayer)->DynamicCast(IID_PLAYER);
			if (pPlayer == NULL)
			{
				return FALSE;
			}

			if (pMsg->wResult == SAWisperGMMsg::RT_ERROR)
			{
				
				TalkToDnid(pPlayer->m_ClientIndex,"你密的GM不在线上!");
				return TRUE;
			}

			SAChatWisperMsg aChatWisper;

			dwt::strcpy(aChatWisper.szName,pMsg->szGM,CONST_USERNAME);
			dwt::strcpy(aChatWisper.cChatData,pMsg->szSay,MAX_CHAT_LEN);
			aChatWisper.byType=1;

			g_StoreMessage(pPlayer->m_ClientIndex,&aChatWisper,sizeof(SAChatWisperMsg));

		}
		break;
	case SGMMMsg::EPRO_GM_CLOSE:
		{
			SQCloseGMMsg *pMsg=(SQCloseGMMsg*)pGMMMsg;
			SACloseGMMsg aCloseMsg;
			dwt::strcpy(aCloseMsg.szName,pMsg->szName,CONST_USERNAME);

			CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szName)->DynamicCast(IID_PLAYER);

			aCloseMsg.wResult=SACloseGMMsg::RT_ERROR;
			dwt::strcpy(aCloseMsg.szGM, pMsg->szGM, CONST_USERNAME);

			if (pMsg->wPara == SQCloseGMMsg::PARA_CLOSE)
			{
				if (MoveSomebody(pMsg->szName,101,20,20))
				{
					aCloseMsg.wResult=SACloseGMMsg::RT_CLOSESUC;
				}
			}

			if (pMsg->wPara == SQCloseGMMsg::PARA_SETFREE)
			{
				if (MoveSomebody(pMsg->szName,1,93,63))
				{
					aCloseMsg.wResult=SACloseGMMsg::RT_SETFREESUC;
				}
			}
			GetApp()->m_LoginServer.SendMsgToLoginSrv(&aCloseMsg,sizeof(SACloseGMMsg));
		}
		break;
	case SGMMMsg::EPRO_GM_CUT:
		{
			SQCutGMMsg *pMsg=(SQCutGMMsg*)pGMMMsg;
			
			CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szName)->DynamicCast(IID_PLAYER);

			if (pPlayer == NULL)
			{
				return FALSE;
			}

			pPlayer->Logout(true);
			
			g_CutClient(pPlayer->m_ClientIndex);
		}
		break;
	case SGMMMsg::EPRO_GM_PUBLICINFO:
		{
			SQPublicInfoGMMsg *pMsg=(SQPublicInfoGMMsg*)pGMMMsg;

			if (pMsg->wType == SQPublicInfoGMMsg::TP_ROLL)
			{
				SQPublicInfoGMMsg qPublicInfoMsg;

				memcpy(&qPublicInfoMsg,pMsg,sizeof(SQPublicInfoGMMsg));
				
				qPublicInfoMsg.byLength=dwt::strlen(qPublicInfoMsg.szSay,128);

				BroadcastMsg(&qPublicInfoMsg,qPublicInfoMsg.GetMySize());
				break;
			}

			if (pMsg->wType == SQPublicInfoGMMsg::TP_CHANNEL)
			{
				TalkToAll(pMsg->szSay);

				break;
			}
		}
		break;
    case SGMMMsg::EPRO_GM_CHECKGMIDINF:
        {
            if ( pGMMMsg == NULL || GetApp() == NULL || (GetApp()->m_pGameWorld) == NULL )
                return FALSE;

            SQCheckGMInfMsg *pMsg = (SQCheckGMInfMsg*)pGMMMsg;

            CPlayer *pPlayer = (CPlayer*)GetPlayerByName(pMsg->szName)->DynamicCast(IID_PLAYER);
            if (pPlayer == NULL)
                return FALSE;

            GetApp()->m_pGameWorld->DispatchGMIDCheckMsg(dnidClient, pMsg, pPlayer);
        }
        break;
	//--------------------------------------------------------
    }
    return TRUE;
}


BOOL CNetGMModule::GetORBChatMessage(SORBMsg *pMsg)
{
    SAGMCheckChatGMMsg AGMCheckChatGMMsg;
    BOOL bSendMessage = FALSE;
    AGMCheckChatGMMsg.wChatLen = 0;
    AGMCheckChatGMMsg.wChatType = 0;
    AGMCheckChatGMMsg.wGMLevel = 0;
    //if(m_bSendChatCheck)
    //{
        switch(pMsg->_protocol)
        {
        //case SORBMsg::EPRO_CTRL_TEAMGROUP:
        //    {
        //        bSendMessage = TRUE;
        //        SATeamTalkMsg * pTeamTalkMsg = (SATeamTalkMsg*)(STeamOperationMsg*)(SCtrlTeamGroup*)pMsg;
        //        memcpy(AGMCheckChatGMMsg.szMessage,pTeamTalkMsg->szTalkMsg,50);
        //        memcpy(AGMCheckChatGMMsg.szNameTalker,pTeamTalkMsg->szTalkerName,MAX_NAMELEN);
        //        AGMCheckChatGMMsg.wChatType = SChatBaseMsg::EPRO_CHAT_TEAM;
        //    }
        //    break;
        case SORBMsg::EPRO_CTRL_CHAT:
            {
                SORBChatBaseMsg * pORBChatBaseMsg = (SORBChatBaseMsg *)pMsg;
                switch(pORBChatBaseMsg->_protocol)
                {
                case SORBChatBaseMsg::EPRO_CHAT_WISPER:
                    {
                        SAORBChatWisperMsg * pAORBChatWisperMsg = (SAORBChatWisperMsg *)pORBChatBaseMsg;
                        if(pAORBChatWisperMsg->byType == 1)
                        {
                            bSendMessage = TRUE;
                            memcpy(AGMCheckChatGMMsg.szNameTalker,pAORBChatWisperMsg->cName,CONST_USERNAME);
                            memcpy(AGMCheckChatGMMsg.szNameRecver,pAORBChatWisperMsg->cNameII,CONST_USERNAME);
                            memcpy(AGMCheckChatGMMsg.szMessage,pAORBChatWisperMsg->cChatData,MAX_CHAT_LEN);
                            AGMCheckChatGMMsg.wChatType = SChatBaseMsg::EPRO_CHAT_WHISPER;
                            if(pAORBChatWisperMsg->dwGMLevel != 0)
                                AGMCheckChatGMMsg.wGMLevel = (WORD)pAORBChatWisperMsg->dwGMLevel;
                            else if(pAORBChatWisperMsg->dwGMLevelII != 0)
                                AGMCheckChatGMMsg.wGMLevel = (WORD)pAORBChatWisperMsg->dwGMLevelII;
                            else
                                AGMCheckChatGMMsg.wGMLevel = 0;
                        }
                    }
                    break;
                }
            }
            break;
        }
        if(bSendMessage == TRUE)
            GetApp()->m_LoginServer.SendMsgToLoginSrv(&AGMCheckChatGMMsg,sizeof(SAGMCheckChatGMMsg));
    //}
    return TRUE;
}
BOOL CNetGMModule::GetChatMessage(DNID dnidClient,SChatBaseMsg * pChatBaseMsg,CPlayer *pPlayer)
{
    SAGMCheckChatGMMsg AGMCheckChatGMMsg;
    AGMCheckChatGMMsg.wChatLen = 0;
    BOOL bSendMessage = TRUE;
    memcpy(AGMCheckChatGMMsg.szMessage,"\0",1);
    memcpy(AGMCheckChatGMMsg.szNameRecver,"\0",1);
    memcpy(AGMCheckChatGMMsg.szNameTalker,"\0",1);

    //if(m_bSendChatCheck)
    //{
        AGMCheckChatGMMsg.wChatType = pChatBaseMsg->_protocol;
        AGMCheckChatGMMsg.wGMLevel = (WORD)pPlayer->m_Property.m_GMLevel;
        switch(pChatBaseMsg->_protocol)
        {
        case SChatBaseMsg::EPRO_CHAT_GLOBAL:
            {
                SQChatGlobalMsg * pQChatGlobalMsg = (SQChatGlobalMsg *)pChatBaseMsg;
                AGMCheckChatGMMsg.wChatPara = pQChatGlobalMsg->byType;
                memcpy(AGMCheckChatGMMsg.szMessage,pQChatGlobalMsg->cChatData,MAX_CHAT_LEN);
                memcpy(AGMCheckChatGMMsg.szNameTalker,pQChatGlobalMsg->cTalkerName,CONST_USERNAME);

                // 特殊处理！
                if ( AGMCheckChatGMMsg.wChatPara == SAChatGlobalMsg::ECT_SPECIAL + 10 )
                {
                    AGMCheckChatGMMsg.wChatPara = SAChatGlobalMsg::ECT_TRADE;
                    dwt::strcpy( AGMCheckChatGMMsg.szNameTalker, pPlayer ? pPlayer->GetName() : "", CONST_USERNAME );
                }
            }
            break;
        case SChatBaseMsg::EPRO_CHAT_SYSMSG:
            {
                SChatSysMsg * pChatSysMsg = (SChatSysMsg *)pChatBaseMsg;
                AGMCheckChatGMMsg.wChatPara = pChatSysMsg->btMsgID;
                if(pPlayer != NULL)
                {
                    memcpy(AGMCheckChatGMMsg.szNameTalker,pPlayer->GetName(),CONST_USERNAME);
                }
                memcpy(AGMCheckChatGMMsg.szMessage,pChatSysMsg->cSystem,MAX_CHAT_LEN);
            }
            break;
        case SChatBaseMsg::EPRO_CHAT_SYSCALL:			// 系统公告
            {
                SChatToAllMsg * pChatToAllMsg = (SChatToAllMsg *)pChatBaseMsg;
                memcpy(AGMCheckChatGMMsg.szMessage,pChatToAllMsg->cChat,MAX_CHAT_LEN);
                AGMCheckChatGMMsg.wChatLen = pChatToAllMsg->wChatLen;
            }
            break;
        case SChatBaseMsg::EPRO_CHAT_PUBLIC:			// 公众
            {
                SQChatPublic * pQChatPublic = (SQChatPublic*)pChatBaseMsg;
                memcpy(AGMCheckChatGMMsg.szMessage,pQChatPublic->cCharData,MAX_CHAT_LEN);
                AGMCheckChatGMMsg.wChatLen = pQChatPublic->wLength;
                if(pPlayer != NULL)
                {
                    memcpy(AGMCheckChatGMMsg.szNameTalker,pPlayer->GetName(),CONST_USERNAME);
                }
            }
            break;
        case SChatBaseMsg::EPRO_CHAT_WHISPER:			// 密聊
            {
                SQChatWisperMsg * pChatWisper = (SQChatWisperMsg*)pChatBaseMsg;
                memcpy(AGMCheckChatGMMsg.szMessage,pChatWisper->cChatData,MAX_CHAT_LEN);
                memcpy(AGMCheckChatGMMsg.szNameRecver, pChatWisper->szRecvName, CONST_USERNAME);
                if(pPlayer != NULL)
                    memcpy(AGMCheckChatGMMsg.szNameTalker,pPlayer->GetName(),CONST_USERNAME);
            }
            break;
        case SChatBaseMsg::EPRO_CHAT_SCHOOL:
            {
                SQChatSchool *pSchool = ( SQChatSchool *)pChatBaseMsg;
                 if(pPlayer != NULL)
                     AGMCheckChatGMMsg.wChatPara = pPlayer->m_Property.m_School;

                memcpy(AGMCheckChatGMMsg.szMessage,pSchool->cChatData,MAX_CHAT_LEN);
                memcpy(AGMCheckChatGMMsg.szNameTalker,pSchool->cTalkerName,CONST_USERNAME);

            }
            break;
        default:
            {
                bSendMessage = FALSE;
            }
            break;
        }
        if(bSendMessage == TRUE)
            GetApp()->m_LoginServer.SendMsgToLoginSrv(&AGMCheckChatGMMsg,sizeof(SAGMCheckChatGMMsg));
    //}
    return TRUE;
}
#endif
