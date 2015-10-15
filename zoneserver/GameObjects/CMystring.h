#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

struct strInfo
{
	std::string s_strValue;		//字符串	
	BYTE bparamcount;		//参数数量
};
// 提供对字符串的通用属性解析
class CMyString
{
	//typedef std::hash_map<std::string, strInfo*>		   SMyStringInfo;
	typedef std::map<std::string, strInfo*>		   SMyStringInfo;
private:
	CMyString();
	// 禁止拷贝构造和拷贝赋值
	CMyString(CMyString &);
	CMyString& operator=(CMyString &);

public:
	static CMyString& GetInstance()
	{
		static CMyString instance;
		return instance;
	}
	~CMyString();
	bool Init();
	bool ReLoad();
	void Clear();
	bool MyStringFormat(const char* key,std::vector<lite::Variant> &vlvariant,std::string &StringFormat);
	const char* GetFormatString(const char* key);
private:
	SMyStringInfo m_smystringinfo;
	BYTE		parsestrparamcount(const char* strvalue,int nLenth);//解析字符串参数数量
};

#pragma warning(pop)