#include "Stdafx.h"
#include <utility>
#include "CJingMaiDefine.h"
#include "CJingMaiService.h"

#pragma warning(push)
#pragma warning(disable:4996)

CJingMaiService::CJingMaiService() {}

CJingMaiService::~CJingMaiService() { Clear(); }

void CJingMaiService::Clear()
{
	for (JinMaiBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SJingMaiBaseData *temp = it->second;
		delete temp;
	}

	for (JinMaiBaseData::iterator it = m_awardData.begin(); it != m_awardData.end(); ++it)
	{
		SJingMaiBaseData *temp = it->second;
		delete temp;
	}

	m_baseData.clear();
	m_awardData.clear();
	m_MXData.clear();
}

bool CJingMaiService::ReLoad()
{
	Clear();
	Init();
	
	return true;
}

bool CJingMaiService::Init()
{
	dwt::ifstream stream("Data\\经脉\\经脉表.txt");
	if (!stream.is_open())
		return false;

	char buffer[2048];
	SJingMaiBaseData *pData = 0;
    DWORD lastMailID = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SJingMaiBaseData;

		linebuf >> pData->m_Name;

		DWORD temp = 0;
		pData->m_MaiID			= ((linebuf >> temp), temp); temp = 0;
		pData->m_XueID			= ((linebuf >> temp), temp); temp = 0;
		for (size_t i = 0; i < JMP_MAX; i++) linebuf >> pData->m_Factor[i];
		pData->m_NeedItem		= ((linebuf >> temp), temp); temp = 0;
		pData->m_Rate			= ((linebuf >> temp), temp); temp = 0;
		pData->m_NeedSp			= ((linebuf >> temp), temp); temp = 0;

		//计算穴位的前置穴位
		if(lastMailID != pData->m_MaiID)
		{
			pData->m_NeedFrontXueID = 0;   //0代表无前置穴位
			lastMailID = pData->m_MaiID;
		}
		else if(pData->m_XueID > 0)
		{
            pData->m_NeedFrontXueID = (pData->m_XueID -1);
		}

		MY_ASSERT(pData->m_MaiID);

		std::pair<JinMaiBaseData::iterator, bool> result1;

		if (pData->m_XueID)
			result1 = m_baseData.insert(make_pair(pData->m_XueID, pData));
		else
			result1 = m_awardData.insert(make_pair(pData->m_MaiID, pData));

		if (false == result1.second)
		{
			rfalse(2, 1, "发现了一个重复的脉线/穴位号！%d", pData->m_XueID);
			delete pData;
		}
		else if (pData->m_XueID)
			m_MXData[pData->m_MaiID].push_back(pData->m_XueID);
	}

	return true;
}

const SJingMaiBaseData *CJingMaiService::GetJingMaiAwardData(WORD MaiID) const
{
	if (0 == MaiID)
		return 0;

	JinMaiBaseData::const_iterator temp = m_awardData.find(MaiID);
	if (temp != m_awardData.end())
		return temp->second;
	else
		return 0;
}

const SJingMaiBaseData *CJingMaiService::GetJingMaiBaseData(WORD XueID) const
{
	if (0 == XueID)
		return 0;

	JinMaiBaseData::const_iterator temp = m_baseData.find(XueID);
	if (temp != m_baseData.end())
		return temp->second;
	else
		return 0;
}

const std::list<WORD> *CJingMaiService::GetXueList(WORD MaiID) const
{
	if (0 == MaiID)
		return 0;

	MaiXueMapData::const_iterator temp = m_MXData.find(MaiID);
	if (temp != m_MXData.end())
		return &temp->second;
	else
		return 0;
}


/////add by ly 2014/3/17	这里信仰系统和经脉系统为类似功能，几乎为仿照经脉系统功能来实现（该系统没有经脉系统这个功能）

#pragma warning(push)
#pragma warning(disable:4996)

CXinYangService::CXinYangService() {}

CXinYangService::~CXinYangService() { Clear(); }

void CXinYangService::Clear()
{
	for (XinYangBaseData::iterator it = m_baseData.begin(); it != m_baseData.end(); ++it)
	{
		SXinYangBaseData *temp = it->second;
		delete temp;
	}
	m_baseData.clear();
}

bool CXinYangService::ReLoad()
{
	Clear();
	Init();

	return true;
}

bool CXinYangService::Init()
{
	dwt::ifstream stream("Data\\信仰\\BeliefTable.txt");
	if (!stream.is_open())
		return false;

	char buffer[2048];
	SXinYangBaseData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SXinYangBaseData;

		linebuf >> pData->m_Name;

		DWORD temp = 0;
		pData->m_ZhuShenID = ((linebuf >> temp), temp); temp = 0;
		pData->m_XingxiuID = ((linebuf >> temp), temp); temp = 0;
		for (int i = 0; i < XYP_MAX; i++)
			linebuf >> pData->m_Factor[i];
		//pData->m_AddHP = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddMP = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddGongJi = ((linebuf >> temp), temp); temp = 0;

		//pData->m_AddFangYu = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddBaoJi = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddPoJi = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddChuanCi = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddKangBao = ((linebuf >> temp), temp); temp = 0;

		//pData->m_AddKangPo = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddMingZhong = ((linebuf >> temp), temp); temp = 0;
		//pData->m_AddShanBi = ((linebuf >> temp), temp); temp = 0;
		pData->m_SuccessRate = ((linebuf >> temp), temp); temp = 0;
		pData->m_ConsumeYinLiang = ((linebuf >> temp), temp); temp = 0;

		pData->m_ConsumeJinBi = ((linebuf >> temp), temp); temp = 0;
		pData->m_ConsumeItem = ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_XingXiuDescript;
		pData->m_PlayerVipLevel = ((linebuf >> temp), temp); temp = 0; 
		pData->m_CanHaveSkillID = ((linebuf >> temp), temp); temp = 0;
		pData->m_ConsumeSp = ((linebuf >> temp), temp); temp = 0;


		m_baseData.insert(make_pair(pData->m_XingxiuID, pData));
	}
	return true;
}

const SXinYangBaseData* CXinYangService::GetXingXiuDataByID(BYTE XingXiuID) const	//通过星宿ID获取当前星宿的基本数据
{
	if (0 == XingXiuID)
		return 0;

	XinYangBaseData::const_iterator temp = m_baseData.find(XingXiuID);
	if (temp != m_baseData.end())
		return temp->second;
	return 0;
}

#pragma warning(pop)