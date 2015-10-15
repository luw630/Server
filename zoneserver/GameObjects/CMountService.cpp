#include "Stdafx.h"
#include <utility>
#include "CMountDefine.h"
#include "CMountService.h"

#pragma warning(push)
#pragma warning(disable:4996)

CMountService::CMountService() {}

CMountService::~CMountService() { CleanUp(); }

void CMountService::CleanUp()
{
	for (MountBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		for (std::hash_map<WORD, SMountBaseData *>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			SMountBaseData *temp = ti->second;
			delete temp;
		}
	}

	m_baseData.clear();
}

bool CMountService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

bool CMountService::Init()
{
	dwt::ifstream mountStream;
	mountStream.open("Data\\骑乘\\骑乘配置表.txt");
	if (!mountStream.is_open())
		return false;

	char buffer[2048];
	
	SMountBaseData *pMount = 0;

	while (!mountStream.eof())
	{
		mountStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		pMount = new SMountBaseData;
		
		linebuf >> pMount->m_Name;

		DWORD temp = 0;
		pMount->m_ID			= ((linebuf >> temp), temp); temp = 0;
		pMount->m_Level			= ((linebuf >> temp), temp); temp = 0;
		pMount->m_Quality		= ((linebuf >> temp), temp); temp = 0;
		pMount->m_RideNum		= ((linebuf >> temp), temp); temp = 0;
		pMount->m_CanUpdate		= ((linebuf >> temp), temp); temp = 0;
		pMount->m_NeedExp		= ((linebuf >> temp), temp); temp = 0;
		pMount->m_SpeedUp		= ((linebuf >> temp), temp); temp = 0;
		for (size_t i = 0; i < EMA_MAX; i++) linebuf >> pMount->m_Attri[i][0], linebuf >> pMount->m_Attri[i][1];
		pMount->m_ModelID		= ((linebuf >> temp), temp); temp = 0;
		
		pair<std::hash_map<WORD, SMountBaseData*>::iterator, bool> result;
		result = m_baseData[pMount->m_ID].insert(make_pair(pMount->m_Level, pMount));

		if (false == result.second)
		{
			rfalse("发现了一个重复的骑乘等级编号！");
			delete pMount;
		}
	}

	return true;
}

const SMountBaseData* CMountService::GetMountBaseData(WORD index, BYTE level) const
{
	if (level > 9)
		return 0;

	MountBaseData::const_iterator it = m_baseData.find(index);
	if (it != m_baseData.end())
	{
		std::hash_map<WORD, SMountBaseData *>::const_iterator detail = it->second.find(level);
		if (detail != it->second.end())
			return detail->second;
	}

	return 0;
}

#pragma warning(pop)