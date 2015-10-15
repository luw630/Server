#include "StdAfx.h"
#include "BulletMgr.h"
#include "FightObject.h"
#include "CSkillService.h"
#include <d3dx9math.h>
#include <map>


Bullet::Bullet(CFightObject *sender, CFightObject *target)
{
	MY_ASSERT(sender);
	m_Sender = sender;
	m_Target = target;
}

Bullet::~Bullet()
{
	// 清除枪手和目标的交叉引用
	MY_ASSERT(m_Sender);
	std::set<DWORD>::iterator it = m_Sender->m_FiredToOther.find(m_BulletIndex);
	MY_ASSERT(it != m_Sender->m_FiredToOther.end());
	m_Sender->m_FiredToOther.erase(it);

	if (SAST_SINGLE == m_AtkType)
	{
		MY_ASSERT(m_Target);
		std::set<DWORD>::iterator it = m_Target->m_FiredToMe.find(m_BulletIndex);
		MY_ASSERT(it != m_Target->m_FiredToMe.end());
		m_Target->m_FiredToMe.erase(it);
	}
}

bool Bullet::ProcessTrack()
{
	DWORD curTime = timeGetTime();
	float temp = curTime - m_StartTime;
	m_StartTime = curTime;

	if (BUTT_TRACK == m_TrackType && SAST_SINGLE == m_AtkType)
	{
		m_DesX = m_Target->m_curX;
		m_DesY = m_Target->m_curY;
	}

	D3DXVECTOR2 curPos(m_CurX, m_CurY);
	D3DXVECTOR2 desPos(m_DesX, m_DesY);
	D3DXVECTOR2 desVector = desPos - curPos;

	D3DXVec2Normalize(&desVector, &desVector);

	// 计算从上一帧到现在走了多少距离
	float len = temp * (m_BltSpeed / 1000.f);

	D3DXVECTOR2 speed  = desVector * len;
	D3DXVECTOR2 newPos = curPos + speed;

	DWORD newLen = D3DXVec2Length(&(newPos - curPos));
	DWORD desLen = D3DXVec2Length(&(desPos - curPos));

	if (newLen + 10 >= desLen)
	{
		m_HitTime	= timeGetTime();
		m_CurPhase	= BUP_HIT;
		return true;
	}
	else
	{
		m_CurX = newPos.x;
		m_CurY = newPos.y;
		return false;
	}
}

bool Bullet::Update()
{
	bool Finished = false;

	// 追踪处理
	if (BUP_TRACK == m_CurPhase)
	{
		if (!ProcessTrack())
			return Finished;
	}

	// 命中处理
	if (BUP_HIT == m_CurPhase)
	{
		DWORD CurrentTime = timeGetTime();

		while (true)
		{
			DWORD e = CurrentTime - m_HitTime;
			e += 5;		// 做一次微调

			if (e >= m_ActiveInteval)
			{
				m_HitTime += m_ActiveInteval;

				m_CurActive++;

				if (m_ActiveCount == m_CurActive)
				{
					ProcessHit();
					Finished = true;
					break;
				}
				else
					ProcessHit();
			}
			else
				break;
		}
	}

	return Finished;
}

bool Bullet::ProcessHit()
{
	switch (m_AtkType)
	{
	case SAST_SINGLE:			// 单攻
		MY_ASSERT(m_Sender);
		m_Sender->DamageObject_Single_Bullet(this);
		break;

	case SAST_POINT_CIRCLE:		// 圆形群攻
		rfalse("有目标点的圆形群攻, 追踪到目标");

		break;

	case SAST_POINT_SINGLE:		// 对目标点释放（瞬移，陷阱，产生各种效果。。。）
		rfalse("对目标点释放SAST_POINT_SINGLE, 追踪到目标");
		break;

	default:
		MY_ASSERT(0);
	}

	// 此处执行脚本，有单独的参数

	return true;
}

DWORD BulletManager::BulletID = 0;

BulletManager& BulletManager::GetInstance()
{
	static BulletManager instance;
	return instance;
}

BulletManager::~BulletManager()
{
	MY_ASSERT(m_BulletList.empty());
}

bool BulletManager::FireBullet(WORD id, class CFightObject *sender, class CFightObject *target, WORD atkType)
{
	if (0 == id || !sender)
		return false;

	if (SAST_SINGLE == atkType && !target)
		return false;

	const BulletBaseData *pBulletData = CSkillService::GetInstance().GetBulletBaseData(id);
	if (!pBulletData)
		return false;

	Bullet *pBullet = new Bullet(sender, target);

	pBullet->m_ID				= pBulletData->m_ID;
	pBullet->m_TrackType		= pBulletData->m_TrackType;
	pBullet->m_StopCondition	= pBulletData->m_StopCondition;
	pBullet->m_BltSpeed			= pBulletData->m_BltSpeed;
	pBullet->m_ActiveCount		= pBulletData->m_ActiveCount;
	pBullet->m_ActiveInteval	= pBulletData->m_ActiveInteval;

	pBullet->m_BulletIndex		= ++BulletID;
	pBullet->m_StartTime		= timeGetTime();
	pBullet->m_HitTime			= 0;
	pBullet->m_AtkType			= atkType;
	pBullet->m_CurActive		= 0;
	pBullet->m_CurPhase			= BUP_TRACK;
	
	// 设置目标点
	if (SAST_POINT_CIRCLE == pBullet->m_AtkType || SAST_POINT_SINGLE == pBullet->m_AtkType)
	{
		pBullet->m_DesX			= sender->m_AtkContext.mDefenderWorldPosX;
		pBullet->m_DesY			= sender->m_AtkContext.mDefenderWorldPosY;
	}
	else if (SAST_SINGLE == pBullet->m_AtkType)
	{
		pBullet->m_DesX			= target->m_curX;
		pBullet->m_DesY			= target->m_curY;
	}
	else
		MY_ASSERT(0);

	pBullet->m_CurX				= sender->m_curX;
	pBullet->m_CurY				= sender->m_curY;

	pair<BulletList::iterator, bool> result =  m_BulletList.insert(make_pair(pBullet->m_BulletIndex, pBullet));
	if (false == result.second)
	{
		rfalse(2, 1, "发射了一个已经存在的子弹--> ID:%d, Index:%d", pBullet->m_ID, pBullet->m_BulletIndex);
		delete pBullet;
	}

	// 设置索引
	sender->m_FiredToOther.insert(pBullet->m_BulletIndex);
	if (target) 
		target->m_FiredToMe.insert(pBullet->m_BulletIndex);

	return true;
}

bool BulletManager::StopBullet(DWORD index)
{
	BulletList::iterator it = m_BulletList.find(index);
	MY_ASSERT(it != m_BulletList.end());

	Bullet *pBullet = it->second;
	m_BulletList.erase(it);
	delete pBullet;

	return true;
}

bool BulletManager::Update()
{
	for (BulletList::iterator it = m_BulletList.begin(); it != m_BulletList.end();)
	{
		Bullet *pBullet = it->second;
		if (pBullet->Update())
		{
			it = m_BulletList.erase(it);
			delete pBullet;
		}
		else
			++it;
	}
	
	return true;
}
