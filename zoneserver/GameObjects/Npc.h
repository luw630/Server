#pragma once

#include "ScriptObject.h"

const __int32 IID_NPC = 0x11210094; 

enum NpcMoveType
{
	NMT_NOMOVE = 0,		// �Ӳ��ƶ�
	NMT_PATH,			// ��·���ƶ�

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
		WORD	wX;							// �ڳ����ϵ�λ��
		WORD	wY;					
		WORD	wDir;						// ����
		WORD	wChose;						// �ɷ�ѡ��
		WORD	m_byMoveType;				// NPC�ƶ�����
		WORD	wSchool;					// NPC��������
		DWORD	wImageID;					// ʹ�õ�NPCģ�ͱ��
		DWORD	wClickScriptID;				// ����ű���ţ�Ψһ��������ҵ����ʱ�򴥷���
		DWORD	wLiveTime;					// ����ʱ�䣬������ô��ڣ���Ϊ0xffffffff
		DWORD	controlId;					// ���Ʊ��,���ڽű�����
	};

public:
	struct SNpcProperty
	{
		WORD	m_BirthPosX;				// �ڳ����ϵ�λ��
		WORD	m_BirthPosY;				// 
		WORD	m_Dir;						// ����
		WORD	m_Chose;					// �ɷ�ѡ��
		WORD	m_byMoveType;				// ��ǰ���ƶ�����
		WORD    m_wSchool;					// NPC��������
		DWORD	m_ImageID;					// ʹ�õ�NPCͼƬ���
		DWORD	m_wClickScriptID;			// ����ű���ţ�������ҵ����ʱ�򴥷���
		DWORD	m_LiveTime;					// ����ʱ��
		DWORD   controlId;					// ���Ʊ��,���ڽű����ƣ�Ϊ0��ʾ����Ҫʹ���ⲿ�ӿ������٣�
		WORD	m_wCurRegionID;				// ��ǰ���ڵ�ͼ��ID
	};

	SNpcProperty	m_Property;				// NPC����
	int				m_ActionFrame;			// ������ʱ�䣬����Npc��AI
	bool			m_IsDisappear;			// �Ƿ�������״̬��
	DWORD			m_DisappearTime;		// ����ʱ��
};
