#pragma once

#include "CItemDropDefine.h"

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SMonsterDropTable;
class DropEquipRandomAtt;
class SMonsterDropSpecial;

enum SpecialType
{
	DROP_GOODID,
	DROP_MONSTERID,
	DROP_TIME,
	DROP_GOODCOUNT,
};
// 1. 全局掉落表
// 2. 怪物掉落配置表
// 3. 装备宝石掉落随机属性配置表
//4.玩家掉落装备几率表
class CItemDropService
{
	typedef std::hash_map<WORD, SMonsterDropTable*>		MonsterDropTable;		//<怪物ID，表>
	typedef std::hash_map<DWORD, std::vector<DWORD> >	GlobalDropTable;		//<全局ID，掉落道具>
	typedef std::hash_map<WORD, SMonsterDropSpecial* >	MonsterDropSpecial;		//<表ID，表>
	typedef std::hash_map<WORD, PlayerDropTable*>			CPlayerDropTable;		//<掉落几率，表>
private:
	CItemDropService();

public:
	static CItemDropService& GetInstance()
	{
		static CItemDropService instance;
		return instance;
	}

	bool Reload();
	bool Init();

	bool LoadDropSpecial(WORD tableid,DWORD valuetype,SpecialType stype);

	bool LoadSpecialDropItem(WORD tableid,WORD itemID,WORD itemrate);
	bool LoadSpecialMonsterID(WORD tableid,DWORD monsterID);
	bool LoadSpecialother(WORD tableid,DWORD ItemNum,DWORD TimeInterval,BYTE type);
	~CItemDropService();

	const SMonsterDropTable *GetMonsterDropTable(WORD id) const;
	const std::vector<DWORD>& GetGlobalDropTable(DWORD index) const;
	const PlayerDropTable *GetPlayerDrop() const; //获取玩家装备掉落几率表	
private:
	CItemDropService(CItemDropService &);
	CItemDropService& operator=(CItemDropService &);

	void CleanUp();

private:
	MonsterDropTable	m_MDTable;
	GlobalDropTable		m_GDTable;
	MonsterDropSpecial  m_SpecialTable;
	CPlayerDropTable		m_playerdropTable;
};

#pragma warning(pop)
