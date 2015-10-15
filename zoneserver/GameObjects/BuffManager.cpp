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
		MY_ASSERT(m_buffList.empty() && "Buff�������е�Buffû�б���ɾ�!");
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

	// ����Ƿ�����˲��buff����ֱ�Ӵ����������¼�������
	if (1 == curBuff->m_WillActTimes)
	{
		curBuff->OnTimer(0);			// ��ʼ
		curBuff->OnTimer(-1);			// ��������
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

	// ����������һ���¼�
	//if (e->m_WillActTimes>2)
	{
		e->OnTimer(0);
	}
	

	e->SendNotify(true);

	// ����Buff��ӳ��
	BuffGroup::iterator it = m_buffGroup.find(e->m_GroupID);
	MY_ASSERT(it == m_buffGroup.end());
	m_buffGroup.insert(make_pair(e->m_GroupID, e->m_ID));

	// ���뽻������
	e->m_pusher->m_buffMgr.m_buffToOther.insert(e);

	return TRUE;
}

void BuffManager::ClearStub(BuffModify *e, bool IsInterrupt)
{
	if (!e)
		return;

	e->m_curStep = IsInterrupt ? -2 : -1;

	// ���������Ƴ�/�ж��¼�
	e->OnTimer(e->m_curStep);

	e->SendNotify(false);

	// ɾ��Buff��ӳ��
	BuffGroup::iterator it = m_buffGroup.find(e->m_GroupID);
	if (it != m_buffGroup.end())
	{
		m_buffGroup.erase(it);
	}
	else
	{
		rfalse(2, 1, "ɾ��Buff��ӳ�� ����");
	}
// 	MY_ASSERT(it != m_buffGroup.end());
// 	m_buffGroup.erase(it);

	// ɾ����������
	if (e->m_pusher)
	{
		BuffToOther::iterator it = e->m_pusher->m_buffMgr.m_buffToOther.find(e);
		if (it != e->m_pusher->m_buffMgr.m_buffToOther.end())
		{
			e->m_pusher->m_buffMgr.m_buffToOther.erase(it);
		}
		else
		{
			rfalse(2, 1, "ɾ���������� ����");
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
		if (m_curObj->GetGID() != pSender->GetGID()) //��Ŀ�����BUFF
		{
			if (!m_curObj->m_bPlayerAction[CST_ADDBUFF])
			{
				rfalse(2, 1, "Ŀ�굱ǰ�޷����BUFF");
				return false;
			}
		}
	}

	BuffGroup::iterator groupIt = m_buffGroup.find(pData->m_GroupID);
	if (groupIt == m_buffGroup.end())
	{
		//rfalse(2, 1, "�������û��Buff��%d�е�Buff", pData->m_GroupID);
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
	//TODO:Tony Modify [2012-3-11]Comment:[IF��������;]
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

	// ����ɱ��滻����ô�鿴Ȩ��
	if (buffIt->second->m_Weight > pData->m_Weight)
		return false;

	// ����
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

	// �����ж��Ƿ�Ϊ����Buff
	if (0 == buff->m_WillActTimes)
	{
		// ���Ϊ����Buff�����������¼���������ȡBuff
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
// �ָ����ߴ洢��buff�б�Ŀǰֻ�����Լ����Լ���ӵ�buff
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
					// ��Ҫ�����Ͳ��ԣ��˴�buff�¼����裺 ����ʱbuff��ʣ��ʱ�� - ����ε�½���ϴ��뿪��ʱ��
					DWORD dwRemainTime = pcSaveData->m_ActedTimes * pData->m_Margin;
					// �뿪��Ϸ���´ε�½��Ϸ��ʱ����λs
					INT64 illElapsedTime = (pPlayer->_L_GetCurrentLoginTime() - pPlayer->_L_GetLastLeaveTime()) * 1000;
					if (dwRemainTime > illElapsedTime)
					{
						// todo..�������ͨ����Ҫ�ſ�
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

		// ����Ƿ�����˲��buff����ֱ�Ӵ����������¼�������
		if (1 == curBuff->m_WillActTimes)
		{
			curBuff->OnTimer(0);			// ��ʼ
			curBuff->OnTimer(-1);			// ��������
			delete curBuff;
			continue ;
		}
		curBuff->SetSaveBuffData(pcSaveData);
		BuildStub(curBuff);	
		++bySuccessNum;
	}

	return bySuccessNum;
}

// ��ȡ��Ҫ�����buff�б�
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