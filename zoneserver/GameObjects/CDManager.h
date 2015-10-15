#pragma once

#include "EventMgr.h"

// ���е���ȴ�����й����ж����ᱻ���٣�ֻ�������
class CDManager
{
	typedef std::map<DWORD, class CoolDownBase *> CDMap;

public:
	CDManager(){}

public:
	~CDManager();

public:
	BOOL	Active(DWORD type, DWORD remain);		// ������ȴƵ��
	DWORD	Check(DWORD type);						// ���ʣ����ȴʱ�䣨���룩
	BOOL	Deactive(DWORD type);					// �����ȴ
	void	Free();

private:
	CDManager(CDManager &);
	CDManager& operator=(CDManager &);

private:
	CDMap	m_cdMap;								// ��ȴƵ��
};