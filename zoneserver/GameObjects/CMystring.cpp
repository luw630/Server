#include "Stdafx.h"
#include <utility>
#include "CMystring.h"
#include "ScriptManager.h"
#include "Player.h"

#pragma warning(push)
#pragma warning(disable:4996)


#pragma warning(pop)

CMyString::CMyString(){}

CMyString::~CMyString()
{

}


bool CMyString::Init()
{
	dwt::ifstream StrinfStream("string\\chineseGB\\chinese.txt");
	if (!StrinfStream.is_open())
	{
		return false;
	}
	// 开始读取配置了~~~
	char buffer[2048];

	while (!StrinfStream.eof())
	{
		StrinfStream.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		std::string strkey;
		std::string strvalue;
		linebuf >> strkey;
		linebuf >> strvalue;
		strInfo *sinfo = new strInfo;
		sinfo->bparamcount = parsestrparamcount(strvalue.c_str(),strvalue.length());
		sinfo->s_strValue = strvalue.c_str();
		m_smystringinfo.insert(make_pair(strkey, sinfo));
	}
	return true;
}

bool CMyString::MyStringFormat( const char* key,std::vector<lite::Variant> &vlvariant,std::string &StringFormat)
{
	std::map<std::string,strInfo*>::iterator iter =  m_smystringinfo.find(key);
	if (iter!=m_smystringinfo.end())
	{
		char strBuff[MAX_PATH]={};
		std::string strformat(iter->second->s_strValue);
		if (iter->second->bparamcount != vlvariant.size())
		{
			rfalse(2,1,"[%s]参数数量不一致",key);
			return false;
		}
		if (vlvariant.size() == 0) //无参数 直接返回
		{
			StringFormat = iter->second->s_strValue;
			return true;
		}
		std::string::iterator iterstr = strformat.begin();
		unsigned int paramindex = 0;
		while(iterstr != strformat.end())
		{
			if (paramindex  > vlvariant.size())
			{
				rfalse(1,1,"[%s] 字符串参数数量错误",key);
				return false;
			}

			if (*iterstr == '%')
			{
				if (*(iterstr+1) == 'd')
				{
					if (vlvariant[paramindex].dataType != lite::Variant::VT_I32_24)
					{
						rfalse(1,1,"%s 参数类型错误 %d个参数类型为%d",key,paramindex+1,vlvariant[paramindex].dataType);
						return false;
					}
					iterstr++;
					char nstr[10]={};
					itoa(vlvariant[paramindex],nstr,10);
					StringFormat +=nstr;
					paramindex++;
				}
				else if (*(iterstr+1) == 's')
				{	
					if (vlvariant[paramindex].dataType != lite::Variant::VT_STRING)
					{
						rfalse(1,1,"%s 参数类型错误 %d个参数类型为%d",key,paramindex+1,vlvariant[paramindex].dataType);
						return false;
					}
					iterstr++;
					StringFormat +=vlvariant[paramindex];
					paramindex++;
				}
			}
			else
			{
				StringFormat += *iterstr;
			}

			iterstr++;
		}
		//rfalse(StringFormat.c_str());
		return true;
	}
	return false;
}

const char* CMyString::GetFormatString( const char* key )
{
	std::map<std::string,strInfo*>::iterator iter =  m_smystringinfo.find(key);
	if (iter!=m_smystringinfo.end())
	{
		//rfalse(iter->second->s_strValue.c_str());
		return iter->second->s_strValue.c_str();
	}
	return 0;
}

BYTE CMyString::parsestrparamcount( const char* strvalue,int nLenth )
{
	int ncount = 0;
	 while(*strvalue !='\0')
	 {
		 if (*strvalue == '%')
		 {
			 if (*(strvalue+1) == 'd')
			 {
				 ncount ++;
				 strvalue++;
			 }
			 else if (*(strvalue+1) == 's')
			 {
				 ncount ++;
				 strvalue++;
			 }
		 }
		 strvalue++;
	 }
	 return ncount;
	
}

