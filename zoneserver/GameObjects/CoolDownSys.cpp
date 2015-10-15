#include "Stdafx.h"
#include "CoolDownSys.h"

// 当CD被中断或者终结时，只需要将是否完成标记设置为true即可
void CoolDownBase::OnCancel()
{
	m_IsOver = true;
}	

void CoolDownBase::OnActive()
{
	m_IsOver = true;
}