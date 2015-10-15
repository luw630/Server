#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SBuffBaseData;

class CBuffService
{
	typedef std::hash_map<DWORD, SBuffBaseData*> BuffBaseData;

private:
	CBuffService();

public:
	static CBuffService& GetInstance()
	{
		static CBuffService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CBuffService();

	const SBuffBaseData* GetBuffBaseData(DWORD index) const;

private:
	void CleanUp();

	CBuffService(CBuffService &);
	CBuffService& operator=(CBuffService&);

private:
	BuffBaseData  m_baseData;				// Buff的基本属性
};

#pragma warning(pop)