#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SJingMaiBaseData;

class CJingMaiService
{
	typedef std::hash_map<WORD, SJingMaiBaseData*>  JinMaiBaseData;
	typedef std::hash_map<WORD, std::list<WORD> >	MaiXueMapData;

private:
	CJingMaiService();

public:
	static CJingMaiService& GetInstance()
	{
		static CJingMaiService instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	~CJingMaiService();

	const SJingMaiBaseData *GetJingMaiBaseData(WORD XueID) const;
	const SJingMaiBaseData *GetJingMaiAwardData(WORD MaiID) const;
	const std::list<WORD> *GetXueList(WORD MaiID) const;

private:
	// ��ֹ��������Ϳ�����ֵ
	CJingMaiService(CJingMaiService &);
	CJingMaiService& operator=(CJingMaiService &);
	
private:
	JinMaiBaseData	m_baseData;		// ������������
	JinMaiBaseData	m_awardData;	// ���⽱��
	MaiXueMapData	m_MXData;		// ��Ѩӳ���
};

#pragma warning(pop)


/////add by ly 2014/3/17	��������ϵͳ�;���ϵͳΪ���ƹ��ܣ�����Ϊ���վ���ϵͳ������ʵ�֣���ϵͳû�о���ϵͳ������ܣ�

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SXinYangBaseData;

class CXinYangService
{
	typedef std::hash_map<WORD, SXinYangBaseData*>  XinYangBaseData;

private:
	CXinYangService();

public:
	static CXinYangService& GetInstance()
	{
		static CXinYangService instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	const SXinYangBaseData* GetXingXiuDataByID(BYTE XingXiuID) const;	//ͨ������ID��ȡ��ǰ���޵Ļ�������

	~CXinYangService();

private:
	// ��ֹ��������Ϳ�����ֵ
	CXinYangService(CXinYangService &);
	CXinYangService& operator=(CXinYangService &);

private:
	XinYangBaseData	m_baseData;		// ������������
};

#pragma warning(pop)