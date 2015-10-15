#pragma once

#include "ScriptObject.h"

const __int32 IID_NPC = 0x11210094; 

enum NpcMoveType
{
	NMT_NOMOVE = 0,		// 从不移动
	NMT_PATH,			// 沿路径移动

	NMT_MAX,			// Max
};

class CNpc : public CScriptObject
{
public:
	IObject *VDC(const __int32 IID) 
	{
		if (IID == IID_ACTIVEOBJECT) 
			return this;
		
		return NULL; 
	}

public:
	void OnClose();
	void OnRun();
	void OnRunEnd();
	void MakeDisappear(DWORD time);
	BOOL DoCurAction();

protected:
	int OnCreate(_W64 long pParameter);
	
private:
	void Thinking();

public:
	struct SASynNpcMsg *GetStateMsg();

public:
	struct SParameter
	{
		WORD	wX;							// 在场景上的位置
		WORD	wY;					
		WORD	wDir;						// 方向
		WORD	wChose;						// 可否选中
		WORD	m_byMoveType;				// NPC移动类型
		WORD	wSchool;					// NPC所属门派
		DWORD	wImageID;					// 使用的NPC模型编号
		DWORD	wClickScriptID;				// 点击脚本编号（唯一，当被玩家点击的时候触发）
		DWORD	wLiveTime;					// 生存时间，如果永久存在，则为0xffffffff
		DWORD	controlId;					// 控制编号,用于脚本控制
	};

public:
	struct SNpcProperty
	{
		WORD	m_BirthPosX;				// 在场景上的位置
		WORD	m_BirthPosY;				// 
		WORD	m_Dir;						// 方向
		WORD	m_Chose;					// 可否选中
		WORD	m_byMoveType;				// 当前的移动类型
		WORD    m_wSchool;					// NPC所属门派
		DWORD	m_ImageID;					// 使用的NPC图片编号
		DWORD	m_wClickScriptID;			// 点击脚本编号（当被玩家点击的时候触发）
		DWORD	m_LiveTime;					// 生存时间
		DWORD   controlId;					// 控制编号,用于脚本控制（为0表示不需要使用外部接口来销毁）
		WORD	m_wCurRegionID;				// 当前所在地图的ID
	};

	SNpcProperty	m_Property;				// NPC属性
	int				m_ActionFrame;			// 动作的时间，用于Npc的AI
	bool			m_IsDisappear;			// 是否处于隐身状态中
	DWORD			m_DisappearTime;		// 隐身时间
};
