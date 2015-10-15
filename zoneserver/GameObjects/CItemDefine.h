#ifndef CITEMDEFINE_H
#define CITEMDEFINE_H

#include "Networkmodule\ItemTypedef.h"

// ���ߵ�����
enum ItemOrbit
{
	IO_NONE = 0,			// ����������
	IO_WUYUE,				// ����
	IO_SHUSHAN,				// ��ɽ
	IO_SHAOLIN,				// ����
	IO_CIHANG,				// �Ⱥ�
	IO_RIYUE,				// ����

	IO_XIAKE,				// ����ͨ��
	IO_XA_WUYUE,				// ��������
	IO_XA_SHUSHAN,				// ������ɽ
	IO_XA_SHAOLIN,				// ��������
	IO_XA_CIHANG,				// ���ʹȺ�
	IO_XA_RIYUE,				// ��������
	
	I0_MAX,					// Max
};

// ���ߵ��Ա�����
enum ItemSex
{
	IS_NONE = 0,
	IS_MALE,				// ��
	IS_FEMALE,				// Ů
};

// ���ߵ���ɫ
enum ItemColor
{
	IC_WHITE = 1,			// ��ɫ
	IC_Green = 2,			// ��ɫ
	IC_BLUE,				// ��ɫ
	IC_PURPLE,				// ��ɫ
	IC_GOLD,				// ��ɫ(��ɫ)

	IC_MAX
};

// �;øı䷽ʽ
enum WearChangeType
{
	WCT_NONE		= 0x0,	// ���ı�
	WCT_ATTACK		= 0x1,	// ����
	WCT_BEATTACED	= 0x2,	// ������
	WCT_DIE			= 0x4,	// ����
	WCT_EQUIP		= 0x8,	// װ��
	WCT_PACKED		= 0x10,	// Я��
};

// �����������
enum BuyItemMoneyType
{
	BIMT_ONLYBIND	= 1,	// ֻʹ�ð󶨻���	����
	BIMT_ONLYUNBIND,		// ֻʹ�÷ǰ󶨻��� ����
	BIMT_BINDFIRST,			// ������ʹ�ã������ٿ۷ǰ�
};

// װ����������
enum
{
	MAX_REDUCE_EQUIP_GRADE = 9,
	MAX_EQUIP_GRADE = 10,
};

// ��װ�������Ե�����
// ��itemMsg�ļ��е� װ����װ�ӳ�ö�� ��Ӧ
enum SUIT_ATTRI_TYPE
{
	SAT_LEVEL,
	SAT_GRADE,
	SAT_COLOR,
	SAT_JEWEL,
	SAT_STAR,

	SAT_MAX,
};

// [11-12-2012 16:17 gw]+��Ʒ������ 
enum ITEM_BIND_TYPE
{
	EITEM_BIND_NONE = 0, // ������
	EITEM_BIND_GET = 0x01, //��õ�ʱ���
	EITEM_BIND_EQUIP=0x02, //װ����ʱ���
};

// ���ߵĻ�������
struct SItemBaseData
{
public:
	SItemBaseData() { memset(this, 0, sizeof(SItemBaseData)); }

public:
	char	m_Name[ITEMNAME_MAXLEN];	// ����
	DWORD	m_BuyPrice;					// ����۸�
	DWORD	m_SellPrice;				// �����۸�
	DWORD	m_LocSrvCoolingTime;		// ��ȴʱ��
	DWORD	m_LocSrvPreTime;			// ����ʱ��
	DWORD	m_ModelIDMan;				// ģ��ID���ɼ�װ���У�
	DWORD	m_ModelIDWoman;				// ģ��ID���ɼ�װ��Ů��
	DWORD	m_ID;						// ΨһID
	DWORD	m_ScriptID;					// �ű�ID
	DWORD   m_StepNum;                  // ����2011.8.9  2013.12.25 ����ΪƷ��
	WORD	m_Overlay;					// ��������
	WORD	m_Level;					// �ȼ�����
	WORD	m_MaxWear;					// ����;���
	WORD	m_WearReduceCount;			// �;øı���
	WORD	m_DieDropRate;				// �����������
	WORD	m_LocSrvCDGroupID;			// ��ȴ��
	WORD	m_TaskID;					// ����������ID
	WORD	m_TaskNeed;					// ������������
	BYTE	m_BuyMoneyType;				// �������ģʽ
	BYTE	m_SellMoneyType;			// ������������
	BYTE	m_WearChange;				// �;øı䷽ʽ
	BYTE	m_Type;						// ��������
	BYTE	m_School;					// ��������
	BYTE	m_Sex;						// �Ա�����
	BYTE	m_Color;					// ������ɫ
	bool	m_CanUse;					// �ɷ�ʹ��
	bool	m_CanDrop;					// �ɷ���
	bool	m_CanSale;					// �ɷ�����
	bool	m_CanTran;					// �ɷ���
	bool	m_CanFix;					// �ɷ�����
	bool	m_CanStroage;				// �ɷ����ֿ�
	bool	m_CanJianDing;				// �Ƿ�ɼ���
	// ===============������=====================
	bool	m_CanQuality;				// �ɷ���Ʒ��
	bool	m_CanGrade;					// �ɷ�����
	bool	m_CanRefine;				// �ɷ�����(������������)
	bool	m_CanRefresh;				// �ɷ�ˢ��(����������ֵ)
	bool	m_CanStar;					// �ɷ�����
	bool	m_CanUpdate;				// �ɷ�����
	bool	m_CanMakeHole;				// �ɷ���
	bool	m_CanTakeOff;				// �ɷ�ժ��
	bool	m_CanUnStar;				// �ɷ�ϴ��
	bool	m_CanEquipDescomposition;	// �ɷ�װ���ֽ�
	bool	m_CanEquipSmelting;			// װ������
	BYTE    m_GemType;					// ��ʯ����	

	BYTE	m_BindType;					// [11-12-2012 16:23 gw]+  ������
};

// �������Լӳ�
class SItemFactorData
{
public:
	SItemFactorData(){ memset(this, 0, sizeof(SItemFactorData)); }
	
public:
	int		m_ValueFactor[SEquipDataEx::EEA_MAX];
	int		m_PrecentFactor[SEquipDataEx::EEA_MAX];
};

// װ������������Ϣ
struct SItemUpdateGradeInfo
{
	BYTE	grade; // ����
	WORD	baseSuccessRate; // �ɹ���
	WORD	baseFailedReduceGradeRate; // ʧ��֮�󽵽׵ĸ���
	WORD	baseFailedDestroyRate; // ʧ�ܺ����ٵĸ���
	BYTE    byMoneyType;//Ǯ����
	DWORD	needMoney;	// ��Ҫ��Ǯ��Ŀ	

	DWORD   dwNeedItemID; // ��Ҫ����ID
	BYTE	byNeedItemNum; // ��Ҫ���ϵ�����

	DWORD	dwLuckyCharmID; // �������˷�ID
	BYTE	byLuckyCharmNum; // �������˷�������Ŀ
	WORD	wLuckyCharmAddSuccessRate; // ���˷���ӳɹ���
	
};

// [2012-8-29 12-42 gw: +�������Ե�����ṹ,��Ҫ����ϴ��������������]  
struct SEquipExtraAttriRequire
{
	static const int msc_iXiLianExtraAttriMax = 6; // ���ϴ����������������Ϊ6��
	// ��������
	enum TExtraAttrDataElem
	{	
		EExtraData_LowerLimit = 0,	// ����
		EExtraData_UpperLimit,		// ����
		EExtraData_Rate,			// ����
		EExtraData_JingLianAdd,		// �������ֵ

		EExtraData_Max,
	};
	int		iaExtraData[SEquipDataEx::EEA_MAX][EExtraData_Max]; // �������Ե���ϸ��Ϣ

	WORD  waExtraAttriRate[msc_iXiLianExtraAttriMax]; // �������Բ��������ĸ���

	DWORD ResetItemID;		//ϴ����Ҫ�ĵ���ID
	BYTE  ResetNeedNum;		//ϴ����Ҫ�ĵ�������
	BYTE  ResetMoneyType;	//ϴ����Ҫ��Ǯ����	
	long  ResetNeedMoney;	//ϴ����Ҫ��Ǯ	

	DWORD RefreshItemID;	//������Ҫ�ĵ���ID	
	BYTE  RefreshNeedNum;	//������Ҫ�ĵ�������
	BYTE  RefreshMoneyType;	//������Ǯ����	
	long  RefreshNeedMoney;	//������Ҫ��Ǯ	

	DWORD ExternLockedItemID; //ϴ����������ʯID
	byte  ExternLockeItemNum; // ϴ������һ��������Ҫ����ʯ������
};

// װ�����׾�����������
struct SItemUpgradeAttribute
{
	// ���׻�������������������
	static const int msc_iStarAttriDataNum = 10;
	DWORD	wID;				// ID��Ψһ��ͨ���������
	WORD	wType;				// װ�������ͣ��·������󡢿���...��
	BYTE	bLevel;				// װ���ĵȼ�
	BYTE	bColor;				// װ����Ʒ��
	WORD	wAttri[msc_iStarAttriDataNum];			// ���׵Ļ�����������
	WORD	wStarAttri[SEquipment::MAX_STAR_NUM];		// ���ǵĻ����������ֵ

	struct SEquipExtraAttriRequire equipRefineRequire;	//װ��ϴ����������������
};

// װ����Ʒ��
struct SItemUpdateQuality
{
	BYTE	bColor;
	long	dwNeedMoney;
	BYTE    m_Type;
	WORD	rate[IC_MAX - 1];
	BYTE	bStoneNum;
	DWORD   NeedID;
	long    m_DestroyRate;
};

// װ������
struct SEquipLevelUp
{
	BYTE	level;
	BYTE	StoneNum;
	WORD	SuccessRate;
	DWORD	NeedMoney;
	BYTE    m_Type;
	DWORD   NeedID;
	long    m_DestroyRate;
};

// װ������
struct SEquipStar
{
	BYTE	num;
	DWORD	Rate;
	BYTE	StarMoneyType;
	long	money;
	DWORD   StarNeedID;
	BYTE    StarNum;
	BYTE	ReMoveStarMoneyType;
	long	ReMovemoney;
	DWORD   ReMoveStarNeedID;
	BYTE    RemoveStarNum;

	byte	bNeedReduceStarFailed; //ʧ�ܺ��Ƿ���Ҫ����
	DWORD	dwProtectionCharacterID; // ���ǿ��ñ�����ID
	byte	byProtectionCharacterNum; // ���ñ������۳�����
	DWORD	dwLuckyCharmID; // ���ǿ������˷�ID
	byte	byLuckyCharmNum; // �������˷��۳�����
	WORD	wLuckyCharmAddSuccessRate; //���˷����ӵĳɹ���
};

// װ�������������ֵ
struct SMaxExtraAttri
{
	DWORD	index;
	WORD	level;
	BYTE	color;
	BYTE	grade;

	WORD	ExtraData[SEquipDataEx::EEA_MAX];
};

struct SJewelAttribute
{
	DWORD	index;
	//BYTE	type;
	int		ExtraData[SEquipDataEx::EEA_MAX];
};

// ��Ԫ�������������ñ�
struct SHuiYuanDanInfo
{
	WORD	wLevel;
	WORD	wSPValue;
	WORD	wSuccessRate;
	WORD	wLianzhiRate;
	WORD	wXMTJRate;
};
struct SMakeHoleInfo
{
	//��ױ��ID	�����Ҫ�Ĳ���ID	���Ĳ�������	���Ǯ����	���Ǯ����	���ʧ�ܼ���
	DWORD   Index;			//��ױ��ID	
	DWORD	HoleNeedID;		//�����Ҫ�Ĳ���ID	
	DWORD   HoleNum;		//���Ĳ�������	
	BYTE    HoleMoneyType;	//���Ǯ����
	long    HoleMoney;		//���Ǯ����
	WORD    wMakeSuccessRate;	//��׳ɹ�����

	DWORD	dwProtectionCharacterID; // ���ǿ��ñ�����ID
	byte	byProtectionCharacterNum; // ���ñ������۳�����
	DWORD	dwLuckyCharmID; // ���ǿ������˷�ID
	byte	byLuckyCharmNum; // �������˷��۳�����
	WORD	wLuckyCharmAddSuccessRate; //���˷����ӵĳɹ���
	WORD	wReduceHoleRate; // ʧ�ܺ󽵿׵ĸ���
};
struct SInsertGem
{
	DWORD   Index;			//������ID	
	BYTE    GemMoneyType;	//��Ƕ���õ�Ǯ����	
	long    GemMoney;		//��Ƕ���õ�Ǯ
};
struct STakeOffGem
{
	DWORD Index;				//ժ������ID			
	BYTE  TakeOffGemMoneyType;	//ժ��Ǯ����
	long  TakeOffGemMoney;		//ժ������
	DWORD TakeOffGemItemID;		//ժ������ID
	BYTE  TakeOffGemNum;		//ժ����������
	long  TakeOffGemHoleRale;	//ժ����ʯ�ⶴ����
};
struct SDefineReset
{
	BYTE	m_Color;					// ������ɫ
	short   m_Rate[20];					// ���ּ���
};

// װ���ֽ�����ô洢Ԫ��
struct SEquipDecomposition
{
	DWORD	dwIndex;			//װ���ֽ�����
	byte	byColor;			// װ��Ʒ��
	byte	byStarNum;			// װ��ǿ������
	byte	byMoneyType;		// ��Ҫ��Ǯ����
	long	lNeedMoney;			// ��Ҫ���ٽ�Ǯ
	DWORD	dwGenNormalItemID;	// ������ͨ����ID
	byte	byGenNormalItemNum; // ������ͨ��������
	WORD	wGenNormalItemRate; // ������ͨ���ϵļ���	
	DWORD	dwGenSpecialItemID;	// �����������ID
	byte	byGenSpecialItemNum; // ���������������
	WORD	wGenSpecialItemRate; // ����������ϵļ���

};

// װ������
struct SEquipSmelting
{
	//DWORD	dwIndex;				// װ������
	byte	byColor;				// װ��Ʒ��
	byte	byMoneyType;			// ��Ҫ��Ǯ����
	long	lNeedMoney;				// ��Ҫ���ٽ�Ǯ
	DWORD	dwGenNormalItemID;		// ������ͨ����ID
	byte	byGenNormalItemNum;		// ������ͨ��������
	byte	byGenNormalItemColor;	// �������ϵ�Ʒ��
	byte	byAttriFactor;			// ����ϵ������ʱ��Ҫ/100��
	byte	byStarNum;			// װ��ǿ������
};

// ��ƷԤ��������������
struct SEquipBestPreviewExtraAttri
{
	byte	byEquipType;		// װ������
	byte	byExtraAttriType;	// ��Ʒ������������
};

// �鸽�����������
struct SEquipSpiritAttachBodyAttr
{
	WORD	wSuitEquipID; // ��װID
	DWORD	dwNeedItemID; // ��������ID��Ԫ�أ�
	byte	byNeedItemNum; // �����������
	byte	byMoneyType; // �����Ǯ����
	long	lNeedMoney;	// ��Ҫ���ٽ�Ǯ
	WORD	wLevelLimit; // �ȼ����ƣ�������Ҫ�ƶ���lua�ļ��ȽϺ��ʣ�
};

#endif // CITEMDEFINE_H