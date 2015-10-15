#pragma once
#include "ScriptManager.h"
//�ṹ �ܳ���+stLuaData+ʵ������+stLuaData+ʵ������
struct DataPos
{
	int nDataSize;		//���ݴ�С
	int nDataPos;		//������m_bluaBuffer�е�λ��
	BYTE *pDataPos;		//������m_bluaBuffer�е�λ��
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
	int CoverData(const char* tablename,BYTE *data,WORD lenth);//��������
	
private:
	int m_ndatapos;	//��ǰλ��
	int m_nemptypos;//��ǰ���Դ�����ݵ�λ��
	BYTE *m_pemptypos;//��ǰ���Դ�����ݵ�λ��
	int m_nAlldatasize;//�������ݴ�С
	bool m_bIsInit;
	CPlayer *m_player;
	std::map<std::string,DataPos>m_mapkeydata;//���ݴ��λ��
};

