#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "Player.h"
using namespace boost::interprocess;
typedef struct BackupPlayer
{
	//20150530 wk 30��ΪMAX_ACCOUNT
	//char m_szAccount[30];
	char m_szAccount[MAX_ACCOUNT];

	DWORD m_dgid;
	//20150117 wk �޸� ������ָ�ʱ�ָ���SFixData�����ṹ��,�������¼�ʹ��SFixData,SFixPropertyû����,���崻��ָ���崻�����
	//�޸����� �� memory_object.h��memory_object.cpp�н�SFixProperty �滻ΪSFixData
	//RecoverPlayer(CPlayer *pbackPlayer) �� SetFixProperty---SetFixData   m_Property��Ϊm_FixData
	//:RecoverPlayer(SFixData &pbackPlayer, std::string szAccount, DWORD Gid)  �� SetFixProperty---SetFixData  //20150117 wk  �ָ��Ƚ�����û�е�ͼ,ע�͵�
	//PlayerManager.cpp  �ָ�����ʱ����&player->m_FixData

	//SFixProperty m_Property;
	SFixData m_Property;
	//20150117 wk �޸� ������ָ�ʱ�ָ���SFixData�����ṹ��,�������¼�ʹ��SFixData,SFixPropertyû����,���崻��ָ���崻�����
}SBackupPlayer;

class Cmemory_object
{
public:
	Cmemory_object();
	~Cmemory_object();
	void CreateShareMemory(const char *name, offset_t length);
	void CloseShareMemory();
	//void BackupPlayer(CPlayer *pPlayer);
	//void RecoverPlayer(CPlayer *pPlayer);

	void BackupPlayer(SFixData *pPlayer, std::string szAccount, DWORD Gid);
	//void BackupPlayer(SBackupPlayer &backupPlayer);
	bool RecoverPlayer(SFixData &pbackPlayer, std::string szAccount, DWORD Gid);

	bool RecoverAllPlayerProperty();//�ָ�����

	int OnPlayerLogout(DWORD sid);//����˳�ʱ���ڴ����������


	int Onrefreshmemory();//ˢ�����е����ݵ������ڴ���
public:
	std::string  m_strbackname;
	offset_t m_dbufferlength;
	bool m_bIscreateshare;
	BYTE *m_pshAddress;
// 	std::list<DWORD> m_backplayer;//��������б�
// 	std::map<DWORD, CPlayer&> m_mapbackplayer;//���������б�
	std::list<SBackupPlayer*> m_listbackplayerproperty;//���������б�
	//std::list<CPlayer*> m_listbackplayer;//���������б�
	BYTE  m_bmemoryIndex;
};

