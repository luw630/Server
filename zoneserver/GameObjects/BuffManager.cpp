#include "stdafx.h"
#include "EventMgr.h"
#include "BuffManager.h"
#include "BuffSys.h"
#include "FightObject.h"
#include "CBuffDefine.h"
#include "CBuffService.h"
#include "Player.h"

BuffManager::~BuffManager()
{
	MY_ASSERT(m_buffList.empty());
	MY_ASSERT(m_buffGroup.empty());
	MY_ASSERT(m_buffToOther.empty());
}

void BuffManager::Init(CFightObject *object)
{
	if (!object)
	{
		rfalse(4,1,"BuffManager::Init(CFightObject *object)");
	}
	MY_ASSERT(object);
	m_curObj = object;
}

void BuffManager::Free()
{
	BuffList copy(m_buffList);

	for (BuffList::iterator it = copy.begin(); it != copy.end(); ++it)
		RemoveBuff(it->second);

	if (g_InDebugMode)
		MY_ASSERT(m_buffList.empty() && "Buff管理器中的Buff没有被清干净!");
}


bool BuffManager::AddBuff(DWORD id, CFightObject *pSender)
{
	if (0 == id || !m_curObj || !pSender || 0 == pSender->m_CurHp)
		return false;

	const SBuffBaseData *pData = CBuffService::GetInstance().GetBuffBaseData(id);
	if (!pData)
		return false;

	if (!m_curObj->CheckAddBuff(pData))
		return false;

	if (!CanAddBuff(pData,pSender))
		return false;

	BuffModify *curBuff = new BuffModify(m_curObj, pSender);

	if (!curBuff->InitBuffData(pData))
	{
		delete curBuff;
		return false;
	}

	// 如果是非永久瞬发buff，则直接处理，不进入事件管理器
	if (1 == curBuff->m_WillActTimes)
	{
		curBuff->OnTimer(0);			// 开始
		curBuff->OnTimer(-1);			// 主动结束
		delete curBuff;
		return true;
	}

	BuildStub(curBuff);

	return true;
}

bool BuffManager::BuildStub(BuffModify *e)
{
	if (!e)
		return false;

	MY_ASSERT(1 != e->m_WillActTimes);

	m_buffList[e->m_ID] = e;

	e->m_curStep = 0;

	if (0 != e->m_WillActTimes)
		EventManager::GetInstance().Register(e, e->m_Margin);

	// 主动触发第一个事件
	//if (e->m_WillActTimes>2)
	{
		e->OnTimer(0);
	}
	

	e->SendNotify(true);

	// 建立Buff组映射
	BuffGroup::iterator it = m_buffGroup.find(e->m_GroupID);
	MY_ASSERT(it == m_buffGroup.end());
	m_buffGroup.insert(make_pair(e->m_GroupID, e->m_ID));

	// 加入交叉索引
	e->m_pusher->m_buffMgr.m_buffToOther.insert(e);

	return TRUE;
}

void BuffManager::ClearStub(BuffModify *e, bool IsInterrupt)
{
	if (!e)
		return;

	e->m_curStep = IsInterrupt ? -2 : -1;

	// 主动触发移除/中断事件
	e->OnTimer(e->m_curStep);

	e->SendNotify(false);

	// 删除Buff组映射
	BuffGroup::iterator it = m_buffGroup.find(e->m_GroupID);
	if (it != m_buffGroup.end())
	{
		m_buffGroup.erase(it);
	}
	else
	{
		rfalse(2, 1, "删除Buff组映射 错误");
	}
// 	MY_ASSERT(it != m_buffGroup.end());
// 	m_buffGroup.erase(it);

	// 删除交叉索引
	if (e->m_pusher)
	{
		BuffToOther::iterator it = e->m_pusher->m_buffMgr.m_buffToOther.find(e);
		if (it != e->m_pusher->m_buffMgr.m_buffToOther.end())
		{
			e->m_pusher->m_buffMgr.m_buffToOther.erase(it);
		}
		else
		{
			rfalse(2, 1, "删除交叉索引 错误");
		}
// 		MY_ASSERT(it != e->m_pusher->m_buffMgr.m_buffToOther.end());
// 		e->m_pusher->m_buffMgr.m_buffToOther.erase(it);
	}

	return;
}

bool BuffManager::CanAddBuff(const SBuffBaseData *pData, CFightObject *pSender)
{
	if (!pData)
	{
		rfalse(4,1,"BuffManager::CanAddBuff(const SBuffBaseData *pData)1");
		return false;
	}

	if (m_curObj&&pSender)
	{
		if (m_curObj->GetGID() != pSender->GetGID()) //给目标添加BUFF
		{
			if (!m_curObj->m_bPlayerAction[CST_ADDBUFF])
			{
				rfalse(2, 1, "目标当前无法添加BUFF");
				return false;
			}
		}
	}

	BuffGroup::iterator groupIt = m_buffGroup.find(pData->m_GroupID);
	if (groupIt == m_buffGroup.end())
	{
		//rfalse(2, 1, "玩家身上没有Buff组%d中的Buff", pData->m_GroupID);
		//MY_ASSERT(m_buffList.end() == m_buffList.find(pData->m_ID));
		return true;
	}

	BuffList::iterator buffIt = m_buffList.find(groupIt->second);
	if (buffIt == m_buffList.end())
	{
		rfalse(4,1,"BuffManager::CanAddBuff(const SBuffBaseData *pData)2");
		return false;
	}
	MY_ASSERT(m_buffList.end() != buffIt);
	//TODO:Tony Modify [2012-3-11]Comment:[IF语句后面有;]
	//if (pData->m_GroupID != buffIt->second->m_GroupID);
	///////////////////////////////////////////////////////////////////
	if (pData->m_GroupID != buffIt->second->m_GroupID)
	{
		rfalse(4,1,"BuffManager::CanAddBuff(const SBuffBaseData *pData)3");
		return false;
	}
	MY_ASSERT(pData->m_GroupID == buffIt->second->m_GroupID);
	
	if (!buffIt->second->m_CanBeReplaced)
		return false;

	// 如果可被替换，那么查看权重
	if (buffIt->second->m_Weight > pData->m_Weight)
		return false;

	// 顶替
	RemoveBuff(buffIt->second);

	return true;
}

void BuffManager::CheckDelBuffSpot(DWORD delFlag)
{
	if (0 == delFlag || (delFlag & (delFlag-1)))
		return;

	BuffList copy(m_buffList);

	for (BuffList::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		if ((it->second->m_DeleteType & delFlag)&& (it->second->m_InturptRate != 0 || delFlag == BDT_DEAD) )
			RemoveBuff(it->second);
	}

	return;
}

void BuffManager::CheckDelBuffType(DWORD delType)
{
	BuffList copy(m_buffList);

	for (BuffList::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		if (delType == it->second->m_PropertyID)
			RemoveBuff(it->second);
	}
}

void BuffManager::DeleteBuff(DWORD id)
{
	BuffList::iterator it = m_buffList.find(id);
	if (it != m_buffList.end())
	{
		m_buffList.erase(id);
	}
	//MY_ASSERT(it != m_buffList.end());
	//return;
}

void BuffManager::RemoveBuff(BuffModify *buff)
{
	if (!buff)
	{
		rfalse(4,1,"BuffManager::RemoveBuff(BuffModify *buff)1");
		return;
	}
	/*if (buff->m_ActedTimes == 1)
	{
		rfalse(4,1,"BuffManager::RemoveBuff(BuffModify *buff)2");
		return;
	}*/
	MY_ASSERT(buff);
	//MY_ASSERT(1 != buff->m_WillActTimes);

	// 首先判断是否为永久Buff
	if (0 == buff->m_WillActTimes)
	{
		// 如果为永久Buff，则跳过从事件管理器获取Buff
		buff->OnCancel();
		buff->SelfDestory();
	}
	else
		buff->Interrupt();

	return;
}

void BuffManager::PusherNotAvaliable()
{
	for (BuffToOther::iterator it = m_buffToOther.begin(); it != m_buffToOther.end(); ++it)
		(*it)->m_pusher = 0;
	
	m_buffToOther.clear();
}

#pragma region "HANDLE_BUFF_SAVE_READ"
// 恢复下线存储的buff列表，目前只处理自己给自己添加的buff
BYTE BuffManager::RecoverSaveBuffList(const TSaveBuffData*& aSaveData, BYTE byMaxCount)
{
	BYTE bySuccessNum = 0;
	for (BYTE i = 0; i < byMaxCount; ++i)
	{
		const TSaveBuffData *pcSaveData = &(aSaveData[i]);
		if (0 == pcSaveData->m_dwBuffID || !m_curObj || 0 == m_curObj)
			continue ;		

		const SBuffBaseData *pData = CBuffService::GetInstance().GetBuffBaseData(pcSaveData->m_dwBuffID);
		if (!pData)
			continue ;
		if (!m_curObj->CheckAddBuff(pData))
			continue ;

		if (!CanAddBuff(pData,0))
			continue ;

		bool bNeedHandle = false;
		switch (pData->m_SaveType)
		{
		case BST_SAVE_WITH_TIME:
			{				
				CPlayer *pPlayer = (CPlayer *)m_curObj->DynamicCast(IID_PLAYER);
				if (NULL != pPlayer)
				{
					// 需要调整和测试：此处buff事件重设： 保存时buff的剩余时间 - （这次登陆和上次离开的时间差）
					DWORD dwRemainTime = pcSaveData->m_ActedTimes * pData->m_Margin;
					// 离开游戏和下次登陆游戏的时间差，单位s
					INT64 illElapsedTime = (pPlayer->_L_GetCurrentLoginTime() - pPlayer->_L_GetLastLeaveTime()) * 1000;
					if (dwRemainTime > illElapsedTime)
					{
						// todo..如果测试通过需要放开
						//pcSaveData->m_ActedTimes = ((dwRemainTime - illElapsedTime) / pData->m_Margin) + 1;
						bNeedHandle = true;
					}
				}
			}
			break;
		case BST_SAVE_NO_TIME:
			bNeedHandle = true;
			break;
		default:
			break;;
		}
		if (!bNeedHandle)
		{
			continue;
		}		

		BuffModify *curBuff = new BuffModify(m_curObj, m_curObj);

		if (!curBuff->InitBuffData(pData))
		{
			delete curBuff;
			continue ;
		}

		// 如果是非永久瞬发buff，则直接处理，不进入事件管理器
		if (1 == curBuff->m_WillActTimes)
		{
			curBuff->OnTimer(0);			// 开始
			curBuff->OnTimer(-1);			// 主动结束
			delete curBuff;
			continue ;
		}
		curBuff->SetSaveBuffData(pcSaveData);
		BuildStub(curBuff);	
		++bySuccessNum;
	}

	return bySuccessNum;
}

// 获取需要保存的buff列表
BYTE BuffManager::GetSaveBuffList(TSaveBuffData*& aSaveData, BYTE byMaxCount) const
{
	BYTE byRetNum = 0;
	BuffList copy(m_buffList);
	for (BuffList::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		if (byRetNum >= byMaxCount)
			break;
		if (it->second->m_SaveType == BST_SAVE_WITH_TIME || it->second->m_SaveType == BST_SAVE_NO_TIME)
		{
			it->second->GetSaveBuffData(&aSaveData[byRetNum++]);
		}
	}
	return byRetNum;
}
#pragma endregion "HANDLE_BUFF_SAVE_READ"