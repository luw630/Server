#include"TabReader.h"
#include "Stdafx.h"
#include"Windows.h"

using namespace std;
#define BEGIN_INDEX 1
#define DATA_START_INDEX 2
CTabTableReader::CTabTableReader(const std::string & fileNameStr)
	:curLineIndex(0),
	maxLineNum(0),
	maxRowNum(0),
	m_ppFileData(nullptr)
{
	CTabTableReader::OpenTabTable(fileNameStr.c_str());
	
}


bool CTabTableReader::OpenTabTable(const string & fileNameStr)
{
	return OpenTabTable(fileNameStr.c_str());
}


bool CTabTableReader::OpenTabTable(const char* fileName)
{
	//打开文件，按行读取文件内容到容器m_FileLineContent
	m_FileLineContent.clear();
	ifstream myfile(fileName, ios::in);
	bool result = true;
	string tempStr;
	if (myfile.is_open())
	{
		while (myfile.good())
		{
			tempStr = _StreamStringReader(myfile);
			if (tempStr == "")
				continue;
			m_FileLineContent.push_back(tempStr);
		}
	
		vector<string> tempContainer;
		m_ppFileData = new string*[m_FileLineContent.size()]; //开辟二维表行空间
		for (int lineIndex = 0; lineIndex < m_FileLineContent.size(); lineIndex++)
		{
			tempContainer.clear();
			StringSplit(m_FileLineContent[lineIndex], m_TabDelim, &tempContainer);
			m_ppFileData[lineIndex] = new string[tempContainer.size()]; //开辟二表维列空间
			for (int strIndex = 0; strIndex < tempContainer.size(); strIndex++)
			{
				m_ppFileData[lineIndex][strIndex] = tempContainer[strIndex]; //给列元素赋值
			}
			if (lineIndex == 0)
				maxRowNum = tempContainer.size();
		}
		maxLineNum = m_FileLineContent.size();
		curLineIndex = BEGIN_INDEX; //初始化行下标
	}
	else
	{
		result = false;
	}
	myfile.clear();
	myfile.close();

	return result;
	
}

string CTabTableReader::_StreamStringReader(istream& stream)
{
	string line;
	getline(stream, line);	
	return line;
}


void CTabTableReader::_ProcessFileContent()
{

	
}


bool CTabTableReader::Release()
{
	m_FileLineContent.clear();
	
	if (m_ppFileData!= nullptr)
	{ 
		for (int i = 0; i < maxLineNum; i++)
		{
			if (m_ppFileData[i] != nullptr)
				delete[]m_ppFileData[i];
		}
		delete[]m_ppFileData;
		
	}
	return true;
}

bool CTabTableReader::ReadLine()
{	
	++curLineIndex;
	if (curLineIndex >= maxLineNum)
		return false;
	return true;
}

string CTabTableReader::GetStrValue(const string& valueName)
{
	for (int rowIndex = 0; rowIndex < maxRowNum; rowIndex++)
	{
		if (valueName == m_ppFileData[0][rowIndex])
			return m_ppFileData[curLineIndex][rowIndex];
	}
	rfalse("属性%s读不到", valueName.c_str());
	return "";
}

int32_t CTabTableReader::GetIntValue(const string& valueName)
{
	string tempString = "";
	for (int rowIndex = 0; rowIndex < maxRowNum; rowIndex++)
	{
		if (valueName == m_ppFileData[0][rowIndex])
		{
			tempString = m_ppFileData[curLineIndex][rowIndex];
			int result = atoi(tempString.c_str());
			return result; 
		}
	}
	rfalse("属性%s读不到", valueName.c_str());
	return 0;
}


float CTabTableReader::GetFloatValue(const string& valueName)
{
	string tempString = "";
	for (int rowIndex = 0; rowIndex < maxRowNum; rowIndex++)
	{
		if (valueName == m_ppFileData[0][rowIndex])
		{
			tempString = m_ppFileData[curLineIndex][rowIndex];
			float result = atof(tempString.c_str());
			return result;
		}
	}
	rfalse("属性%s读不到", valueName.c_str());
	return 0;
}


int32_t CTabTableReader::GetSecondRowIntValue(const string& valueName)
{
	string tempStr = "";
	for (int lineIndex = 0; lineIndex < maxLineNum; lineIndex++)
	{
		if (valueName == m_ppFileData[lineIndex][0])
		{
			tempStr = m_ppFileData[lineIndex][1];
			int result = atoi(tempStr.c_str());
			return result;
		}
	}
	rfalse("属性%s读不到", valueName.c_str());
	return 0;
}
string CTabTableReader::GetSecondRowStrValue(const string& valueName)
{
	string tempStr = "";
	for (int lineIndex = 0; lineIndex < maxLineNum; lineIndex++)
	{
		if (valueName == m_ppFileData[lineIndex][0])
		{
			tempStr = m_ppFileData[lineIndex][1];
			return tempStr;
		}
	}
	rfalse("属性%s读不到", valueName.c_str());
	return "";
}


float CTabTableReader::GetSecondRowFloatValue(const string& valueName)
{
	string tempStr = "";
	for (int lineIndex = 0; lineIndex < maxLineNum; lineIndex++)
	{
		if (valueName == m_ppFileData[lineIndex][0])
		{
			tempStr = m_ppFileData[lineIndex][1];
			float result = atof(tempStr.c_str());
			return result;
		}
	}
	rfalse("属性%s读不到", valueName.c_str());
	return 0;
}

int32_t CTabTableReader::GetDataLineCount() const
{
	return max(maxLineNum - DATA_START_INDEX, 0);
}
