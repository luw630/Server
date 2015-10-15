#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <map>

// 杀怪任务的怪物关联表
class CKillTaskService
{
private:
	CKillTaskService();

public:
	static CKillTaskService& GetInstance()
	{
		static CKillTaskService instance;
		return instance;
	}

	bool Init();
	const 
	
};