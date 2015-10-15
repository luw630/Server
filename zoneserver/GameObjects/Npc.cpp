#include "StdAfx.h"
#include "npc.h"
#include "networkmodule/movemsgs.h"
#include "networkmodule/regionmsgs.h"
#include "Random.h"
#include "Region.h"
#include "Area.h"

int CNpc::OnCreate(_W64 long pParameter)
{
	SParameter *pParam = (SParameter*)pParameter;

	m_Property.m_BirthPosX		= pParam->wX;
	m_Property.m_BirthPosY		= pParam->wY;
	m_Property.m_Dir			= pParam->wDir;
	m_Property.m_Chose			= pParam->wChose;
	m_Property.m_byMoveType		= pParam->m_byMoveType;
	m_Property.m_wSchool		= pParam->wSchool;
	m_Property.m_ImageID		= pParam->wImageID;
	m_Property.m_wClickScriptID = pParam->wClickScriptID;
	m_Property.m_LiveTime		= pParam->wLiveTime;
	m_Property.controlId		= pParam->controlId;
	
	if (0xffffffff != m_Property.m_LiveTime)
		m_Property.m_LiveTime = timeGetTime() + m_Property.m_LiveTime*1000;

	// 初始化移动数据
	m_curX			= m_Property.m_BirthPosX << TILE_BITW;
	m_curY			= m_Property.m_BirthPosY << TILE_BITH;
	m_curTileX		= m_Property.m_BirthPosX;
	m_curTileY		= m_Property.m_BirthPosY;
	m_Direction		= (pParam->wDir / 180.0f) * 3.1416f;
	m_ActionFrame	= 50;
	m_IsDisappear	= false;
	m_DisappearTime = 0;

	if (NMT_NOMOVE == m_Property.m_byMoveType)
	{
		m_OriSpeed	= 0;
		m_Speed		= 0.0f;
	}
	else
	{
		m_OriSpeed = PLAYER_BASIC_SPEED;
		m_Speed = m_OriSpeed / 1000.0f;
	}

	return 1;
}

void CNpc::Thinking()
{
	return;
}

BOOL CNpc::DoCurAction()
{
	switch (GetCurActionID())
	{
	case EA_STAND:
		if (NMT_NOMOVE != m_Property.m_byMoveType && --m_ActionFrame <= 0)
		{
			// 可以开始移动了
		}
		break;

	default:
		return CActiveObject::DoCurAction();
	}
	return TRUE;
}

void CNpc::OnRun()
{
	if (m_Property.m_LiveTime != 0xffffffff && (timeGetTime() > m_Property.m_LiveTime))
	{
		if (m_ParentRegion && m_ParentArea)
		{
			if (m_Property.controlId)
				m_ParentRegion->RemoveObjectByControlId(m_Property.controlId);
			else
			{
				LPIObject pObject = m_ParentRegion->LocateObject(GetGID());
				if (pObject)
				{
					SADelObjectMsg msg;

					msg.dwGlobalID = GetGID();
					m_ParentArea->SendAdj(&msg, sizeof(SADelObjectMsg), -1);
					m_ParentArea->DelObject(pObject);
				}
			}
		}
		return;
	}
	
	if (m_IsDisappear && timeGetTime() >= m_DisappearTime)
	{
		m_IsDisappear	= false;
		m_DisappearTime = 0;

		if (m_ParentRegion)
			m_ParentRegion->Move2Area(self.lock(), m_curTileX, m_curTileY, true);
	}

	CActiveObject::OnRun();
}

void CNpc::OnRunEnd()
{
	// 进入站立等待时间
	m_ActionFrame = 50;
}

void CNpc::MakeDisappear(DWORD time)
{
	if (0 == time && !m_ParentRegion && !m_ParentArea)
		return;

	// 已经处于隐身状态
	if (m_IsDisappear)
		return;

	m_IsDisappear	= true;
	m_DisappearTime = timeGetTime() + time;

	// 删除对象
	SADelObjectMsg msg;

	LPIObject pObject = m_ParentRegion->LocateObject(GetGID());
	if (pObject)
	{
		msg.dwGlobalID = GetGID();
		m_ParentArea->SendAdj(&msg, sizeof(SADelObjectMsg), -1);
		m_ParentArea->DelObject(pObject);
	}
}

void CNpc::OnClose()
{
	return;
}

SASynNpcMsg *CNpc::GetStateMsg()
{
	static SASynNpcMsg msg;

	msg.wImageID		= m_Property.m_ImageID;
	msg.wClickScriptID	= m_Property.m_wClickScriptID;
	msg.dwGlobalID		= GetGID();
	msg.dwControlId		= m_Property.controlId;
	msg.wChose			= m_Property.m_Chose;
	msg.byDir			= m_Direction;
	msg.mMoveSpeed		= m_Speed;

	FillSynPos(&msg.ssp);

	return &msg;
}