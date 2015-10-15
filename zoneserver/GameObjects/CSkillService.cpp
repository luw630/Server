#include "Stdafx.h"
#include <utility>
#include "CSkillDefine.h"
#include "CSkillService.h"
#include "BulletMgr.h"

#pragma warning(push)
#pragma warning(disable:4996)

CSkillService::~CSkillService()
{
	CleanUp();
}

void CSkillService::CleanUp()
{
	for (SkillBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		for (std::hash_map<WORD, SSkillBaseData*>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			SSkillBaseData *temp = ti->second;
			delete temp;
		}
	}

	for (XinFaBaseData::iterator it = m_baseXinFa.begin(); it != m_baseXinFa.end(); ++it)
	{
		for (std::hash_map<WORD, SXinFaData*>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			SXinFaData *temp = ti->second;
			delete temp;
		}
	}

	for (SkillHitSpot::iterator it = m_HitSpot.begin(); it != m_HitSpot.end(); ++it)
	{
		SSkillHitSpot *temp = it->second;
		delete temp;
	}

	for (SBulletBaseData::iterator it = m_BulletData.begin(); it != m_BulletData.end(); ++it)
	{
		BulletBaseData *temp = it->second;
		delete temp;
	}

	m_baseData.clear();
	m_baseXinFa.clear();
	m_HitSpot.clear();
	m_groupCDTime.clear();
	m_TSMap.clear();
	m_BulletData.clear();
	m_SkillFactor.clear();
	m_fpskillupgrade.clear();
	return;
}

bool CSkillService::ReLoad()
{
	CleanUp();
	Init();

	return true;
}

bool CSkillService::Init()
{
	//bool wuyue		= false;
	//bool shushan	= false;
	//bool shaolin	= false;
	//bool cihang		= false;
	//bool riyue		= false;
	//bool other		= false;
	bool skill		= false;
	bool telergy	= false;
	bool groupCD	= false;
	bool hitSpot	= false;
	bool tsMap		= false;
	bool bullet		= false;
	bool factor		= false;
	bool bfpskill		= false;
	//dwt::ifstream wuyueStream;
	//dwt::ifstream shushanStream;
	//dwt::ifstream shaolinStream;
	//dwt::ifstream cihangStream;
	//dwt::ifstream riyueStream;
	//dwt::ifstream otherStream;
	dwt::ifstream groupStream;
	dwt::ifstream skillStream;
	dwt::ifstream TelergyStream;
	dwt::ifstream hitStream;
	dwt::ifstream tsStream;
	dwt::ifstream bulletSteam;
	dwt::ifstream factorStream;
	dwt::ifstream fpskillStream;
	//wuyueStream.open("Data\\技能\\五岳心法配置表.txt");
	//if (!wuyueStream.is_open())
	//{
	//	rfalse("无法打开五岳心法配置表！");
	//	goto _exit;
	//}
	//else
	//	wuyue = true;

	//shushanStream.open("Data\\技能\\蜀山心法配置表.txt");
	//if (!shushanStream.is_open())
	//{
	//	rfalse("无法打开蜀山心法配置表！");
	//	goto _exit;
	//}
	//else
	//	shushan = true;

	//shaolinStream.open("Data\\技能\\少林心法配置表.txt");
	//if (!shaolinStream.is_open())
	//{
	//	rfalse("无法打开少林心法配置表！");
	//	goto _exit;
	//}
	//else
	//	shaolin = true;

	//cihangStream.open("Data\\技能\\慈航心法配置表.txt");
	//if (!cihangStream.is_open())
	//{
	//	rfalse("无法打开慈航心法配置表！");
	//	goto _exit;
	//}
	//else
	//	cihang = true;

	//riyueStream.open("Data\\技能\\日月心法配置表.txt");
	//if (!riyueStream.is_open())
	//{
	//	rfalse("无法打开日月心法配置表！");
	//	goto _exit;
	//}
	//else
	//	riyue = true;

	//otherStream.open("Data\\技能\\其他心法配置表.txt");
	//if (!otherStream.is_open())
	//{
	//	rfalse("无法打开其他心法配置表！");
	//	goto _exit;
	//}
	//else
	//	other = true;

	factorStream.open("Data\\技能\\技能系数表.txt");
	if (!factorStream.is_open())
	{
		rfalse("无法打开技能系数表！");
		goto _exit;
	}
	else
		factor = true;

	groupStream.open("Data\\技能\\技能组冷却配置表.txt");
	if (!groupStream.is_open())
	{
		rfalse("无法打开技能组冷却配置表！");
		goto _exit;
	}
	else
		groupCD = true;

	skillStream.open("Data\\技能\\SkillTable.txt");
	if (!skillStream.is_open())
	{
		rfalse("无法打开技能配置表！");
		goto _exit;
	}
	else
		skill = true;

	TelergyStream.open("Data\\技能\\心法配置表.txt");
	if (!TelergyStream.is_open())
	{
		rfalse("无法打开心法配置表！");
		goto _exit;
	}
	else
		telergy = true;

	hitStream.open("Data\\技能\\技能打击点配置表.txt");
	if (!hitStream.is_open())
	{
		rfalse("无法打开技能打击点配置表！");
		goto _exit;
	}
	else
		hitSpot = true;

	tsStream.open("Data\\技能\\心法技能对应表.txt");
	if (!tsStream.is_open())
	{
		rfalse("无法打开心法技能对应表！");
		goto _exit;
	}
	else
		tsMap = true;

	bulletSteam.open("Data\\技能\\子弹配置表.txt");
	if (!bulletSteam.is_open())
	{
		rfalse("无法打开子弹配置表！");
		goto _exit;
	}
	else
		bullet = true;

	fpskillStream.open("Data\\技能\\侠客技能升级几率表.txt");
	if (!fpskillStream.is_open())
	{
		rfalse("侠客技能升级几率表！");
		goto _exit;
	}
	else
		bfpskill = true;

	char buffer[2048];

	// 读取心法配置表
	SXinFaData *pXinFa = 0;
	while (!TelergyStream.eof())
	{
		TelergyStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pXinFa = new SXinFaData;

		linebuf >> pXinFa->m_Name;

		DWORD temp = 0;
		pXinFa->m_ID						= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_Level						= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_NeedSp					= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_NeedMoney					= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddAttack					= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddDefence				= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddLife					= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddInterforce				= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPhysiforce				= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddDodgeforce				= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddCriceforce				= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPctBloodSucker			= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPctCutInterforce		= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPctCutPhysiforce		= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPctCutAttakNum			= ((linebuf >> temp), temp); temp = 0;
		pXinFa->m_AddPctCutDefecNum			= ((linebuf >> temp), temp); temp = 0;

		std::pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
		result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

		if (false == result.second)
		{
			rfalse("发现了一个重复的心法等级！");
			delete pXinFa;
		}
	}
	//// 读取五岳心法配置
	//while (!wuyueStream.eof())
	//{
	//	wuyueStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;

	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	//// 读取蜀山心法配置
	//while (!shushanStream.eof())
	//{
	//	shushanStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;

	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	//// 读取日月心法配置
	//while (!riyueStream.eof())
	//{
	//	riyueStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;

	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	//// 读取少林心法配置
	//while (!shaolinStream.eof())
	//{
	//	shaolinStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;

	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	//// 读取慈航心法配置
	//while (!cihangStream.eof())
	//{
	//	cihangStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;

	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	//// 读取其他心法配置
	//while (!otherStream.eof())
	//{
	//	otherStream.getline(buffer, sizeof(buffer));
	//	if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
	//		continue;

	//	std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
	//	pXinFa = new SXinFaData;
	//
	//	linebuf >> pXinFa->m_ID >> pXinFa->m_Level >> pXinFa->m_NeedSp >> pXinFa->m_NeedMoney;

	//	pair<std::hash_map<WORD, SXinFaData*>::iterator, bool> result;
	//	result = m_baseXinFa[pXinFa->m_ID].insert(make_pair(pXinFa->m_Level, pXinFa));

	//	if (false == result.second)
	//	{
	//		rfalse("发现了一个重复的心法等级编号！");
	//		delete pXinFa;
	//	}
	//}

	// 读取技能组冷却配置
	while (!groupStream.eof())
	{
		groupStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		DWORD GroupID;
		DWORD CDTime;

		linebuf >> GroupID >> CDTime;

		m_groupCDTime[GroupID] = CDTime;
	}

	// 读取技能配置
	SSkillBaseData *pData = 0;
	while (!skillStream.eof())
	{
		skillStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SSkillBaseData;

		linebuf >> pData->m_Name;

		DWORD temp = 0;
		pData->m_ID					= ((linebuf >> temp), temp); temp = 0;
		pData->m_Level				= ((linebuf >> temp), temp); temp = 0;
		pData->m_UpNeedSp			= ((linebuf >> temp), temp); temp = 0;
		pData->m_UpNeedMoney		= ((linebuf >> temp), temp); temp = 0;
		pData->m_School				= ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillType			= ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillPropertyType  = ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillSubProperty   = ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillAttack		= ((linebuf >> temp), temp); temp = 0;
		pData->m_AtkType			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CDGroup			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CDTime				= ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillMaxDistance	= ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillMinDistance	= ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillAtkRadius = ((linebuf >> temp), temp); temp = 0;
		pData->m_SkillAtkWidth  = ((linebuf >> temp), temp); temp = 0;		
		pData->m_AtkAmount			= ((linebuf >> temp), temp); temp = 0;
		pData->m_ConsumeMP			= ((linebuf >> temp), temp); temp = 0;
		pData->m_ConsumeHP			= ((linebuf >> temp), temp); temp = 0;
		pData->m_HitBackDis			= ((linebuf >> temp), temp); temp = 0;
		pData->m_HateValue			= ((linebuf >> temp), temp); temp = 0;
		pData->m_IsNormalSkill		= ((linebuf >> temp), temp); temp = 0;
		pData->m_DamageRate			= ((linebuf >> temp), temp); temp = 0;
		pData->m_CanUseInDizzy		= ((linebuf >> temp), temp); temp = 0;
		pData->m_BulletID			= ((linebuf >> temp), temp); temp = 0;
		pData->m_SingTime			= ((linebuf >> temp), temp); temp = 0;

		//add 2014.3.4
		pData->m_IconID				= ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_Description;
		linebuf >> pData->m_Detail;
		pData->m_SkillTypePos		= ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_FindSkillDes;
		pData->m_SkillBookID		= ((linebuf >> temp), temp); temp = 0; 
		pData->m_ConsumeItemID		= ((linebuf >> temp), temp); temp = 0;
		pData->m_OnceNeedItemCount = ((linebuf >> temp), temp); temp = 0; 
		pData->m_NeedPlayerLevel = ((linebuf >> temp), temp); temp = 0;


		std::pair<std::hash_map<WORD, SSkillBaseData*>::iterator, bool> result;
		result = m_baseData[pData->m_ID].insert(make_pair(pData->m_Level, pData));

		if (false == result.second)
		{
			rfalse("发现了一个重复的技能等级！");
			delete pData;
		}
	}

	// 读取打击点表
	SSkillHitSpot *pHit = 0;
	while (!hitStream.eof())
	{
		hitStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pHit = new SSkillHitSpot;
		
		DWORD temp = 0;
		pHit->m_ID = ((linebuf >> temp), temp); temp = 0;

		for (size_t i = 0; i < 12; i++)
		{
			pHit->m_HitPots[i] = ((linebuf >> temp), temp); temp = 0;
			
			if ((WORD)-1 != pHit->m_HitPots[i])
				pHit->m_HitCount++;
		}

		MY_ASSERT(pHit->m_HitCount <= 12);

		for (size_t i = 0; i < 12; i++)
			pHit->m_CalcDmg[i] = ((linebuf >> temp), temp); temp = 0;

		std::pair<SkillHitSpot::iterator, bool> result;
		result = m_HitSpot.insert(make_pair(pHit->m_ID, pHit));

		if (false == result.second)
		{
			rfalse("发现一个重复的技能打击点！");
			delete pHit;
		}
	}

	// 读取心法技能对应表
	while (!tsStream.eof())
	{
		tsStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		DWORD TelegryID = 0;
		linebuf	>> TelegryID;

		DWORD temp = 0;
		for (size_t i = 0; i < 10; i++)
		{
			linebuf >> temp;

			if (0 == temp)
				break;

			m_TSMap[TelegryID].push_back(temp);
		}
	}

	// 读取子弹配置
	BulletBaseData *pBullet = 0;
	while (!bulletSteam.eof())
	{
		bulletSteam.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pBullet = new BulletBaseData;

		linebuf >> pBullet->m_ID >> pBullet->m_TrackType >> pBullet->m_StopCondition >> pBullet->m_BltSpeed >> pBullet->m_ActiveCount >> pBullet->m_ActiveInteval;

		pair<SBulletBaseData::iterator, bool> result;
		result = m_BulletData.insert(make_pair(pBullet->m_ID, pBullet));

		if (false == result.second)
		{
			rfalse("发现了一个重复的子弹编号！");
			delete pBullet;
		}
	}

	// 读取技能攻击力系数
	while (!factorStream.eof())
	{
		factorStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		
		DWORD SkillID = 0;
		linebuf >> SkillID;

		WORD temp = 0;
		for (size_t index = 0; index < 4; ++index)
		{
			linebuf >> temp;
			if (0 == temp)
				break;

			m_SkillFactor[SkillID].push_back(temp);
		}
	}

	// 读取侠客技能组升级配置
	while (!fpskillStream.eof())
	{
		fpskillStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		WORD skilllevel;
		WORD chance;

		linebuf >> skilllevel >> chance;
		m_fpskillupgrade[skilllevel] = chance;
	}

_exit:
	//if (wuyue)
	//	wuyueStream.close();
	//if (shushan)
	//	shushanStream.close();
	//if (shaolin)
	//	shaolinStream.close();
	//if (cihang)
	//	cihangStream.close();
	//if (riyue)
	//	riyueStream.close();
	//if (other)
	//	otherStream.close();
	if (skill)
		skillStream.close();
	if (telergy)
		TelergyStream.close();
	if (hitSpot)
		hitStream.close();
	if (tsMap)
		tsStream.close();
	if (bullet)
		bulletSteam.close();
	if (factor)
		factorStream.close();
	if(bfpskill)
		fpskillStream.close();
	//return wuyue && shushan && shaolin && cihang && riyue && other && skill && hitSpot && tsMap && bullet;
	return skill && telergy && hitSpot && tsMap && bullet && factor&&bfpskill;
}

const std::vector<WORD>	*CSkillService::GetSkillFactor(DWORD index) const
{
	if (0 == index)
		return 0;

	SSkillFactor::const_iterator it = m_SkillFactor.find(index);
	if (it != m_SkillFactor.end())
		return &it->second;
	else
		return 0;
}
const std::vector<DWORD> *CSkillService::GetTelegrySkillMap(WORD index) const
{
	if (0 == index)
		return 0;

	STelegrySkillMap::const_iterator it = m_TSMap.find(index);
	if (it != m_TSMap.end())
		return &it->second;
	else
		return 0;
}

DWORD CSkillService::GetGroupCDTime(DWORD groupID) const
{
	if (0 == groupID)
		return 0;

	GroupCDTime::const_iterator it = m_groupCDTime.find(groupID);
	if (it != m_groupCDTime.end())
		return it->second;
	else
		return 0;
}

const SSkillHitSpot *CSkillService::GetSkillHitSpot(WORD index) const
{
	if (0 == index)
		return 0;

	SkillHitSpot::const_iterator it = m_HitSpot.find(index);
	if (it != m_HitSpot.end())
		return it->second;
	else
		return 0;
}

const WORD CSkillService::GetSkillIdByBookId(DWORD BookIndex) const
{
	if (0 == BookIndex)
		return 0;
	SkillBaseData::const_iterator it1 = m_baseData.begin();
	for (; it1 != m_baseData.end(); it1++)
	{
		std::hash_map<WORD, SSkillBaseData*>::const_iterator it2 = it1->second.begin();
		for (; it2 != it1->second.end(); it2++)
		{
			if (it2->second->m_SkillBookID == BookIndex)
				return it2->second->m_ID;
		}
	}
	return 0;
}

const SSkillBaseData *CSkillService::GetSkillBaseData(WORD index, WORD level) const
{
	if (0 == index || 0 == level || level > MaxSkillLevel)
		return 0;

	SkillBaseData::const_iterator it = m_baseData.find(index);
	if (it != m_baseData.end())
	{
		std::hash_map<WORD, SSkillBaseData*>::const_iterator detail = it->second.find(level);
		if (detail != it->second.end())
			return detail->second;
	}
	return 0;
}

const SXinFaData *CSkillService::GetXinFaBaseData(WORD index, WORD level) const
{
	if (0 == index || 0 == level || level > MaxXinFaLevel)
		return 0;

	XinFaBaseData::const_iterator it = m_baseXinFa.find(index);
	if (it != m_baseXinFa.end())
	{
		std::hash_map<WORD, SXinFaData*>::const_iterator detail = it->second.find(level);
		if (detail != it->second.end())
			return detail->second;
	}

	return 0;
}

const BulletBaseData *CSkillService::GetBulletBaseData(WORD index) const
{
	if (0 == index)
		return 0;

	SBulletBaseData::const_iterator it = m_BulletData.find(index);
	if (it != m_BulletData.end())
		return it->second;
	else
		return 0;
}

WORD CSkillService::_fpGetSkillupgradeChance( WORD level ) const
{
	if(level == 0)return 0;
	fpskillupgrade::const_iterator ite = m_fpskillupgrade.find(level);
	return ite!=m_fpskillupgrade.end()?ite->second:0;
}

#pragma warning(pop)