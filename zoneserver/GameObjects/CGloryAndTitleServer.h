#pragma once
#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
/*******************add by ly 2014/3/24*********************/
//荣耀和称号服务管理
class SGloryBaseData;
class STitleBaseData;
class CGloryAndTitleServer
{
	typedef std::hash_map<WORD, SGloryBaseData*>  GloryBaseData;
	typedef std::hash_map<WORD, STitleBaseData*>  TitleBaseData;
private:
	CGloryAndTitleServer(); 
public:
	static CGloryAndTitleServer& GetInstance()
	{
		static CGloryAndTitleServer instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	//通过荣耀ID获取荣耀基本信息
	const SGloryBaseData* GetGloryBaseDataByID(WORD GloryID) const;
	//获取荣耀所有的ID
	WORD* ReturnAllGloryID(int *ArrSize);

	//通过称号ID获取称号基本信息
	const STitleBaseData* GetTitleBaseDataByID(WORD TitleID) const;
	const STitleBaseData* GetTitleBaseDatabyName(const char* Name) const;


	~CGloryAndTitleServer();

private:
	// 禁止拷贝构造和拷贝赋值
	CGloryAndTitleServer(CGloryAndTitleServer &);
	CGloryAndTitleServer& operator=(CGloryAndTitleServer &);

private:
	GloryBaseData m_GloryBaseData;
	TitleBaseData m_TitleBaseData;
};
#pragma warning(pop)
