#include "Stdafx.h"
#include <utility>
#include "CBuffDefine.h"
#include "CBuffService.h"

#pragma warning(push)
#pragma warning(disable:4996)

CBuffService::CBuffService() {}

CBuffService::~CBuffService()  { CleanUp(); }

void CBuffService::CleanUp()
{
	// 清除Buff信息
	for (BuffBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SBuffBaseData *temp = it->second;
		delete temp;
	}

	m_baseData.clear();
}

bool CBuffService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

const SBuffBaseData *CBuffService::GetBuffBaseData(DWORD index) const
{
	if (0 == index)
		return 0;

	BuffBaseData::const_iterator buff = m_baseData.find(index);
	if (buff != m_baseData.end())
		return buff->second;
	else
		return 0;
}

bool CBuffService::Init()
{
	dwt::ifstream stream("Data\\Buff\\Buff配置表.txt");
	if (!stream.is_open())
		return false;

	char buffer[2048];
	SBuffBaseData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SBuffBaseData;

		DWORD temp = 0;

		pData->m_ID						= ((linebuf >> temp), temp); temp = 0;
		pData->m_DeleteType				= ((linebuf >> temp), temp); temp = 0;
		pData->m_SaveType				= ((linebuf >> temp), temp); temp = 0;
		pData->m_ScriptID				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Margin					= ((linebuf >> temp), temp); temp = 0;
		pData->m_WillActionTimes		= ((linebuf >> temp), temp); temp = 0;
		pData->m_GroupID				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Weight					= ((linebuf >> temp), temp); temp = 0;
		pData->m_InturptRate			= ((linebuf >> temp), temp); temp = 0;
		pData->m_PropertyID				= ((linebuf >> temp), temp); temp = 0;
		pData->m_ActionType				= ((linebuf >> temp), temp); temp = 0;
		pData->m_Type					= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanBeReplaced			= ((linebuf >> temp), temp); temp = 0;
		 
		// 读取修正间隔
		for (int i = BATS_ADD; i < BATS_MAX; i++)
		{
			linebuf >> pData->m_Action[i].m_Value;
			pData->m_Action[i].m_TriggetSpot = (0 == pData->m_Action[i].m_Value) ? BATS_MAX : i;
		}
		pData->m_ReginType = ((linebuf >> temp), temp); temp = 0;
		pData->m_ReginShape = ((linebuf >> temp), temp); temp = 0;
		pData->m_ReginHeight = ((linebuf >> temp), temp); temp = 0;
		pData->m_ReginWidth = ((linebuf >> temp), temp); temp = 0;

		std::pair<BuffBaseData::iterator, bool> result;
		result = m_baseData.insert(make_pair(pData->m_ID, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的Buff ID:%d!", pData->m_ID);
			delete pData;
		}
	}

	stream.close();
	return true;
}

#pragma warning(pop)