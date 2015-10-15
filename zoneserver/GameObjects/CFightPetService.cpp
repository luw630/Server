#include "Stdafx.h"
#include <utility>
#include "CFightPetDefine.h"
#include "CFightPetService.h"

#pragma warning(push)
#pragma warning(disable:4996)

void CFightPetService::CleanUp()
{
	for (FightPetBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SFightPetBaseData *temp = it->second;
		delete temp;
	}
	m_baseData.clear();

	for (FightPetRise::iterator it = m_baseRiseData.begin(); it != m_baseRiseData.end(); ++it)
	{
		SFightPetRise *temp = it->second;
		delete temp;
	}
	m_baseRiseData.clear();
}

bool CFightPetService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

bool CFightPetService::Init()
{
#ifdef XYD_DC
	Loadfoodmenulist();
#endif
	return LoadPetBaseData()&LoadPetBaseRise();
}

bool CFightPetService::LoadPetBaseData()
{
	dwt::ifstream stream("Data\\侠客\\侠客属性配置表.txt");
	if (!stream.is_open())
	{
		rfalse("无法打开侠客配置表！");
		return false;
	}

	char buffer[2048];
	SFightPetBaseData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SFightPetBaseData;
		DWORD temp = 0;

		pData->m_ID = ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_Name;
		//pData->m_CanChangeName = ((linebuf >> temp), temp); temp = 0;
		for (size_t i = 0; i < 4; i++)
		{
			pData->m_Attri[i][0]=((linebuf >> temp), temp); temp = 0;
		}
		pData->m_RareMin= ((linebuf >> temp), temp); temp = 0;
		pData->m_RareMax= ((linebuf >> temp), temp); temp = 0;
		pData->m_nexp = ((linebuf >> temp), temp); temp = 0;
		pData->m_nmodelID = ((linebuf >> temp), temp); temp = 0;
		pData->m_niconID = ((linebuf >> temp), temp); temp = 0;
		pData->m_nLimitLevel = ((linebuf >> temp), temp); temp = 0;
		pData->m_bSex = ((linebuf >> temp), temp); temp = 0;
		pData->m_Type =((linebuf >> temp), temp); temp = 0;
		std::pair<FightPetBaseData::iterator, bool> result;
		result = m_baseData.insert(make_pair(pData->m_ID, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的侠客ID！！！");
			delete pData;
		}
	}

	stream.close();
	return true;
}

const SFightPetBaseData *CFightPetService::GetFightPetBaseData(WORD index) const
{
	FightPetBaseData::const_iterator pet = m_baseData.find(index);
	if (pet != m_baseData.end())
		return pet->second;
	else
		return 0;
}

bool CFightPetService::LoadPetBaseRise()
{
	dwt::ifstream stream("Data\\侠客\\侠客成长值.txt");
	if (!stream.is_open())
	{
		rfalse("无法打开侠客配置表！");
		return false;
	}

	char buffer[2048];
	SFightPetRise *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SFightPetRise;
		DWORD temp = 0;


		pData->m_nRare = ((linebuf >> temp), temp); temp = 0;
		pData->m_nHpRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nHpRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nMpRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nMpRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nGongJiRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nGongJiRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nFangYuRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nFangYuRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nShanBiRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nShanBiRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nBaojiRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nBaojiRiseMax = ((linebuf >> temp), temp); temp = 0;

		pData->m_nWuXinRiseMin = ((linebuf >> temp), temp); temp = 0;
		pData->m_nWuXinRiseMax = ((linebuf >> temp), temp); temp = 0;

		std::pair<FightPetRise::iterator, bool> result;
		result = m_baseRiseData.insert(make_pair(pData->m_nRare, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的侠客稀有度成长值！！！");
			delete pData;
		}
	}

	stream.close();
	return true;
}

const SFightPetRise * CFightPetService::GetFightPetRise( BYTE Rare ) const
{
	FightPetRise::const_iterator petrise = m_baseRiseData.find(Rare);
	if (petrise != m_baseRiseData.end())return petrise->second;
	return 0;
}

const QWORD CFightPetService::GetFightPetLevelExp( WORD Level ,BYTE index) const
{
	const SFightPetBaseData *pdata=GetFightPetBaseData(index);
	if(!pdata)return 0;
	QWORD exp=pdata->m_nexp;
	exp+=(Level-1)*10;
	return exp;
}

bool CFightPetService::SetFightPetBaseData( WORD index,BYTE Attri,WORD valuedata )
{
	if(Attri>FPA_SHANBI||Attri<FPA_MAXHP)return false;
	FightPetBaseData::iterator petiter = m_baseData.find(index);
	if (petiter != m_baseData.end())
	{
		 SFightPetBaseData *pdata = petiter->second;
		 pdata->m_Attri[Attri][0] = valuedata;
		 petiter->second = pdata;
		 return true;
	}
	return false;
}





#pragma warning(pop)