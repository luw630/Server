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
	// 禁止拷贝构造和拷贝赋值
	CJingMaiService(CJingMaiService &);
	CJingMaiService& operator=(CJingMaiService &);
	
private:
	JinMaiBaseData	m_baseData;		// 经脉基本数据
	JinMaiBaseData	m_awardData;	// 额外奖励
	MaiXueMapData	m_MXData;		// 脉穴映射表
};

#pragma warning(pop)


/////add by ly 2014/3/17	这里信仰系统和经脉系统为类似功能，几乎为仿照经脉系统功能来实现（该系统没有经脉系统这个功能）

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

	const SXinYangBaseData* GetXingXiuDataByID(BYTE XingXiuID) const;	//通过星宿ID获取当前星宿的基本数据

	~CXinYangService();

private:
	// 禁止拷贝构造和拷贝赋值
	CXinYangService(CXinYangService &);
	CXinYangService& operator=(CXinYangService &);

private:
	XinYangBaseData	m_baseData;		// 经脉基本数据
};

#pragma warning(pop)