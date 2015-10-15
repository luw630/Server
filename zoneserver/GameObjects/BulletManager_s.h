#ifndef _BULLET_MANAGER_H_
#define _BULLET_MANAGER_H_
/*
	技能子弹管理器，_s表示的是此子弹管理是服务器的，因为在客户端也
	有一个Bullet Manager
*/

class CBulletManager_s
{
private:
	CBulletManager_s();
	~CBulletManager_s();
public:
	// 得到技能子弹管理器的单件
	static CBulletManager_s* GetSingleton();

	// 在服务器上发射一颗子弹
	bool FireBullet(DWORD hitTime,class CFightObject* attacker );

	// 技能子弹管理器的更新函数
	void Update();

	struct SBullet_s
	{
		DWORD m_hitTime;
		class CFightObject* attacker;
	};

	// 当子弹命中的时候需要发送伤害消息
	void SendWoundMessage(SBullet_s* bullet);
private:
	// 当free list的子弹不够的时候需要重新分配一些子弹加到free list中去
	void AllocBullets();

	typedef std::list<SBullet_s*> SBulletList_s;
	SBulletList_s m_FreeBulletList;
	SBulletList_s m_ActiveBulletList;
};
#endif