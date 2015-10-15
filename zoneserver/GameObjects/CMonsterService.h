#ifndef CMONSTERSERVICE_H
#define CMONSTERSERVICE_H

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SMonsterBaseData;

// �ṩ�Թ���������ݵķ���
// 1. ����������Ա�
class CMonsterService
{
	typedef std::hash_map<WORD, SMonsterBaseData *> MonsterBaseData;

private:
	CMonsterService();
	bool LoadMonsterBaseData();				// ���ع���Ļ������ݱ�
	
public:
	static CMonsterService& GetInstance()
	{
		static CMonsterService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();							// ��ʼ��
	~CMonsterService();

	const SMonsterBaseData* GetMonsterBaseData(WORD index) const;

private:
	void CleanUp();

	// ��ֹ��������Ϳ�����ֵ
	CMonsterService(CMonsterService &);
	CMonsterService& operator=(CMonsterService &);

private:
	MonsterBaseData	m_baseData;				// ����Ļ�������
};

#pragma warning(pop)

#endif // CMONSTERSERVICE_H