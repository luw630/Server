#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include<memory>
#include<vector>
#include<map>
#include <set>
#include"../PUB/Singleton.h"

///@brief �����佫�����Ϣ
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
	///@brief ��tab��ʽ���ļ�����ȡ����
	bool OpenTabTable(const std::string & fileNameStr);
	///@brief ��tab��ʽ���ļ�����ȡ��������
	bool OpenTabTable(const char * fileName);

	///@brief ���ָ�������string����
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

	///@brief ���ָ�������string����
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

	///@brief ���ָ�������string����
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

	///@��ȡ��һ������
	bool ReadLine();

	///@brief �ͷ���Դ
	bool Release();

	///@brief  ���ݴ����������ض�ӦԪ�ص�stringֵ
	std::string GetStrValue(const std::string &valueName);

	///@brief ���ݴ����������ض�ӦԪ�ص�int32_tֵ
	int32_t GetIntValue(const std::string& valueName);

	///@brief ���ݴ����������ض�ӦԪ�ص�floatֵ
	float GetFloatValue(const std::string& valueName);
	
	///@brief ���ݴ��������ȡ�ڶ���Ԫ��int32_tֵ
	int32_t GetSecondRowIntValue(const std::string& valueName);

	///@brief ���ݴ��������ȡ�ڶ���Ԫ��stringֵ
	std::string GetSecondRowStrValue(const std::string& valueName);
	
	///@brief ���ݴ��������ȡ�ڶ���Ԫ��floatֵ
	float GetSecondRowFloatValue(const std::string& valueName);

	int32_t GetDataLineCount() const;
private:
	int32_t  curLineIndex, maxLineNum, maxRowNum;
	const std::string m_TabDelim = "\t";
	std::vector<std::string> m_FileLineContent;
	std::string _StreamStringReader(std::istream& stream);

	std::string **m_ppFileData;

	///@brief �����ļ����ݣ�����tab�ָ�����ȡtab��ʽ��ÿ������
	void _ProcessFileContent();
	
};