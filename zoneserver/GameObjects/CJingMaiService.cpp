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
	dwt::ifstream stream("Data\\����\\������.txt");
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

		//����Ѩλ��ǰ��Ѩλ
		if(lastMailID != pData->m_MaiID)
		{
			pData->m_NeedFrontXueID = 0;   //0������ǰ��Ѩλ
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
			rfalse(2, 1, "������һ���ظ�������/Ѩλ�ţ�%d", pData->m_XueID);
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


/////add by ly 2014/3/17	��������ϵͳ�;���ϵͳΪ���ƹ��ܣ�����Ϊ���վ���ϵͳ������ʵ�֣���ϵͳû�о���ϵͳ������ܣ�

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
	dwt::ifstream stream("Data\\����\\BeliefTable.txt");
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

const SXinYangBaseData* CXinYangService::GetXingXiuDataByID(BYTE XingXiuID) const	//ͨ������ID��ȡ��ǰ���޵Ļ�������
{
	if (0 == XingXiuID)
		return 0;

	XinYangBaseData::const_iterator temp = m_baseData.find(XingXiuID);
	if (temp != m_baseData.end())
		return temp->second;
	return 0;
}

#pragma warning(pop)