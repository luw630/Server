#include "Stdafx.h"
#include "CoolDownSys.h"

// ��CD���жϻ����ս�ʱ��ֻ��Ҫ���Ƿ���ɱ������Ϊtrue����
void CoolDownBase::OnCancel()
{
	m_IsOver = true;
}	

void CoolDownBase::OnActive()
{
	m_IsOver = true;
}