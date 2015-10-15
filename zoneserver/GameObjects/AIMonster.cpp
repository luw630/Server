#include "StdAfx.h"
#include <map>
#include "monster.h"
#include "gameworld.h"
#include "area.h"
#include "item.h"
#include "region.h"
#include "Player.h"
#include "Trigger.h"
#include "ScriptManager.h"
extern CScriptManager g_Script;

#include "TeamManager.h"
#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "networkmodule/upgrademsgs.h"
#include "NetWorkModule/TeamMsgs.h"
#include "区域服务器/Environment.h"
#include "DSpcialItemData.h"
#include "BuffBaseData.h"
#include "Random.h"
#include "CMonsterService.h"
#include "CItemDropService.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "CMonsterDefine.h"
#include "Pet.h"
#include "DynamicRegion.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
void CMonster::ProcessScript()
{
	if(0 == m_AIProperty.preUpdateTime)
	{	//第一次，调用初始化怪物的脚本
		m_AIProperty.startUpdateTime =
			m_AIProperty.preUpdateTime = timeGetTime();

		g_Script.SetCondition( this, NULL, NULL );
		LuaFunctor(g_Script, FormatString( "Monster_Init_%d", m_AIParameter.wAIID ))[m_ParentRegion->GetGID()][GetGID()][m_AIParameter.wListID]();
		g_Script.CleanCondition();

		return;
	}

	//if ((timeGetTime() - m_AIProperty.preUpdateTime) > 1000) //每秒检查一次 
	{
		g_Script.SetCondition( this, NULL, NULL );
		LuaFunctor( g_Script, FormatString( "Monster_Update_%d", m_AIParameter.wAIID ))
			[m_ParentRegion->GetGID()][GetGID()][m_AIParameter.wListID][timeGetTime()-m_AIProperty.startUpdateTime]();
		g_Script.CleanCondition();

		m_AIProperty.preUpdateTime = timeGetTime();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CMonster::SetMonsterPath(CMonster::SParameter param)
{
	memcpy(&m_AIParameter.m_Path, &param.m_Path, sizeof(SPath));
}

void CMonster::MakeSynPathMsg(SQSynPathMsg &msg)
{
	msg.m_GID				= GetGID();
	msg.m_move2What			= SQSynPathMsg::normal;

	memcpy(&msg.m_Path, &m_AIParameter.m_Path, sizeof(SPath));
	msg.m_Path.m_path[0].m_X = m_curX;
	msg.m_Path.m_path[0].m_Y = m_curY;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
