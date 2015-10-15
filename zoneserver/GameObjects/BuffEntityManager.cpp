#include "Stdafx.h"
#include "FightObject.h"
#include "BuffSys.h" 
#include "BuffBaseData.h"
#include "BuffEntityManager.h"

// CBuffModifyManager::CBuffModifyManager()
// {
// 	m_CBuffEntity = NULL;
// }
// void CBuffModifyManager::Init( CFightObject* Object,BuffContainer* Container )
// {
// 	m_CFightObj = Object;
// 	m_CBuffContainer = Container;
// }
// CBuffModifyManager::~CBuffModifyManager()
// {
// 	m_CBuffEntity = NULL;
// 	m_CFightObj = NULL;
// 	m_CBuffContainer = NULL;
// }
// bool CBuffModifyManager::AddBuff(WORD id,BOOL interrupt,int level,int fightType,BOOL isForever)
// {
//  	if (IsOwnerBuff(id))
//  	{
//  		DropBuff(id);
//  	}
// 	CBuffModify* curBuff = new(CBuffModify);
// 	assert(curBuff);
// 	if(curBuff)
// 	{
// 		if(!curBuff->InitBuffData(id,level,fightType,isForever))
// 		{
// 			delete curBuff;
// 			return false;
// 		}
// 		if (curBuff->BuffBegin(m_CFightObj,m_CBuffContainer))
// 		{
// 			m_lBuffEntity.push_back(curBuff);
// 			return true;
// 		}	
// 	}
// 	return false;
// }
// bool CBuffModifyManager::RenewBuff( WORD id,BOOL interrupt,DWORD MaxTime,int level /*= 1*/,int fightType /*= 0*/)
// {
// 	if (IsOwnerBuff(id))
// 	{
// 		DropBuff(id);
// 	}
// 	CBuffModify* curBuff = new(CBuffModify);
// 	assert(curBuff);
// 	if(curBuff)
// 	{
// 		if(!curBuff->InitBuffData(id,level,fightType,MaxTime,true))
// 		{
// 			delete curBuff;
// 			return false;
// 		}
// 		if (curBuff->BuffBegin(m_CFightObj,m_CBuffContainer))
// 		{
// 			m_lBuffEntity.push_back(curBuff);
// 			return true;
// 		}	
// 	}
// 	return false;
// }
// 
// bool CBuffModifyManager::ClearBuffFromId( BuffContainer* owner ,int id)
// {
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 
// 		if (buff->GetBuffId() == id)
// 		{
// 			buff->Reset();
// 			return true;
// 		}
// 	}
// 	return false;
// }
// 
// void CBuffModifyManager::DropBuff( WORD id )
// {
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 		if (buff->GetBuffId() == id)
// 		{
// 			buff->Reset();
// 			return;
// 		}
// 	}
// }
// 
// void CBuffModifyManager::DropBuffNode( WORD id )
// {
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 		if (buff->GetBuffId() == id)
// 		{
// 			it = m_lBuffEntity.erase( it );
// 			delete buff;
// 			return;
// 		}
// 	}
// }
// 
// void CBuffModifyManager::InterruptBuff(BuffContainer* owner)
// {
// 	bool isInterruptBuff = false;
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 		//判断打断几率
// 		if (buff->GetBuffProperty() > 0)
// 		{
// 			WORD random =  rand() % 100;
// 			if (random < buff->GetBuffProperty())
// 			{	
// 				buff->Reset();	
// 				isInterruptBuff = true;
// 				break;
// 			}	
// 		}
// 	}
// 	if( isInterruptBuff && m_lBuffEntity.size() > 0 )
// 		InterruptBuff(owner);
// }
// 
// void CBuffModifyManager::Free()
// {
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 		if (buff)
// 			buff->Reset();
// 	}
// 	m_lBuffEntity.clear();
// }
// 
// bool CBuffModifyManager::IsOwnerBuff( WORD id )
// {
// 	//用名字来做判断
// 	BUFFENTITYLIST::iterator it = m_lBuffEntity.begin();
// 	for (;it != m_lBuffEntity.end(); it++)
// 	{
// 		CBuffModify* buff = *it;
// 		char* name = BuffBaseData::getSingle().GetBuffNameFromIndex(id);
// 		if (name && strcmp(buff->GetBuffName(),name) == 0)
// 		{
// 			return true;
// 		}
// 	}
// 	return false;
// }





