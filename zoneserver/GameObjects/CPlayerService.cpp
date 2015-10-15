#include "Stdafx.h"
#include <utility>
#include "CPlayerDefine.h"
#include "CPlayerService.h"
#include "networkmodule/playertypedef.h"

#pragma warning(push)
#pragma warning(disable:4996)

//add by ly 2014/5/20 新增一个标识来标识来判断角色升级表是否重新加载；由服务器输入sreset命令引起这个表重新加载了。
BOOL g_ReloadPersonUpgradeFlag;

CPlayerService::CPlayerService() {}

CPlayerService::~CPlayerService()
{
	// 清除角色属性升级表数据
	for (PlayerUpdateData::iterator it = m_PlayerUpdate.begin(); it != m_PlayerUpdate.end(); ++it)
	{
		SBaseAttribute *temp = it->second;
		delete temp;
	}
}

bool CPlayerService::Init()
{
	dwt::ifstream Player("Data\\升级\\角色升级表.txt");
	if (!Player.is_open())
	{
		rfalse("无法打开角色升级表！");
		return false;
	}

	char buffer[2048];
	WORD level;
	SBaseAttribute *pData = 0;
	
	while (!Player.eof())
	{
		Player.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SBaseAttribute;

// 		linebuf >> level >> pData->m_FreePoint >> pData->m_GongJi >> pData->m_FangYu >> pData->m_BaoJi >> pData->m_ShanBi >> pData->m_BaseHp
// 				>> pData->m_BaseMp >> pData->m_BaseTp >> pData->m_BaseJp >> pData->m_BaseSp >> pData->m_Exp >> pData->m_HpRecoverSpeed 
// 				>> pData->m_MpRecoverSpeed >> pData->m_TpRecoverSpeed >> pData->m_HpRecoverInterval >> pData->m_MpRecoverInterval
// 				>> pData->m_TpRecoverInterval;

		linebuf >> level >> pData->m_BaseHp >> pData->m_BaseJp >> pData->m_GongJi >> pData->m_FangYu >> pData->m_Hit >> pData->m_ShanBi
			>> pData->m_BaoJi >> pData->m_uncrit>>pData->m_wreck >> pData->m_unwreck >> pData->m_puncture >> pData->m_unpuncture >> pData->m_BaseTp 
			>> pData->m_TpRecover >> pData->m_TpRecoverInterval >> pData->m_Exp;
			

		std::pair<PlayerUpdateData::iterator, bool> result;
		result = m_PlayerUpdate.insert(make_pair(level, pData));
		if (false == result.second)
		{
			rfalse("发现了一个重复的等级设定！");
			delete pData;
		}
	}



	return true;
}

bool CPlayerService::Init(WORD wjob, WORD level, SBaseAttribute *pData)
{
	std::pair<PlayerUpdateData::iterator, bool> result;
	SBaseAttribute *playerdata = new SBaseAttribute;
	memcpy(playerdata, pData, sizeof(SBaseAttribute));
	result = m_PlayernewUpdate[wjob - 1].insert(make_pair(level, playerdata));
	if (false == result.second)
	{
		rfalse("发现了一个重复的等级设定！");
		delete playerdata;
	}
	return true;
}

const SBaseAttribute *CPlayerService::GetPlayerUpdateData(WORD level) const
{
	PlayerUpdateData::const_iterator player = m_PlayerUpdate.find(level);
	
	if (player != m_PlayerUpdate.end())
		return player->second;
	else
		return 0;
}

const SBaseAttribute* CPlayerService::GetPlayerUpdateData(WORD wjob, WORD level) const
{
	//if (wjob == 0)
		//return NULL;
	wjob -= 1;
	PlayerUpdateData::const_iterator player = m_PlayernewUpdate[wjob].find(level);
	if (player != m_PlayernewUpdate[wjob].end())
		return player->second;
	else
		return 0;
}

QWORD CPlayerService::GetPlayerUpdateExp(WORD level) const
{
	PlayerUpdateData::const_iterator player = m_PlayerUpdate.find(level);

	if (player != m_PlayerUpdate.end())
		return player->second->m_Exp;
	else
		return 0;
}

QWORD CPlayerService::GetPlayerUpdateExp(WORD wjob, WORD level) const
{
	wjob -= 1;
	PlayerUpdateData::const_iterator player = m_PlayernewUpdate[wjob].find(level);
	if (player != m_PlayernewUpdate[wjob].end())
		return player->second->m_Exp;
	else
		return 0;
}

bool CPlayerService::LoadXwzFromFile()
{
	dwt::ifstream Player("Data\\升级\\修为值换算.txt");
	if (!Player.is_open())
	{
		rfalse("无法打开修为值换算表！");
		return false;
	}

	char buffer[2048];
	while (!Player.eof())
	{

		Player.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		linebuf >> m_PlayerXwzData.m_Atk >> m_PlayerXwzData.m_Defence >> m_PlayerXwzData.m_CtrAtk >> m_PlayerXwzData.m_Escape
			>> m_PlayerXwzData.m_Hp >> m_PlayerXwzData.m_Mp >> m_PlayerXwzData.m_Tp >> m_PlayerXwzData.m_ATKSPEED >> m_PlayerXwzData.m_SPEED
			>> m_PlayerXwzData.m_ABSDAMAGE >> m_PlayerXwzData.m_REDUCEDAMAGE >> m_PlayerXwzData.m_NONFANGYU >> m_PlayerXwzData.KylinArmLevel;

	}
	return true;
}

bool CPlayerService::ReloadPlayerData()
{
	for (int i = 0; i < 4; i++)
	{
		for (PlayerUpdateData::iterator it = m_PlayernewUpdate[i].begin(); it != m_PlayernewUpdate[i].end(); ++it)
		{
			SBaseAttribute *temp = it->second;
			delete temp;
		}
		m_PlayernewUpdate[i].clear();
	}
	g_ReloadPersonUpgradeFlag = TRUE;
	return true;
}

#pragma warning(pop)