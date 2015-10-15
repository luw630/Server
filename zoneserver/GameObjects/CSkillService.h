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
	const WORD				  GetSkillIdByBookId(DWORD BookIndex) const;		//ͨ��������ID��ȡ���ܵ�ID
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

	// ��ֹ��������Ϳ�����ֵ
	CSkillService(CSkillService &);
	CSkillService& operator=(CSkillService &);

private:
	SkillBaseData		m_baseData;				// ���ܻ�������
	XinFaBaseData		m_baseXinFa;			// �ķ���������
	GroupCDTime			m_groupCDTime;			// �鼼����ȴ
	SkillHitSpot		m_HitSpot;				// �����
	STelegrySkillMap	m_TSMap;				// �ķ����ܶ�Ӧ��
	SBulletBaseData		m_BulletData;			// �ӵ����ñ�
	SSkillFactor		m_SkillFactor;			// ���ܹ�����ϵ��
	fpskillupgrade		m_fpskillupgrade;		//���ͼ�����������
};

#pragma warning(pop)

#endif // CSKILLSERVICE_H