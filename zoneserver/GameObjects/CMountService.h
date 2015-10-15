#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class CMountService
{
	typedef std::hash_map<WORD, std::hash_map<WORD, class SMountBaseData*> > MountBaseData;

private:
	CMountService();

public:
	static CMountService& GetInstance()
	{
		static CMountService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CMountService();

	const SMountBaseData *GetMountBaseData(WORD index, BYTE level) const;

private:
	void CleanUp();

	// ½ûÖ¹¿½±´¹¹ÔìºÍ¿½±´¸³Öµ
	CMountService(CMountService &);
	CMountService& operator=(CMountService &);

private:
	MountBaseData m_baseData;
};

#pragma warning(pop)