#pragma once

#include "networkmodule/playertypedef.h"
class CKylinArmService
{
public:
	struct KylinArmData
	{
		std::string		strName;
		WORD			wAttack;
		DWORD			wDistance;
		WORD			wTrigerRate;
		DWORD			wYuanqiMax;
		WORD			wNeedLevel;
		DWORD			wNeedMoney;
		DWORD			wNeedSP;
		DWORD			wNeedItemID;
		WORD			wNeedItemNum;
		WORD			wUpgradeRate;
		DWORD           dwBuffID[6];
		WORD			wBuffTrigRate[6];
	};
	static CKylinArmService& GetInstance()
	{
		static CKylinArmService instance;
		return instance;
	}
	bool Init()
	{
		fstream cfg("Data\\麒麟臂数据表.txt");
		if (!cfg.is_open())
		{
			rfalse("无法打开体质数据表！");
			return false; 
		}
		char buffer[2048];
		cfg.getline(buffer,sizeof(buffer));	//跳过第一行
		int level = 0;
		while (!cfg.eof())
		{
			cfg.getline(buffer,sizeof(buffer));
			if (!buffer[0]) continue;
			std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
			linebuf>>level
				   >>m_data[level].strName
				   >>m_data[level].wAttack
				   >>m_data[level].wDistance
				   >>m_data[level].wTrigerRate
				   >>m_data[level].wYuanqiMax
				   >>m_data[level].wNeedLevel
				   >>m_data[level].wNeedMoney
				   >>m_data[level].wNeedSP
				   >>m_data[level].wNeedItemID
				   >>m_data[level].wNeedItemNum
				   >>m_data[level].wUpgradeRate
				   >>m_data[level].dwBuffID[0]
				   >>m_data[level].wBuffTrigRate[0]
				   >>m_data[level].dwBuffID[1]
				   >>m_data[level].wBuffTrigRate[1]
				   >>m_data[level].dwBuffID[2]
				   >>m_data[level].wBuffTrigRate[2]
				   >>m_data[level].dwBuffID[3]
				   >>m_data[level].wBuffTrigRate[3]
				   >>m_data[level].dwBuffID[4]
				   >>m_data[level].wBuffTrigRate[4]
				   >>m_data[level].dwBuffID[5]
				   >>m_data[level].wBuffTrigRate[5];
		}
		return true;
	}
	const KylinArmData& getKylinArmData(int level)
	{
		return m_data[level-1];
	}
protected:
private:
	CKylinArmService(){}
	CKylinArmService(CKylinArmService &){}
	CKylinArmService& operator=(CKylinArmService &){}
	KylinArmData m_data[SPlayerKylinArm::MaxLevel];
};