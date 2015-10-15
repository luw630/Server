#pragma once

class CFightObject;
class CRegion;

enum BulletTrackType
{
	BUTT_NOTRACK,		// 不追踪
	BUTT_TRACK,			// 追踪
};

enum BulletStopCondition
{
	BUSC_NEVERSTOP,		// 追到底
	BUSC_TIMELIMIT,		// 时间限制
	BUSC_DESTTOOFAR,	// 目标太远
};

enum BulletPhase
{
	BUP_TRACK,			// 追击阶段
	BUP_HIT,			// 命中阶段
};

class BulletBaseData
{
public:
	WORD	m_ID;				// 子弹ID
	WORD	m_TrackType;		// 追踪类型
	WORD	m_StopCondition;	// 停止条件
	DWORD	m_BltSpeed;			// 子弹速度
	WORD	m_ActiveCount;		// 子弹命中后，会产生几次效果
	WORD	m_ActiveInteval;	// 每次产生效果的间隔
};

// 子弹
class Bullet : public BulletBaseData
{
	friend class BulletManager;
	friend class CFightObject;

public:
	Bullet(CFightObject *sender, CFightObject *target);
	~Bullet();

private:
	DWORD	m_BulletIndex;		// 当前子弹索引
	DWORD	m_StartTime;		// 子弹释放时间
	DWORD	m_HitTime;			// 子弹命中时间
	WORD	m_AtkType;			// 攻击类型（所属技能）
	WORD	m_CurActive;		// 当前已产生效果次数
	WORD	m_CurPhase;			// 当前处理阶段
	float	m_DesX;				// 目标点坐标
	float	m_DesY;				// 目标点坐标
	float	m_CurX;				// 当前点坐标
	float	m_CurY;				// 当前点坐标

	CFightObject *m_Sender;		// 子弹所属释放者
	CFightObject *m_Target;		// 子弹目标，如果是群攻，为空

private:
	bool Update();

private:
	bool ProcessTrack();
	bool ProcessHit();
};

// 全局子弹管理器
class BulletManager
{
	typedef std::map<DWORD, Bullet *> BulletList;

	static DWORD BulletID;

private:
	BulletManager(){}
	~BulletManager();

public:
	static BulletManager& GetInstance();

public:
	bool FireBullet(WORD id, class CFightObject *sender, class CFightObject *target, WORD atkType);
	bool StopBullet(DWORD index);
	bool Update();

private:
	BulletList m_BulletList;	// 弹夹
};
