#include "StdAfx.h"

#include <time.h>
#include "RankList.h"
#include "networkmodule\refreshmsgs.h"

// ��ʼ��
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

// ��DB�������а�����
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

// ��DB�����ݳ�ʼ�����а�����
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

// ��ȡ��ǰ������ҵ����а�����
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

// ���ʱ��
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

// �������а�����
bool CSingleRank::ProcessTheRankList()
{
	if (SvrNum < 0)
		return false;

	// ���µĴ������һ����ʵ����������Ϸ�������е���ң�һ������DB�������ݡ�
	// Ҳ����RawRankList��һ����������е�pCurSvrRankList�е����ݣ��������ݿ��ܲ�ͬ��
	// ����Ҫ����ʹ�õģ����������µ����ݣ�Ҳ����˵��Ӧ����pCurSvrRankList�е�����ȥ�滻
	// RawRankList,Ȼ������
	RawRankMap rawMap;			// DB List + Server List

	// ������
	for (size_t i = 0; i < static_cast<size_t>(SvrNum); ++i)
	{
		if (!pCurSvrRankList)
			break;

		rawMap.insert(std::make_pair(pCurSvrRankList[i].name, pCurSvrRankList[i]));			
	}

	// DB������
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

	// ��������
	WORD number = (DBNum > SvrNum) ? DBNum : SvrNum;

	if (!number)
		return;

	for (size_t i = 0; i < static_cast<size_t>(number); ++i)
	{
		temRank[RT_LEVEL].insert(std::make_pair(RawRankList[i].Level, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_LEVEL), RawRankList[i].Level, RawRankList[i].School)));
		temRank[RT_MONEY].insert(std::make_pair(RawRankList[i].Money, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_MONEY), RawRankList[i].Money, RawRankList[i].School)));
		temRank[RT_BOSSKILL].insert(std::make_pair(RawRankList[i].BossNum, Rank4Client(RawRankList[i].name, static_cast<BYTE>(RT_BOSSKILL), RawRankList[i].BossNum, RawRankList[i].School)));
	}

	// �����Ƕ����ݵĴ����õ���Ҫ���͸��ͻ��˵����а�����
	for (size_t i = RT_LEVEL; i < RT_MAX; ++i)
		RankList_4_Client(player, temRank[i], i);

	return;
}

// �ϰ�ȼ�����
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

// ���а�����ɸѡ������
void CSingleRank::RankList_4_Client(CPlayer* player, RankMap &rankDataMap, BYTE rankType)
{
	if (!player)
		return;

	if (rankDataMap.empty())
		return;

	// �Ѿ�������������ݶ�Ϊ0���˳�����������.
	RankMap::reverse_iterator rank_Iter = rankDataMap.rbegin();
	if (0 == rank_Iter->first)
		return;

	// ��Ҫ���͸��ͻ��˵����а���Ŀ
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