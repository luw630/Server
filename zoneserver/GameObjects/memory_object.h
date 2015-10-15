#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "Player.h"
using namespace boost::interprocess;
typedef struct BackupPlayer
{
	//20150530 wk 30改为MAX_ACCOUNT
	//char m_szAccount[30];
	char m_szAccount[MAX_ACCOUNT];

	DWORD m_dgid;
	//20150117 wk 修改 共享缓存恢复时恢复至SFixData三国结构体,因三国新加使用SFixData,SFixProperty没用了,解决宕机恢复再宕机问题
	//修改内容 在 memory_object.h和memory_object.cpp中将SFixProperty 替换为SFixData
	//RecoverPlayer(CPlayer *pbackPlayer) 中 SetFixProperty---SetFixData   m_Property改为m_FixData
	//:RecoverPlayer(SFixData &pbackPlayer, std::string szAccount, DWORD Gid)  中 SetFixProperty---SetFixData  //20150117 wk  恢复萌将三国没有地图,注释掉
	//PlayerManager.cpp  恢复缓存时传入&player->m_FixData

	//SFixProperty m_Property;
	SFixData m_Property;
	//20150117 wk 修改 共享缓存恢复时恢复至SFixData三国结构体,因三国新加使用SFixData,SFixProperty没用了,解决宕机恢复再宕机问题
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

	bool RecoverAllPlayerProperty();//恢复数据

	int OnPlayerLogout(DWORD sid);//玩家退出时从内存数据中清除


	int Onrefreshmemory();//刷新所有的数据到共享内存区
public:
	std::string  m_strbackname;
	offset_t m_dbufferlength;
	bool m_bIscreateshare;
	BYTE *m_pshAddress;
// 	std::list<DWORD> m_backplayer;//备份玩家列表
// 	std::map<DWORD, CPlayer&> m_mapbackplayer;//保存的玩家列表
	std::list<SBackupPlayer*> m_listbackplayerproperty;//保存的玩家列表
	//std::list<CPlayer*> m_listbackplayer;//保存的玩家列表
	BYTE  m_bmemoryIndex;
};

