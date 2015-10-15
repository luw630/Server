#include "stdafx.h"
#include "CGloryATitleBaseData.h"
#include "CGloryAndTitleServer.h"

CGloryAndTitleServer::CGloryAndTitleServer() {}

CGloryAndTitleServer::~CGloryAndTitleServer() { Clear(); }

void CGloryAndTitleServer::Clear()
{
	//��ҫ���������ͷ�
	for (GloryBaseData::iterator it = m_GloryBaseData.begin(); it != m_GloryBaseData.end(); ++it)
	{
		SGloryBaseData *temp = it->second;
		delete temp;
	}
	m_GloryBaseData.clear();

	//�ƺŻ��������ͷ�
	for (TitleBaseData::iterator it = m_TitleBaseData.begin(); it != m_TitleBaseData.end(); ++it)
	{
		STitleBaseData *temp = it->second;
		delete temp;
	}
	m_TitleBaseData.clear();
}

bool CGloryAndTitleServer::ReLoad()
{
	Clear();
	Init();

	return true;
}

bool CGloryAndTitleServer::Init()
{
	//������ҫ��������
	dwt::ifstream stream("Data\\��ҫ�ͳƺ�\\GloryTable.txt");
	if (!stream.is_open())
		return false;

	char buffer[2048];
	SGloryBaseData *pData = 0;

	while (!stream.eof())
	{
		stream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		pData = new SGloryBaseData;

		DWORD temp = 0;
		pData->m_GloryID= ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_GloryName;
		pData->m_GloryType = ((linebuf >> temp), temp); temp = 0;
		for (int i = 0; i < GLORY_AMAX; i++)
			linebuf >> pData->m_AwardFactor[i];
		for (int i = 0; i < GLORY_CMAX; i++)
			linebuf >> pData->m_GetConditionFactor[i]; 
		pData->m_GloryConditionIndex = ((linebuf >> temp), temp); temp = 0;
		linebuf >> pData->m_ConditionDetail;
		pData->m_GloryIcon = ((linebuf >> temp), temp); temp = 0;


		m_GloryBaseData.insert(make_pair(pData->m_GloryID, pData));
	}

	//���سƺŻ�������
	dwt::ifstream stream1("Data\\��ҫ�ͳƺ�\\TitleTable.txt");
	if (!stream1.is_open())
		return false;

	char buffer1[2048];
	STitleBaseData *pData1 = 0;

	while (!stream1.eof())
	{
		stream1.getline(buffer1, sizeof(buffer1));
		if (0 == buffer1[0] || '//' == (*(LPWORD)buffer1))
			continue;

		std::strstream linebuf1(buffer1, (std::streamsize)strlen(buffer1));
		pData1 = new STitleBaseData;

		DWORD temp = 0; 
		pData1->m_TitleID = ((linebuf1 >> temp), temp); temp = 0;
		linebuf1 >> pData1->m_TitleName;
		for (int i = 0; i < TITLE_AMAX; i++)
			linebuf1 >> pData1->m_AddFactor[i];
		for (int i = 0; i < TITLE_CMAX; i++)
			linebuf1 >> pData1->m_ConsumeFactor[i];
		pData1->m_ConsumeItemID = ((linebuf1 >> temp), temp); temp = 0; 
		pData1->m_TitleIcon = ((linebuf1 >> temp), temp); temp = 0;
		linebuf1 >> pData1->m_TitleDescript;
		m_TitleBaseData.insert(make_pair(pData1->m_TitleID, pData1));
	}
	return true;
}

//ͨ����ҫID��ȡ��ҫ������Ϣ
const SGloryBaseData* CGloryAndTitleServer::GetGloryBaseDataByID(WORD GloryID) const
{
	GloryBaseData::const_iterator it = m_GloryBaseData.begin();
	for (; it != m_GloryBaseData.end(); it++)
		if (it->second->m_GloryID == GloryID)
			return it->second;
	return NULL;
}

//��ȡ��ҫ���е�ID
WORD* CGloryAndTitleServer::ReturnAllGloryID(int *ArrSize)
{
	*ArrSize = m_GloryBaseData.size();
	WORD *GloryIDArr = new WORD[*ArrSize];
	memset(GloryIDArr, 0, *ArrSize);
	GloryBaseData::const_iterator it = m_GloryBaseData.begin();
	for (int i = 0; it != m_GloryBaseData.end(); it++, i++)
		GloryIDArr[i] = it->second->m_GloryID;
	return GloryIDArr;
}

//ͨ���ƺ�ID��ȡ�ƺŻ�����Ϣ
const STitleBaseData* CGloryAndTitleServer::GetTitleBaseDataByID(WORD TitleID) const
{
	TitleBaseData::const_iterator it = m_TitleBaseData.begin();
	for (; it != m_TitleBaseData.end(); it++)
	if (it->second->m_TitleID == TitleID)
		return it->second;
	return NULL;
}

//ͨ���ƺ����ƻ�ȡ�ƺŻ�����Ϣ
const STitleBaseData* CGloryAndTitleServer::GetTitleBaseDatabyName(const char* Name) const
{
	TitleBaseData::const_iterator it = m_TitleBaseData.begin();
	for (; it != m_TitleBaseData.end(); it++)
	if (strcmp(it->second->m_TitleName, Name) == 0)
		return it->second;
	return NULL;
}