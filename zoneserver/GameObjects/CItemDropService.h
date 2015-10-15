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
// 1. ȫ�ֵ����
// 2. ����������ñ�
// 3. װ����ʯ��������������ñ�
//4.��ҵ���װ�����ʱ�
class CItemDropService
{
	typedef std::hash_map<WORD, SMonsterDropTable*>		MonsterDropTable;		//<����ID����>
	typedef std::hash_map<DWORD, std::vector<DWORD> >	GlobalDropTable;		//<ȫ��ID���������>
	typedef std::hash_map<WORD, SMonsterDropSpecial* >	MonsterDropSpecial;		//<��ID����>
	typedef std::hash_map<WORD, PlayerDropTable*>			CPlayerDropTable;		//<���伸�ʣ���>
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
	const PlayerDropTable *GetPlayerDrop() const; //��ȡ���װ�����伸�ʱ�	
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
