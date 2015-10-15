#ifndef _ATTACK_AND_DEFENSE_H_
#define _ATTACK_AND_DEFENSE_H_
/*
	角色升级表。
	这个表用来存放角色不同等级下的数据。属于只读数据
*/
class CRoleLevelUpTbl
{
private:
	CRoleLevelUpTbl();
	~CRoleLevelUpTbl();
public:
	struct SDataTbl 
	{
		// 血量值
		INT32 m_Blood;		
		// 内力值
		INT32 m_InnerForce;		
		// 真气值
		INT32 m_VitalForce;	
		// 杀气值
		INT32 m_KillForce;	
		// 外攻基础 内攻基础 外防基础 内防基础
//		INT32 m_wAttackDefenseBase[ SPlayerXiaYiShiJie::ATTACK_DEFENSE_MAX ];
		// 身法值
		INT32 m_BodyWork ;
		//冲穴点
		INT32 m_VenapointCount;
		// 攻击点 防御点
		INT32 m_wAddPoints[ 2 ];
		// 升级所需要的经验
		INT32 m_MaxExp;
	};

public:
	// 得到角色升级表的单件
	static CRoleLevelUpTbl* GetSingleton();

	// 通过配置文件载入角色升级表
	bool LoadRoleLevelUpTbl();

	// 取得角色最大等级
	BYTE GetMaxLevel() { return (BYTE)m_totalLevel; }

	// 通过传入level得到数据
	inline SDataTbl* GetPlayerDataByLevel(int level)
	{
        if (m_dataTbl == NULL )
            return NULL;
		if ((DWORD)level > m_totalLevel || level < 1)
			return &m_dataTbl[m_totalLevel - 1]; // 保证在等级越界后也能够取到值
		return &m_dataTbl[level - 1];
	}
private:
	// 升级表数据
	SDataTbl* m_dataTbl;
	// 总的等级
	DWORD m_totalLevel;
};

#endif