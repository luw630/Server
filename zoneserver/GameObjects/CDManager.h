#pragma once

#include "EventMgr.h"

// 所有的冷却在运行过程中都不会被销毁，只用来标记
class CDManager
{
	typedef std::map<DWORD, class CoolDownBase *> CDMap;

public:
	CDManager(){}

public:
	~CDManager();

public:
	BOOL	Active(DWORD type, DWORD remain);		// 加入冷却频道
	DWORD	Check(DWORD type);						// 检查剩余冷却时间（毫秒）
	BOOL	Deactive(DWORD type);					// 解除冷却
	void	Free();

private:
	CDManager(CDManager &);
	CDManager& operator=(CDManager &);

private:
	CDMap	m_cdMap;								// 冷却频道
};