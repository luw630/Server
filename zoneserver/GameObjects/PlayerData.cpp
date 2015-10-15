#include "StdAfx.h"
#include "PlayerData.h"
#include "Player.h"

CPlayerData::CPlayerData(void):m_ndatapos(0),m_nemptypos(0),m_nAlldatasize(0),m_bIsInit(false)
{
	CPlayer *pPlayer = (CPlayer *)this;
	BYTE *pdata =pPlayer->m_FixData.m_bluaBuffer;
	int nPlayerDLen = (int)*pdata;
	m_nemptypos = nPlayerDLen + sizeof(int);
	m_pemptypos = 0;


}



CPlayerData::~CPlayerData(void)
{
}

bool CPlayerData::SavePlayerTable( const char* tablename,BYTE *data,WORD lenth )
{
	if (!m_bIsInit)
	{
		InitLuaData();
	}

	CPlayer *pPlayer = (CPlayer *)this;
	BYTE *pDatepos = findDataAddress(tablename);
	int ndatasize = GetDataSize(tablename);
	if (pDatepos&& ndatasize > 0) //�����Ѿ�����
	{
		if (ndatasize != lenth)    //��С����ͬ,����ֱ��ɾ��������ݣ������ݺ���ȫ����ǰ�������ݷ������
		{
			//BYTE *pdata = new BYTE[LUABUFFERSIZE];
			//memset(pdata,0,LUABUFFERSIZE);
			//BYTE *pmovedata = pdata;
			//
			//m_nAlldatasize  = 0;								//���¼����С
			//std::map<std::string,DataPos>::iterator iter= m_mapkeydata.begin();
			//while(iter != m_mapkeydata.end())
			//{
			//	std::string keytable = iter->first;
			//	if (dwt::strcmp(tablename, keytable.c_str(), 30) != 0 && iter->second.pDataPos)
			//	{
			//		int nsize= iter->second.nDataSize + sizeof(stLuaData);
			//		memcpy(pmovedata, iter->second.pDataPos, nsize);
			//		pmovedata += nsize;
			//		m_nAlldatasize += nsize;
			//	}
			//	iter++;
			//}
			//memset(pPlayer->m_FixData.m_bluaBuffer,0,LUABUFFERSIZE);								//�������
			//memcpy(pPlayer->m_FixData.m_bluaBuffer, &m_nAlldatasize, sizeof(int));				//�������ݴ�С
			//m_pemptypos = pPlayer->m_FixData.m_bluaBuffer;		
			////20150123 wk ���һ����,��ַ+�˿���,Ӱ���� m_FixData.m_bluaBuffer������sid
			////m_pemptypos += sizeof(int);		
			////memcpy(m_pemptypos, pdata, LUABUFFERSIZE);														//�������¿���
			//
			//memcpy(m_pemptypos, pdata, LUABUFFERSIZE);														//�������¿���
			//m_pemptypos += sizeof(int);
			//
			//SAFE_DELETE_ARRAY(pdata);

			//LPD 2015-8-2 ��С��һ�� ���LUA�����°� ����"data" �������PushBackData 
			if (dwt::strcmp(tablename, "data", 30) != 0)
			{
				rfalse("123����m_mapkeydata����key��Ϊ 'data'��Ԫ�� ");
				try
				{
					rfalse("tablenameΪ%s" , tablename);
				}
				catch (std::exception e)
				{
					rfalse("��׽����tablename �޷�ת���ַ���");
				}
				return false;
			}
				
			memset(pPlayer->m_FixData.m_bluaBuffer, 0, LUABUFFERSIZE);								//�������
			InitLuaData();																											//���´���maptable
		}
		else  //���ݴ�С��ֱͬ����ԭ��λ���ϸ���
		{
			CoverData(tablename,data,lenth);
			return true;
		}
	}
	if (dwt::strcmp(tablename, "data", 30) != 0)
	{
		rfalse("789����m_mapkeydata����key��Ϊ 'data'��Ԫ�� ");
		try
		{
			rfalse("tablenameΪ%s", tablename);
		}
		catch (std::exception e)
		{
			rfalse("��׽����tablename �޷�ת���ַ���");
		}
		return false;
	}
	return PushBackData(tablename, data, lenth) > 0 ? true : false;
}

const void* CPlayerData::LoadPlayerTable( const char* tablename,int &ntablesize )
{
	if (!m_bIsInit)
	{
		InitLuaData();
	}
	CPlayer *pPlayer = (CPlayer *)this;
	ntablesize = 0;
	int ndatapos = findDataPos(tablename);
	BYTE *pdatepos = findDataAddress(tablename);
	int ndatasize = GetDataSize(tablename);
	if (pdatepos && ndatasize > 0) //�����Ѿ�����
	{
		ntablesize = ndatasize;
		return (pdatepos + sizeof(stLuaData));
	}
	return 0;
}

int CPlayerData::findDataPos( const char* tablename )
{
	std::map<std::string,DataPos>::iterator iter= m_mapkeydata.find(tablename);
	if (iter != m_mapkeydata.end())
	{
		return iter->second.nDataPos;
	}
	return 0;
}

void CPlayerData::InitLuaData()
{
	m_bIsInit = true;
	CPlayer *pPlayer = (CPlayer *)this;
	m_pemptypos = pPlayer->m_FixData.m_bluaBuffer;
	memcpy(&m_nAlldatasize, m_pemptypos, sizeof(int));
	m_pemptypos += sizeof(int);
	m_mapkeydata.clear();		//���´���maptable
	stLuaData sluadata;
	if (m_nAlldatasize > 0)
	{
		while(1)
		{
			memset(&sluadata,0,sizeof(stLuaData));
			memcpy(&sluadata,m_pemptypos,sizeof(stLuaData));
			if (sluadata.wDataLen > 0)
			{
				DataPos  playerdatapos;
				std::string strkey = sluadata.cstrDataKey;
				playerdatapos.nDataSize = sluadata.wDataLen;
				playerdatapos.nDataPos = int(m_pemptypos - pPlayer->m_FixData.m_bluaBuffer) + sizeof(stLuaData);  //����m_bluaBuffer��λ�ô�С
				playerdatapos.pDataPos = m_pemptypos;				//��ַָ��
				m_mapkeydata[strkey] = playerdatapos;
				m_pemptypos +=(sizeof(stLuaData) + sluadata.wDataLen);
			}
			else
			{
				m_nemptypos = int(m_pemptypos - pPlayer->m_FixData.m_bluaBuffer);    //����wDataLen ��СΪ0�������Ժ�Ӧ���ǿ�����
				break;
			}
		}
	}
	memcpy(pPlayer->m_FixData.m_bluaBuffer, &m_nAlldatasize, sizeof(int));
}

int CPlayerData::GetDataSize( const char* tablename )
{
	std::map<std::string,DataPos>::iterator iter= m_mapkeydata.find(tablename);
	if (iter != m_mapkeydata.end())
	{
		return iter->second.nDataSize;
	}
	return 0;
}


int CPlayerData::AddDataToMap( const char* tablename,BYTE *data,WORD lenth )
{
	int ndatapos = findDataPos(tablename);
	int ndatasize = GetDataSize(tablename);
	if (ndatapos > 0 && ndatasize > 0) //�����Ѿ�����
	{
		return -1;
	}
	stLuaData stdata;
	strcpy(stdata.cstrDataKey,tablename);
	stdata.wDataLen = lenth;
	return 1;
}

int CPlayerData::PushBackData( const char* tablename,BYTE *data,WORD lenth )
{
	int ndatasize = GetDataSize(tablename);
	if (ndatasize > 0) //�����Ѿ�����
	{
		return -1;
	}
	if (m_nAlldatasize + lenth >= LUABUFFERSIZE)
	{
		rfalse(2, 1, "Lua ���ݱ���ʧ�ܡ�Lua���ݿռ䲻��");
		return -1;
	}

	CPlayer *pPlayer = (CPlayer *)this;
	stLuaData stdata;
	strcpy(stdata.cstrDataKey,tablename);
	stdata.wDataLen = lenth;
	if (m_pemptypos)
	{
		DataPos  playerdatapos;
		std::string strkey = stdata.cstrDataKey;
		playerdatapos.nDataSize = lenth;
		playerdatapos.pDataPos = m_pemptypos;
		m_mapkeydata[strkey] = playerdatapos;

		memcpy(m_pemptypos,&stdata,sizeof(stLuaData));
		m_pemptypos += sizeof(stLuaData);
		memcpy(m_pemptypos,data,lenth);
		m_pemptypos += lenth;

		m_nAlldatasize += sizeof(stLuaData)+lenth;
		memcpy(pPlayer->m_FixData.m_bluaBuffer, &m_nAlldatasize, sizeof(int));
		return 1;
	}
	return 0;
}

int CPlayerData::CoverData( const char* tablename,BYTE *data,WORD lenth )
{
	int ndatasize = GetDataSize(tablename);
	BYTE *Pdatapos = findDataAddress(tablename);
	CPlayer *pPlayer = (CPlayer *)this;
	if (Pdatapos && ndatasize == lenth)
	{
		Pdatapos += sizeof(stLuaData);
		memcpy(Pdatapos, data, lenth);
		return 1;
	}
	return 0;
}

BYTE * CPlayerData::findDataAddress(const char* tablename)
{
	std::map<std::string, DataPos>::iterator iter = m_mapkeydata.find(tablename);
	if (iter != m_mapkeydata.end())
	{
		return iter->second.pDataPos;
	}
	return 0;
}


