#ifndef _BULLET_MANAGER_H_
#define _BULLET_MANAGER_H_
/*
	�����ӵ���������_s��ʾ���Ǵ��ӵ������Ƿ������ģ���Ϊ�ڿͻ���Ҳ
	��һ��Bullet Manager
*/

class CBulletManager_s
{
private:
	CBulletManager_s();
	~CBulletManager_s();
public:
	// �õ������ӵ��������ĵ���
	static CBulletManager_s* GetSingleton();

	// �ڷ������Ϸ���һ���ӵ�
	bool FireBullet(DWORD hitTime,class CFightObject* attacker );

	// �����ӵ��������ĸ��º���
	void Update();

	struct SBullet_s
	{
		DWORD m_hitTime;
		class CFightObject* attacker;
	};

	// ���ӵ����е�ʱ����Ҫ�����˺���Ϣ
	void SendWoundMessage(SBullet_s* bullet);
private:
	// ��free list���ӵ�������ʱ����Ҫ���·���һЩ�ӵ��ӵ�free list��ȥ
	void AllocBullets();

	typedef std::list<SBullet_s*> SBulletList_s;
	SBulletList_s m_FreeBulletList;
	SBulletList_s m_ActiveBulletList;
};
#endif