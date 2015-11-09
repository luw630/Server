// ----- CDuplicateExtraRewards.h -----
//
//   --  Author: Jonson
//   --  Date:   15/10/20
//   --  Desc:   �ȶ������ĸ����������ж�
// --------------------------------------------------------------------
//   --  ����ĳһ��������һЩ���⽱���������½ڵ��Ǽ�����
//---------------------------------------------------------------------   
#pragma once
#include <set>

class CDuplicateExtraRewards
{
public:
	explicit CDuplicateExtraRewards(const set<int>* duplicateIDSet);
	~CDuplicateExtraRewards();

private:
	const set<int>* m_ptrDuplicateIDList;
};

