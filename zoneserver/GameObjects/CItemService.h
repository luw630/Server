#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

struct SItemBaseData;
class SItemFactorData;

struct SItemUpdateGradeInfo;
struct SItemUpgradeAttribute;
struct SItemUpdateQuality;	// װ����Ʒ��
struct SEquipLevelUp;		// װ������
struct SEquipStar;			// װ������
struct SMaxExtraAttri;		// �����������ֵ
struct SJewelAttribute;		// ��ʯ����
struct SHuiYuanDanInfo;		// ��Ԫ�������������ñ�
struct SMakeHoleInfo;		// ��׻���
struct SInsertGem;			// ��Ƕ����
struct STakeOffGem;			// �ⶴ����
struct SDefineReset;
struct SEquipDecomposition; // װ���ֽ�
struct SEquipSmelting;		// װ������
struct SEquipBestPreviewExtraAttri; // װ����ƷԤ����������
struct SEquipSpiritAttachBodyAttr; // �鸽����
// �ṩ�Ե���������ݵķ���
// 1. ���߻������Ա�
// 2. ���߼ӳ����Ա�
// 3. ������CD��

struct SEquipExtraAttriTable
{
	DWORD EquipId;
	WORD  MinNum;
	WORD  MaxNum;
	int	  ExtraData[SEquipDataEx::EEA_MAX][3];
	DWORD ResetItemID;		//��Ҫ�����õ���ID
	BYTE  ResetNeedNum;		//������Ҫ������
	BYTE  ResetMoneyType;	//��Ҫ��Ǯ����	
	long  ResetNeedMoney;	//������Ҫ��Ǯ	
	DWORD RefreshItemID;	//��Ҫ��ˢ�µ���ID	
	BYTE  RefreshNeedNum;	//ˢ����Ҫ������
	BYTE  RefreshMoneyType;	//ˢ�µ�Ǯ����	
	long  RefreshNeedMoney;	//ˢ����Ҫ��Ǯ	
	DWORD ExternLockedItemID; //��������ʯID
	byte  ExternLockeItemNum; // ����һ��������Ҫ����ʯ������
};



class CItemService
{	
	typedef std::hash_map<DWORD, SEquipExtraAttriTable* > EquipExtraAttriTable;
	typedef std::hash_map<DWORD, SItemBaseData*  > ItemBaseData;
	typedef std::hash_map<DWORD, SItemFactorData*> ItemFactorData;
	typedef std::hash_map<DWORD, DWORD> ItemGroupCD;
	typedef std::hash_map<BYTE, SItemUpdateGradeInfo*> ItemUpdateGradeTable;
	typedef std::hash_map<DWORD, SItemUpgradeAttribute*> ItemUpgradeAttribute;
	typedef std::hash_map<BYTE, SItemUpdateQuality*> ItemUpdateQuality;	// ��Ʒ��
	typedef	std::hash_map<BYTE,	SEquipLevelUp*>	EquipLevelUp;			// ����
	typedef std::hash_map<BYTE, SEquipStar*> EquipStar;					// װ������
	typedef std::hash_map<DWORD, SMaxExtraAttri*> MaxExtraAttri;		// װ������������ֵ
	typedef std::hash_map<DWORD, SJewelAttribute*> JewelAttribute;		// ��ʯ����ֵ
	typedef	std::hash_map<DWORD, SHuiYuanDanInfo*> HuiYuanDanInfo;		// ��Ԫ���������ñ�
	typedef std::hash_map<DWORD,  SMakeHoleInfo>	   MakeHoleInfo;	
	typedef std::hash_map<DWORD, SInsertGem>		   SInsertGemInfo;
	typedef std::hash_map<DWORD, STakeOffGem>		   STakeOffGemInfo;
	typedef std::hash_map<BYTE,  SDefineReset>		   SDefineResetInfo;
	typedef std::hash_map<DWORD,  SEquipDecomposition*>	   SEquipDecompositionInfo;
	typedef std::hash_map<BYTE, SEquipSmelting*>	SEquipSmeltingInfo;
	typedef std::hash_map<BYTE, SEquipBestPreviewExtraAttri*> SEquipBestPreviewExtraAttriInfo;
	typedef std::hash_map<WORD, SEquipSpiritAttachBodyAttr*> SEquipSpiritAttachBodyAttrInfo;

private:
	CItemService();

public:
	static CItemService& GetInstance()
	{
		static CItemService instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	~CItemService();

	DWORD GetItemGroupCDTime(DWORD group) const;

	const SItemBaseData *GetItemBaseData(DWORD index) const;
	const SItemFactorData *GetItemFactorData(DWORD index) const;

	//const SEquipExtraAttriTable *GetExtraAttriTable(DWORD index) const;		// װ����������
	const SItemUpdateGradeInfo *GetUpgradeInfo(BYTE grade) const;			// װ��������Ϣ
	const SItemUpgradeAttribute *GetUpgradeAttribute(const struct SItemBaseData *pItemData) const; // װ�����׸�������
	const SItemUpgradeAttribute *GetUpgradeAttribute(DWORD ) const;			// װ�����׸�������
	const SItemUpdateQuality *GetUpdateQuality(BYTE color) const;			// װ����Ʒ��
	const SEquipLevelUp	*GetLevelUpData(BYTE level)	const;					// װ������
	const SEquipStar *GetEquipStarData(BYTE num) const;						// װ������
	const SMaxExtraAttri *GetMaxExtraAttri(DWORD index) const;				// װ���������������ñ�	
	const SMaxExtraAttri *GetMaxExtraAttri(const struct SItemBaseData *pItemData, byte byCurGrade) const; // ��ȡ���׵ĸ����������޵Ķ�ȡ
	const SJewelAttribute *GetJewelAttribute(DWORD index) const;			// ��ʯ�ĸ�������
	const SHuiYuanDanInfo *GetHuiYuanDanInfo(DWORD index) const;			// ��Ԫ������������
	const SMakeHoleInfo *GetMakeHoleInfo(DWORD index) const;
	const SInsertGem *GetInsertGemInfo(DWORD index) const;
	const STakeOffGem *GetSTakeOffGemInfo(DWORD index) const;	
	const SEquipDecomposition *GetSEquipDecompositionInfo(const struct SItemBaseData *pItemData, byte byStarNum) const; // ��ȡװ���ķֽ�����
	const SEquipSmelting *GetSEquipSmeltingInfo(byte byColor) const; // ��ȡװ��������������	
	const SEquipBestPreviewExtraAttri *GetSEquipBestPreviewExtraAttri(byte byEquipType) const; // ��ȡװ����ƷԤ������������Ϣ

	WORD GetSuitEquipIDbyScrollID(DWORD dwScrollID) const; // ͨ������ID���Ҷ�Ӧ����װID
	const SEquipSpiritAttachBodyAttr *GetSEquipSpiritAttachBodyAttri(WORD wSuitEquipID) const;// ��ȡ�鸽��װ��Ϣ

	bool LoadExtraAttri();
	long GetRandomNum(BYTE Color,long RandomNum);
	// ������Ϣ�ṹ���ͻ���
	BOOL SendItemInfo(DNID dnidClient);

	bool GetFactorData(DWORD item, WORD &attr,WORD &nvalue,WORD nIndex);
private:
	// ���õ������Լӳ�
	void SetFactorValue(DWORD item, WORD attri, int value, bool IsPre);
	
	// ��ֹ��������Ϳ�����ֵ
	CItemService(CItemService &);
	CItemService& operator=(CItemService &);

private:
	ItemBaseData	m_baseData;			// ���ߵĻ�������
	ItemFactorData	m_factorData;		// ���ߵļӳ�����
	ItemGroupCD		m_groupCD;			// ��CD
	EquipExtraAttriTable	m_EATable;
	ItemUpdateGradeTable	m_gradeTalbe;		// װ��������Ϣ
	ItemUpgradeAttribute	m_upgradeAttri;		// װ�����׵ĸ�������
	ItemUpdateQuality		m_updateQuality;	// װ����Ʒ��
	EquipLevelUp			m_EquipLevelUp;		// װ������
	EquipStar				m_EquipStar;		// װ������

	MaxExtraAttri			m_MaxAttri;			// ����������
	JewelAttribute			m_JewAttri;			// ��ʯ����
	HuiYuanDanInfo			m_HYDInfo;			// ��Ԫ����Ϣ
	MakeHoleInfo			m_MakeHoleInfo;
	SInsertGemInfo			m_SInsertGemInfo;
	STakeOffGemInfo         m_STakeOffGemInfo;
	SDefineResetInfo        m_SDefineResetInfo; // 
	SEquipDecompositionInfo m_SEquipDecompositionInfo; // װ���ֽ�����
	SEquipSmeltingInfo		m_SEquipSmeltingInfo; // װ����������
	SEquipBestPreviewExtraAttriInfo m_SEquipBestPreviewExtraAttri; // װ����ƷԤ��������������
	SEquipSpiritAttachBodyAttrInfo	m_SEquipSpiritAttachBodyAttri; // �鸽��صĴ���
};

#pragma warning(pop)