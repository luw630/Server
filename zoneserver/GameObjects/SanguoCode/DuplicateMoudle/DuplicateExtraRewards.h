// ----- CDuplicateExtraRewards.h -----
//
//   --  Author: Jonson
//   --  Date:   15/10/20
//   --  Desc:   萌斗三国的副本的限制判断
// --------------------------------------------------------------------
//   --  管理某一个副本的一些额外奖励，比如章节的星级奖励
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

