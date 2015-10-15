#include "Stdafx.h"
#include <utility>
#include "CFriendService.h"

#pragma warning(push)
#pragma warning(disable:4996)

void CFriendService::CleanUp()
{
	for (FriendBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SFriendData *temp = it->second;
		delete temp;
	}
	m_baseData.clear();
}

bool CFriendService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

bool CFriendService::Init()
{
	return LoadFriendBaseData();
}

bool CFriendService::LoadFriendBaseData()
{
	dwt::ifstream stream("Data\\好友\\好友奖励.txt");
	if (!stream.is_open())
	{
		rfalse("无法打开好友奖励配置表！");
		return false;
	}

	char buffer[2048];
	SFriendData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SFriendData;
		DWORD temp = 0;


		pData->nlevel = ((linebuf >> temp), temp); temp = 0;
		pData->nrewarditemid = ((linebuf >> temp), temp); temp = 0;
		pData->nOnlineTime = ((linebuf >> temp), temp); temp = 0;
		pData->ntotalCount = ((linebuf >> temp), temp); temp = 0;
		pData->nrewardExp = ((linebuf >> temp), temp); temp = 0;
		
		std::pair<FriendBaseData::iterator, bool> result;
		result = m_baseData.insert(make_pair(pData->nlevel, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的好友等级！！！");
			delete pData;
		}
	}

	stream.close();
	return true;
}

const CFriendService::SFriendData * CFriendService::GetFriendBaseData( WORD levle ) const
{
	std::hash_map<WORD, SFriendData *>::const_iterator itera=m_baseData.find(levle);
	if (itera!=m_baseData.end())
	{
		return itera->second;
	}
	return 0;
}

const DWORD CFriendService::GetFriendRewarditem( WORD levle ) const
{
	std::hash_map<WORD, SFriendData *>::const_iterator itera=m_baseData.find(levle);
	if (itera!=m_baseData.end())
	{
		return itera->second->nrewarditemid;
	}
	return 0;
}

const DWORD CFriendService::GetFriendRewardExp( WORD levle ) const
{
	std::hash_map<WORD, SFriendData *>::const_iterator itera=m_baseData.find(levle);
	if (itera!=m_baseData.end())
	{
		return itera->second->nrewardExp;
	}
	return 0;
}

const DWORD CFriendService::GetFriendRewardTime( WORD levle ) const
{
	std::hash_map<WORD, SFriendData *>::const_iterator itera=m_baseData.find(levle);
	if (itera!=m_baseData.end())
	{
		return itera->second->nOnlineTime;
	}
	return 0;
}

const WORD CFriendService::GetFriendRewardCount( WORD levle ) const
{
	std::hash_map<WORD, SFriendData *>::const_iterator itera=m_baseData.find(levle);
	if (itera!=m_baseData.end())
	{
		return itera->second->ntotalCount;
	}
	return 0;
}


#pragma warning(pop)