#pragma once


///@brief 根据读取配置表随机名字
class RobotNameConfig
{
public:
	RobotNameConfig();
	~RobotNameConfig();
	void AddPrefixName(const string name){ m_PrefixName.push_back(name); }
	void AddSufffixName(const string name){ m_SufffixName.push_back(name); }
	void AddThirdName(const string name){ m_ThirdName.push_back(name); }
	const string RandomName();
private:
	std::vector<string> m_PrefixName;
	std::vector<string> m_SufffixName;
	std::vector<string> m_ThirdName;
	std::vector<string> m_RecentRandomNameList;
};

