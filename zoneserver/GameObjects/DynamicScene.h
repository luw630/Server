#pragma once
#include "DynamicRegion.h"
const __int32 IID_DYNAMICSCENE = 0x19eaa189; 

class CDynamicScene : public CDynamicRegion
{
public:
	CDynamicScene(void);
	~CDynamicScene(void);

public:
	IObject *VDC(const __int32 IID) { return (IID == IID_DYNAMICSCENE) ? this : CDynamicRegion::VDC(IID); }

protected:
	int OnCreate(_W64 long pParameter);
	void OnClose();

public:
	virtual int AddObject(LPIObject pChild);
	virtual int DelObject(LPIObject pChild);
	virtual int DelMonster(DWORD dwGID);
	// һ��ֻ�ڴ���ʱ����
	void SetCopySceneGID(DWORD GID) { m_CopySceneGID = GID; }
	DWORD GetCopySceneGID() { return m_CopySceneGID; }

protected:
	void OnRun(void);

protected:
	DWORD	m_CopySceneGID;
	DWORD	m_playerID;				// ����˶�̬��������ҵ�GID
public:
	DWORD	m_DynamicIndex;			// ���ñ��е�����
};

