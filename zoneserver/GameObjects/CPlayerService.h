#ifndef CPLAYERSERVICE_H
#define CPLAYERSERVICE_H

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

extern BOOL g_ReloadPersonUpgradeFlag;

class SBaseAttribute;

// 提供对玩家升级属性的访问
class CPlayerService
{
	typedef std::hash_map<WORD, SBaseAttribute*> PlayerUpdateData;

private:
	CPlayerService();

public:
	static CPlayerService& GetInstance()
	{
		static CPlayerService instance;
		return instance;
	}

	bool Init();
	~CPlayerService();
	bool Init(WORD wjob, WORD level, SBaseAttribute *pData);
	bool ReloadPlayerData();
	const SBaseAttribute* GetPlayerUpdateData(WORD level) const;
	QWORD GetPlayerUpdateExp(WORD level) const;
	const SBaseAttribute* GetPlayerUpdateData(WORD wjob,WORD level) const;
	QWORD GetPlayerUpdateExp(WORD wjob,WORD level) const;
	bool LoadXwzFromFile();

private:
	// 禁止拷贝构造和拷贝赋值
	CPlayerService(CPlayerService &);
	CPlayerService& operator=(CPlayerService &);

private:
	PlayerUpdateData m_PlayerUpdate;
	PlayerUpdateData m_PlayernewUpdate[4];
	XWZ_Attribute		m_PlayerXwzData;  // 玩家修为值计算
};

#pragma warning(pop)

#endif // CPLAYERSERVICE_H