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
	if (resultName.length()>21)//大于7个字,utf8 汉字占3字节
	{
		return RandomName();
	}
	if (find(m_RecentRandomNameList.begin(), m_RecentRandomNameList.end(), resultName) != m_RecentRandomNameList.end()) //如果随机出的名字同样存在于最近随机列表中，则再次随机
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
