#include "stdafx.h"
#include "NetworkModule/OrbMsgs.h"
#include "Relation.h"
#include "Player.h"
#include "BaseObject.h"
#include "GlobalFunctions.h"
#include "relationextra.h"

//#include "NETWORKMODULE/RelationMsgs.h"

extern LPIObject GetPlayerByName(LPCSTR szName);

CRelations::CRelations()
{
};

CRelations::~CRelations()
{
};

int CRelations::Init(SFixProperty *pData)// 初始化人际关系列表
{
	return 0;
}

int CRelations::Init(SFixData *pData)
{
	if (!pData)
	{
		return 0;
	}
// 	for (size_t i = 0; i < MAX_FRIEND_NUMBER; i++)
// 	{
// 		if (dwt::strlen(pData->m_sPlayerRelation[i].m_PlayerFriends->szName, CONST_USERNAME) > 0)
// 		{
// 			AddRelation_new(pData->m_sPlayerRelation.m_PlayerFriends[i]->szName, pData->m_sPlayerRelation.m_PlayerFriends[i]->byRelation, TRUE);
// 		}
// 	}
	return 0;
}



extern LPIObject GetPlayerByDnid(DNID dnidClient);

BOOL CRelations::AddRelation_new( BYTE byRelation,tRelationData relationdata ,BOOL bSend)
{
	CPlayer *p = static_cast<CPlayer*>(this);
	BOOL m_bRet = FALSE;
	switch(byRelation)
	{
	case  RT_FRIEND: //好友
		{
			///好友最大数量不能大于MAX_FRIEND_NUMBER
			if (GetFriendNum()<MAX_FRIEND_NUMBER)
			{
				//在好友列表中没有找到则添加
				if (!FindRelation_new(relationdata.sName,byRelation))
				{
					dwt::stringkey<char[CONST_USERNAME]> strname;
					dwt::strcpy(strname, relationdata.sName, CONST_USERNAME);
					m_mapFriendList[strname]=relationdata;
					m_bRet = TRUE;
				}

				//如果在黑名单列表中应删除
				if(FindRelation_new(relationdata.sName,RT_BLACKLIST)){
					RemoveRelation_new(relationdata.sName,RT_BLACKLIST);
				}
			}
		}
		break;
	case  RT_BLACKLIST: //黑名单
		{
			//if (GetBlockNumber()<MAX_BLOCK_NUMBER)
			{
				//在黑名单列表中没有找到则添加
				if (!FindRelation_new(relationdata.sName,byRelation))
				{
					dwt::stringkey<char[CONST_USERNAME]> strname;
					dwt::strcpy(strname, relationdata.sName, CONST_USERNAME);
					m_mapBlockList[strname]=relationdata;
					m_bRet = TRUE;
				}
				//如果在好友列表中应删除
				if(FindRelation_new(relationdata.sName,0)){
					RemoveRelation_new(relationdata.sName,0);
				}

				//如果在临时列表中应删除
				if(FindRelation_new(relationdata.sName,RT_TEMP)){
					RemoveRelation_new(relationdata.sName,RT_TEMP);
				}
			}
		}
		break;
	}
		//给玩家发送消息
		if (m_bRet&&bSend)
		{

			
		}
	return m_bRet;
}

BOOL CRelations::AddRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend, int nDegree)
{
	CPlayer *p = static_cast<CPlayer*>(this);
	if(!strName)return FALSE;
	CPlayer *pDesPlayer = (CPlayer*)GetPlayerByName(strName)->DynamicCast(IID_PLAYER);
	if(!pDesPlayer)return FALSE;
	tRelationData relationdata;
	dwt::strcpy(relationdata.sName, pDesPlayer->GetName(), CONST_USERNAME);

	relationdata.wLevle=pDesPlayer->m_Property.m_Level;
	if (p)
	{
		return p->AddRelation_new(byRelation,relationdata,bSend);
	}
	return FALSE;
}

//好友上线通知
void CRelations::FriendsOnline(SFriendOnlineMsg *pFriendOnlineMsg)
{
	char szFriendsName[MAX_FRIEND_NUMBER][CONST_USERNAME];
    
    int iFriendsCount;
    SQORBFriendOnlineMsg orbmsg;

    CPlayer * pPlayer;
    pPlayer = static_cast<CPlayer *>(this);
    int iLooper;
	//清空字串
    for(iLooper = 0 ;iLooper < MAX_FRIEND_NUMBER ;iLooper++)
        szFriendsName[iLooper][0] = '\0';

    //取得好友的列表字符串
	iFriendsCount = 0;

	//给上线好友发好友上线通知
    for(iLooper = 0;iLooper < iFriendsCount;iLooper ++)
    {
        // ### 说明：为了取消分布式玩家管理器，该功能被弱化！以后可尝试通过orbserver来实现
        // 该地方的分布操作主要是为了判断对方是否在本机，以及是否在游戏中
        // 应该该功能不会被弱化，只可能会稍微增加内网数据流量
        //SSimplePlayer simplayer;
        //if(TRUE == LocateDistributedSimplePlayer(szFriendsName[iLooper],simplayer))
        {
            //如果玩家在本台服务器
            // if(simplayer.wServerID == GetServerID())
            if ( CPlayer *theFriend = ( CPlayer* )GetPlayerByName( szFriendsName[iLooper] )->DynamicCast( IID_PLAYER ) )
            {
                if ( theFriend->m_OnlineState == 1 )
                {
//                    if ( theFriend->FindRelation( pPlayer->GetName(), 0 ) )
                    {
                        g_StoreMessage( theFriend->m_ClientIndex, pFriendOnlineMsg, sizeof(SFriendOnlineMsg) );
                    }
                }
            }
            else
            //如果玩家在别的服务器
            {
				SendMessageToPlayer( szFriendsName[iLooper], pFriendOnlineMsg, sizeof( SFriendOnlineMsg ) );
            }
        }
    }
}





BOOL CRelations::FindRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation)
{
	MY_ASSERT(strName);
	switch(byRelation)
	{
	case  RT_FRIEND://好友
		{
			if (m_mapFriendList.empty())return FALSE;
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_mapFriendList.find(strName);
			return m_frienditer!=m_mapFriendList.end();
		}
		break;
	case  RT_BLACKLIST://黑名单
		{
			if(m_mapBlockList.empty())return FALSE;
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_mapiter = m_mapBlockList.find(strName);
			return m_mapiter!=m_mapBlockList.end();
		}
		break;
	}
	return FALSE;
}

BOOL CRelations::RemoveRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend)
{
	MY_ASSERT(strName);
	BOOL bResult = FALSE;
	switch(byRelation)
	{
	case  RT_FRIEND://好友
		{
			if (m_mapFriendList.empty())return FALSE;
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::iterator m_mapiter = m_mapFriendList.find(strName);
			 if(m_mapiter!=m_mapFriendList.end()){
				 m_mapFriendList.erase(m_mapiter);
				 bResult = TRUE;
			 }
		}
		break;
	case  RT_BLACKLIST://黑名单
		{
			if(m_mapBlockList.empty())return FALSE;
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::iterator m_mapiter = m_mapBlockList.find(strName);
			if(m_mapiter!=m_mapBlockList.end()){
				m_mapBlockList.erase(m_mapiter);
				 bResult = TRUE;
			}
		}
		break;
	}
		if (bSend&&bResult)
		{
			CPlayer *pPlayer =  static_cast<CPlayer*>(this);
			if (pPlayer)
			{
				SADelFriendMsg msg;
				memcpy(msg.szName, strName, CONST_USERNAME);
				msg.byResult = bResult;
				msg.byType = byRelation;
				g_StoreMessage(pPlayer->m_ClientIndex,&msg,sizeof(SADelFriendMsg));
			}
		}
	return bResult;
}
//byRealtion 
//0 -- 好友
//1 -- 黑名单
//2 -- 仇人名单
//3 -- 结拜名单
void CRelations::SynAllRelations_new( DNID dnidClient )
{
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_tempFriendList[4];
	m_tempFriendList[RT_FRIEND]=m_mapFriendList;			//好友
	m_tempFriendList[RT_BLACKLIST]=m_mapBlockList;			//1 -- 黑名单
	m_tempFriendList[RT_ENEMIES]=m_mapEnemieList;		//2 -- 仇人名单
	m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单
	//m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单
	for(int i=0;i<4;i++)
	{
		if (!m_tempFriendList[i].empty())
		{
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_tempFriendList[i].begin();
			while(m_frienditer!=m_tempFriendList[i].end())
			{
// 				tRelationData relationdata=m_frienditer->second;
// 				SAAddFriendMsg msg;
// 				dwt::strcpy(msg.szName, relationdata.sName, CONST_USERNAME);
// 				msg.byType = i;
// 
// 				msg.wLevle = relationdata.wLevle;
// 				if ( CPlayer *player = ( CPlayer* )GetPlayerByName( msg.szName )->DynamicCast( IID_PLAYER ) )
// 					msg.byStatus = ( player->m_OnlineState == 1 );
// 				else
// 					msg.byStatus = 0; 
// 				msg.byResult = TRUE ;
// 				g_StoreMessage(dnidClient,&msg,sizeof(SAAddFriendMsg));
// 				m_frienditer++;
			}
		}
	}

}

int CRelations::GetFriendDegree_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation /*= 0 */)
{
	if(byRelation<RT_FRIEND||byRelation>RT_TEMP)return byRelation;

	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_tempFriendList[5];
	m_tempFriendList[RT_FRIEND]=m_mapFriendList;			//好友
	m_tempFriendList[RT_BLACKLIST]=m_mapBlockList;			//1 -- 黑名单
	m_tempFriendList[RT_ENEMIES]=m_mapEnemieList;		//2 -- 仇人名单
	m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单
	m_tempFriendList[RT_TEMP]=m_mapTempList;			//4 -- 结拜名单



	return 0;
}

BOOL CRelations::SetPlayerRelations_new( SPlayerRelation *lpPR )
{
	m_mapFriendList.clear();
	m_mapBlockList.clear();
	m_mapEnemieList.clear();
	m_mapMateList.clear();
	m_mapTempList.clear();
	if(!lpPR)return FALSE;
	///好友
	int iLooper = 0;
	for (; iLooper < MAX_FRIEND_NUMBER ;iLooper++)
	{
		if (dwt::strlen(lpPR->m_PlayerFriends[iLooper].szName, CONST_USERNAME) != 0)
		{
			if(lpPR->m_PlayerFriends[iLooper].byRelation == RT_FRIEND) 
			{
				tRelationData relationdata;
				dwt::strcpy(relationdata.sName, lpPR->m_PlayerFriends[iLooper].szName, CONST_USERNAME);

				AddRelation_new(RT_FRIEND,relationdata,FALSE);
				//AddRelation_new(lpPR->m_PlayerFriends[iLooper].szName,RT_FRIEND, lpPR->m_PlayerFriends[iLooper].wDearValue,FALSE);
			}
		}
	}

	return TRUE;
}

BOOL CRelations::ChangeDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName, int &nDegree)
{
// 	MY_ASSERT(strName);
// 	CPlayer *pdest=(CPlayer*)GetPlayerByName(strName)->DynamicCast(IID_PLAYER);
// 	if (!pdest||(pdest&&pdest->m_OnlineState==CPlayer::OST_NOT_INIT))return FALSE;
// 
// 	if (GetFriendNum() == 0 )return FALSE;
// 	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::iterator m_mapiter = m_mapFriendList.find(strName);
// 	if(m_mapiter!=m_mapFriendList.end())
// 	{
// 		tRelationData reladata=m_mapiter->second;
// 		int changegree = reladata.nDegree;
// 		int nTempDegree = reladata.nDegree+nDegree;
// 		reladata.nDegree=nDegree>0?((nTempDegree>2000)?2000:nTempDegree):((nTempDegree<0)?0:nTempDegree);
// 		m_mapiter->second = reladata ;
// 		
// 		// 发个消息过去更新
// 		SFriendOnlineMsg msg;
// 		msg.eState = SFriendOnlineMsg::FRIEND_STATE_REFRESH;
// 		dwt::strcpy(msg.cName, strName, CONST_USERNAME);
// 		msg.byType = RT_FRIEND;
// 		msg.wDearValue = reladata.nDegree;
// 		g_StoreMessage( dnidClient, &msg, sizeof( msg ) );
// 		nDegree = reladata.nDegree - changegree;
// 		char str[200];
// 		sprintf(str,"你与玩家%s的好友度增加到%d",pdest->GetName(),reladata.nDegree);
// 		TalkToDnid(dnidClient,str);
// 		return TRUE;
// 	}
	return FALSE;
}

BOOL CRelations::SavePlayerRelations_new( SPlayerRelation *lpPR )
{
	if (lpPR)
		memset (lpPR, 0, sizeof (SPlayerRelation));
	else
		return FALSE;

	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_tempFriendList[5];
	m_tempFriendList[RT_FRIEND]=m_mapFriendList;			//好友
	m_tempFriendList[RT_BLACKLIST]=m_mapBlockList;			//1 -- 黑名单
	m_tempFriendList[RT_ENEMIES]=m_mapEnemieList;		//2 -- 仇人名单
	m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单

	int nCount = 0;
	
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_tempFriendList[RT_FRIEND].begin();
	while(m_frienditer!=m_tempFriendList[RT_FRIEND].end())
	{
		tRelationData relationdata=m_frienditer->second;
		dwt::strcpy(lpPR->m_PlayerFriends[nCount].szName, relationdata.sName, CONST_USERNAME);
		lpPR->m_PlayerFriends[nCount].wLevle = relationdata.wLevle;
		lpPR->m_PlayerFriends[nCount].byRelation = RT_FRIEND;

		nCount++;
		m_frienditer++;
	}

	nCount = 0 ;
	m_frienditer=m_tempFriendList[RT_BLACKLIST].begin();
	while(m_frienditer!=m_tempFriendList[RT_BLACKLIST].end())
	{
		tRelationData relationdata=m_frienditer->second;

		nCount++;
		m_frienditer++;
	}

	nCount = 0 ;
	m_frienditer=m_tempFriendList[RT_ENEMIES].begin();
	while(m_frienditer!=m_tempFriendList[RT_ENEMIES].end())
	{
		tRelationData relationdata=m_frienditer->second;

		nCount++;
		m_frienditer++;
	}

// 	nCount = 0 ;
// 	m_frienditer=m_tempFriendList[3].begin();
// 	while(m_frienditer++!=m_tempFriendList[3].end())
// 	{
// 		tRelationData relationdata=m_frienditer->second;
// 		dwt::strcpy(lpPR->m_PlayerMates[nCount].szName, relationdata.sName, CONST_USERNAME);
// 		lpPR->m_PlayerMates[nCount].wDearValue = relationdata.nDegree;
// 		lpPR->m_PlayerMates[nCount].bySchool = relationdata.bySchool;
// 		lpPR->m_PlayerMates[nCount].wLevle = relationdata.wLevle;
// 		lpPR->m_PlayerMates[nCount].byRelation = RT_MATE;
// 		nCount++;
// 	}

	return TRUE;
}

void CRelations::FriendsOnline_new( struct SFriendOnlineMsg *pFriendOnlineMsg,BYTE byRelation )
{
	if(!pFriendOnlineMsg)return;
	CPlayer * pPlayer;
	pPlayer = static_cast<CPlayer *>(this);
	if(!pPlayer)return;

	if (byRelation<RT_FRIEND||byRelation>RT_TEMP)return;

	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_tempFriendList[5];
	m_tempFriendList[RT_FRIEND]=m_mapFriendList;			//好友
	m_tempFriendList[RT_BLACKLIST]=m_mapBlockList;			//1 -- 黑名单
	m_tempFriendList[RT_ENEMIES]=m_mapEnemieList;		//2 -- 仇人名单
	m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单
	m_tempFriendList[RT_TEMP]=m_mapTempList;			//4 -- 结拜名单
	
	if (m_tempFriendList[byRelation].empty())return;
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_tempFriendList[byRelation].begin();
	while(m_frienditer!=m_tempFriendList[byRelation].end())
	{
		tRelationData relationdata=m_frienditer->second;
		if ( CPlayer *theFriend = ( CPlayer* )GetPlayerByName( relationdata.sName )->DynamicCast( IID_PLAYER ) )
		{
			if ( theFriend&&theFriend->m_OnlineState == CPlayer::OST_NORMAL )
			{
				///更新好友列表时查找玩家是否已经把我删除
				if ( theFriend->FindRelation_new( pPlayer->GetName(), byRelation ) )
				{
					g_StoreMessage( theFriend->m_ClientIndex, pFriendOnlineMsg, sizeof(SFriendOnlineMsg) );

					char str[200];
					if (pFriendOnlineMsg->eState ==SFriendOnlineMsg::FRIEND_STATE_ONLINE )
					{
						sprintf(str,"你的好友%s正在进入游戏",pPlayer->GetName());
					}
					else if(pFriendOnlineMsg->eState ==SFriendOnlineMsg::FRIEND_STATE_OUTLINE)
					{
						sprintf(str,"你的好友%s离开了游戏",pPlayer->GetName());
					}
					else if(pFriendOnlineMsg->eState ==SFriendOnlineMsg::FRIEND_STATE_LEVELUP)
					{
						sprintf(str,"你的好友%s升级到%d了，你也加油哦",pPlayer->GetName(),pFriendOnlineMsg->byFriendLevel);
					}
					else if(pFriendOnlineMsg->eState ==SFriendOnlineMsg::FRIEND_STATE_LEVELUPWITHITEM)
					{
						sprintf(str,"你的好友%s升级了还给你送道具",pPlayer->GetName());
					}
					TalkToDnid(theFriend->m_ClientIndex,str);
				}
				else if(byRelation == RT_FRIEND )
				{
					pPlayer->RemoveRelation_new(theFriend->GetName(),RT_FRIEND);
				}
			}
		}
		m_frienditer++;
	}
}

int CRelations::GetFriends_new(char szFriends[MAX_PEOPLE_NUMBER][CONST_USERNAME])
{
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_tempFriendList[5];
	m_tempFriendList[RT_FRIEND]=m_mapFriendList;			//好友
	m_tempFriendList[RT_BLACKLIST]=m_mapBlockList;			//1 -- 黑名单
	m_tempFriendList[RT_ENEMIES]=m_mapEnemieList;		//2 -- 仇人名单
	m_tempFriendList[RT_MATE]=m_mapMateList;			//3 -- 结拜名单
	m_tempFriendList[RT_TEMP]=m_mapTempList;			//4 -- 临时名单

	int iCount = 0;
	for(int i=0;i<4;i++)
	{
		if (!m_tempFriendList[i].empty())
		{
			std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_tempFriendList[i].begin();
			while(m_frienditer!=m_tempFriendList[i].end())
			{
				dwt::strcpy(szFriends[iCount], m_frienditer->first, CONST_USERNAME);
				iCount ++;
				m_frienditer++;
			}
		}
	}
	return iCount;
}

void CRelations::RefreshFriends_new()
{

}

int CRelations::GetOnlineFriendsNum()
{
	int iCount = 0;
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> ::const_iterator m_frienditer = m_mapFriendList.begin();
	while(m_frienditer!=m_mapFriendList.end())
	{
		if ( CPlayer *player = ( CPlayer* )GetPlayerByName( m_frienditer->first )->DynamicCast( IID_PLAYER ) )
		{
				if (player&&player->m_OnlineState == 1)
				{
					iCount ++;
				}
		}
		m_frienditer++;
	}
		
	return iCount;
}

BOOL CRelations::ProcessRelationMsg(DNID dnidClient, struct SRelationBaseMsg *pRelationMsg, CPlayer *pPlayer)
{
	if (pRelationMsg == NULL || dnidClient == INVALID_DNID || pPlayer == NULL)
		return FALSE;
	switch (pRelationMsg->_protocol)
	{
	case SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_ADD:
	{
		SQAddFriendMsg* pMsg = static_cast<SQAddFriendMsg*>(pRelationMsg);
		if (pMsg)
		{
			return OnRecvAddFriendMsg(dnidClient, pMsg, pPlayer);
		}
	}
	break;
	case SRelationBaseMsg::EPRO_CHARACTER_FRIENDLIST_INFO_DEL:
	{

	}
	break;
	case SRelationBaseMsg::EPRO_REGUEST_ADDFRIEND:
	{

	}
	break;
	}
	return TRUE;
}

BOOL CRelations::OnRecvAddFriendMsg(DNID dnidClient, struct SQAddFriendMsg *pRelationMsg, CPlayer *pPlayer)
{
	if (!pRelationMsg || !pPlayer)
	{
		return FALSE;
	}
	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("ProcessFriend"))
	{
		g_Script.PushParameter(pPlayer->GetSID());
		g_Script.PushParameter(pRelationMsg->dwFriendSid);
		g_Script.PushParameter(pRelationMsg->szName);
		g_Script.PushParameter(pRelationMsg->byType);
		g_Script.Execute();
	}
	
	g_Script.CleanPlayer();
	return TRUE;
}

BOOL CRelations::OnInitFriendData(CPlayer *pPlayer)
{
	if (!pPlayer)
	{
		return FALSE;
	}

	g_Script.SetPlayer(pPlayer);
	if (g_Script.PrepareFunction("LoadFriendData"))
	{
		g_Script.PushParameter(pPlayer->GetGID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	return TRUE;
}














// -----------------------------------------------------------------
