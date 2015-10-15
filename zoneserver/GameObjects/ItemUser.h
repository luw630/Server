#pragma once

#include "fightobject.h"
#include "randomsys.h"
#include "Networkmodule\ItemTypedef.h"
#include "EventManager.h"
#include "EventTimer.h"
#include "SingleItem.h"
const __int32 IID_ITEMUSER = 0x117ca49d;

class CNpc;

// �����3����CELL�ṹ
struct SItemWithFlag
{
	enum
	{
		IWF_ACTIVED		= 0x0001,		// �˸����Ƿ񱻼���
		IWF_LOCKED		= 0x0002,		// �˸����Ƿ�����
		IWF_ITEMIN		= 0x0004,		// �˸����Ƿ��е���
	};

	WORD			m_State;			// ����״̬
	SPackageItem	m_Item;				// ��������
};

// �Ѵ�װ��������״̬
struct SEquipedWithFlag
{
	WORD m_State;	// �Ѵ�װ��������״̬��ʹ��SItemWithFlag�е�ö��״̬
};

// �����3��������
enum XYD3PackageType
{
	XYD_PT_BASE = 1,		// ����+��չ1+��չ2+��չn...
	//XYD_PT_TASK,			// ����

	XYD_PT_MAX,				// Max
};

// �����3���ҵ��߷���
enum XYD3FindItemType
{
	XYD_FT_ONLYLOCK = 1,	// ֻ������
	XYD_FT_ONLYUNLOCK,		// ֻ�ҷ�����
	XYD_FT_WHATEVER,		// ���
};

// �����3����ʹ�ù���
enum XYD3MoneyUseRule
{
	XYD_UM_ONLYUNBIND = 1,	// XYD3--->����  �ǰ� �����
	XYD_UM_ONLYBIND,		// XYD3--->����  �� ��Ϸ��
	//XYD_UM_ONLYUNBIND,		

	XYD_UM_ZENGBAO,			// ���� = XYD3--->��ȯ
	XYD_UM_YUANBAO,			// Ԫ�� = XYD3--->Ԫ��

	XYD_UM_BINDFIRST,		// ������ʹ��/���ӷǰ�����ɼ��뵽����
};

// �ֽ���
const WORD BaseStart	= 0;
const WORD BaseEnd		= 72/*4 * (PackageWidth * PackageHight)*/;
const WORD BaseActivedPackage = 11/* PackageWidth * PackageHight*/; //��ʼ������ı���11�������ǵ�һ�ν���Ϸ���͵ı���������

// ������Ӳ���
struct SAddItemInfo
{
	SAddItemInfo() : m_Index(0), m_Count(0) {}
	SAddItemInfo(DWORD index, WORD count) : m_Index(index), m_Count(count) {}

	DWORD	m_Index;
	WORD	m_Count;
};

class CItemUser : public CFightObject
{ 
public:
    BOOL AutoUseItem(WORD wItemID, DWORD useType = 0);
    BOOL AutoUseItem(SPackageItem *pItem, DWORD useType = 0);
	BOOL CheckItemChange();

public:
	static BYTE GetSItemBaseBindState(BYTE eCurBindType, BYTE byItemDataBindType); // [11-12-2012 17:16 gw]+��ȡ��Ʒ��״̬ 	
	static BOOL GenerateNewItem(SRawItemBuffer &itemBuffer, const SAddItemInfo &params,BYTE index = 0);
	// ������װ�����������
	static BOOL AddRandomAttri(SEquipment* equip);	
	static BOOL ResetRandomExtraAttriEntry(SEquipment* equip, const vector<int> &vecLockedIndex);// [2012-8-23 18-06 gw: +��������������Ե���Ŀ�������Ѿ���������Ŀ������ ������]
	static BOOL OnUpdataGrade(SEquipment * pEquip);
	virtual BOOL _UseItem(SPackageItem *pItem,DWORD useType=0);
	BOOL SetPreTime(SPackageItem *pItem, DWORD in_dwLocSrvPreTime = 0, DWORD in_dwLocSrvBreakOdds = 0);
	//BOOL BreakOperation();
	CNpc *GetCurrentNpc(void);
	BOOL BuyItem(DWORD index, WORD count);
	WORD GetBlankCell(BYTE type);
	WORD GetRemainOverlap(DWORD index);
	WORD GetNeedBlank(const struct SItemBaseData *pData, const SAddItemInfo &Info);
	BOOL CanAddItems(std::list<SAddItemInfo> &itemList, WORD reuseBaseBlank = 0, WORD reuseTaskBlank = 0);
	WORD TryPossibleMerge(DWORD index, WORD count, WORD maxOverlap);
	BOOL AddExistingItem(const SItemBase &item, WORD pos, BOOL synchro);
	BOOL FillItemPos(WORD pos, BYTE type, bool mark);	
	SPackageItem *FindItemByPos(WORD pos, WORD LookType);
	//add by ly 2014/3/25
	SPackageItem *FindItemByItemID(DWORD ItemID, WORD LookType = XYD_FT_WHATEVER);	//������Ӧ�ĵ���ID�Ƿ��ڱ�����,����ڷ��ض�Ӧ����Ϣ
	void SendItemSynMsg(SPackageItem *pItem, WORD LookType = XYD_FT_ONLYUNLOCK);
	BOOL DelItem(const SPackageItem &pItem, LPCSTR info = NULL, bool autoUnLock = false);
	WORD FindBlankPos(BYTE type);
	BOOL StartAddItems(std::list<SAddItemInfo> &itemList);
	int AddItem(SAddItemInfo &siteminfo); //���ӵ������ߣ�����λ��
	void SetPlayerInitItems(CPlayer *pPlayer, bool OnlyInit);
	BOOL RecvAddItem(DWORD wIndex, WORD iNum, WORD level = 1);
	BOOL IsItemPosValid(WORD wCellPos, BOOL canBeLock = false, BOOL canBeEmpty = false);
	BOOL CanEquipIt(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData);
	BOOL CanEquipItFightPet(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData,BYTE index);//�ܷ�Ϊ����װ��
	BOOL CanUseIt(CPlayer *pPlayer, SPackageItem *pIB, const struct SItemBaseData *pData);
	BOOL _EquipItem(SPackageItem *pItem);

	BOOL _UseBookUpSkill(SPackageItem *pItem);		//ʹ�ü�������������
	BOOL _UseFightPetItem(SPackageItem *pItem,BYTE index);		//ʹ���Ѿ����ɵ����͵���
	BOOL _EquipFightPetItem(SPackageItem *pItem,BYTE index);				// װ�����͵���
	BOOL PickOneItem(SRawItemBuffer &goundItem);							// ʰȡһ���������Ʒ
	BOOL PickSingleItem(CSingleItem &SingleItem);							// ʰȡһ���������Ʒ
	BOOL DelEquipment(BYTE equipID);			                            // ɾ��һ��װ��
	BOOL _fpDelEquipment(BYTE equipID);			                            // ��������ɾ��һ��װ��
	BOOL InPackage(WORD wCellPos1, WORD wCellPos2);				// �������ƶ�
	BOOL ToPackage(BYTE byPos,BOOL onlydel = FALSE);									// �϶�жװ,�Ƿ��ɾ��
	BOOL _fpToPackage(BYTE byPos,BYTE index);								// ����ж��װ��
	BOOL ToEquip(WORD wCellPos);										// װ��
	BOOL _fpToEquip(WORD wCellPos,BYTE index);							// ����װ��
	BOOL SynItemFlag(SPackageItem *item);									// ͬ�����ߵ�����
	BOOL LockBatchCells(std::vector<WORD> &vecCell, bool lock, bool sync = true);// +Func: �������߽���һ������
	BOOL LockItemCell(WORD wCellPos, bool lock, bool sync = true);		// ����/����һ������
	BOOL CheckGoods(DWORD index, DWORD number, BOOL checkOnly);				// �������û�б������ĵ��ߣ���
	int InsertItem(DWORD index, WORD willCreate, const SItemBaseData *pData, SRawItemBuffer &item, bool genNew);
	void ChangeOverlap(SPackageItem *pItem, WORD delta, bool Add, bool Sync = true);
	DWORD GetItemNum(DWORD wIndex, WORD GetType); //ȡ�õ�������
	DWORD GetAllItemNum(WORD GetType= XYD_FT_ONLYUNLOCK); //ȡ�ñ��������е�������,�ظ��ĵ�����1��
	DWORD GetRadomItem(WORD GetType = XYD_FT_ONLYUNLOCK); //ȡ�ñ����ڵ�һ����Ч�ĵ���
	int GetRadomItemPos(WORD GetType = XYD_FT_ONLYUNLOCK); //ȡ�ñ����ڵ�һ����Ч�ĵ���
	bool ValidationPos(WORD pos);
	BOOL LockItemCellTwo(WORD wCellPos, bool lock, bool sync  = true );
	//����������½ӿ�
	long IsHaveSpaceCell();
	long CreateNewItemInfo(long index,long Flag,SPackageItem &Item,long FLAGX);
	void InSertBlessItem(const SItemBase &item, WORD pos);
	void OnAllNotice();
	// �������������
	void UpdateItemRelatedTaskFlag(class CPlayer *pPlayer,	DWORD index);
	void UpdateItemTaskFlag(class CPlayer *pPlayer,			DWORD index);
	void UpdateUseItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateGiveItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateGetItemTaskFlag(class CPlayer *pPlayer,		DWORD index);
	void UpdateMapTaskFlag();

	// ��Ϣ������
	BOOL RecvUnEquipItem(struct SQUnEquipItemMsg *pMsg);					// �ͻ���Ҫ��жװ
	BOOL RecvUnfpEquipItem(struct SQfpUnEquipItemMsg *pMsg);				// �ͻ���Ҫ��ж������װ��
	BOOL RecvPickupItem(struct SQPickupItemMsg* pMsg);						// �ͻ���Ҫ��ӵ��ϼ���һ������
	BOOL OnPickupItem(CSingleItem *pItem);	
	BOOL RecvDropItem(struct SQDropItemMsg* pMsg);							// �ͻ���Ҫ��ӵ������ӵ�һ������
	BOOL RecvUseItem(struct SQUseItemMsg* pMsg);							// �ͻ���Ҫ��ʹ��һ������
	BOOL RecvSpecialUseItem(struct SQSpecialUseItemMsg* pMsg);				// �ͻ���Ҫ��װ�����͵���
	BOOL RecvMoveItem(struct SQMoveItemMsg* pMsg);							// �ͻ���Ҫ������Ʒ�����ƶ�����
	BOOL RecvBuyItem(struct SBuy* pMsg);									// ���߽���
	BOOL RecvActiveExtPackage(struct SQActiveExtPackage *msg);				// �յ����������Ϣ
	BOOL RecvSplitItem(struct QuerySplitItemMsg *pMsg);						// �ֿ�����
	BOOL RecvOpenBuyBackList();												// �򿪻ع��б�
	BOOL RecvQuestFollowItem(struct SQQuestFollowItemMsg *pMsg);			// �����̵�
	BOOL RecvBuyFollowItem(struct SQBUYFollowItemMsg *pMsg);				// �����������
	BOOL RecvBlessExcellenceData(struct SQBlessExcellenceDataMsg *pMsg);
	BOOL RecvBlessBlessReQuest(struct SQBlessItemReQuestMsg *pMsg);
	BOOL RecvBlessQuestAddItems(struct SQBlessQuestAddItems* pMsg);
	BOOL RecvChannelCallSellInfoQuest(struct SQChannelCallSellInfo* pMsg);
	// װ���޸�
	BOOL RecvFixOneItem(struct SQEquipFixOne *pMsg);
	BOOL RecvFixAllItem(struct SQEquipFixAll *pMsg);

	// װ���޸����ڲ�ʹ�ú���
	SEquipment* GetEquipByPos(BYTE type, BYTE pos);

	BOOL StartPickSingleOne(CPlayer *pPlayer, class CItem *pItem, WORD index);

	// CD����
	void StartItemCDTimer(const SItemBaseData *pItemData);
	int CheckItemCDTime(const SItemBaseData *pItemData);

	BOOL RecvForgeItem(struct SQStartForge *pMsg);							// �յ�װ���������Ϣ
	BOOL RecvDragMetrail(struct SQDragMetrial *pMsg);						// �յ����������Ϣ
	BOOL RecvEndForgeItem(struct SQEndForge *pMsg);							// �յ�ȡ��������Ϣ
	BOOL RecvMakeHole(struct SQMakeHole *pMsg);								// �յ������Ϣ
	BOOL RecvInLayJewel(struct SQInLayJewel *pMsg);							// �յ���Ƕ��Ϣ
	BOOL RecvTakeoffJewel(struct SQTakeOffJewel *pMsg);						// �յ�ȡ��ʯ��Ϣ
	BOOL StartMakeHole();													// ��ʼ���

	// ===================װ��ǿ��===================ahj	
	// ��ȡһ��λ�� �ֲ����,�����ǰ�����������װ����
	enum TItemPosType
	{
		EPackage_PosType = 0,  // ����
		EEquipColumn_PosType, // װ����
		EError_PosType, // �����λ��
	};
	static const WORD sciEquipColumnBaseLine = 1000; // װ������λ�÷ָ���
	static TItemPosType GetItemPosType(WORD wEquipPos);
	SEquipment * GetEquipmentbyItem(WORD wEquipPos, WORD LookType = XYD_FT_WHATEVER); // ͨ��λ�û�ȡװ��(�����ǴӰ����л�ȡ��Ҳ�����Ǵ�װ���������ȡ)	
	void CheckAndUpdateEquipColumnAttribyPos(CPlayer *pPlayer, WORD wEquipPos); // ͨ��ͨ��λ�ü�Ⲣ�Ҹ���װ����������	

	WORD GetEquipColumnPos(WORD wEquipPos, bool bAddEquipColumnBaseLine); // ��ȡ��������λ�ã�ע���Ƿ���Ҫ���װ������׼��		
	BOOL IsLockedEquipColumnCell(WORD wEquipPos); // ���װ�����Ƿ�������		
	BOOL LockEquipColumnCell(WORD wEquipPos, bool lock, bool sync);// �����������װ�����ĸ���
	// ����
	BOOL RecvDragStrengthenEquip(struct SQAddEquip* pMsg);					// ����װ��
	BOOL RecvDragStrengthenMetrial(struct SQAddMetrial *pMsg);				// �������

	BOOL RecvBeginRefineGrade(struct SQBeginRefineGrade *pMsg);				// ����
	BOOL RecvBeginRefineStar(struct SQBeginRefineStar *pMsg);				// ����
	BOOL RecvBeginReMoveStar(struct SQBeginReMoveStar *pMsg);				// ϴ��	
	BOOL RecvBeginUpgradeQuality(struct SQStartQuality *pMsg);				// ��Ʒ��
	

	BOOL RecvBeginDecomposition(struct SQBeginIdentifyMsg* pMsg);			// װ���ֽ�
	BOOL RecvBeginShengJie(struct SQBeginIdentifyMsg* pMsg);				// װ�����ף������������Ե����ޣ�
	BOOL RecvBeginJingLian(struct SQBeginIdentifyMsg *pMsg);				// װ������������װ���ĸ�������ֵ��			
	BOOL RecvBeginPolished(struct SQBeginIdentifyMsg *pMsg);				// װ��ϴ�������ɸ���������Ŀ�����û�������ĸ�������ֵ��
	BOOL RecvBeginSmelting(struct SQBeginIdentifyMsg* pMsg);				// װ������	
	BOOL RecvBeginSpiritAttachBody(struct SQBeginIdentifyMsg* pMsg);		// װ���鸽
		
	BOOL RecvIdentifyExchange(struct SQIdentifyExchangeMsg *pMsg);// [2012-8-24 10-11 gw: +�յ�ϴ����Ľ�����Ϣ]
	
	BOOL RecvEndRefine(struct SQEndRefineMsg *pMsg);						// ����ǿ��

	// ���������ڲ�����	
	BOOL GetUpgradeResult(SEquipment* pE, BYTE &result,bool bHaveLuckyCharmItem);

	// ȡ��ϴ��
	BOOL RecvEndResetPoint(struct SQEndResetPoint* pMsg);

	// ����������Ŀǰֻ���л�Ԫ����
	BOOL RecvDragUpdateItem(struct SQDragUpdateItemMsg *pMsg);
	BOOL RecvDragUpdateMetrial(struct SQDragUpdateMetrialMsg *pMsg);
	BOOL RecvBeginUpdateItem(struct SQBeginUpdateItem *pMsg);
	BOOL RecvEndUpdateItem(struct SQEndUpdateItem *pMsg);
	BOOL BeginUpdateItem();		// �ű��ص�����

	// װ��������װ����
	BOOL RecvGetSuitAttributes(struct SQSuitAttributeMsg *pMsg);
	BOOL SendNeedMETRIALInfo(CPlayer *pPlayer,BYTE  bType);//������Ҫǿ���ĵ�����Ϣ
	long OnGetNeedMetrial(CPlayer* pPlayer,DWORD NeedNum,std::vector<SPackageItem*> & vec,long ItemID, WORD LookType = XYD_FT_ONLYUNLOCK);//�ӱ����еõ����ʵĵ��߼���
	long OnProcessNeedMetrial(CPlayer* pPlayer,std::vector<SPackageItem*> vec,long NeedNum,std::string str,bool& bHaveBindMetrial);
	void OnthSysNotice(WORD index);
public://��δ������������
	BOOL RecvRequireItemInfoMsg(struct SQRequireItemInfoMsg *pMsg);			// �ͻ�Ҫ���ȡװ����Ϣ
    BOOL RecvQueryUpdateItemData( struct QueryUpdateItemDataMsg *pMsg );	// �û������Ի�ȡĳ�����ߵ���������
	BOOL ToForge(BYTE byX, BYTE byY);
	BOOL ToFuse(BYTE byX, BYTE byY);
	void SendUseIntEffMsg(WORD type, DWORD time, WORD in_wKindOfCD);		// ��ʱʹ����ǰ�ķ�ʽ������Ϣ

public:
	bool GetEquipment(BYTE btype,BYTE bIndex,SEquipment &equip); //��ȡһ��װ������ ��Ҫʹ����lua
	bool SetEquipment(BYTE btype,BYTE bIndex,const SEquipment *sequip);//����һ��װ������ ��Ҫʹ����lua


	void OnGetIntensifyInfo(struct SQIntensifyInfo *pintensifyinfo);
	void SendIntensifyInfo(const struct SAIntensifyInfo *pIntensifyInfo);

	void OnGetQualityInfo(const struct SQEquipQualityInfo *pQualityInfo);
	void SendQualityInfo(const struct SAEquipQualityInfo *pQualityInfo,WORD msgsize);

	BOOL RecvBeginUpgradeLevel(struct SQBeginEquipLevelUpMsg *pMsg);		// װ������
	BOOL SendIntensifyResult(struct SABeginEquipLevelUpMsg *pMsg); //װ��ǿ�����

	BOOL SendQualityResult(struct SAStartQuality *pMsg);//װ�����׽��
	BOOL OnQuestEquipSwitch(struct SQEquipSwitch* pMsg);//�ͻ��������л�����

	BOOL DeleteItem(DWORD ItemID,WORD num);//ɾ������
	void OnRecvSaleItem(struct SQItemSale *pMsg); //���۵���

	void OnRecvCleanColdTime(struct SQCleanColdtime *pMsg);
	void SendCleanColdTimeResult(struct SACleanColdtime *pMsg);

	void OnGetRiseStarInfo(const struct SQRiseStarInfo *pRiseStarInfo);  //��������������Ϣ
	void SendRiseStarInfo(const struct SARiseStarInfo *pRiseStarInfo, WORD msgsize); //��������������Ϣ
	void SendRefineStarResult(const struct SABeginRefineStar *pRefineStar,WORD msgsize);//�������ǽ��
	void SendMsgToPlayer(const  LPVOID strarresule, WORD msgsize);

	void OnGetEquiptKeYinInfo(const struct SQKeYinInfo *pKeYinInfo); //�����ӡ����
	void SendEquiptKeYinInfo(const struct SAKeYinInfo *pKeYinInfo, WORD msgsize); //���Ϳ�ӡ����
	void OnBeginKeYin(const  struct SQBeginKeYin *pBeginKeYinInfo); //����ʼ��ӡ
	void SendKeYinResult(const  struct SABeginKeYin *pBeginKeYinInfo, WORD msgsize); //���ؿ�ӡ���
	void OnBeginKeYinChange(const struct SQKeYinChange *pBeginKeYinChange);//��ӡת��
	void OnGetKeYinChangeInfo(const struct SQKeYinChangeInfo *pKeYinChangeInfo);//��ӡת����Ϣ
	void SendKeYinChangeData(const struct SAKeYinChange *pKeYinChange);//����׼��ת���Ŀ�ӡ����
	void OnBeginEquipJDing(const struct SQEquipJDing* pEquipJDing);//װ������
	void SendEquipJDingResult(const struct SAEquipJDing *pEquipJDing,WORD msgsize);//װ���������
	void OnGetEquipJDingInfo(const struct SQEquipJDingInfo* pEquipJDingInfo);//����װ����������
	void SendEquipJDingInfo(const struct SAEquipJDingInfo *pEquipJDingInfo, WORD msgsize);//����װ����������
	void OnGetEquipJDingColor(const struct SQEquipJDingColor *pEquipJDingColor);//������ɫ��Ϣ
	void SendEquipJDingColor(const struct SAEquipJDingColor *pEquipJDingColor, WORD msgsize);//���ͼ�������ɫ
	void OnGetSuitcondition(const struct SQSuitcondition *pSuitcondition);//������װ����
	void SendSuitcondition(const struct SASuitcondition *pSuitcondition,WORD msgsize);//������װ����
	void OnGetEquipPower(const struct SQEquipPower *pEquipPower);//��ȡװ��ս����
	void OnBuyPlayerTp(const struct SQBuyPlayerTp* pPlayerTp); //��ҹ�������

	
	//2014.2.20 add
	void OnReturnQiDaoMsg(struct SQQiDaoInfo *pMsg);	//������������Ϣ
	void OnAddQiDaoAndReturn(struct SQAddQiDaoInfo *pMsg);	//���տͻ�����ȡ����Ϣ����Ӧ�Ĵ������ؽ��
	//2014.2.21 add
	void OnReturnYuanBaoMsg(struct SQGiftPacketMsg *pMsg);		//����Ԫ������е�װ����Ϣ
	void OnBueYuanBaoGift(struct SQBuyGiftPacketMsg *pMsg);		//����Ԫ������е�װ��

	//add 2014.3.5 ly
	void OnOpenSetSkillPlan(struct SQOpenSetSkillBaseMsg *pMsg);		//����򿪼�������������ü���
	void OnUdtPlayerSkillAndPatern(struct SQUpdateSkillBaseMsg *pMsg);		//������ҵ�ǰ��ʹ�õļ��ܺ͵�ǰ����̬

	//add by ly 2014/4/14
	void OnReturnCellCountDown(const struct SQCellCountDown *pMsg);	//���󱳰����ӵ���ʱʱ��


	void OnUpgradeSkill(struct SQUpgradeSkillBaseMsg *pMsg);		//��������
public:
    CItemUser(void);
    ~CItemUser(void);

	BOOL UpdateTimeRecycleItem();

public:
	///////////////////////////////////////////////////////////////////////////////////
	// �����3����ϵͳ���Ż������ܺʹ洢��ʽ����
	typedef std::map<DWORD, DWORD> ItemState;
	
	SItemWithFlag	m_PackageItems[PackageAllCells];	// �����б�
	ItemState		m_ItemsState;						// ���ߴ���״̬
	SEquipedWithFlag m_EquipedColumnFlag[EQUIP_P_MAX];		// ��¼���װ�����ĸ�������״̬

	DWORD			m_CommNpcGlobalID;					// ��ǰ��NpcID
	WORD			m_CurShopID;						// ��ǰ�ĵ���ID
	DWORD			ArrangeIntervalTime;				// ��������ʱ����
	SPackageItem	m_EmptyItem4Debug;					// ����ASSERT��֤��
	///////////////////////////////////////////////////////////////////////////////////
	std::vector<long> m_Blessvec;
public:
	// �ȴ�������Ϣ���������֮��������ٴι���
	BOOL m_bWaitBuyResult;

public:
	// һ��ǿ��˽�к���
	BOOL Equip_Is_Tyler(DWORD wIndex);				// װ���Ƿ�������װ��
	BOOL RemoveRefineMetrial();						// ���������ȼ�ʱ�����ԭ����

	void SendUpdateEquipInfo(SEquipment* equip);	// ������ʾ��Ϣ����.����װ������
	BOOL SendNextLevelEquip(BYTE type);				// �õ�Ҫ��ʾ��ǿ�����װ����Ϣ	
	BOOL SendRefineEquipTheBestPreview(BYTE byType); // [���칦���﷢�ͼ�ƷԤ��]
	
	BOOL AddRefineAttribute(SEquipment* pEquip);	// ������׺����Ǻ�ĸ�������
	BOOL AddMaxExtraAttri(SEquipment* pEquip, BOOL bFillRefineAttri = FALSE);		// �����Ʒ�����������󸽼�����
	long AddStrengthenEquip(DWORD ItemId = 0);						// ������Ʒ�������װ��

	static WORD calculateRefineStarSuccessRate(byte bStarNum, bool bHaveLuckyCharmItem=false); // �������ǣ�ǿ�����ĳɹ���
	static BOOL IsReduceStarbyCalculateRefineStarFailed(byte byStarNum); // �ж�ǿ��ʧ��֮���Ƿ����
	
};
