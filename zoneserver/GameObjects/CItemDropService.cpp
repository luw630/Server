#include "Stdafx.h"
#include <utility>
#include "CItemDropDefine.h"
#include "CItemDropService.h"
#include "ScriptManager.h"

#pragma warning(push)
#pragma warning(disable:4996)

extern CScriptManager g_Script;

CItemDropService::CItemDropService() {}

CItemDropService::~CItemDropService() { CleanUp(); };

void CItemDropService::CleanUp()
{
	for (MonsterDropTable::iterator it = m_MDTable.begin(); it != m_MDTable.end(); ++it)
	{
		SMonsterDropTable *temp = it->second;
		delete temp;
	}

	m_MDTable.clear();
	m_GDTable.clear();
	m_playerdropTable.clear();
	return;
}

bool CItemDropService::Reload()
{
	CleanUp();
	Init();

	return true;
}

bool CItemDropService::Init()
{
	dwt::ifstream streamMD("Data\\掉落\\怪物掉落配置表.txt");
	if (!streamMD.is_open())
	{
		rfalse("无法打开怪物掉落配置表！");
		return false;
	}

	dwt::ifstream streamGD("Data\\掉落\\全局掉落表.txt");
	if (!streamGD.is_open())
	{
		rfalse("无法打开全局掉落配置表！");
		streamMD.close();
		return false;
	}

	dwt::ifstream streamPD("Data\\掉落\\PK掉落几率配置.txt");
	if (!streamPD.is_open())
	{
		rfalse("无法打开PK掉落几率配置！");
		streamPD.close();
		return false;
	}

	char buffer[2048];
	SMonsterDropTable *monsterDrop;

	while (!streamMD.eof())
	{
		streamMD.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		monsterDrop = new SMonsterDropTable;

		linebuf >> monsterDrop->m_MonsterID;
		linebuf >> monsterDrop->m_MoneyType;
		linebuf >> monsterDrop->m_MoneyMount;
		linebuf >> monsterDrop->m_MoneyRate;
		
		for (int i=0; i<10; i++)
		{
			for (int j=0; j<4; j++)
				linebuf >> monsterDrop->m_DropTable[i][j];
		}

		std::pair<MonsterDropTable::iterator, bool> result;
		result = m_MDTable.insert(make_pair(monsterDrop->m_MonsterID, monsterDrop));

		if (false == result.second)
		{
			rfalse("发现了一个重复的怪物掉落表配置！");
			delete monsterDrop;
		}
	}
	streamMD.close();

	while (!streamGD.eof())
	{
		streamGD.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		std::vector<DWORD> table;
		DWORD temp = 0;
		DWORD index;
		linebuf >> index;

		for (int i=0; i<10; i++)
		{
			linebuf >> temp;
			if (temp) table.push_back(temp);
		}	

		std::pair<GlobalDropTable::iterator, bool> result;
		result = m_GDTable.insert(make_pair(index, table));
	}

	streamGD.close();

	PlayerDropTable *playerDrop;

	while (!streamPD.eof())
	{
		streamPD.getline(buffer, sizeof(buffer));
		if (0 == buffer[0] || '//' == (*(LPWORD)buffer))
			continue;

		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));

		playerDrop = new PlayerDropTable;

		linebuf >> playerDrop->m_wDropItem_body;
		linebuf >> playerDrop->m_wDropItem_bag;

		std::pair<CPlayerDropTable::iterator, bool> result;
		result = m_playerdropTable.insert(make_pair(playerDrop->m_wDropItem_body, playerDrop));
	}

	streamPD.close();
	
	
	//g_Script.CallFunc( "DropItem" );
	return true;
}

const SMonsterDropTable *CItemDropService::GetMonsterDropTable(WORD id) const
{
	MonsterDropTable::const_iterator it = m_MDTable.find(id);
	
	if (it != m_MDTable.end())
		return it->second;
	else
		return 0;
}

const std::vector<DWORD>& CItemDropService::GetGlobalDropTable(DWORD index) const
{
	static std::vector<DWORD> temp;		// Empty Array

	GlobalDropTable::const_iterator it = m_GDTable.find(index);
	if (it != m_GDTable.end())
		return it->second;
	else
		return temp;
}


bool CItemDropService::LoadDropSpecial( WORD tableid,DWORD valuetype,SpecialType stype )
{
	MonsterDropSpecial::iterator it=m_SpecialTable.find(tableid);
	if (it!=m_SpecialTable.end())
	{
		if (stype==DROP_GOODID)
		{
			WORD GoodsID=(WORD)valuetype;
			it->second->m_vGoodsID.push_back(GoodsID);
		}
		else if (stype==DROP_MONSTERID)
		{
			WORD monterID=(WORD)valuetype;
			it->second->m_vMonsterID.push_back(monterID);
		}
		else if (stype==DROP_TIME)
		{
			it->second->m_DropTime=valuetype;
		}
		else if (stype==DROP_GOODCOUNT)
		{
			it->second->g_DropCount=valuetype;
		}
		return true;
	}
	else
	{
		SMonsterDropSpecial *dropSpecial=new SMonsterDropSpecial;
		if (stype==DROP_GOODID)
		{
			WORD GoodsID=(WORD)valuetype;
			dropSpecial->m_vGoodsID.push_back(GoodsID);
		}
		else if (stype==DROP_MONSTERID)
		{
			WORD monterID=(WORD)valuetype;
			dropSpecial->m_vMonsterID.push_back(monterID);
		}
		else if (stype==DROP_TIME)
		{
			dropSpecial->m_DropTime=valuetype;
		}
		else if (stype==DROP_GOODCOUNT)
		{
			dropSpecial->g_DropCount=valuetype;
		}
		m_SpecialTable[tableid]=dropSpecial;
		return true;
	}
	return false;
}

bool CItemDropService::LoadSpecialDropItem( WORD tableid,WORD itemID,WORD itemrate )
{
	MonsterDropSpecial::iterator it=m_SpecialTable.find(tableid);
	if (it!=m_SpecialTable.end())
	{
		it->second->m_vGoods[itemID]=itemrate;
		return true;
	}
	return false;
}

const PlayerDropTable * CItemDropService::GetPlayerDrop( ) const
{
	CPlayerDropTable::const_iterator it = m_playerdropTable.begin();
	return it!=m_playerdropTable.end() ? it->second : NULL;
}



#pragma warning(pop)