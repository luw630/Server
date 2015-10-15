#ifndef CSKILLSERVICE_H
#define CSKILLSERVICE_H

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
#include <vector>

class SSkillBaseData;
class SXinFaData;
class BulletBaseData;

class CSkillService
{
	typedef std::hash_map<WORD, std::hash_map<WORD, SSkillBaseData*> > SkillBaseData;
	typedef std::hash_map<WORD, std::hash_map<WORD, SXinFaData*> > XinFaBaseData;
	typedef std::hash_map<DWORD, DWORD> GroupCDTime;
	typedef std::hash_map<WORD, SSkillHitSpot*> SkillHitSpot;
	typedef std::hash_map<WORD, std::vector<DWORD> > STelegrySkillMap;
	typedef std::hash_map<WORD, BulletBaseData *> SBulletBaseData;
	typedef	std::hash_map<DWORD, std::vector<WORD> > SSkillFactor;
	typedef std::hash_map<WORD, WORD> fpskillupgrade;
private:
	CSkillService() {}

public:
	static CSkillService& GetInstance()
	{
		static CSkillService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CSkillService();
	const WORD				  GetSkillIdByBookId(DWORD BookIndex) const;		//通过技能书ID获取技能的ID
	const SSkillBaseData	 *GetSkillBaseData(WORD index, WORD level = 1) const;
	const SXinFaData		 *GetXinFaBaseData(WORD index, WORD level = 1) const;
	const SSkillHitSpot		 *GetSkillHitSpot(WORD index) const;
	const std::vector<DWORD> *GetTelegrySkillMap(WORD index) const;
	const BulletBaseData	 *GetBulletBaseData(WORD index) const;
	const std::vector<WORD>	 *GetSkillFactor(DWORD index) const;

	DWORD GetGroupCDTime(DWORD groupID) const;
	WORD  _fpGetSkillupgradeChance(WORD level) const;
private:
	void CleanUp();

	// 禁止拷贝构造和拷贝赋值
	CSkillService(CSkillService &);
	CSkillService& operator=(CSkillService &);

private:
	SkillBaseData		m_baseData;				// 技能基本属性
	XinFaBaseData		m_baseXinFa;			// 心法基本属性
	GroupCDTime			m_groupCDTime;			// 组技能冷却
	SkillHitSpot		m_HitSpot;				// 打击点
	STelegrySkillMap	m_TSMap;				// 心法技能对应表
	SBulletBaseData		m_BulletData;			// 子弹配置表
	SSkillFactor		m_SkillFactor;			// 技能攻击力系数
	fpskillupgrade		m_fpskillupgrade;		//侠客技能升级几率
};

#pragma warning(pop)

#endif // CSKILLSERVICE_H