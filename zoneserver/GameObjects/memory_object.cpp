#include "StdAfx.h"
#include "memory_object.h"
#include "GameWorld.h"
#include "PlayerManager.h"
#include "Region.h"
extern int  backupNum;
Cmemory_object::Cmemory_object() :m_bIscreateshare(false), m_pshAddress(0), m_dbufferlength(0), m_bmemoryIndex(0)
{
}


Cmemory_object::~Cmemory_object()
{
}

void Cmemory_object::CreateShareMemory(const char *name, offset_t length)
{
	char strname[512] = {};
	sprintf_s(strname, "%s_%d", name, m_bmemoryIndex);
	m_strbackname = strname;
	m_dbufferlength = length;
	try
	{
		shared_memory_object shmobject(create_only, m_strbackname.c_str(), read_write);
		shmobject.truncate(length);
		mapped_region shregion(shmobject, read_write);
		m_pshAddress = static_cast<BYTE*>(shregion.get_address());
		memset(m_pshAddress, 0, length);
		m_bIscreateshare = true;
	}
	catch (boost::interprocess::interprocess_exception &e)
	{
		if (e.get_error_code() == already_exists_error)
		{
			rfalse("already_exists,start RecoverPlayer");
			//CloseShareMemory();
			//RecoverAllPlayer();
			RecoverAllPlayerProperty();
		}

	}

}

// void Cmemory_object::BackupPlayer(CPlayer *pPlayer)
// {
// 	bool isfindplayer = false;
// 	if (pPlayer)
// 	{
// 		std::list<CPlayer*>::iterator iter = m_listbackplayer.begin();
// 		while (iter != m_listbackplayer.end())
// 		{
// 			CPlayer *plistplayer = *iter;
// 			if (plistplayer&&plistplayer->GetSID() == pPlayer->GetSID())  //相同玩家
// 			{
// 				m_listbackplayer.erase(iter);
// 				memcpy(plistplayer, pPlayer, sizeof(CPlayer));
// 				m_listbackplayer.push_back(plistplayer);
// 				isfindplayer = true;
// 				break;
// 			}
// 			iter++;
// 		}
// 
// 		if (isfindplayer == false)
// 		{
// 			CPlayer *plist = new CPlayer;
// 			memcpy(plist, pPlayer, sizeof(CPlayer));
// 			m_listbackplayer.push_back(plist);
// 		}
// 	}
// }

void Cmemory_object::BackupPlayer(SFixData *pPlayer, std::string szAccount, DWORD Gid)
{
	bool isfindplayer = false;
	if (pPlayer)
	{
		std::list<SBackupPlayer*>::iterator iter = m_listbackplayerproperty.begin();
		while (iter != m_listbackplayerproperty.end())
		{
			SBackupPlayer *plistplayer = *iter;
			if (plistplayer)
			{
				if (plistplayer->m_Property.m_dwStaticID == pPlayer->m_dwStaticID)  //相同玩家
				{
					memcpy(&plistplayer->m_Property, pPlayer, sizeof(SFixData));
					strcpy(plistplayer->m_szAccount, szAccount.c_str());
					plistplayer->m_dgid = Gid;
					*iter = plistplayer;
					isfindplayer = true;
					break;
				}
			}

			iter++;
		}

		if (isfindplayer == false)
		{
			//20150423 wk 增加保护措施,m_listbackplayerproperty最大缓存 backupNum 个
			if (m_listbackplayerproperty.size()>=backupNum)
			{
				rfalse(2, 1, "共享缓存内存已满! [ 账号 = %s  备份失败]", szAccount.c_str());
				return;
			}
			SBackupPlayer *plistplayer = new SBackupPlayer;
			memcpy(&plistplayer->m_Property, pPlayer, sizeof(SFixData));
			strcpy(plistplayer->m_szAccount, szAccount.c_str());
			plistplayer->m_dgid = Gid;
			m_listbackplayerproperty.push_back(plistplayer);
		}
	}
}

// 
// 
// void Cmemory_object::RecoverPlayer(CPlayer *pbackPlayer)
// {
// 	if (pbackPlayer)
// 	{
// 		DWORD GID = pbackPlayer->GetGID();
// 
// 		// 先要判断新建目标是否有重复
// 		LPIObject pObj =GetGW()->CPlayerManager::LocateObject(GID);
// 		assert(!pObj);
// 		if (pObj)
// 		{
// 			rfalse(2, 1, "恢复角色时,发现一个重复的玩家! [ GID = %d ]", GID);
// 			return;
// 		}
// 		
// 		extern LPIObject GetPlayerByAccount(LPCSTR szAccount);
// 		pObj = GetPlayerByAccount(pbackPlayer->GetAccount());
// 		assert(!pObj);
// 		if (pObj)
// 		{
// 			rfalse(2, 1, "恢复角色时,发现一个重复的玩家! [ account = %s ]", pbackPlayer->GetAccount());
// 			return;
// 		}
// 		
// 
// 		CPlayer::SParameter Param;
// 		Param.dnidClient = 0;
// 		Param.dwVersion = 1;
// 		pObj = GetGW()->CPlayerManager::GenerateObject(IID_PLAYER, GID, (LPARAM)&Param);
// 		assert(pObj);
// 		if (!pObj)
// 		{
// 			rfalse(2, 1, "恢复角色时,创建玩家对象失败! [ account = %s ]", pbackPlayer->GetAccount());
// 			return;
// 		}
// 		
// 
// 		CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
// 		if (pPlayer == NULL)
// 			return;
// 
// 		pPlayer->SetAccount(pbackPlayer->GetAccount());
// 		pPlayer->m_ClientIndex = 0;
// 
// 		// 在查询映射表内保存对象
// 		BOOL BindAccountRelation(LPCSTR szAccount, LPIObject pObj);
// 		if (!BindAccountRelation(pbackPlayer->GetAccount(), pObj))
// 		{
// 			rfalse(2, 1, "恢复角色时,为玩家对象绑定帐号失败! [ account = %s ]", pbackPlayer->GetAccount());
// 			return;
// 		}
// 		
// 
// 		pPlayer->m_Property.m_Name[0] = 0;
// 
// 		LPIObject GetPlayerByName(LPCSTR szName);
// 		if (GetPlayerByName(pbackPlayer->m_Property.m_Name)->DynamicCast(IID_PLAYER) != NULL)
// 		{
// 			rfalse(2, 1, "恢复角色时,发现了一个重复的角色名! [ account = %s, name = %s ]", pbackPlayer->GetAccount(), pbackPlayer->m_Property.m_Name);
// 			return;
// 		}
// 		
// 		//20150117 wk  恢复SetFixData ,将  m_Property改为m_FixData
// 	//	if (!pPlayer->SetFixProperty(&pbackPlayer->m_Property))  
// 		if (!pPlayer->SetFixData(&pbackPlayer->m_FixData))
// 			
// 		{
// 			rfalse(2, 1, "恢复角色时,设置角色属性失败! [ account = %s, name = %s ]", pbackPlayer->GetAccount(), pbackPlayer->m_Property.m_Name);
// 			return;
// 		}
// 
// 
// 		//if (!pPlayer->SetTempData(pTempData)) 
// 		//    return FALSE;
// 
// 		pPlayer->LoginChecks();
// 
// 		LPIObject FindRegionByID(DWORD ID);
// 		CRegion *pRegion = (CRegion*)FindRegionByID(pPlayer->m_Property.m_CurRegionID)->DynamicCast(IID_REGION);
// 		if (pRegion == NULL)
// 		{
// 			rfalse(2, 1, "恢复角色时,设置角色场景失败! [ account = %s, name = %s, regionId = %d ]", pbackPlayer->GetAccount(), pbackPlayer->m_Property.m_Name, pPlayer->m_Property.m_CurRegionID);
// 			return;
// 		}
// 	
// 
// 		if (pRegion->AddObject(pObj) == 0)
// 		{
// 			rfalse(2, 1, "恢复角色时,将角色放入场景失败! [ account = %s, name = %s, regionId = %d, x = %d, y = %d ]", pbackPlayer->GetAccount(), pbackPlayer->getname(), pPlayer->m_Property.m_CurRegionID, ((DWORD)pPlayer->m_Property.m_X) >> 32, ((DWORD)pPlayer->m_Property.m_Y) >> 32);
// 			return;
// 		}
// 		pPlayer->RecoverPlayer(pbackPlayer);
// 		pPlayer->m_OnlineState = CPlayer::OST_HANGUP;
// 		pPlayer->m_bInit = true;
// 	}
// }

bool Cmemory_object::RecoverPlayer(SFixData &pbackPlayer, std::string szAccount, DWORD Gid)
{
	DWORD GID = pbackPlayer.m_dwStaticID;

	extern LPIObject GetPlayerBySID(DWORD dwStaticID);
	// 先要判断新建目标是否有重复
	LPIObject pObj = GetPlayerBySID(GID);
	assert(!pObj);
	if (pObj)
	{
		rfalse(2, 1, "恢复角色时,发现一个重复的玩家! [ GID = %d ]", GID);
		return false;
	}

	CPlayer::SParameter Param;
	Param.dnidClient = 0;
	Param.dwVersion = 1;
	pObj = GetGW()->CPlayerManager::GenerateObject(IID_PLAYER, Gid, (LPARAM)&Param);
	assert(pObj);
	if (!pObj)
	{
		rfalse(2, 1, "恢复角色时,创建玩家对象失败! [ account = %s ]", pbackPlayer.m_Name);
		return false;
	}


	CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
	if (pPlayer == NULL)
		return false;

	pPlayer->SetAccount(szAccount.c_str());
	pPlayer->m_ClientIndex = 0;

	// 在查询映射表内保存对象
	BOOL BindAccountRelation(LPCSTR szAccount, LPIObject pObj);
	if (!BindAccountRelation(szAccount.c_str(), pObj))
	{
		rfalse(2, 1, "恢复角色时,为玩家对象绑定帐号失败! [ account = %s ]", szAccount.c_str());
		return false;
	}


	pPlayer->m_Property.m_Name[0] = 0;

	LPIObject GetPlayerByName(LPCSTR szName);
	if (GetPlayerByName(pbackPlayer.m_Name)->DynamicCast(IID_PLAYER) != NULL)
	{
		rfalse(2, 1, "恢复角色时,发现了一个重复的角色名! [ name = %s ]", pbackPlayer.m_Name);
		return false;
	}

	//20150117 wk  恢复SetFixData
	//if (!pPlayer->SetFixProperty(&pbackPlayer))
	if (!pPlayer->SetFixData(&pbackPlayer))
		
	{
		rfalse(2, 1, "恢复角色时,设置角色属性失败! [ name = %s ]",  pbackPlayer.m_Name);
		return false;
	}


	//if (!pPlayer->SetTempData(pTempData)) 
	//    return FALSE;

	pPlayer->LoginChecks();

	LPIObject FindRegionByID(DWORD ID);
	CRegion *pRegion = (CRegion*)FindRegionByID(pPlayer->m_Property.m_CurRegionID)->DynamicCast(IID_REGION);
	if (pRegion == NULL)
	{
		//rfalse(2, 1, "恢复角色时,设置角色场景失败! [  name = %s, regionId = %d ]", pbackPlayer.m_Name, pPlayer->m_Property.m_CurRegionID);
		//20150117 wk  恢复萌将三国没有地图,注释掉
		//return false;
	}

	//20150117 wk  恢复萌将三国没有地图,注释掉
	/*
	if (pRegion->AddObject(pObj) == 0)
	{
		
		rfalse(2, 1, "恢复角色时,将角色放入场景失败! [ name = %s, regionId = %d, x = %d, y = %d ]", pbackPlayer.m_Name, pPlayer->m_Property.m_CurRegionID, ((DWORD)pPlayer->m_Property.m_X) >> 32, ((DWORD)pPlayer->m_Property.m_Y) >> 32);
		
		return false;
	}
	*/
//	pPlayer->RecoverPlayer(pbackPlayer);
	pPlayer->m_OnlineState = CPlayer::OST_HANGUP;
	pPlayer->m_bInit = true;
	if (pPlayer->m_dwLoginTime == 0)
		_time64(&pPlayer->m_dwLoginTime); //当前的登陆时间。用于记录

	//add by ly 2014/4/14
	pPlayer->m_AddOnceTpFlag = TRUE;
	pPlayer->m_Property.m_LeaveTime += pPlayer->m_dwLoginTime - pPlayer->m_Property.m_dLeaveTime64;	//计算玩家不在游戏时间总和了

	pPlayer->Logout(true);  //恢复就直接退出到db保存
	return true;
}


void Cmemory_object::CloseShareMemory()
{
	//m_listbackplayer.clear();
	m_listbackplayerproperty.clear();
	shared_memory_object::remove(m_strbackname.c_str());
}

int Cmemory_object::Onrefreshmemory()
{
// 	if (m_listbackplayer.size()>0 && m_dbufferlength > 0)
// 	{
// 		try
// 		{
// 			shared_memory_object shmobject(open_only, m_strbackname.c_str(), read_write);
// 			mapped_region shregion(shmobject, read_write);
// 			m_pshAddress = static_cast<BYTE*>(shregion.get_address());
// 			BYTE *ptemp = m_pshAddress;
// 			memset(ptemp, 0, m_dbufferlength);
// 
// 			std::list<CPlayer*>::iterator iter = m_listbackplayer.begin();
// 			while (iter != m_listbackplayer.end())
// 			{
// 				CPlayer *pPlayer = *iter;
// 				memcpy(ptemp,pPlayer,sizeof(CPlayer));
// 				iter++;
// 				ptemp += sizeof(CPlayer);
// 			}
// 		}
// 		catch (boost::interprocess::interprocess_exception &e)
// 		{
// 			//if (e.get_error_code() == already_exists_error)
// 			{
// 				rfalse("Onrefreshmemory Error  code = %d", e.get_error_code());
// 			}
// 
// 		}
// 		return 1;
// 	}

	if (m_listbackplayerproperty.size() > 0 && m_dbufferlength > 0)
	{
		try
		{
			shared_memory_object shmobject(open_only, m_strbackname.c_str(), read_write);

			mapped_region shregion(shmobject, read_write);
			m_pshAddress = static_cast<BYTE*>(shregion.get_address());
			BYTE *ptemp = m_pshAddress;
			memset(ptemp, 0, m_dbufferlength);

			std::list<SBackupPlayer*>::iterator iter = m_listbackplayerproperty.begin();
			while (iter != m_listbackplayerproperty.end())
			{
				SBackupPlayer *pPlayer = *iter;
				if (pPlayer)
				{
					memcpy(ptemp, pPlayer, sizeof(SBackupPlayer));
				}
				iter++;
				ptemp += sizeof(SBackupPlayer);
			}
		}
		catch (boost::interprocess::interprocess_exception &e)
		{
			if (e.get_error_code() == not_found_error)
			{
				CreateShareMemory(m_strbackname.c_str(), m_dbufferlength);
			}
			//if (e.get_error_code() == already_exists_error)
			{
				rfalse(4,1,"Onrefreshmemory Error  code = %d ,%s", e.get_error_code(),e.what());
			}

		}
		return 1;
	}
	return 0;
}

int Cmemory_object::OnPlayerLogout(DWORD sid)
{
	bool biserase = false;
	//WORD num = 0;
// 	std::list<CPlayer*>::iterator iter = m_listbackplayer.begin();
// 	while (iter != m_listbackplayer.end())
// 	{
// 		CPlayer *plistplayer = *iter;
// 		if (plistplayer&&plistplayer->GetSID() == sid)  //相同玩家,玩家正常下线则清除缓存
// 		{
// 			m_listbackplayer.erase(iter);		
// 			biserase = true;
// 			break;
// 		}
// 		iter++;
// 	}

	std::list<SBackupPlayer*>::iterator fixiter = m_listbackplayerproperty.begin();
	while (fixiter != m_listbackplayerproperty.end())
	{
		SBackupPlayer *plistplayer = *fixiter;
		if (plistplayer)
		{
			if (plistplayer->m_Property.m_dwStaticID == sid)  //相同玩家,玩家正常下线则清除缓存
			{
				m_listbackplayerproperty.erase(fixiter);
				biserase = true;
				break;
			}
		}

		fixiter++;
	}

	if (biserase)  //重新刷新
	{
		rfalse(2, 1, "OnPlayerLogout sid = %d", sid);
		Onrefreshmemory();
	}
	return 1;
}

bool Cmemory_object::RecoverAllPlayerProperty()
{
	size_t playernum = 0;
	try
	{
		shared_memory_object shmobject(open_only, m_strbackname.c_str(), read_write);
		mapped_region shregion(shmobject, read_write);
		m_pshAddress = static_cast<BYTE*>(shregion.get_address());
		while (1)
		{
			if (m_pshAddress)
			{ 
				SBackupPlayer *pPlayer = new SBackupPlayer;
				memcpy(pPlayer, m_pshAddress, sizeof(SBackupPlayer));
				if (pPlayer->m_Property.m_dwStaticID > 0)  //数据有效
				{
					if (RecoverPlayer(pPlayer->m_Property, pPlayer->m_szAccount, pPlayer->m_dgid))
					{
						playernum++;
					}
// 					BackupPlayer(&pPlayer->m_Property, pPlayer->m_szAccount, pPlayer->m_dgid);   //添加到备份列表
					//m_listbackplayer.push_back(*pPlayer);
				}
				else
				{
					break;
				}
				m_pshAddress += sizeof(SBackupPlayer);
			}
		}

	}
	catch (boost::interprocess::interprocess_exception &e)
	{
		//if (e.get_error_code() == already_exists_error)
		{
			rfalse("RecoverAllPlayer Error  code = %d,%s", e.get_error_code(),e.what());
		}
	}
	if (playernum > 0)
	{
		rfalse(2, 1, "重启以后恢复了%d个玩家数据", playernum);
	}
	return true;
}
