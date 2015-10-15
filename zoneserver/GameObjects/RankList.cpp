#include "StdAfx.h"

#include <time.h>
#include "RankList.h"
#include "networkmodule\refreshmsgs.h"

// 初始化
void CSingleRank::Initialized()
{
	//memset(RawRankList, 0, sizeof(RawRankList) * ALL_RANK_NUM);
	pCurSvrRankList	= 0;

	DBNum	= 0;
	SvrNum	= 0;

	IsGetTheRawDataFromDB = false;
	IsGetCurSvrData	= false;
	IsProcess = false;
	IsGetNewRank = false;
	
	_time64(&oldTime);
}

// 向DB请求排行榜数据
void CSingleRank::SendToDB_2_GetRankData(const char *pName)
{
// 	if (IsGetTheRawDataFromDB)
// 		return;

	if (!pName || IsGetTheRawDataFromDB)
		return;

	int level = GetTheLevelLimitFromScript();
	if (level < 0)
		return;

	SQRefreshRanklist sMsg;

	sMsg.level = level;
	memcpy(sMsg.name, pName, sizeof(char) * CONST_USERNAME);

	SendToLoginServer(&sMsg,sizeof(SQRefreshRanklist));
}

// 用DB的数据初始化排行榜数据
void CSingleRank::GetRankData_From_DB(NewRankList* pRankList, int num)
{
	if (!pRankList || !num)
	{
		DBNum = 0;
		return;
	}
		
	DBNum = num;
	memcpy(RawRankList, pRankList, sizeof(NewRankList) * ALL_RANK_NUM);

	IsGetTheRawDataFromDB = true;
}

// 获取当前在线玩家的排行榜数据
void CSingleRank::GetRankData_From_ZoneServer(PLAYERLIST playerList)
{
	if (IsGetCurSvrData || playerList.empty())
		return;
	
	int temNum = playerList.size();
	
	if (temNum <= 0)
		return;

	int levelLimit = GetTheLevelLimitFromScript();
	if (levelLimit < 0)
		return;

	vector<NewRankList> vec;

	for (PLAYERLIST_ITERATOR iter = playerList.begin(); iter != playerList.end(); ++iter)
	{
		if (!(*iter))
		{
			rfalse("Current zone server ranklist has a NULL player point!!");
			--temNum;
			continue;
		}

		
		if ((*iter)->m_Property.m_Level < levelLimit)
		{
			--temNum;
			continue;
		}
		
		NewRankList tem;
		tem.Level = (*iter)->m_Property.m_Level;
		tem.Money = (*iter)->m_Property.m_BindMoney;
		tem.BossNum = (*iter)->m_Property.m_KillBossNum;
		tem.School = (*iter)->m_Property.m_School;

		vec.push_back(tem);
	}

	SvrNum = temNum;
 	pCurSvrRankList = new NewRankList[SvrNum];
	memcpy(pCurSvrRankList, &vec[0], sizeof(NewRankList) * SvrNum);

	IsGetCurSvrData = true;
}

// 检测时间
bool CSingleRank::CheckTime(INT64 CurTime)
{
	if (CurTime - oldTime > 24 * 60 * 60)
	{
		oldTime = CurTime;

		IsGetTheRawDataFromDB = false;
		IsGetCurSvrData = false;

		IsProcess = false;

		return true;
	}

	return false;
}

// 处理排行榜数据
bool CSingleRank::ProcessTheRankList()
{
	if (SvrNum < 0)
		return false;

	// 以下的处理基于一个事实，所有在游戏服务器中的玩家，一定会在DB中有数据。
	// 也就是RawRankList中一定会包含所有的pCurSvrRankList中的数据，但是数据可能不同。
	// 我们要排序使用的，是所有最新的数据，也就是说，应该用pCurSvrRankList中的数据去替换
	// RawRankList,然后排序
	RawRankMap rawMap;			// DB List + Server List

	// 新数据
	for (size_t i = 0; i < static_cast<size_t>(SvrNum); ++i)
	{
		if (!pCurSvrRankList)
			break;

		rawMap.insert(std::make_pair(pCurSvrRankList[i].name, pCurSvrRankList[i]));			
	}

	// DB旧数据
	for (size_t i = 0; i < static_cast<size_t>(DBNum); ++i)
	{
		rawMap.insert(std::make_pair(RawRankList[i].name, RawRankList[i]));
	}

	delete [] pCurSvrRankList;
	pCurSvrRankList = 0;

	IsProcess = true;

	return TRUE;
}

void CSingleRank::GetTheRankList(CPlayer *player)
{
	if (!player)
		return;

	RankMap temRank[RT_MAX];

	// 所有数据
	WORD number = (DBNum > SvrNum) ? DBNum : SvrNum;

	if (!number)
		return;

	for (size_t i = 0; i < static_cast<size_t>(number); ++i)
	{
		temRank[RT_LEVEL].insert(std::make_pair(RawRankList[i].Level, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_LEVEL), RawRankList[i].Level, RawRankList[i].School)));
		temRank[RT_MONEY].insert(std::make_pair(RawRankList[i].Money, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_MONEY), RawRankList[i].Money, RawRankList[i].School)));
		temRank[RT_BOSSKILL].insert(std::make_pair(RawRankList[i].BossNum, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_BOSSKILL), RawRankList[i].BossNum, RawRankList[i].School)));
	}

	// 这里是对数据的处理，得到需要发送给客户端的排行榜数据
	for (size_t i = RT_LEVEL; i < RT_MAX; ++i)
		RankList_4_Client(player, temRank[i], i);

	return;
}

// 上榜等级限制
int CSingleRank::GetTheLevelLimitFromScript()
{
	lite::Variant ret;
	LuaFunctor( g_Script, "GetRankListLevelLimit" )(&ret);

	if(lite::Variant::VT_INTEGER != ret.dataType && lite::Variant::VT_NULL != ret.dataType)
	{
		rfalse("The ret's dataType isn't INTs....");
		return -1;
	}

	return ret;
}

// 排行榜数据筛选与排序
void CSingleRank::RankList_4_Client(CPlayer* player, RankMap &rankDataMap, BYTE rankType)
{
	if (!player)
		return;

	if (rankDataMap.empty())
		return;

	// 已经排序后，最大的数据都为0，退出，不作处理.
	RankMap::reverse_iterator rank_Iter = rankDataMap.rbegin();
	if (0 == rank_Iter->first)
		return;

	// 需要发送给客户端的排行榜数目
	int RankNum = DBNum < MAX_RANKLIST_NUM ? DBNum : MAX_RANKLIST_NUM;

	bool bIsIn = false;
	int  myRank = 0;

	while(rank_Iter != rankDataMap.rend())
	{
		if (myRank < RankNum)
			memcpy(&player->m_RankList[rankType][myRank], &rank_Iter->second, sizeof(Rank4Client));

		++myRank;

		if (!bIsIn && !strcmp(rank_Iter->second.name, player->m_Property.m_Name))
		{
			bIsIn = true;
			player->m_RankNum[rankType] = myRank;
		}

		++rank_Iter;
	}
}