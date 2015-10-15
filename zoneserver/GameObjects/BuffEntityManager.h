#pragma once

#include "EventManager.h"

// class CBuffModifyManager 
// {
// public:
// 	CBuffModifyManager();
// 	~CBuffModifyManager();
// 	void Init(CFightObject* Object,BuffContainer* Container);	
// 	bool AddBuff(WORD id,BOOL interrupt,int level = 1,int fightType = 0,BOOL isForever = false);
// 	bool RenewBuff(WORD id,BOOL interrupt,DWORD MaxTime,int level = 1,int fightType = 0);		//重新恢复
// 	void InterruptBuff(BuffContainer* owner);
// 	bool ClearBuffFromId(BuffContainer* owner,int id);
// 	bool IsOwnerBuff(WORD id);	//是否存在的buff
// 	void DropBuff(WORD id);		//释放BUFF	
// 	void DropBuffNode(WORD id);	//去掉管理器里的BUFF
// 	void Free();
// 
// 	typedef std::list< CBuffModify* > BUFFENTITYLIST;
// private:
// 	BUFFENTITYLIST m_lBuffEntity;
// 	CBuffModify* m_CBuffEntity;
// 	CFightObject* m_CFightObj;
// 	BuffContainer* m_CBuffContainer;
// };
