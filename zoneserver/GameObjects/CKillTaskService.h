#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <map>

// ɱ������Ĺ��������
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