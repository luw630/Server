#include "stdafx.h"
#include "RobotNameConfig.h"
#include "Random.h"
using namespace std;
RobotNameConfig::RobotNameConfig()
{
	m_PrefixName.clear();
	m_SufffixName.clear();
	m_RecentRandomNameList.clear();
}


RobotNameConfig::~RobotNameConfig()
{

}

const string RobotNameConfig::RandomName()
{
	int prefixRandomIndex = CRandom::RandRange(0, m_PrefixName.size()-1);
	int suffixRandomIndex = CRandom::RandRange(0, m_SufffixName.size()-1);
	int thirdRandomIndex = CRandom::RandRange(0, m_ThirdName.size() - 1);
	string resultName = m_PrefixName[prefixRandomIndex] + m_SufffixName[suffixRandomIndex] + m_ThirdName[thirdRandomIndex];
	if (resultName.length()>21)//����7����,utf8 ����ռ3�ֽ�
	{
		return RandomName();
	}
	if (find(m_RecentRandomNameList.begin(), m_RecentRandomNameList.end(), resultName) != m_RecentRandomNameList.end()) //��������������ͬ���������������б��У����ٴ����
	{
		return RandomName();
	}
	else
	{
		m_RecentRandomNameList.push_back(resultName);
		if (m_RecentRandomNameList.size() > 100)
			m_RecentRandomNameList.pop_back();
	}
	return resultName;
}
