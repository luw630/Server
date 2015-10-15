#pragma once


#include "BuffFPBase.h"

// 少林跳跃 这种轻功比较特别,没有buffBegin
// class CBuffJump
// 	:public CBuffFlyBase
// {
// public:
// 	CBuffJump(CFightObject* pFighter);
// 	
// 	int		GetMaxJumpTile();
// private:
// 	//BuffBegin 不应使用
// 	//virtual BOOL BuffBegin( BuffContainer* owner,int times=1 ){ return FALSE;}
// 	virtual void OnBegin();
// 	virtual int	 GetRemainTime();
// 	virtual void	OnLevelUp();
// };