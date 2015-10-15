#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

struct strInfo
{
	std::string s_strValue;		//�ַ���	
	BYTE bparamcount;		//��������
};
// �ṩ���ַ�����ͨ�����Խ���
class CMyString
{
	//typedef std::hash_map<std::string, strInfo*>		   SMyStringInfo;
	typedef std::map<std::string, strInfo*>		   SMyStringInfo;
private:
	CMyString();
	// ��ֹ��������Ϳ�����ֵ
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
	BYTE		parsestrparamcount(const char* strvalue,int nLenth);//�����ַ�����������
};

#pragma warning(pop)