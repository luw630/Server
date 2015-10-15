#pragma once
#include "ScriptManager.h"
//结构 总长度+stLuaData+实际数据+stLuaData+实际数据
struct DataPos
{
	int nDataSize;		//数据大小
	int nDataPos;		//数据在m_bluaBuffer中的位置
	BYTE *pDataPos;		//数据在m_bluaBuffer中的位置
	DataPos::DataPos():nDataPos(0),nDataSize(0){}
};
class CPlayerData
{
public:
	CPlayerData(void);
	~CPlayerData(void);
	void InitLuaData();
public:
	bool SavePlayerTable(const char* tablename,BYTE *data,WORD lenth);
	const void* LoadPlayerTable(const char* tablename,int &ntablesize); 
	int findDataPos(const char* tablename);
	int GetDataSize(const char* tablename);
	BYTE *findDataAddress(const char* tablename);
	int AddDataToMap(const char* tablename,BYTE *data,WORD lenth);
	int PushBackData(const char* tablename,BYTE *data,WORD lenth);
	int CoverData(const char* tablename,BYTE *data,WORD lenth);//覆盖数据
	
private:
	int m_ndatapos;	//当前位置
	int m_nemptypos;//当前可以存放数据的位置
	BYTE *m_pemptypos;//当前可以存放数据的位置
	int m_nAlldatasize;//所有数据大小
	bool m_bIsInit;
	CPlayer *m_player;
	std::map<std::string,DataPos>m_mapkeydata;//数据存放位置
};

