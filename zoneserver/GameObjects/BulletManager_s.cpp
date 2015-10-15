#include "stdafx.h"
#include "FightObject.h"
#include "BulletManager_s.h"

//----------------------------------------------------------------------------------------------------------
CBulletManager_s* CBulletManager_s::GetSingleton()
{
	static CBulletManager_s s;
	return &s;
}

//----------------------------------------------------------------------------------------------------------
CBulletManager_s::CBulletManager_s()
{
	AllocBullets();
}

//----------------------------------------------------------------------------------------------------------
void CBulletManager_s::AllocBullets()
{
	for (int i = 0;i < 100;++ i)
	{
		m_FreeBulletList.push_back(new SBullet_s()); 
	}
}
 
//----------------------------------------------------------------------------------------------------------
CBulletManager_s::~CBulletManager_s()
{
	for ( std::list< SBullet_s* >::iterator it = m_FreeBulletList.begin();
		it != m_FreeBulletList.end(); it ++ )
	{
		if((*it))
		{
			delete (*it);
		}
	}

	for ( std::list< SBullet_s* >::iterator it = m_ActiveBulletList.begin();
		it != m_ActiveBulletList.end(); it ++ )
	{
		if((*it))
		{
			delete (*it);
		}
	}
}

//----------------------------------------------------------------------------------------------------------
bool CBulletManager_s::FireBullet(DWORD hitTime,CFightObject* attacker)
{
	if (m_FreeBulletList.size() == 0)
	{
		AllocBullets();
	}
	SBullet_s* bullet = m_FreeBulletList.front();
	m_FreeBulletList.pop_front();


	bullet->m_hitTime = hitTime;
	bullet->attacker = attacker;
	bool isFind = false;

	for ( std::list< SBullet_s* >::iterator it = m_ActiveBulletList.begin();
		it != m_ActiveBulletList.end(); it ++ )
	{
		SBullet_s* tempBullet = *it;
		if(tempBullet->m_hitTime > bullet->m_hitTime)
		{
			m_ActiveBulletList.insert(it,bullet);
			isFind = true;
			break;
		}
	}

	// 没有找到，说明list是空的那么直接插入到list的头部
	if (!isFind)
	{
      m_ActiveBulletList.push_front(bullet);
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------
void CBulletManager_s::Update()
{
	DWORD time = timeGetTime();
	for ( std::list< SBullet_s* >::iterator it = m_ActiveBulletList.begin();
		it != m_ActiveBulletList.end(); )
	{
		SBullet_s* tempBullet = *it;
		if (time > tempBullet->m_hitTime)
		{
			it = m_ActiveBulletList.erase(it);
			SendWoundMessage(tempBullet);
		}
		else
		{
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------	
void CBulletManager_s::SendWoundMessage(SBullet_s* bullet)
{
}	

