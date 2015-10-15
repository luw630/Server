#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include<memory>
#include<vector>
#include<map>
#include <set>
#include"../PUB/Singleton.h"

///@brief 分配武将相关消息
class CTabTableReader
{
	
public:
	CTabTableReader(const std::string & fileNameStr);
	CTabTableReader()
		: curLineIndex(0), maxLineNum(0), maxRowNum(0), m_ppFileData(nullptr)
	{

	}

	~CTabTableReader()
	{
		Release();
	}
	///@brief 打开tab格式的文件并读取内容
	bool OpenTabTable(const std::string & fileNameStr);
	///@brief 打开tab格式的文件并读取内容重载
	bool OpenTabTable(const char * fileName);

	///@brief 按分隔符分离string内容
	static inline void StringSplit(const std::string& s, const std::string& delim, std::vector< std::string >* ret)
	{
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != std::string::npos)
		{
			ret->push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}
		if (index - last>0)
		{
			ret->push_back(s.substr(last, index - last));
		}
	}

	///@brief 按分隔符分离string内容
	static void StringSplit(const std::string& targetStr, const char* delim, std::vector<int>& datas)
	{
		if (!targetStr.empty())
		{
			int strSize = targetStr.size();
			char* findedStr = nullptr;
			char* next_token = nullptr;
			char* splitArray = new char[strSize + 1];

			strcpy_s(splitArray, strSize + 1, targetStr.c_str());
			char* deltedArray = splitArray;

			do
			{
				findedStr = strtok_s(splitArray, delim, &next_token);
				if (findedStr != nullptr)
					datas.push_back(std::move(atoi(findedStr)));

				if (splitArray != nullptr)
				{
					splitArray = nullptr;
				}
			} while (findedStr != nullptr);

			if (deltedArray != nullptr)
			{
				delete[] deltedArray;
				deltedArray = nullptr;
			}
		}
	}

	///@brief 按分隔符分离string内容
	static void StringSplit(const std::string& targetStr, const char* delim, std::set<int>& datas)
	{
		if (!targetStr.empty())
		{
			int strSize = targetStr.size();
			char* findedStr = nullptr;
			char* next_token = nullptr;
			char* splitArray = new char[strSize + 1];

			strcpy_s(splitArray, strSize + 1, targetStr.c_str());
			char* deltedArray = splitArray;

			do
			{
				findedStr = strtok_s(splitArray, delim, &next_token);
				if (findedStr != nullptr)
					datas.insert(std::move(atoi(findedStr)));

				if (splitArray != nullptr)
				{
					splitArray = nullptr;
				}
			} while (findedStr != nullptr);

			if (deltedArray != nullptr)
			{
				delete[] deltedArray;
				deltedArray = nullptr;
			}
		}
	}

	static void StringSplit(const std::string& targetStr, const char* delim, std::vector<float>& datas)
	{
		if (!targetStr.empty())
		{
			int strSize = targetStr.size();
			char* findedStr = nullptr;
			char* next_token = nullptr;
			char* splitArray = new char[strSize + 1];

			strcpy_s(splitArray, strSize + 1, targetStr.c_str());
			char* deletedArray = splitArray;

			do
			{
				findedStr = strtok_s(splitArray, delim, &next_token);
				if (findedStr != nullptr)
					datas.push_back(std::move(atof(findedStr)));

				if (splitArray != nullptr)
				{
					splitArray = nullptr;
				}
			} while (findedStr != nullptr);

			if (deletedArray != nullptr)
			{
				delete[] deletedArray;
				deletedArray = nullptr;
			}
		}
	}

	///@读取下一行内容
	bool ReadLine();

	///@brief 释放资源
	bool Release();

	///@brief  根据传入列名返回对应元素的string值
	std::string GetStrValue(const std::string &valueName);

	///@brief 根据传入列名返回对应元素的int32_t值
	int32_t GetIntValue(const std::string& valueName);

	///@brief 根据传入列名返回对应元素的float值
	float GetFloatValue(const std::string& valueName);
	
	///@brief 根据传入参数获取第二列元素int32_t值
	int32_t GetSecondRowIntValue(const std::string& valueName);

	///@brief 根据传入参数获取第二列元素string值
	std::string GetSecondRowStrValue(const std::string& valueName);
	
	///@brief 根据传入参数获取第二列元素float值
	float GetSecondRowFloatValue(const std::string& valueName);

	int32_t GetDataLineCount() const;
private:
	int32_t  curLineIndex, maxLineNum, maxRowNum;
	const std::string m_TabDelim = "\t";
	std::vector<std::string> m_FileLineContent;
	std::string _StreamStringReader(std::istream& stream);

	std::string **m_ppFileData;

	///@brief 处理文件内容，按照tab分隔符读取tab格式表每行内容
	void _ProcessFileContent();
	
};