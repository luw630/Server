#pragma once

class CFightObject;
class CRegion;

enum BulletTrackType
{
	BUTT_NOTRACK,		// ��׷��
	BUTT_TRACK,			// ׷��
};

enum BulletStopCondition
{
	BUSC_NEVERSTOP,		// ׷����
	BUSC_TIMELIMIT,		// ʱ������
	BUSC_DESTTOOFAR,	// Ŀ��̫Զ
};

enum BulletPhase
{
	BUP_TRACK,			// ׷���׶�
	BUP_HIT,			// ���н׶�
};

class BulletBaseData
{
public:
	WORD	m_ID;				// �ӵ�ID
	WORD	m_TrackType;		// ׷������
	WORD	m_StopCondition;	// ֹͣ����
	DWORD	m_BltSpeed;			// �ӵ��ٶ�
	WORD	m_ActiveCount;		// �ӵ����к󣬻��������Ч��
	WORD	m_ActiveInteval;	// ÿ�β���Ч���ļ��
};

// �ӵ�
class Bullet : public BulletBaseData
{
	friend class BulletManager;
	friend class CFightObject;

public:
	Bullet(CFightObject *sender, CFightObject *target);
	~Bullet();

private:
	DWORD	m_BulletIndex;		// ��ǰ�ӵ�����
	DWORD	m_StartTime;		// �ӵ��ͷ�ʱ��
	DWORD	m_HitTime;			// �ӵ�����ʱ��
	WORD	m_AtkType;			// �������ͣ��������ܣ�
	WORD	m_CurActive;		// ��ǰ�Ѳ���Ч������
	WORD	m_CurPhase;			// ��ǰ����׶�
	float	m_DesX;				// Ŀ�������
	float	m_DesY;				// Ŀ�������
	float	m_CurX;				// ��ǰ������
	float	m_CurY;				// ��ǰ������

	CFightObject *m_Sender;		// �ӵ������ͷ���
	CFightObject *m_Target;		// �ӵ�Ŀ�꣬�����Ⱥ����Ϊ��

private:
	bool Update();

private:
	bool ProcessTrack();
	bool ProcessHit();
};

// ȫ���ӵ�������
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
	BulletList m_BulletList;	// ����
};
