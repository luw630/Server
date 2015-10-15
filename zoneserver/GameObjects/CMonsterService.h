#ifndef CMONSTERSERVICE_H
#define CMONSTERSERVICE_H

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SMonsterBaseData;

// 提供对怪物相关数据的访问
// 1. 怪物基本属性表
class CMonsterService
{
	typedef std::hash_map<WORD, SMonsterBaseData *> MonsterBaseData;

private:
	CMonsterService();
	bool LoadMonsterBaseData();				// 加载怪物的基本数据表
	
public:
	static CMonsterService& GetInstance()
	{
		static CMonsterService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();							// 初始化
	~CMonsterService();

	const SMonsterBaseData* GetMonsterBaseData(WORD index) const;

private:
	void CleanUp();

	// 禁止拷贝构造和拷贝赋值
	CMonsterService(CMonsterService &);
	CMonsterService& operator=(CMonsterService &);

private:
	MonsterBaseData	m_baseData;				// 怪物的基本数据
};

#pragma warning(pop)

#endif // CMONSTERSERVICE_H