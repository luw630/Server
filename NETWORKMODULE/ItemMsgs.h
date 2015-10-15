#pragma once

#include "NetModule.h"
#include "StallDef.h"
#include "../pub/ConstValue.h"

// ���������Ϣ��
DECLARE_MSG_MAP(SItemBaseMsg, SMessage, SMessage::EPRO_ITEM_MESSAGE)
//{{AFX
EPRO_DROP_ITEM,                             // ������Ʒ
EPRO_PICKUP_ITEM,                           // ʰȡ
EPRO_ADDPACKAGE_ITEM,                       // ��Ʒ���м���Ʒ
EPRO_ADDWAREHOUSE_ITEM,                     // �ֿ��е���Ʒ
EPRO_ADDGROUND_ITEM,                        // �������Ʒ
EPRO_CHECKGROUND_ITEM,						// �鿴�������
EPRO_CLOSEGROUND_ITEM,						// ȡ���鿴�������
EPRO_PRE_USE_ITEM,							// ��ƷԤ����
EPRO_USE_ITEM,                              // ʹ����Ʒ
EPRO_DELPACKAGE_ITEM,                       // ɾ����Ʒ���е���
EPRO_DELGROUND_ITEM,                        // ɾ���ر���Ʒ
EPRO_MOVE_ITEM,                             // �ƶ���Ʒ����Ʒ����λ��
EPRO_UNLOAD_EQUIP,							// ��װ��������Ҽ�жװ
EPRO_REQUIRE_ITEMINFO,						// �����ȡһ����Ʒ��������Ϣ
EPRO_EQUIP_ITEM,                            // װ�����ɼ�����Ʒ��Ⱥ����
EPRO_CHANGE_EQUIPCOL,                       // �ı�װ����ɫ
EPRO_EXCHANGE_MESSAGE,                      // ���߽���
EPRO_BUY_MESSAGE,                           // ���߽���
RPRO_EQUIP_INFO,                            // װ���׵���Ϣ
RPRO_WAREHOUSE_BASE,                        // �ֿ����
RPRO_EQUIPMENTINFO,                         // ����һ���˵�װ������
RPRO_SALEITEM_MESSAGE,                      // ���۵������
EPRO_AUTOUSE_ITEM,							// �Զ���ҩ
EPRO_CONSUMEPOINT_UPDATE,                   // ���ѻ��ֱ仯����������Ϊ����й�����Ϊ
EPRO_SPLIT_ITEM,                            // �����ֵ���
EPRO_UPDATE_ITEMINFO,                       // ���£���ȡ��ĳ�����������Ϣ
EPRO_TREASURE_SHOP,                         // �ٱ��̳�
EPRO_DIAL,                                  // ��ת�̲���
EPRO_NEATEN,                                // ������
EPRO_UPDATE_TEMPITEM,                       // ����������ϵ���ʱ��Ʒ
EPRO_AUTOFIGHT,							    // �Զ�ս��
EPRO_USEINTERVAL_EFFECT,                    // ����ʹ�ü����Ч
EPRO_PLUGINGAME_MONEY,                      // ��Ϸ������Ǯ�ı�
EPRO_EQUIP_WEAR_CHANGE,						// װ���;öȱ仯
EPRO_UPDATEGROUND_ITEM,						// ���µ�����Ʒ
EPRO_ACTIVEEXTPACKAGE,						// �����
EPRO_UPDATEITEM_ATTRIBUTE,					// ���µ�������
EPRO_SPLITITEM_INPROGRESS,					// �Ƿ��ֵ��߽�����
EPRO_START_FORGE,							// ����װ��׼������
EPRO_DRAG_METRIAL,							// �������
EPRO_END_FORGE,								// ��ɶ���
EPRO_MAKE_HOLE,								// ���
EPRO_INLAY_JEWEL,							// ��Ƕ��ʯ
EPRO_TAKEOFF_JEWEL,							// ȡ�±�ʯ
EPRO_SALEDITEM_LIST,						// �鿴�ع��б�
EPRO_BUY_SALEDITEM,							// ��һ���ع���Ʒ
EPRO_LOCKITEM_CELL,							// ����һ������

EPRO_GROUNDITEM_TAKECHANCE,					// ��ȡ����ҡ��
EPRO_GROUNDITEM_CHANCEBACK,					// �ͻ���ҡ�Ż���

EPRO_FIX_ONE_ITEM,							// ��������װ��
EPRO_FIX_ALL_ITEM,							// ȫ������װ��

EPRO_ADD_EQUIP,								// ǿ������װ��
EPRO_ADD_METRIAL,							// ǿ��������ϣ�ԭ���ϡ��������ϣ�

EPRO_BEGIN_REFINE_GRADE,					// ���� ����
EPRO_BEGIN_REFINE_STAR,						// ���� ����
EPRO_BEGIN_UPGRADE_LEVEL,					// װ��ǿ���ȼ�
EPRO_BEGIN_UPGRADE_QUALITY,					// ��Ʒ��


EPRO_BEGIN_RESET_ATTRIBUTE,					// װ������
EPRO_BEGIN_IDENTIFY,						// װ������
EPRO_BEGIN_IDENTIFY_SYNEQUIP,				// ��ʼϴ����ͬ����װ������
EPRO_BEGIN_IDENTIFY_EXCHANGE,				// ��ʼϴ��֮�󽻻�

EPRO_END_STRENGTHEN,						// ����ǿ��

EPRO_AFTER_UPGRADE_EQUIP,					// ��ʾǿ�����װ��
EPRO_EQUIP_INFO_IN_BAG,						// ������װ����Ϣ


EPRO_SPECIAL_USEITEM,                       // ����ʹ����Ʒ
EPRO_FIGHTPET_EQUIPITEM,                    // ����װ�����ɼ�����Ʒ��Ⱥ����
RPRO_FIGHTPET_EQUIPINFO,                    // ����װ���׵���Ϣ
EPRO_UNLOAD_FIGHTPETEQUIP,					// ��װ��������Ҽ�ж������װ��
EPRO_FIGHTPET_FIGHTPETTOITEM,				// ����ת��Ϊ��Ӧ����
EPRO_FIGHTPET_ITEMTOFIGHTPET,				// ���͵���ת������
EPRO_RANK_EQUIPMENT_INFO,					// ���а��в鿴�������װ����Ϣ

EPRO_RESETPOINT_METRIAL,					// ����ϴ��Ĳ���
EPRO_END_RESETPOINT,						// ȡ��ϴ��

EPRO_EQUIP_ATTRICHANGE,						// װ�������Է����仯

EPRO_DRAG_UPDATE_ITEM,						// ����ǿ�����-������������
EPRO_DRAG_UPDATE_METRIAL,					// �����������ߵĲ���
EPRO_BEGIN_UPDATE_ITEM,						// ��ʼ��������
EPRO_END_UPDATE_ITEM,						// ȡ����������

EPRO_HUISHENDAN_LIST_INFO,					// ������Ϣ�б���Ϣ

EPRO__GET_ONLINEGIFT,						// �ͻ��������ȡ�����������
EPRO_PACK_BUFFERPACK,						// ������ƷID�����Ϣ���ͻ���	

EPRO_FOLLOWITEM_QuestData,					// ���������̳��б�����
EPRO_BUYITEM_FOLLOW,						// ������ߵ�����
EPRO_Bless_ReQuest,							// ������ʼ
EPRO_Bless_ExcellenceData,					// ���õ���ǰ������һ����ǰ��10�εĸ߼�����ͨ�� 
EPRO_Bless_AllServerNotice,					// ����õ���ȫ����ͨ��
EPRO_Bless_StopBless,						// ֹͣ��
EPRO_Bless_QuestAddItems,					// ������ӵ���

EPRO_EQUIP_FASHIONCHANGE,					// ʱװģʽ������ͨģʽ
EPRO_EQUIP_SUIT_ATTRIBUTE,					// ��ʾ��װ���������Ƿ񼤻�

EPRO_ChannelCallSell_INFO,					// ������Ϣ
EPRO_ChannetCallBuy_INFO,					// Ƶ������

EPRO_STRENGTHEN_NeedMETRIAL,				// ǿ����Ҫ����
EPRO_BEGIN_ReMove_STAR,						// ϴ��

EPRO__SEND_REWARD,			//������ػ�������Ϣ

EPRO_INTENSIFY_INFO,				//ǿ������
EPRO_QUALITY_INFO,				//��������Quality
EPRO_RISE_INFO,						//��������
EPRO_KEYIN_INFO,				//��ӡ���� 

EPRO__EQUIP_SWITCH,			//�����л�
EPRO__ITEM_SALE,			//���߳���
EPRO_CLEAN_COLDTIME,				//���ǿ����ȴʱ�� 

EPRO_BEGIN_KEYIN,							//����������ӡ����
EPRO_BEGIN_KEYINCHANGE,				//�����ӡ����ת��
EPRO_BEGIN_JDING,				//����װ������
EPRO_JDING_INFO,				//����װ������������Ϣ
EPRO_JDING_COLOR,				//ͬ��װ����������ɫ��Ϣ
EPRO_BEGIN_KEYINCHANGEINFO,				//�����ӡ����ת����Ϣ
EPRO_SUIT_CONDITION,				//��װ����
EPRO_SUIT_PROPERTY,				//��װ����
EPRO_EQUIPT_POWER,				//����װ��ս����

//2014.2.20  Add
EPRO_EVERYDAY_ALLQIDAO,		//ÿ��VIP��õ�����Ϣ
EPRO_EVERYDAY_ADDQIDAO,		//�����ص�����Ϣ

EPRO_GET_YUANBAOGIFTINF,	//Ԫ���������Ϣ
EPRO_BUY_ALLYUANBAOGIFT,		//һ������Ԫ������е�װ��

EPRO_BUY_PLAYERTP,		//��ҹ�������
//2014/4/14 add by ly
EPRO_CELL_COUNTDOWN,	//������ӵ���ʱ��Ϣ

//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG(SGroundItemTakeChance, SItemBaseMsg, SItemBaseMsg::EPRO_GROUNDITEM_TAKECHANCE)
struct SAGroundItemTakeChance : public SGroundItemTakeChance
{
	enum
	{
		SGITC_NOUSE,			// ������
		SGITC_USE,				// ����
	};

	BYTE			m_Type;		// ����
	DWORD			m_Index;	// ҡ��ID
	SRawItemBuffer	m_Item;		// ����
};

DECLARE_MSG(SGroundItemChanceBack, SItemBaseMsg, SItemBaseMsg::EPRO_GROUNDITEM_CHANCEBACK)
struct SQGroundItemChanceBack : public SGroundItemChanceBack
{
	enum
	{
		SQGICB_GO,				// ��ʼҡ��
		SQGICB_GIVEUP,			// ������
		SQGICB_GIVEUPALL,		// ȫ������
	};

	BYTE	m_Choice;			// ���ѡ��
	DWORD	m_Index;			// ҡ��ID
};

struct SAGroundItemChanceBack : public SGroundItemChanceBack
{
	enum
	{
		SAGICB_GO,				// ҡ�Ž��
		SAGICB_GIVEUP,			// �������
		SAGICB_TIMEOUT,			// ��ʱ
		SAGICB_ITEM_GONE,		// ������ʧ
	};

	BYTE	m_Choice;			// ���ѡ��
	WORD	m_Result;			// ���
	DWORD	m_Index;			// ҡ��ID
};

// �����3�ع�ϵͳ
struct SaledItemType
{
	SPackageItem sItem;					// ����
	DWORD		 dwValue;				// �۸�
};

DECLARE_MSG(SSaledItemList, SItemBaseMsg, SItemBaseMsg::EPRO_SALEDITEM_LIST)
struct SQSaledItemList : public SSaledItemList
{
	DWORD dwSerial;
};

struct SASaledItemList : public SSaledItemList
{
	BYTE bCount;						// ����
	SaledItemType iSaledList[CONST_BUYBACK_COUNT];
};

DECLARE_MSG(SBuySaledItem, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_SALEDITEM)
struct SQBuySaledItem : public SBuySaledItem
{
	BYTE bIndex;			// Ҫ�ع���������0xff��ʾȫ��
};

struct SABuySaledItem : public SBuySaledItem
{
	BYTE bIndex;			// �ɹ��ع����ߣ�0xff��ʾȫ��
};

DECLARE_MSG(SLockItemCell, SItemBaseMsg, SItemBaseMsg::EPRO_LOCKITEM_CELL)
struct SALockItemCell : public SLockItemCell
{
	enum
	{
		SLC_PACKAGE,		// ��ұ���
		SLC_EQUIPCOLUMN,	// װ����
	};

	BYTE bTyte;
	bool bLock;				// �Ƿ�����
	WORD wPos;				// ����λ��
};

DECLARE_MSG(SStartForge, SItemBaseMsg, SItemBaseMsg::EPRO_START_FORGE)
struct SQStartForge : public SStartForge
{
	enum
	{
		ASF_NONE,			// Nothing
		ASF_MAKE_HOLE,		// ���
		ASF_INLAY_JEWEL,	// ��Ƕ��ʯ
		ASF_TAKEOUT_JEWEL,	// ȡ��ʯ
	};

	WORD wCellPos;
	BYTE bType;		// ���
};
struct SAStartForge : public SStartForge
{
	enum
	{
		SAS_SUCCESS,
		SAS_INLAY_NoGEM,	//���ǲ��뱦ʯ��Э�鷢����
		SAS_INLAY_Error,	//���ڵ�״̬ʱ���뱦ʯ�������㷢����������
		SAS_INLAY_EQUIP,	//�򶯵�װ��Ϊ��
		SAS_INLAY_XY,		//����������λ��û���ҵ�
		SAS_INLAY_XY_LOCK,	//����������λ����������
		SAS_NoFindGEM,		//û���ҵ�������ĵ���ID�ڽű�����
		SAS_NoGEMITEM,		//�����ĸ�����������ĵ��߲��Ǳ�ʯ����
	};
	BYTE Result;		// ���
};

/// װ��ǿ����ص��ⲿѡ��
enum TStrengthenExternChoose
{	
	//EChooseNone = 0x0, //ʲô��ûѡ
	EStrengthenChoose_LuckyCharm = 0x1,	//ѡ�����˷�
	EStrengthenChoose_ProtectionCharacter= 0x2, //ѡ�б�����
};

DECLARE_MSG(SDragMetrial, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_METRIAL)
struct SQDragMetrial : public SDragMetrial
{
	const static WORD msc_wMaxLen = 32; // [2012-9-17 11-38 gw: +Ŀǰ���������Ӹ���]
	//BYTE bX;		// ����
	//BYTE bY;		
	BYTE byPosNum;
	WORD waPos[msc_wMaxLen]; // x * PACKAGEWITH + y
};

// ֪ͨ�������
DECLARE_MSG(SEndForge, SItemBaseMsg, SItemBaseMsg::EPRO_END_FORGE)
struct SQEndForge : public SEndForge
{
	enum
	{
		QEF_EQUIP_OFF,
		QEF_METRAIL_OFF,
		QEF_CLOSE,
	};

	BYTE bOps;
};

DECLARE_MSG(STakeOffJewel, SItemBaseMsg, SItemBaseMsg::EPRO_TAKEOFF_JEWEL)
struct SQTakeOffJewel : public STakeOffJewel
{
	BYTE pos;		// ȡ��ʯ��λ��
	BYTE SelectType;// 0 û��ѡ�� 1 �ɹ��� 2 ����ʯ
};

struct SATakeOffJewel : public STakeOffJewel
{
	enum
	{
		ATJ_SUCCESS,		// �ɹ�
		ATJ_LACK_MONEY,		// ��Ǯ����
		ATJ_LACK_SPACE,		// �����ռ䲻��
		ATJ_LACK_EXP,       // ���鲻��
		ATJ_CLOSED,         // ��ⶴ��
		ATJ_NoFindHole,		// û���ҵ���Ƕ�ñ�ʯ�Ķ������ԾͲ�����ժ���������
		ATJ_protocol_ERROR,	// 0 û��ѡ�� 1 �ɹ��� 2 ����ʯ
		ATJ_Metrial_ERROR,	// ���ϲ���
	};	

	BYTE result;
	BYTE pos;
};

DECLARE_MSG(SInLayJewel, SItemBaseMsg, SItemBaseMsg::EPRO_INLAY_JEWEL)
struct SQInLayJewel : public SInLayJewel
{
};

struct SAInLayJewel : public SInLayJewel
{
	enum
	{
		ALJ_SUCCESS,		// ��Ƕ�ɹ�
		ALJ_LOCK_MONEY,     // û�н��
		ALJ_LOCK_TOOL,      // û�б�ʯ
	};

	BYTE result : 3;
	BYTE index	: 5;
	BYTE type;
	BYTE value;
};

// ���
DECLARE_MSG(SMakeHole, SItemBaseMsg, SItemBaseMsg::EPRO_MAKE_HOLE)
struct SQMakeHole : public SMakeHole
{
	//BYTE SelectType;//0 û��ѡ�� 1 �ɹ��� 2 ����ʯ
	byte byStrengthenExternChoose; // �Ƿ�ѡ�����˷����������ⲿ״̬���ο�TStrengthenExternChoose	
};

// ��׵Ľ��
struct SAMakeHole : public SMakeHole
{
	enum
	{
		AMH_SUCCESS,		// ��׳ɹ�
		AMH_FAILED,			// ���ʧ��
		AMH_LACK_MONEY,		// ���Ҳ���
		AMH_LACK_TOOL,		// ���ϲ���
		AMH_ALL_OPENED,		// ����ȫ��
		AMH_MISMATCH,       // ԭ���ϲ�ƥ��
		AMH_PrctectMetrial,	// �������ϲ���
		AMH_FAILED_REDUCEHOLE, // ʧ�ܼ�����
	};
	
	BYTE result : 3;
	BYTE index	: 5;
};

DECLARE_MSG(SSplitItemInProgress, SItemBaseMsg, SItemBaseMsg::EPRO_SPLITITEM_INPROGRESS)
struct SASplitItemInProgress : public SSplitItemInProgress
{
	enum
	{
		SIIP_START = 1,
		SIIP_END,
	};

	DWORD	dwGlobalID;
	BYTE	SplitStatus;
};

DECLARE_MSG(SUpdateItemAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATEITEM_ATTRIBUTE)
struct SAUpdateItemAttribute : public SUpdateItemAttribute
{
	DWORD	dwGlobalID;
	DWORD	wIndex;
	WORD	wCellPos;	//����
	BYTE	bFlag;
};

// ������ҵ���չ����
DECLARE_MSG(SActiveExtPackage, SItemBaseMsg, SItemBaseMsg::EPRO_ACTIVEEXTPACKAGE)
struct SQActiveExtPackage : public SActiveExtPackage
{
	DWORD	dwGlobalID;						// ��ҵ�GID
	WORD	wWillActiveNum;							// Ҫ����ı�����������
};

struct SAActiveExtPackage : public SActiveExtPackage
{
	enum
	{
		EAEP_SUCCESS,						// ����ɹ�
		EAEP_LACK_ITEM_YUANBAO,					// ��Ʒ��Ԫ������
		EAEP_NUM_ERROR,				// ����ĸ���������		
	};

	WORD	wResult;						// ������
	WORD	wExtendActivedCount;			// ���б��������չ����
	DWORD	dwRemainYuanBao;				// ʣ��Ԫ��
};

//=============================================================================================
// ����������ϵ���ʱ��Ʒ
DECLARE_MSG( SUpdateTempItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATE_TEMPITEM )
struct SAUpdateTempItemMsg : public SUpdateTempItemMsg
{
	DWORD	dwGlobalID;		// �������е�ȫ��Ψһ��ʶ��
	WORD	itemlist[10];  // һ��������ͬʱ��10����ʱ��Ʒ
};
//=============================================================================================
// ������
DECLARE_MSG( SQNeatenMsgBASE, SItemBaseMsg, SItemBaseMsg::EPRO_NEATEN )
struct SQNeatenMsg : public SQNeatenMsgBASE
{
	DWORD	dwGlobalID;		// ���ID
};

struct SANeatenMsg : public SQNeatenMsgBASE
{
	BYTE	m_Result;		// �������ɹ����1Ϊ�ɹ���0Ϊʧ�ܣ�
};

//=============================================================================================
// ��ת��
DECLARE_MSG(SDialMsgBASE, SItemBaseMsg, SItemBaseMsg::EPRO_DIAL)
struct SQDialMsg : public SDialMsgBASE
{
    enum OP_TYPE
    {
        DIALP_START = 1,
        DIALP_TRY,
        DIALP_RECEIVE,
    };

    BYTE type;
    BYTE byDialType;
};

struct SADialMsg : public SDialMsgBASE
{
    enum DIAL_TYPE   // ��ͨ���ƽ𣬽��ң�Ԫ��.....
    {
        DIALT_NORMAL,       // ��ͨ��
        DIALT_GOLD,         // �ƽ�
        DIALT_TIPS,         // ����
        DIALT_SECRETTIPS,   // �������
        DIALT_YUANBAO,      // Ԫ��
        DIALT_LUBANSUO,     // ³����
        DIALT_KONGMINGSUO,  // ������

        //......
        DIALT_MAX,  
    };

    enum OP_TYPE
    {
        DIAL_SHOW = 1,
        DIAL_SHOWAWARD,
        DIAL_RUN,
        DIAL_COLSE,
    };

    BYTE index;
    BYTE type;
    BYTE buffer[1];
};

// �ٱ��̳�
DECLARE_MSG(STreasureShopMsg, SItemBaseMsg, SItemBaseMsg::EPRO_TREASURE_SHOP)
struct SQTreasureShopMsg : public STreasureShopMsg
{
	enum TYPE
	{
		TYPE_BUY,
		TYPE_MONEY,
		TYPE_OPEN,
		TYPE_UPDATE
	};

	BYTE type_;
    WORD itemID;
	BYTE type[2];
	WORD num;
};

struct SATreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	DWORD riches[3];  // Ԫ�������������� ����
};

// ������°ٱ��ڵ���
struct SQUpdateItemTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	BYTE chargeItemType; // ��������: 1111 ��Ʒ���� 1111
};

// ���°ٱ��ڵ��� �ڲ��ṹ����������-��Ʒ����-{ { ��ƷID-�ּ�-ԭ��-����} ... }
struct SAUpdateItemTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
	BYTE chargeItemType;     // ��������: 1111 ��Ʒ���� 1111
	BYTE shows;              // 111111 ÿһλ��ʾÿ��ר���Ƿ�����Ʒ
	WORD itemNum;            // ��Ʒ����
	BYTE buffer[1];
};

// ����򿪰ٱ�����Ϣ
struct SAOpenTreasureShopMsg : public STreasureShopMsg
{
	BYTE type_;
};

// ����/��˶�����Ϣ
DECLARE_MSG(SPreUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PRE_USE_ITEM)
struct SAPreUseItemMsg : public SPreUseItemMsg
{
	enum OperateType
	{
		OT_USE_NORMAL_ITEM,		// ��ʼʹ�õ��߶���
		OT_RIDE,				// ��ʼ�������
		OT_USE_ITEM_CANCEL,		// ȡ��ʹ�õ��߶���
		OT_RIDE_CANCEL,			// ȡ���������
	};

	DWORD	time;
	BYTE	operateType;
};

// ʹ�õ���
DECLARE_MSG(SUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_USE_ITEM)
struct SQUseItemMsg : public SUseItemMsg
{
	WORD wCellPos;
};

struct SAUseItemMsg : public SUseItemMsg
{
    enum EUseGoods 
    { 
        UG_NORMAL = 1 , 
        UG_BIJOU, 
        UG_GESTBOOK, 
		UG_ORDERSKILL,
        UG_ANTIDOTE 
    };

	SPackageItem stUseItem;
	BYTE byWhat;                  //   ���������װ���仯������               
	SSkill stSkill;
};

DECLARE_MSG(SAutoUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AUTOUSE_ITEM)
struct SQAutoUseItemMsg : public SAutoUseItemMsg
{
	// SAutoUseItemSet sSetHP, sSetMP, sSetSP;
    BOOL bSetQXD;    //�¼ӵ���Ѫ������
    BOOL bSetLZ;     // ����
    BYTE type;
};

DECLARE_MSG(SAutoFightMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AUTOFIGHT)
struct SQAutoFightMsg : public SAutoFightMsg
{
    // �ͻ���֪ͨ�������Զ�ս��״̬
	DWORD flags;
};

// ��������
DECLARE_MSG(SDropItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DROP_ITEM)
struct SQDropItemMsg : public SDropItemMsg
{
	SPackageItem stItem;
};

// �񵽵���
DECLARE_MSG(SPickupItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PICKUP_ITEM)
struct SQPickupItemMsg : public SPickupItemMsg
{
	DWORD	dwGlobalID;		// ����GID
	WORD	wIndex;			// ʰȡ������0xffff��ʾȫ��ʰȡ�������ʾҪʰȡ������
};

// ������Ʒ������
DECLARE_MSG(SAddPackageItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDPACKAGE_ITEM)
struct SAAddPackageItemMsg : public SAddPackageItemMsg
{
    SPackageItem stItem; 
};

DECLARE_MSG(SAddWarehouseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDWAREHOUSE_ITEM)
struct SAAddWarehouseItemMsg : public SAddWarehouseItemMsg
{
    SPackageItem stItem;
};


// ----- ��ʱʹ�ã�Ϊ�˲��Է��� ----------------------------------------
struct SQAddPackageItemMsg : public SAddPackageItemMsg
{
    SPackageItem stItem; 
	BYTE byNum;
};

// ɾ����Ʒ������
DECLARE_MSG(SDelPackageItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DELPACKAGE_ITEM)
struct SQDelPackageItemMsg : public SDelPackageItemMsg
{
	WORD DelItemNum;
	DWORD ItemID;
};
struct SADelPackageItemMsg : public SDelPackageItemMsg
{
    SPackageItem stItem;
};

// װ���׵���Ϣ
DECLARE_MSG(SEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::RPRO_EQUIP_INFO)
struct SAEquipInfoMsg : public SEquipInfoMsg
{
	SEquipment	stEquip;
	BYTE		byPos;
};

// ���ӵر����
DECLARE_MSG(SAddGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_ADDGROUND_ITEM)
struct SAAddGroundItemMsg : public SAddGroundItemMsg
{
	DWORD	dwGlobalID;
	DWORD   ItemID;
	WORD		ItemNum;
	WORD	PackageMode;
	WORD	wTileX;
	WORD	wTileY;
};

// ���µ�����Ʒ��֪ͨ�ͻ���ĳ����������Ʒ��ʰȡ
DECLARE_MSG(SUpdateGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATEGROUND_ITEM)
struct SAUpdateGroundItemMsg : public SUpdateGroundItemMsg
{
	DWORD	dwGlobalID;		// ����ID
	WORD	wIndex;			// ����
};

// �鿴�������
DECLARE_MSG(SCheckGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CHECKGROUND_ITEM)
struct SQCheckGroundItemMsg : public SCheckGroundItemMsg
{
	DWORD dwItemGID;		// ����ID
};

struct SACheckGroundItemMsg : public SCheckGroundItemMsg
{
	struct GroundItemWS
	{
		SRawItemBuffer item;
		BYTE		   status;
	};

	DWORD			dwItemGID;		// ����ID��Ϊ0��ʾ���ɲ鿴
	DWORD			dwMoney;		// ��������
	WORD			wMoneyType;		// ��������
	WORD			wItemsInPack;	// ���߸���
	GroundItemWS	m_GroundItem[g_PackageMaxItem];	// ��������
	
	WORD MySize()
	{
		return sizeof(SACheckGroundItemMsg) - ((g_PackageMaxItem-wItemsInPack) * sizeof(SRawItemBuffer));
	}
};

// ȡ���鿴�������
DECLARE_MSG(SCloseGroundItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CLOSEGROUND_ITEM)
struct SQCloseGroundItemMsg : public SCloseGroundItemMsg
{
};

// �ƶ���Ʒ������
DECLARE_MSG(SMoveItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_MOVE_ITEM)
struct SQMoveItemMsg : public SMoveItemMsg
{
	WORD wCellPos1; //����λ�á������жװ���ʾ�ڶ�Ӧλ������
	WORD wCellPos2;
	BYTE byMoveTo;
	BYTE byindex;	//��������
};

struct SAMoveItemMsg : public SMoveItemMsg
{
	WORD	wIndex;
	WORD	wWhat;
	BYTE	byIsChangeWeapon;
};

// װ��һ�����ɼ�����
DECLARE_MSG(SEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_ITEM)
struct SAEquipItemMsg : public SEquipItemMsg
{
	DWORD	dwGlobalID;
    BYTE	byEquipPos;
	DWORD	wItemIndex;
};

// ��װ��������Ҽ�жװ
DECLARE_MSG(SUnEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UNLOAD_EQUIP)
struct SQUnEquipItemMsg : public SUnEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE	byEquipPos;
};

// �����ȡ���ߵ���Ϣ
DECLARE_MSG(SRequirItemInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_REQUIRE_ITEMINFO)
struct SQRequireItemInfoMsg : public SRequirItemInfoMsg
{
	enum ItemLocation
	{
		IL_PACKAGE = 1,			// ����
		IL_EQUIP,				// װ����
	};
	DWORD   dwGlobalID;			// ��������
	DWORD   dwDesID;			// ����������
	WORD	wItem;				// ��ƷID
	BYTE	wLocation;			// ��Ʒ��λ��
	BYTE	wX;					// ���꣬�������װ���������wX��Ч����ʾװ������Ϣ��
	BYTE	wY;					//
};

struct SARequireItemInfoMsg : public SRequirItemInfoMsg
{
	SRawItemBuffer iteminfo;	// ��wIndexΪ0��ʾû�鵽
	// ������Ϣ�Ժ����
};

//=============================================================================================
// ������װ��ɫ
DECLARE_MSG(SEquipColorMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CHANGE_EQUIPCOL)
struct SQAEquipColorMsg : public SEquipColorMsg 
{
    DWORD	dwGlobalID;			// ��������ȫ��Ψһ��ʶ��
    BYTE    byEquipID;          // װ��ID 0-3 ���·������¡�������Ь�ӣ���4-7 �������Ӧ����ɫ
    WORD    wEquipCol;          // װ��Ҫ�ĵ���ɫ
};
DECLARE_MSG(SQuestFollowItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FOLLOWITEM_QuestData)
struct SQQuestFollowItemMsg : public SQuestFollowItemMsg
{
	WORD m_Index;				//1(ҩƷ��) 2(�ӻ���) ,3(ǿ����)
};
struct FollowShopS
{
	WORD	m_Index;				//1(ҩƷ��) 2(�ӻ���) ,3(ǿ����)
	WORD	m_Num;					//�����б����
	DWORD   m_Itemlist[120];			//�����б�
	char    m_name[8][10];
	WORD    m_NameNum;
	FollowShopS():m_Index(0),m_Num(0),m_NameNum(0)
	{
		memset(&m_Itemlist[0],0,sizeof(DWORD) * 120);
		memset(&m_name[0][0],0,sizeof(m_name));
	}
};
struct SAQuestFollowItemMsg : public SQuestFollowItemMsg
{
	FollowShopS m_Follow;
};

DECLARE_MSG(SBUYFollowItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUYITEM_FOLLOW)
struct SQBUYFollowItemMsg : public SBUYFollowItemMsg
{
	WORD  m_Index;				//1(ҩƷ��) 2(�ӻ���) ,3(ǿ����)
	DWORD m_ItemID;				//������ߵı��(ʵ��ID)
	DWORD m_Num;				//�����������
};
DECLARE_MSG(SBlessItemReQuestMsg, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_ReQuest)
struct SQBlessItemReQuestMsg : public SBlessItemReQuestMsg
{
	//������Ҫ�ͻ��˷���Ϣ����Ҫ���κ�����
	BYTE Flag;					//0Ϊ�������Ϊ�����
	BYTE QuickFlag;				//�Ƿ������
};
struct SABlessItemReQuestMsg : public SBlessItemReQuestMsg
{
	BYTE Flag;					//0Ϊ�������Ϊ�����
	BYTE QuickFlag;				//�Ƿ������
	DWORD m_ItemID;				//�õ��ĵ������
	SPackageItem m_ItemIDVec[12];			//���һ���Ƿ������Ѿ����������ID ������γ齱�Ľ��ID
};
DECLARE_MSG(SBlessExcellenceDataMsg, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_ExcellenceData)
struct SQBlessExcellenceDataMsg : public SBlessExcellenceDataMsg
{
	//������Ҫ�ͻ��˷���Ϣ����Ҫ���κ����� ��������
};
struct ExcellenceData
{
	char Name[30];
	SPackageItem Item;
	ExcellenceData()
	{
		memset(Name,0,30);
		//memset(ItemIDName,0,30);
	}
};
struct SABlessExcellenceDataMsg : public SBlessExcellenceDataMsg
{
	//���뵱ǰ���������ߵ��б����10��
	BYTE m_BlessFlag;						//0 ����ʹ����Ѵ��� 1��ʾ����ʹ����Ѵ���
	BYTE m_Num;						//��Ϣ����
	ExcellenceData m_ExData[10];	//��Ϣ����
};

DECLARE_MSG(SBlessAllServerNotice, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_AllServerNotice)
struct SABlessAllServerNoticeMsg : public SBlessAllServerNotice
{
	ExcellenceData m_Notice;
	SABlessAllServerNoticeMsg()
	{
		
	}
};
DECLARE_MSG(SBlessStopBless, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_StopBless)
struct SABlessStopBless : public SBlessStopBless
{
	BYTE BlessStoptype;//���ж�ԭ�� 1 Ǯ���� 2 �������� 3ʱ�����
};
DECLARE_MSG(SBlessQuestAddItems, SItemBaseMsg, SItemBaseMsg::EPRO_Bless_QuestAddItems)
struct SQBlessQuestAddItems : public SBlessQuestAddItems
{
	//������ӵ���
};
struct SABlessQuestAddItems : public SBlessQuestAddItems
{
	//������ӵ���
	SPackageItem Item;
};
//EPRO_Bless_ReQuest,							// ������
//EPRO_Bless_ExcellenceData,					// ���õ�����Ҫȫ��ͨ���ʷʫ����
//EPRO_Bless_AllServerNotice					// ����õ���ȫ��ͨ��

DECLARE_MSG_MAP(SSaleItemBaseMsg, SItemBaseMsg, SItemBaseMsg::RPRO_SALEITEM_MESSAGE)
RPRO_MOVETO_ITEM,								// �ƶ���������
EPRO_ADDSALE_ITEM,								// ��ӵ��ߵ�������
EPRO_DELSALE_ITEM,								// ɾ�����������
EPRO_SEESALE_ITEM,								// �쿴������
EPRO_BUYSALE_ITEM,								// ����������ĵ���
EPRO_CLOSESALE_ITEM,							// ��̯
EPRO_UPDATESALE_ITEM,							// ˢ��
EPRO_SEND_SALEINFO,								// ���Ͱ�̯��Ϣ
EPRO_CHANGE_PRICE,								// �ļ�
EPRO_CHANGE_NAME,								// �޸�̯λ��
EPRO_CHANGE_INFO,								// �޸�˵��
EPRO_SEND_MESSAGE,								// ��������
EPRO_SEND_ALL_MESSAGE,							// �������п��ܵ�����
EPRO_MONEY_IN,									// ��������
END_MSG_MAP() 

// ���¼ܵ�ͳһ��Ϣ�������¼ܣ�dMoney�����壬������
DECLARE_MSG(SMovetoSaleBoxMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::RPRO_MOVETO_ITEM)
struct SQMovetoSaleBoxMsg : public SMovetoSaleBoxMsg
{
    enum 
	{
		MP_SALEBOX,
        MP_PACKAGE
    };

    BYTE	byMoveto;		// �ƶ�����
	WORD	wCellPos;		// ��Ʒ��λ�ã�����MP_PACKAGE����������ʾ�����е�λ��	
	DWORD	wIndex;			// ID��������֤
	DWORD	dMoney;			// ��Ʒ�ĵ���

	size_t MySize()
	{
		return sizeof(SQMovetoSaleBoxMsg) - (MP_PACKAGE == byMoveto) ? (sizeof(DWORD)) : 0;
	}
};

// �ϼܵĻظ�
DECLARE_MSG(SAddSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_ADDSALE_ITEM)
struct SAAddSaleItemMsg : public SAddSaleItemMsg
{
	BYTE			bPos;		// λ��
	DWORD			dMoney;		// ����
	SPackageItem	item;		// ����
};

// �¼ܵĻظ�
DECLARE_MSG(SDelSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_DELSALE_ITEM)
struct SADelSaleItemMsg : public SDelSaleItemMsg
{
	BYTE	bPos;				// Ҫ�¼ܵ�λ��
};

// �ļ���Ϣ
DECLARE_MSG(SChangeItemPriceMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_PRICE)
struct SQChangeItemPriceMsg : public SChangeItemPriceMsg
{
	BYTE	bPos;				// Ҫ�ļ۵���Ʒλ��
	DWORD	newPrice;			// �µ�
};

// ��̯λ��
DECLARE_MSG(SChangeStallNameMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_NAME)
struct SQChangeStallNameMsg : public SChangeStallNameMsg
{
	char	szStallName[MAX_STALLNAME];		// ̯λ��
};

struct SAChangeStallNameMsg : public SChangeStallNameMsg
{
	DWORD	dwGlobalID;						// ΨһID
	char	szStallName[MAX_STALLNAME];		// ̯λ��
};

// ������
DECLARE_MSG(SChangeStallInfoMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CHANGE_INFO)
struct SQChangeStallInfoMsg : public SChangeStallInfoMsg
{
	char	szStallInfo[MAX_SALEINFO];
};

struct SAChangeStallInfoMsg : public SChangeStallInfoMsg
{
	DWORD	dwGlobalID;						// ΨһID
	char	szStallInfo[MAX_SALEINFO];
};

// ��������
DECLARE_MSG(SSendMessageMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_MESSAGE)
struct SQSendMessageMsg : public SSendMessageMsg
{
	DWORD	dwGlobal;						// ��������������Լ����ǲ鿴��
	char	szMsg[MAX_LOGSIZE];
};

struct SASendMessageMsg : public SSendMessageMsg
{
	StallLogInfo	log;					// �ش���ͬ��
};

// ����ҵ㿪��̯���󣬷������п��ܵ���Ϣ
DECLARE_MSG(SSendAllMessageMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_ALL_MESSAGE)
struct SASendAllMessageMsg : public SSendAllMessageMsg
{
	BYTE			bNumber;						// �м�����Ϣ
	StallLogInfo	logs[MAX_LOGLINES];				// ��Ϣ����

	size_t MySize()
	{
		return sizeof(SASendAllMessageMsg) - ((MAX_LOGLINES-bNumber)*(sizeof(LogInfo)));
	}
};

// ��Ʒ���»ָ����ļ�/�����ı��
DECLARE_MSG(SUpdateSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_UPDATESALE_ITEM)
struct SAUpdateSaleItemMsg : public SUpdateSaleItemMsg
{
    SaleItem   stSaleItem;
};

// ����
DECLARE_MSG(SMoneyInMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_MONEY_IN)
struct SAMoneyInMsg : public SMoneyInMsg
{
	DWORD dwMoneyIn;
};

// ��Ҳ鿴����
DECLARE_MSG(SSeeSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEESALE_ITEM)
struct SQSeeSaleItemMsg : public SSeeSaleItemMsg
{
	enum
	{
		SSI_ITEM,
	};

	BYTE			bType;
    SALE_BOTHSIDES	stBothSides;
};

struct SASeeSaleItemMsg : public SSeeSaleItemMsg
{
	SaleSeeItemInfo stItemBaseInfo;

	size_t MySize()
	{
		return sizeof(SASeeSaleItemMsg) - (MAX_SALEITEMNUM-stItemBaseInfo.ItemNumber)*sizeof(SaleItem);
	}
};

// ��̯/�ر������Ϣ
DECLARE_MSG(SCloseSaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_CLOSESALE_ITEM)
struct SQCloseSaleItemMsg : public SCloseSaleItemMsg
{
	enum
	{
		CS_SELF,			// ����Լ���̯
		CS_SELF_NOBACK,		// ���¼�
		CS_BUYER,			// �鿴�Źر�
	};

	BYTE bOperation;
};

struct SACloseSaleItemMsg : public SCloseSaleItemMsg
{
	enum
	{
		CSI_CLOSESEE,		// �رղ鿴���
		CSI_CLOSESTALL,		// ��̯
	};

	DWORD dwGID;			// ��̯��
	BYTE  bOperation;		// ����
};

// ��ʼ��̯
DECLARE_MSG(SSendSaleInfoMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_SEND_SALEINFO)
struct SQSendSaleInfoMsg : public SSendSaleInfoMsg
{
    char szStallName[MAX_STALLNAME];		// ̯λ��
	char szStallInfo[MAX_SALEINFO];			// ̯λ˵��
};

// ���Լ��İ�̯����
struct SASendSaleInfoMsg : public SSendSaleInfoMsg
{
	BYTE InfoLen;
    char szSaleInfo[MAX_STALLNAME];			

    int GetMySize()
    {
		InfoLen = dwt::strlen(szSaleInfo, MAX_STALLNAME-1);
		return sizeof(SASendSaleInfoMsg) - MAX_STALLNAME + InfoLen + 1;
    }
};

DECLARE_MSG(SBuySaleItemMsg, SSaleItemBaseMsg, SSaleItemBaseMsg::EPRO_BUYSALE_ITEM)
struct SQBuySaleItemMsg : public SBuySaleItemMsg
{
    SALE_BUYITEM stBuyItem;
};

// ���߽���
DECLARE_MSG(SBuyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_MESSAGE)
struct SBuy : SBuyMsg
{
	WORD	nCount;		// ��������
	BYTE	nPage;		// ��Ʒ����/ҳ��
	WORD	index;		// ��Ʒ������
	bool	bBuy;		// ��true ����false
	long    Flag;       // Ĭ��0 ��NPC 1�������̵�
	SBuy():nCount(0),nPage(0),index(0),bBuy(0),Flag(0)
	{

	}
};

struct SABuy : SBuyMsg
{
	SABuy() : bShowMessage(1) {};
    BYTE	nRet;			// 0��Ǯ���� 1���ɹ�
	bool	bShowMessage;	
};

//=============================================================================================
// һ�����׶����ύ��һ�����ײ�����ȷ�ϡ�ȡ����������
DECLARE_MSG(SEquipmentListMsg, SItemBaseMsg, SItemBaseMsg::RPRO_EQUIPMENTINFO)
struct SAEquipmentListMsg : 
    SEquipmentListMsg
{
    //DWORD dwGID; 
    //SEquipment Equip[16];
	//char szTitle[CONST_USERNAME];           // �ƺ�
	//WORD wkillValue;            // ɱ��ֵ PKֵ
	//char szFaction[CONST_USERNAME];         // ����
	//char szSpouse[CONST_USERNAME];          // ��ż

    //char szName[CONST_USERNAME];            // ����	OK
    //short nValue;               // ����ֵ
    //DWORD dwExtraState;         // ״̬
    //WORD wMedicalEthics;        // ҽ��ֵ
    //int  nState;                // վ�����Ǵ���
    //BYTE bySex:1;               // ��ҽ�ɫ�Ա�
    //BYTE byBRON:3;              // �������ʾͼƬ����==1��ʾ��ʦ
    //BYTE bySchool:4;            // �����������OK
    //WORD wWeaponIndex;
    //WORD wEquipIndex;		    // ��Ϊװ����Ҫ��ʾ
    //WORD wMutateID;             // ��ҵ�ǰ����ı��
    //WORD wOtherLevel;           // ��ҵȼ�OK

    //// [����չ�Ĺ���]�ű����ɵĸ�����Ϣ�鿴��ַ��
    ////BYTE newMark;   // ����Ϣ��չ��ǣ�����
    ////BYTE infoLength;
    ////char infoString[256];
	
	char		szName[CONST_USERNAME];				// ����	OK
	WORD		wOtherLevel;			// ��ҵȼ�OK
	SEquipment	Equip[16];

	DWORD		dwGlobal;				// GID
	BYTE		bFaceID;				// ��ģ��
	BYTE		bHairID;				// ͷ��ģ��
	DWORD		m_3DEquipID[11];			// 3Dװ���Ҽ���Ʒ��

	float		m_fPositionX;			// λ��x
	float		m_fPositionY;			// λ��y
	float		m_fDirection;			// ����

	BYTE		bySex:1;			    // ��ҽ�ɫ�Ա�
	BYTE		byBRON:3;				// �������ʾͼƬ����==1��ʾ��ʦ
	BYTE		bySchool:4;				// �����������OK

	DWORD		m_OtherMaxHP;
	DWORD		m_OtherMaxMP;
	DWORD		m_OtherMaxTP;
	DWORD		m_OtherHP;	
	DWORD		m_OtherMP;
	DWORD		m_OtherTP;
	WORD		m_OtherGongJi;			// ������
	WORD		m_OtherFangYu;			// ������
	WORD		m_OtherBaoJi;			// ����
	WORD		m_OtherShanBi;			// ����
	float		m_OtherAtkSpeed;		// �����ٶ�
	float		m_OtherMoveSpeed;		// �ƶ��ٶ�

	WORD		m_OtherGongjiPt;		//��������
	WORD		m_OtherFangyuPt;		//��������
	WORD		m_OtherQinggongPt;		//�Ṧ����
	WORD		m_OtherQiangjianPt;		//ǿ������

	QWORD		m_OtherCurExp;			//��ҵ�ǰ�ȼ��ĵ�ǰ����
	QWORD		m_OtherMaxExp;			//��ҵ�ǰ�ȼ��������			

	WORD		m_Otherbingshang;		//����
	WORD		m_Otherbingkang;		//����	
	WORD		m_Otherhuoshang;		//����
	WORD		m_Otherhuokang;			//��
	WORD		m_Otherxuanshang;		//����
	WORD		m_Otherxuankang;		//����
	WORD		m_Otherdushang;			//����
	WORD		m_Otherdukang;			//����

	WORD		m_Othershanghaijianmian;//�˺�����	
	WORD		m_Otherjueduishanghai;	//�����˺�
	WORD		m_Otherwushifangyu;		//���ӷ���
	WORD		m_Otherbaojibeishu;		//��������

	DWORD       m_OtherJP;				//��ҵ�ǰ����
	DWORD       m_OtherMaxJP;			//��������

	WORD		m_OtherShaLu;			//ɱ¾ֵ
	WORD		m_OtherXiaYi;			//����ֵ
	WORD		m_XwzValue;			//��Ϊֵ
};

// �ֿ������Ϣ��
DECLARE_MSG_MAP(SWareHouseBaseMsg, SItemBaseMsg, SItemBaseMsg::RPRO_WAREHOUSE_BASE)
//{{AFX
RPRO_OPEN_ITEM_WAREHOUSE,					// �����һ�����ֿ߲�
RPRO_CLOSE_WAREHOUSE,                       // ����ر�һ���ֿ�
RPRO_ITEM_MOVEIN,                           // ��һ�����ߣ����߽�Ǯ���������ƶ����ֿ�
RPRO_ITEM_MOVEOUT,                          // ��һ�����ߣ����߽�Ǯ���Ӳֿ��ƶ�������
RPRO_ITEM_MOVESELF,                         // ��һ�������ڲֿ����ƶ�
RPRO_ITEM_LOCKIT,							// ����
RPRO_ACTIVE_WAREHOUSE,						// ����
RPRO_ARRANGE_WAREHOUSE,						// ����ֿ�
//}}AFX
END_MSG_MAP()

// �ͻ��˿�ʼ����
DECLARE_MSG(SArrangeWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ARRANGE_WAREHOUSE)
struct SQArrangeWareHouseMsg : SArrangeWareHouseMsg
{
};

// �ͻ��˿�ʼ����
DECLARE_MSG(SActiveWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ACTIVE_WAREHOUSE)
struct SQActiveWareHouseMsg : SActiveWareHouseMsg
{
	DWORD	dwGlobal;
};

struct SAActiveWareHouseMsg : SActiveWareHouseMsg
{
	enum
	{
		AWH_SUCCESS,
		AWH_LACK_MONEY,
		AWH_ALL_ACTIVED,
	};

	BYTE	bResult;
	BYTE	bCurActiveTimes;
};

// �����һ�����ֿ߲�
DECLARE_MSG(SOpenItemWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_OPEN_ITEM_WAREHOUSE)
struct SQOpenItemWareHouseMsg : SOpenItemWareHouseMsg
{
	BYTE	byDirectly;			// �Ƿ�ֱ�ӽ��룬�������루1��ʾ���ԣ�2��ʾ���������룩
	char	szUserpass[CONST_USERPASS];

	WORD MySize()
	{
		return sizeof(SQOpenItemWareHouseMsg) - (byDirectly ? (sizeof(char) * CONST_USERPASS) : 0);
	}
};

struct SAOpenItemWareHouseMsg : SOpenItemWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // �ɹ�
        ERC_ALREADY_OPEN,       // �ֿ��Ѿ����򿪹���
        ERC_SERVER_DISCARD,     // ���β��������������
		ERC_NEEDPASSWORD,		// ��Ҫ�򿪶��������������
		ERC_PASSWORD_ERROR,		// �����������
    };

	BYTE			byRetCode;			// ����������Ϣ
    BYTE			byGoodsNumber;		// ���ֿ���ߵ�����
	BYTE			m_ActiveTimes;		// ����Ĵ���
	DWORD			dwOperationSerial;	// ���β��������к�
    DWORD			nMoneyStoraged;		// �ֿ��еĽ�Ǯ
    SPackageItem	GoodsArray[MAX_WAREHOUSE_ITEMNUMBER];
	DWORD			nBindMoney;	// �ֿ��еİ󶨽�Ǯ
    WORD MySize()
    {
		if ((byGoodsNumber > MAX_WAREHOUSE_ITEMNUMBER) && (byRetCode == ERC_OK))  // �򿪳ɹ������ж�byGoodsNumber
        {
            byGoodsNumber = 0;
            byRetCode	  = ERC_SERVER_DISCARD;
        }

        return sizeof(SAOpenItemWareHouseMsg) - sizeof(SPackageItem)*(MAX_WAREHOUSE_ITEMNUMBER-byGoodsNumber);
    }
};

// ����ر�һ���ֿ⣨֪ͨ�������ֿ�ʹ����ϣ�
DECLARE_MSG(SCloseWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_CLOSE_WAREHOUSE)
struct SQCloseWareHouseMsg : SCloseWareHouseMsg
{
    DWORD	dwOperationSerial;	                    // ���β��������к�
};

struct SACloseWareHouseMsg : SCloseWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // �ɹ�
        ERC_SERVER_DISCARD,     // ���β��������������
    };

	BYTE byRetCode;             // ����������Ϣ
};

// ��������/����һ���ֿ�
DECLARE_MSG(SLockWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_LOCKIT)
struct SQLockWareHouseMsg : SLockWareHouseMsg
{
	char szUserpass[CONST_USERPASS];
	bool bLock;
};

struct SALockWareHouseMsg : SLockWareHouseMsg
{
	bool bLock;
	bool bSuccess;
};

// ��һ�����ߣ����߽�Ǯ���������ƶ����ֿ�
DECLARE_MSG(SMoveItemInWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVEIN)
struct SQMoveItemInWareHouseMsg : SMoveItemInWareHouseMsg
{
    SCellPos Pocket;				// �ڴ�����Ҫ�ƶ��ĵ��ߵ�λ��
    SCellPos Store;					// ��Ҫ�ƶ����ֿ��е�λ��
    DWORD	 wIndex;				// ���β������ߵı�ţ�����У�飩
    DWORD	 dwOperationSerial;		// ���β��������к�
};

struct SAMoveItemInWareHouseMsg : SMoveItemInWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,						// �ɹ�
        ERC_SET_MONEY,				// ��Ǯ�ɹ�
        ERC_CANNOT_PUT,				// �޷����õ��߷ŵ�Ŀ���
        ERC_SERVER_DISCARD,			// ���β�������������������������������ȣ�
    };

	BYTE	byRetCode;				// ����������Ϣ
    DWORD	nMoneyOfStorage;		// �ֿ��зǰ󶨵Ļ���
	DWORD	nBindMoneyOfStorage;	// �ֿ��а󶨵Ļ���
	SPackageItem Item;				// �ֿ����¼ӵĵ���
};

// ��һ�����ߣ����߽�Ǯ���Ӳֿ��ƶ�������
DECLARE_MSG(SMoveItemOutWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVEOUT)
struct SQMoveItemOutWareHouseMsg : SMoveItemOutWareHouseMsg
{
    SCellPos Store;					// ��Ҫ�ƶ��ĵ����ڲֿ��е�λ��
    SCellPos Pocket;				// �ƶ����ڴ��е�Ŀ��λ��
    DWORD	 wIndex;				// ���β������ߵı�ţ�����У�飩
    DWORD	 dwOperationSerial;		// ���β��������к�
};

struct SAMoveItemOutWareHouseMsg : SMoveItemOutWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,						// �ɹ�
        ERC_SET_MONEY,				// ȡǮ�ɹ�
        ERC_CANNOT_PUT,				// �޷����õ��߷ŵ�Ŀ���
        ERC_SERVER_DISCARD,			// ���β�������������������������������ȣ�
    };

	BYTE	byRetCode;				// ����������Ϣ
	DWORD	nMoneyOfStorage;		// �ֿ��зǰ󶨵Ļ���
	DWORD	nBindMoneyOfStorage;	// �ֿ��а󶨵Ļ���
	SPackageItem Item;				// �ֿ����¼ӵĵ���
};

// ��һ�������ڲֿ����ƶ�
DECLARE_MSG(SMoveSelfItemWareHouseMsg, SWareHouseBaseMsg, SWareHouseBaseMsg::RPRO_ITEM_MOVESELF)
struct SQMoveSelfItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
    SCellPos StoreSrc;          // ��Ҫ�ƶ��ĵ����ڲֿ��е�λ��
    SCellPos StoreDest;         // ��Ҫ�ƶ��Ĳֿ��е�Ŀ��λ��
    DWORD	 dwOperationSerial;	// ���β��������к�
};

// ���������ص���Ϣ��������һ����SAAddPackageItemMsg
struct SAMoveSelfItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
	enum ERetCode
    {
        ERC_OK,                 // �ɹ�
        ERC_SERVER_DISCARD,     // ���β�������������������������������ȣ�
    };

	BYTE byRetCode;             // ����������Ϣ
    SPackageItem srcItem;		// Դ
	SPackageItem destItem;		// Ŀ��
};

// �����ֿ��ڵ���������
struct SQExChangeItemWareHouseMsg : SMoveSelfItemWareHouseMsg
{
    SCellPos SrcPos;			// ��Ҫ�ƶ��ĵ����ڲֿ��е�λ��
    SCellPos DestPos;			// ��Ҫ�ƶ��Ĳֿ��е�Ŀ��λ��
    DWORD	 dwOperationSerial; // ���β��������к�
};

// ��Ҽ佻�������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SExchangeBaseMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EXCHANGE_MESSAGE)
//{{AFX
RPRO_QUEST_EXCHANGE,                        // �����ĳ����ҽ��н��ף�������֪ͨ�ͻ��ˣ��ͻ���Ӧ���������
RPRO_MOVE_EXCHANGE_ITEM,                    // �ƶ�һ�����ߵ���������������Ǯ��
RPRO_EXCHANGE_OPERATION,                    // һ�����׶����ύ��һ�����ײ�����ȷ�ϡ�ȡ����������
RPRO_CANCEL_QUEST,							// ȡ���˽��׵�����
RPRO_QUEST_REMOTE_EXCHANGE,                 // �����ĳ��Զ����ҽ��л���ָ�����ߵĽ��ס�A -> S => S -> B => B -> S ��
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

// �����ĳ����ҽ��н���
DECLARE_MSG(SQuestRemoteExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_QUEST_REMOTE_EXCHANGE)
struct SQAQuestRemoteExchangeMsg : SQuestRemoteExchangeMsg
{
    DNID dnidClient;        // У������
    DWORD dwSrcGID;         // ���׷����˵�GID
    DWORD dwDestGID;        // ����Ŀ���GID
    bool bAccept;           // �Ƿ�ͬ�⽻��       <<<< ǰ�������Ҫ��Ϊ�˺���ǰ�Ľ����߼�����һ�£���ǰ��ǿ��ת���ġ�����

    WORD item;              // �����׵�ԭʼ����ID[�������Ч]
    char name[CONST_USERNAME];          // ���׷���������ĵ�Ŀ���������
	BYTE state;             // Ϊ�棺�򿪽����� �٣��򿪽�������
};

// �����ĳ����ҽ��н���
DECLARE_MSG(SQuestExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_QUEST_EXCHANGE)

// ���Ƿ�����֪ͨ�ͻ���
struct SAQuestExchangeMsg : SQuestExchangeMsg
{
	DNID dnidClient;		// У������
    DWORD dwSrcGID;			// ���׷����˵�GID
    DWORD dwDestGID;		// ����Ŀ���GID
	bool  bAccept;			// Ϊ�棺�򿪽����� �٣��򿪽�������
};

// Ȼ���ǿͻ���Ӧ�������
struct SQQuestExchangeMsg : SQuestExchangeMsg
{
    DNID dnidClient;		// У������
    DWORD dwSrcGID;			// ���׷����˵�GID
    DWORD dwDestGID;		// ����Ŀ���GID
    bool bAccept;			// �Ƿ�ͬ�⽻��
};

// ȡ���ҵ��������Ӳ������������
DECLARE_MSG(SCancelQuestExchangeMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_CANCEL_QUEST)
struct SACancelQuestExchangeMsg : SCancelQuestExchangeMsg
{
	enum
	{
		CQE_DEAD,			// ��Ϊ�̱�����
	};
	DWORD dwSrcID;			// ���׷�����
	BYTE  bResult;			// ȡ����ԭ��
};

// �ƶ�һ�����ߵ���������������Ǯ������ʱû�лش�����Ϣ��ͨ����������Ϣ�������
DECLARE_MSG(SPutItemToExchangeBoxMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_MOVE_EXCHANGE_ITEM)
struct SQPutItemToExchangeBoxMsg : SPutItemToExchangeBoxMsg
{
    DWORD dwSrcGID;			// ���׷����˵�GID
    DWORD dwDestGID;		// ����Ŀ���GID

    SPackageItem Item;		// �������Ʒ�������ǽ�Ǯ��

	WORD  wCellX, wCellY;	// Ŀ��λ��
};

struct SAPutItemToExchangeBoxMsg : SPutItemToExchangeBoxMsg
{
    DWORD dwSrcGID;         // ���׷����˵�GID
    DWORD dwDestGID;        // ����Ŀ���GID

    bool  isSrc;            // �Ƿ��Ƿ�����ִ�е��ƶ���Ʒ����

    SPackageItem Item;      // �������Ʒ�������ǽ�Ǯ��

	WORD  wCellX, wCellY;   // Ŀ��λ��
};

// һ�����׶����ύ��һ�����ײ�����ȷ�ϡ�ȡ����
DECLARE_MSG(SExchangeOperationMsg, SExchangeBaseMsg, SExchangeBaseMsg::RPRO_EXCHANGE_OPERATION)
struct SQExchangeOperationMsg : SExchangeOperationMsg
{
    DWORD dwSrcGID;     // ���׷����˵�GID
    DWORD dwDestGID;    // ����Ŀ���GID

    enum EOperation
    {
		LOCK,			// ����
		UNLOCK,			// ȡ������     
		COMMIT,			// �ύ
		UNCOMMIT,		// ȡ���ύ
        CANCEL,			// ȡ��
    };

    EOperation operation;
};

struct SAExchangeOperationMsg : SExchangeOperationMsg
{
	DWORD dwSrcGID;     // ���׷����˵�GID
    DWORD dwDestGID;    // ����Ŀ���GID
    bool  isSrc;        // �Ƿ��Ƿ�����ִ�еĲ���

    enum EOperation
    {
		LOCK,			// ����
		UNLOCK,			// ȡ������
        COMMIT,			// �ύ
		UNCOMMIT,		// ȡ���ύ
        CANCEL,			// ȡ��
        SUCCEED,
    };

    EOperation operation;
};

// ���߽���
DECLARE_MSG(SConsumePointMsg, SItemBaseMsg, SItemBaseMsg::EPRO_CONSUMEPOINT_UPDATE)
struct SAConsumePoint : SConsumePointMsg
{
	SAConsumePoint():dwConsumePoint(0){};
	DWORD dwConsumePoint;
};

// �����ֵ���
DECLARE_MSG(SplitItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_SPLIT_ITEM)
struct QuerySplitItemMsg : SplitItemMsg
{
    SCellPos	src;
    DWORD		count;
};

//=============================================================================================
// ���£���ȡ��ĳ�����������Ϣ
DECLARE_MSG( UpdateItemDataMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UPDATE_ITEMINFO )
struct QueryUpdateItemDataMsg : UpdateItemDataMsg
{
    // �ͻ��������ȡָ�����ߵ��������ݣ���Ч�ĵ���ID�ռ�ΪWORD[0~65535]
    enum ExtraQueryIndex
    {
        RESET = 65536,          // �����ȡ��Ҫ�ų����¸��µ����е��߱�ţ�����һ���������У����������еĵ������ö�����ɾ�����Ӷ�ͨ���������������»�ȡ��
        RANDMONTABLE = 65537    // �����ȡװ������������б������ڵ����б��е�extra_attribute��
    };

    DWORD index; 
    DWORD crc32;
};

struct AnswerUpdateItemDataMsg : UpdateItemDataMsg
{
    // �ͻ��������ȡָ�����ߵ��������ݣ���Ч�ĵ���ID�ռ�ΪWORD[0~65535]
    enum ExtraQueryIndex
    {
        RESET = 65536,          // �����ȡ��Ҫ�ų����¸��µ����е��߱�ţ�����һ���������У����������еĵ������ö�����ɾ�����Ӷ�ͨ���������������»�ȡ��
        RANDMONTABLE = 65537    // �����ȡװ������������б������ڵ����б��е�extra_attribute��
    };

    DWORD index; 

    char streamBuffer[10240];
};
//---------------------------------------------------------------------------------------------

//=============================================================================================
// ����ʹ�ü����Ч
DECLARE_MSG(SUseIntervalEffectMsg, SItemBaseMsg, SItemBaseMsg::EPRO_USEINTERVAL_EFFECT)
struct SAUseIntervalEffect : SUseIntervalEffectMsg
{
    // �������
    enum E_USEINTERVAL_TYPE
    {
        EUIT_HP,
        EUIT_MP,
        EUIT_SP,
        EUIT_HYDXG,      // ���������
        EUIT_XQDx,       // Ѫ����С
        EUIT_XQDd,       // Ѫ������
        EUIT_XQDt,       // Ѫ������
        EUIT_ROSE,       // õ��
        EUIT_LZ1,        // ����1
        EUIT_LZ2,        // ����2
        EUIT_LZ3,        // ����3
        EUIT_LZ4,        // ����4
        EUIT_LZ5,        // ����5
        EUIT_LZ6,        // ����6
        //............
        EUIT_MAX,
    }; 
	WORD type;
	DWORD time;         // ���ʱ��
	WORD wLocSrvKindNumberForCD;		// ��ȴʱ���õ���Ʒ����
};
//////////////////////////////////////////////////////////////////////////
//����
struct SSkillUseIntervalEffect : SUseIntervalEffectMsg
{
	// �������
	enum E_USEINTERVAL_TYPE
	{
		EUIT_SKILL1,
		EUIT_SKILL2,
		EUIT_SKILL3,
		EUIT_SKILL4,
		EUIT_SKILL5,
		EUIT_SKILL6,
		EUIT_SKILL7,
		EUIT_SKILL8,
		EUIT_SKILL9,
		EUIT_SKILL10,
		EUIT_SKILL11,
		EUIT_SKILL12,
		EUIT_SKILL13,
		EUIT_SKILL14,
		EUIT_SKILL15,
		EUIT_SKILL16,

		//............
		EUIT_MAX,
	};
	WORD type;          // �������
	DWORD time;         // ���ʱ��
};

//---------------------------------------------------------------------------------------------


//=============================================================================================
// ��Ϸ������Ǯ�ı�
DECLARE_MSG(_sc_SPlugInGameMoneyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PLUGINGAME_MONEY)
struct sc_PlugInGameMoneyMsg : public _sc_SPlugInGameMoneyMsg
{
    DWORD dwMoney[3]; // Ԫ���������� ����������
};

//=================================================
// װ���;öȸı�
DECLARE_MSG(SEquipWearChange, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_WEAR_CHANGE)
struct SAEquipWearChange : public SEquipWearChange
{
	EQUIP_POSITION equipPosition;
	WORD currWear;
};

// ����һ��װ��
DECLARE_MSG(SEquipFixOne, SItemBaseMsg, SItemBaseMsg::EPRO_FIX_ONE_ITEM)
struct SQEquipFixOne : public SEquipFixOne
{
	enum EQUIP_FIXED_POS 
	{
		EFP_WEAR,
		EEP_BAG,
	};
	DWORD	index;
	BYTE	type;
	BYTE	pos;
};

// ��������װ��
DECLARE_MSG(SEquipFixAll, SItemBaseMsg, SItemBaseMsg::EPRO_FIX_ALL_ITEM)
struct SQEquipFixAll : public SEquipFixAll
{
	
};

//	======װ������=====

// ����װ����Ϣ
DECLARE_MSG(SSAddEquip, SItemBaseMsg, SItemBaseMsg::EPRO_ADD_EQUIP)
struct SQAddEquip : public SSAddEquip
{
	enum
	{
		ADE_NONE,			     // Ĭ��״̬
		ADE_UPDATE_LEVEL = 1,	 // װ������
		ADE_REFINE_GRADE = 2,	 // װ������
		ADE_UPDATE_QUALITY = 3,	 // װ����Ʒ
		ADE_REFINE_STAR = 4,	 // װ������
		ADE_ReMove_STAR = 5,	 // װ��ϴ��		
		ADE_EQUIP_RESET = 6,	 // װ��ϴ��(���ø�����������)
		ADE_EQUIP_REFINE = 7,	 // װ������(���ø���������ֵ)		
		ADE_EQUIP_SHENGJIE=8,	 // װ�����ף������������Ե����ޣ�
		ADE_EQUIP_DECOMPOSITION = 9, // װ���ֽ�
		ADE_ADD_HOLE = 10,		 // װ�����
		ADE_GEM_INSERT = 11,		 // װ����Ƕ��ʯ
		ADE_GEM_REMOVE = 12,	 // װ����ʯժ��	
		ADE_EQUIP_SMELTING = 13,	 // װ������
		ADE_EQUIP_SPIRITATTACHBODY = 14, // װ���鸽		

		ADE_Max,
	};
	static const WORD msc_wMaxLen = 12; // �������������
	static const byte msc_byBatchDecomposition_Max = 8; // ������ֽ��������
	static const byte msc_byBatchSmelting_Max = 5; // �����������������	
	SQAddEquip()
	{	
		bType = 0;
		byPosNum = 0;
		byAssistPosNum = 0;
		memset(&waPos, 0, sizeof(waPos));
	}

	BYTE bType;		// װ��ǿ������
	BYTE byPosNum; // ��װ��������	
	BYTE byAssistPosNum;  // ����λ�õ�����
	WORD waPos[msc_wMaxLen]; // x * PACKAGEWITH + y	, �ȴ����װ�����ٴ�Ÿ���λ�ã�������Ҫ����byPosNum+byAssistPosNum<=msc_wMaxLen��
};
struct SAAddEquip : public SSAddEquip
{
	enum
	{
		ADE_NONE,						// Ĭ��״̬
		ADE_Success = 1,
		ADE_EquipLevel_Less = 2,		// �ȼ�����
		ADE_NoEquip = 3,				// ����װ������
		ADE_GETNEXTEquip_Error = 4,		// �õ���һ��װ��ʧ��
		ADE_SendNEEDITEM_Info_Error = 5,// ���͵�����Ϣʧ��
		ADE_UPDATE_QUALITY_1 = 6,	// װ����Ʒ�� ��ɫװ��������ʾ
		ADE_UPDATE_QUALITY_2 = 7,	// ��װ��������Ʒ��
		ADE_YINBING = 8,	// ����Ʒ�ʵ�װ��(����)����
		ADE_YINGLIANG = 9,	// ����Ʒ�ʵ�װ��(����)����
		ADE_UPDATE_NoCanLEVEL,	//��װ���������ȼ�
		ADE_OnceHaveEquip_Error,	// �Ѿ���װ���ˣ�����ж�²�������װ��
		ADE_REFINE_GRADE_ERROR,	// װ������	��������
		ADE_No_Grade,			// �˵��߲�������
		ADE_REFINE_STAR_ERROR,	// װ������
		ADE_UPDATE_LEVEL_ERROR,	// װ������
		ADE_RESET_Color,		// �������ð�ɫװ��
		ADE_RFRESH_Color,		// ����ˢ�°�ɫװ��
		ADE_NORESET,			// ���ܼ���
		ADE_NORFRESH,			// ����ˢ��
		ADE_REFINE_STAR_FULL,	// װ���ϵ������Ѿ�����
		ADE_REFINE_STAR_Equip,	// ��װ����������
		ADE_REFINE_ReSTAR_Empty,// װ��û��������,���в���ϴ��
		ADE_REFINE_ReSTAR_Equip,// ��װ������ϴ��
		ADE_EQUIP_NoMakeHole,	// ��װ�����ܴ��
		ADE_EQUIP_MakeHoleFUll,	// ��װ��������
		ADE_EQUIP_MakeGemNoEmpty,	// ���뱦ʯ��ʱ��û���ҵ��յ�λ��
		ADE_EQUIP_NoFindGem,		// ��װ��û���Ѿ���Ƕ�˱�ʯ�ĵط�
		ADE_EquipColor_Less,	// ��ɫ����
		ADE_EQUIP_NOMakeDECOMPOSITION, // װ�����ֽܷ�
		ADE_EQUIP_NOMakeSmelting, // װ������
	};
	BYTE Flag;
};
// ��������Ľṹ
struct MetrialItem
{		
	MetrialItem(DWORD dwParamID = 0, WORD wParamSuccessRate = 10000, BYTE byParamNum = 0, BYTE	byParamMetrialType = 0)
	{
		dwID = dwParamID; wSuccessRate = wParamSuccessRate; byNum = byParamNum; byMetrialType = byParamMetrialType;
	}
	DWORD	dwID; // ����ID
	WORD	wSuccessRate; // �ɹ���
	BYTE	byNum; // ��������
	BYTE	byMetrialType;	// ��������	
};	
DECLARE_MSG(SSNeedMETRIAL, SItemBaseMsg, SItemBaseMsg::EPRO_STRENGTHEN_NeedMETRIAL)
struct SANeedMETRIAL : public SSNeedMETRIAL
{
	SANeedMETRIAL()
	{
		bType = 0;
		MoneyType = 0;
		NeedMoney = 0;
		wSuccessRate = 10000;
		byMetrialNum = 0;
		memset(&aMetrialItem, 0, sizeof(aMetrialItem));
	}
	static const int msc_iMetrialMaxLen = 10;

	enum TMETR 
	{
		EMETR_NEED_COMMON = 0, // ��ͨ��������
		EMETR_NEED_PROTECTION, // ������������
		EMETR_NEED_LUCKYCHARM, // ���˷�����
		//EMETR_NEED_MONEY,	 // ��Ҫ����Ǯ����ʱ�ṹ�е�dwID-��ʾǮ��������byNum-��ʾ��
		EMETR_PRODUCE_COMMON, // ������ͨ����
		EMETR_PRODUCE_SPECIAL, // �����������
	};
	
	BYTE  bType;			// װ��ǿ������ ͬ(SQAddEquip Э��ö������)
	BYTE  MoneyType;		// 1������ 2 ������ Ǯ����
	DWORD NeedMoney;		// ��Ҫ��Ǯ
	WORD  wSuccessRate;		// [2012-8-17 +�ɹ�����(û�а��������˷�ʱ���ܳɹ���)]

	BYTE  byMetrialNum; // Ŀǰ����Ŀ 
	MetrialItem	aMetrialItem[msc_iMetrialMaxLen]; // �洢����������߲���
};

// ���������Ϣ
DECLARE_MSG(SSAddMetrial, SItemBaseMsg, SItemBaseMsg::EPRO_ADD_METRIAL)
struct SQAddMetrial : public SSAddMetrial
{
	enum
	{
		ADM_RAW_METRIAL,			// ����ԭ����
		ADM_PROTECT_METRIAL,		// ���ױ�������
		ADM_RATE_METRIAL,			// ���ʲ���
	};
	const static WORD msc_wMaxLen = 32; // [2012-9-17 11-38 gw: +Ŀǰ���������Ӹ���]

	//BYTE bX;		// ����
	//BYTE bY;
	BYTE bType;		// ��������	
	BYTE byPosNum; // ��ǰ��Ч����

	WORD waPos[msc_wMaxLen]; // �洢λ���б�x * PACKAGEWITH + y
};
struct SAAddMetrial : public SSAddMetrial
{
	enum
	{
		ADD_SUCCESS,
		ADD_INLAY_NoGEM,	//���ǲ��뱦ʯ��Э�鷢����
		ADD_INLAY_Error,	//���ڵ�״̬ʱ���뱦ʯ�������㷢����������
		ADD_INLAY_EQUIP,	//�򶯵�װ��Ϊ��
		ADD_INLAY_XY,		//����������λ��û���ҵ�
		ADD_INLAY_XY_LOCK,	//����������λ����������
		ADD_NoFindGEM,		//û���ҵ�������ĵ���ID�ڽű�����
		ADD_NoGEMITEM,		//�����ĸ�����������ĵ��߲��Ǳ�ʯ����
		ADD_GemNoInsert,	//��ʯ���ܲ���
	};

	BYTE Result;
	WORD wCellPos; // ����
};
// ����--��ʼ������Ϣ
DECLARE_MSG(SSBeginRefineGrade, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_REFINE_GRADE)
struct SQBeginRefineGrade : public SSBeginRefineGrade
{
	//byte SelectType;// 0 û��ѡ�� 1 �ɹ��� 2 ����ʯ
	byte byStrengthenExternChoose; // �Ƿ�ѡ�����˷����������ⲿ״̬���ο�TStrengthenExternChoose	
};

struct SABeginRefineGrade : public SSBeginRefineGrade
{
	enum
	{
		BRG_SUCCESS,		// ���׳ɹ�
		BRG_NOMONEY,		// ��Ǯ����
		BRG_FAIL,			// ����ʧ��
		BRG_DESTROY,		// ����ʧ�ܲ�����װ��
		BRG_LACK_METRIAL,	// ȱ�ٲ���
		BRG_YINBING,		// ȱ������
		BRG_YINGLIANG,		// ȱ������
		BRG_GRADE_FULL,		// ���׵�������
	};

	BYTE result;
};

// ����--��ʼ������Ϣ
DECLARE_MSG(SSBeginRefineStar, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_REFINE_STAR)
struct SQBeginRefineStar : public SSBeginRefineStar
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE	 bCount;	//ǿ������
};

struct SABeginRefineStar : public SSBeginRefineStar
{
	enum
	{
		BRS_SUCCESS,		// �ɹ�
		BRS_NOMONEY,		// ��Ǯ����
		BRS_FAIL,			// ʧ��
		BRS_LACK_METRIAL,	// ȱ�ٲ���
		BRS_REFINE_STAR_FULL,
	};

	BYTE result;
};
DECLARE_MSG(SSBeginReMoveStar, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_ReMove_STAR)// ϴ��
struct SQBeginReMoveStar : public SSBeginReMoveStar
{
	
};
struct SABeginReMoveStar : public SSBeginReMoveStar
{
	enum
	{
		ABS_SUCCESS,		// �ɹ�
		ABS_NOMONEY,		// ��Ǯ����
		ABS_FAIL,			// ʧ��
		ABS_LACK_METRIAL,	// ȱ�ٲ���
		ABS_REFINE_STAR_Empty,
	};
	BYTE result;
};

// ��ʼ��Ʒ��
DECLARE_MSG(SSStartQuality, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPGRADE_QUALITY)
struct SQStartQuality : public SSStartQuality
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	//byte byStrengthenExternChoose; // �Ƿ�ѡ�����˷����������ⲿ״̬���ο�TStrengthenExternChoose
};

struct SAStartQuality : public SSStartQuality
{
	enum
	{
		SSQ_SUCCESS,		// ��Ʒ�ʳɹ�
		SSQ_NOMONEY,		// ��Ǯ����
		SSQ_FAIL,			// ��Ʒ��ʧ��
		SSQ_LACK_METRIAL,	// ȱ�ٲ���
		SSQ_LACK_EUQIT,		// ȱ����������
		SSQ_YINBING,		// ���Ҳ���
		SSQ_YINGLIANG,		// ��������
		SSQ_EQUITFail,		// װ���ٻ�
	};

	BYTE  result;
	DWORD index;            //�ɹ�������Ʒ��ID
};

// װ��ǿ���ȼ�
DECLARE_MSG(SSBeginEquipLevelUpMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPGRADE_LEVEL)
struct SQBeginEquipLevelUpMsg : public SSBeginEquipLevelUpMsg
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE	 bCount;	//ǿ������
};

struct SABeginEquipLevelUpMsg : public SSBeginEquipLevelUpMsg
{
	enum
	{
		ELU_SUCCESS,		// �����ɹ�
		ELU_NOMONEY,		// ��Ǯ����
		ELU_FAIL,			// ����ʧ��
		ELU_LACK_METRIAL,	// ȱ�ٲ���
		ELU_LACK_EQUIP,		// ȱ��װ��
		ELU_NOUPDTE_Level,	// ��������
		ELU_YINBING,		// ���Ҳ���
		ELU_YINGLIANG,		// ��������
		ELU_EQUITFail,		// װ���ٻ�
		ELU_COLDTIME,	//��ȴʱ����
	};
	BYTE  result;
	
};

// װ������
DECLARE_MSG(SIdentifyMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY)
struct SQBeginIdentifyMsg : public SIdentifyMsg
{
	SQBeginIdentifyMsg()
	{
		for (int i = 0; i < SEquipment::MAX_BORN_ATTRI; ++i)
		{
			baLockedExtAttri[i] = false;
		}
		bPriorityUseBindMoney = false;
	}
	enum
	{
		QBIM_JINGLIAN,		// װ�������������������ԣ�
		QBIM_XILIAN,		// װ��ϴ��������������ɸ������ԣ�
		QBIM_FENJIE,		// װ���ֽ�
		QBIM_SMELTING,		// װ������
		QBIM_LINGFU,		// װ���鸽		
		QBIM_SHENGJIE,		// װ�����ף������������Ե����ޣ�		
	};

	BYTE type;
	bool baLockedExtAttri[SEquipment::MAX_BORN_ATTRI]; // [2012-8-23 16-16 gw: +��¼�������Ե�����״̬] 
	byte bPriorityUseBindMoney; // [2012-8-23 16-39 gw: +�Ƿ�����ʹ�ð󶨻���]
};

struct SABeginIdentifyMsg : public SIdentifyMsg
{
	enum
	{
		BIM_SUCESS,			// �ɹ�����
		BIM_NOMONEY,		// ȱ�ٽ�Ǯ
		BIM_MISMATCHING,	// �������ϲ�ƥ��
		BIM_NOEQUIP,		// û�м���װ��
		BIM_FAIL,
		BIM_DefineMetrial_Less,//�������ϲ���
		BIM_LockedItem_Less, // ����ʯ����
		BIM_JINGLIAN_FULL, // ��������
		BIM_PACKAGE_FULL, // ��������
		BIM_STAR_LESS,	// ��������
		BIM_LEVEL_LESS, // װ���ȼ�����
	};

	BYTE result; // ���ؽ��
	BYTE type; // ϴ��-����������
};

// ���͸��ͻ���ϴ��֮���װ����Ϣ,���װ��Ŀǰ�ǻ�û���滻
DECLARE_MSG(SIdentifySynEquipMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY_SYNEQUIP)
struct SAUpdateIdentifyedEquipMsg : public SIdentifySynEquipMsg
{
	SEquipment newExternEquip; // ͬ���µ�װ������
};

// ϴ�������󽻻�װ��
DECLARE_MSG(SIdentifyExChangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_IDENTIFY_EXCHANGE)
struct SQIdentifyExchangeMsg : public SIdentifyExChangeMsg
{	
	BYTE type;
};
struct SAIdentifyExchangeMsg : public SIdentifyExChangeMsg
{	
	enum
	{
		IEM_SUCCESS, // ϴ�������ɹ�
		IEM_FAIL,
	};
	BYTE result;
};

// װ����������
// DECLARE_MSG(SResetAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_RESET_ATTRIBUTE)
// struct SQBeginResetAttribute : public SResetAttribute
// {
// 
// };
// 
// struct SABeginResetAttribute : public SResetAttribute
// {
// 	enum
// 	{
// 		BRA_SUCESS,
// 		BRA_NOMONEY,
// 		BRA_MISMATICHING,	// ���ò��ϲ�ƥ��
// 
// 		BRA_FAIL,
// 	};
// 
// 	BYTE result;
// };

// ȡ��ǿ��
DECLARE_MSG(SSEndRefineMsg, SItemBaseMsg, SItemBaseMsg::EPRO_END_STRENGTHEN)
struct SQEndRefineMsg : public SSEndRefineMsg
{
	enum
	{
		ERM_CLOSE,				// �ر����
		ERM_EQUIP_OFF,			// ȡ��װ��
		ERM_METRAIL_OFF,		// ȡ��ԭʼ����
	};

	BYTE bType;
	BYTE bPosX;	// ����
	BYTE bPosY; // ����
};
struct SAEndRefineMsg : public SSEndRefineMsg
{
	enum
	{
		ERM_SUCCESS, // �رջ���ȡ�����ȡ��ԭ���ϳɹ�
		ERM_FAIL, // ����ʧ��
	};

	BYTE bType;
	BYTE bPosX;	// ����
	BYTE bPosY; // ����
	BYTE bResult; // ���ز������
};


// ���ͱ�����װ������Ϣ
DECLARE_MSG(SSEquipInfoInBagMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_INFO_IN_BAG)
struct SAEquipInfoInBagMsg : public SSEquipInfoInBagMsg
{
	SEquipment	equip;
	WORD		wPos; // �������λ��	
};

// ��ʾǿ�����װ��
DECLARE_MSG(SSAfterUpgradeEquipMsg, SItemBaseMsg, SItemBaseMsg::EPRO_AFTER_UPGRADE_EQUIP)
struct SAAfterUpgradeEquipMsg : public SSAfterUpgradeEquipMsg
{
	enum
	{
		UPDATE_LEVEL,
		UPDATE_GRADE,		//����		
		UPDATE_QUALITY,	
		UPDATE_STAR,		// ǿ��
		UPDATE_ReMove,		//ϴ��
		EQUIP_RESET,	 // װ��ϴ��
		EQUIP_REFINE,	 // װ������
		ADD_HOLE,		 // װ�����
		GEM_INSERT,		 // װ����Ƕ��ʯ
		GEM_REMOVE,		 // װ����ʯժ��
	};

	SEquipment	equip;
	BYTE		type;
};



// ʹ��������ߣ�Ŀǰ���ڴ���������װ������ʱ
DECLARE_MSG(SpecialUseItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_SPECIAL_USEITEM)
struct SQSpecialUseItemMsg : public SpecialUseItemMsg
{
	enum
	{
		USEFOR_XIAKE,			// ������ʹ��
		USEFOR_MOUNT,			// ������ʹ��
		USEFOR_NPC,				// ��NPCʹ��
		USEFOR_MONSTER,			// �Թ���ʹ��
		USEFOR_PLAYER,			// �����ʹ��
		USEFOR_NULL,
	};
	BYTE btype;					// ����
	WORD wCellPos;				// ����	
	WORD windex;				// ���͵��������Ժ��������������
};

// ����װ���׵���Ϣ
DECLARE_MSG(SFightPetEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::RPRO_FIGHTPET_EQUIPINFO)
struct SAFightPetEquipInfoMsg : public SFightPetEquipInfoMsg
{
	SEquipment	stEquip;
	BYTE		byPos;
	BYTE		index;			// ��������
};

// ����װ��һ�����ɼ�����
DECLARE_MSG(SFightPetEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FIGHTPET_EQUIPITEM)
struct SAFightPetEquipItemMsg : public SFightPetEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE			byEquipPos;
	DWORD	wItemIndex;
	BYTE		index;			// ��������
};

// ��װ��������Ҽ�ж������װ��
DECLARE_MSG(SfpUnEquipItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_UNLOAD_FIGHTPETEQUIP)
struct SQfpUnEquipItemMsg : public SfpUnEquipItemMsg
{
	DWORD	dwGlobalID;
	BYTE	byEquipPos;
	BYTE		index;			// ���͵�����
};

// XYD3���а��в鿴������ҵ�װ����Ϣ
DECLARE_MSG(SRankEquipInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_RANK_EQUIPMENT_INFO)
struct SAankEquipInfoMsg : public SRankEquipInfoMsg
{
	char		szName[CONST_USERNAME];				// ����	OK
	WORD		wOtherLevel;			// ��ҵȼ�OK
	SEquipment	Equip[16];

	DWORD		dwGlobal;				// GID
	BYTE		bFaceID;				// ��ģ��
	BYTE		bHairID;				// ͷ��ģ��
	DWORD		m_3DEquipID[11];			// 3Dװ���Ҽ���Ʒ��

	float		m_fPositionX;			// λ��x
	float		m_fPositionY;			// λ��y
	float		m_fDirection;			// ����

	BYTE		bySex:1;			    // ��ҽ�ɫ�Ա�
	BYTE		byBRON:3;				// �������ʾͼƬ����==1��ʾ��ʦ
	BYTE		bySchool:4;				// �����������OK
};

//����ת��Ϊ��Ӧ����
DECLARE_MSG(SfpToitemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_FIGHTPET_FIGHTPETTOITEM)
struct SQfpToitemMsg : public SfpToitemMsg
{
	BYTE		index;			// ���͵�����
};

// ȡ��ϴ��
DECLARE_MSG(SSEndResetPoint, SItemBaseMsg, SItemBaseMsg::EPRO_END_RESETPOINT)
struct SQEndResetPoint : public SSEndResetPoint
{
};

// װ�������Է����仯
DECLARE_MSG(SequitattrichangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_ATTRICHANGE)
struct SAequitattrichangeMsg : public SequitattrichangeMsg
{
	enum 
	{
		EQUIP_GONGJI,	// ����
		EQUIP_FANGYU,	// ����
		EQUIP_BAOJI,		// ����
		EQUIP_SHANBI,	// ����
	};	
	BYTE		index;				// �ı������
	WORD  changevalue;	//�ı��ֵ
};

// =================����ǿ���������==================
DECLARE_MSG(SDragUpdateItemMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_UPDATE_ITEM)
struct SQDragUpdateItemMsg : public SDragUpdateItemMsg
{
	enum UPDATE_TYPE
	{
		DUI_HUISHENDAN,

		DUI_NONE,
	};

	WORD wCellPos; // ����
	BYTE bType;		// ������������
};

DECLARE_MSG(SDragUpdateMetrialMsg, SItemBaseMsg, SItemBaseMsg::EPRO_DRAG_UPDATE_METRIAL)
struct SQDragUpdateMetrialMsg : public SDragUpdateMetrialMsg
{
	enum
	{
		DUM_LIANZHI,
		DUM_XUANJING,
	};

	WORD wCellPos;		// ���ʲ�������
	BYTE bType;		// ���߲�������
};

DECLARE_MSG(SBeginUpdateItem, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_UPDATE_ITEM)
struct SQBeginUpdateItem : public SBeginUpdateItem
{

};
struct SABeginUpdateItem : public SBeginUpdateItem
{
	enum
	{
		BUI_SUCCESS,
		BUI_FAILED,
		BUI_MAX_TIMES,
	};

	WORD	wCellPos; // ����

	BYTE	bType;
	BYTE	bSuccessTimes;
	BYTE	bTotalUpdateTimes;
};

DECLARE_MSG(SEndUpdateItem, SItemBaseMsg, SItemBaseMsg::EPRO_END_UPDATE_ITEM)
struct SQEndUpdateItem : public SEndUpdateItem
{
	enum
	{
		EUI_ITEM_OFF,
		EUI_LJS_OFF,
		EUI_XMTJ_OFF,
	};

	BYTE bType;
};

DECLARE_MSG(SHuiShenDanListInfoMsg, SItemBaseMsg, SItemBaseMsg::EPRO_HUISHENDAN_LIST_INFO)
struct SAHuiShenDanListInfoMsg : public SHuiShenDanListInfoMsg
{
	BYTE buffer[1024];
};

#define MAX_PACK_BUFFER 10240
//���ʹ����Ϣ���ͻ���,	�������Ͳ�ͬ��������һЩ��ƷID��Ҳ������������������������� 	
DECLARE_MSG(SBufferPackMsg, SItemBaseMsg, SItemBaseMsg::EPRO_PACK_BUFFERPACK)
struct SABufferPackMsg : public SBufferPackMsg
{
	enum
	{
		PACK_COUNTDOWNGIFT,	//����ʱ���
		PACK_ONLINEGIFT,	//����������
		PACK_ROSERECORD, //���ѵ��ͻ���¼
		PACK_LEVELGIFT,	//�ȼ��������
		PACK_LOGINGIFT,	//������½���
	};
	BYTE		PackType;			//����
	WORD Packnum;			//  ����
	BYTE		PackBuff[MAX_PACK_BUFFER];
};

DECLARE_MSG(SGetOnlienGiftMsg, SItemBaseMsg, SItemBaseMsg::EPRO__GET_ONLINEGIFT)
struct SQGetOnlienGiftMsg : public SGetOnlienGiftMsg
{
	enum
	{
		PACK_COUNTDOWNGIFT,	//����ʱ���
		PACK_ONLINEGIFT,	//����������
		PACK_ROSERECORD, //���ѵ��ͻ���¼
		PACK_LEVELGIFT,	//�ȼ��������
		PACK_LOGINGIFT,	//������½���
	};
	BYTE		PackType;			//����
	BYTE		index;				//����
};

struct SAGetOnlienGiftMsg : public SGetOnlienGiftMsg
{
	enum
	{
		SENDGIFT_FAIL,//�����ȡʧ��
		SENDGIFT_OK,//�����ȡ�ɹ�
		SENDGIFT_ALREADY,//��������Ѿ���ȡ�˻����Ѿ���ȡ�˵�ǰ�ȼ���
		SENDGIFT_ALLSEND,//��������Ѿ�ȫ����ȡ
	};
	BYTE bResult;
	BYTE	 PackType;			//����
	BYTE index;		//����
};


// ʱװģʽ������ͨģʽ���л�
DECLARE_MSG(SFashionChangeMsg, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_FASHIONCHANGE)
struct SQFashionChangeMsg : public SFashionChangeMsg
{
	enum{NORMALMODE,FASHIONMODE};
	BYTE mode;
};

struct SAFashionChangeMsg : public SFashionChangeMsg
{
	enum{NORMALMODE,FASHIONMODE};
	BYTE mode;
	DWORD playergid;
};

// ��װ���������Ƿ񼤻�
DECLARE_MSG(SSuitAttribute, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIP_SUIT_ATTRIBUTE)
struct SQSuitAttributeMsg : public SSuitAttribute
{

};

struct SASuitAttributeMsg : public SSuitAttribute
{
	// װ����װ�ӳ�[ͬ��.ͬ��.ͬƷ��.10�����ϱ�ʯ.ȫ������]
	// ��CItemDefine�ļ��е� ��װ������������ ö�ٶ�Ӧ
	enum
	{
		EQUIP_SAME_LEVEL = 1,
		EQUIP_SAME_GRADE,
		EQUIP_SAME_COLOR,
		EQUIP_SAME_JEWEL,
		EQUIP_SAME_STAR,
	};

	// ��16Ϊ�޷��Ŵ洢״̬������ܹ�����16������״̬״̬
	WORD bSuitAttriState;
};

DECLARE_MSG(SChannelCallSellInfo,SItemBaseMsg,SItemBaseMsg::EPRO_ChannelCallSell_INFO)
struct SQChannelCallSellInfo : public SChannelCallSellInfo
{
	WORD wCellPos;			// ��������
	long AllPrice;			// �ܼ�(����)
	BYTE NoticeRoll;		// ��ʾ���� 0 ����ʾ 1 ��ʾ
};

struct SAChannelCallSellInfo : public SChannelCallSellInfo
{
	BYTE ErrorCode;			// 1(�ɹ�) 2(Ǯ������) 3(�����������п���Խ��) 4(����Ϊ��) 5(�����Ѿ�����) 6() 
};

DECLARE_MSG(SChannetCallBuyInfo,SItemBaseMsg,SItemBaseMsg::EPRO_ChannetCallBuy_INFO)
struct SQChannetCallBuyInfo : public SChannetCallBuyInfo
{
	DWORD m_PlayerGID;		// ���GID
	WORD  m_Pos;			// λ��
};

struct SAChannetCallBuyInfo : public SChannetCallBuyInfo
{
	BYTE ErrorCode;			// 1(�ɹ�) 2(û���ҵ������ң�����Ѿ�����) 3(������λ�ò���)
};

DECLARE_MSG(SSendReward,SItemBaseMsg,SItemBaseMsg::EPRO__SEND_REWARD)
struct SQSendReward : public SSendReward
{
	enum RewardType			//��������
	{
		TOTALLOGIN,	//�ۼƵ�¼
		CONTINLOGIN,	//������¼
		CHENGJIUREWARD,//�ɾ�
		LEAVEREWARD,//����
		BUCHANGREWARD,//����
		CHONGZHIREWARD,//��ֵ
	};
	BYTE	 bRewardtype;	//��������
	WORD	wRewardIndex;//������ȡ��ID
};

struct SASendReward : public SSendReward
{
	enum RewardType			//��������
	{
		TOTALLOGIN,	//�ۼƵ�¼
		CONTINLOGIN,	//������¼
		CHENGJIUREWARD,//�ɾ�
		LEAVEREWARD,//����
		BUCHANGREWARD,//����
		CHONGZHIREWARD,//��ֵ
	};
	enum GETTYPE
	{
		NOTGET,	//û����ȡ �Ѿ�����������û����ȡ
		ISGET,	//�Ѿ���ȡ
		NONEGET,//�޷���ȡ
	};
	BYTE	 bRewardtype;	//��������
	WORD	wRewardNum;//��������
	int	nRewardData;//��������
};

//��ǿ��ǰ����ǿ������
DECLARE_MSG(SIntensifyInfo,SItemBaseMsg,SItemBaseMsg::EPRO_INTENSIFY_INFO)
struct SQIntensifyInfo : public SIntensifyInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

//�������ظ�ǿ��������Ϣ
struct SAIntensifyInfo : public SIntensifyInfo
{
	WORD		bIntensifysuccessrate; //ǿ���ɹ���
	WORD		bupgradrate;// ���׳ɹ���
	WORD		bnextupgradrate;// ��һ�ȼ����׳ɹ���
	WORD		wNextLevelattriValue;//ǿ������һ�ȼ�������ֵ
	DWORD dcoldtime;		//�Ѿ�ʹ�õ�ǿ����ȴʱ��
	DWORD dallcoldtime;		//����ǿ����ȴʱ��
	DWORD dmoneycost;//�������һ���
	BYTE       bCanIntensify;			//�ܷ�ǿ�� 0 ��
};

//�����л�
DECLARE_MSG(SEquipSwitch,SItemBaseMsg,SItemBaseMsg::EPRO__EQUIP_SWITCH)
struct SQEquipSwitch : public SEquipSwitch
{
	BYTE bIndex;		//	����1
	BYTE bIndex2;		//	����2    ����1���������л�������2��
};

struct SAEquipSwitch : public SEquipSwitch
{
	enum
	{
		SUCCESS,   
		FAIL,			//ʧ�ܣ����ӿ��ܴ�������״̬
	};
	BYTE bresule;
};
//��ȡ����Ʒ�ʵ�����
DECLARE_MSG(SEquipQualityInfo, SItemBaseMsg, SItemBaseMsg::EPRO_QUALITY_INFO)
struct SQEquipQualityInfo : public SEquipQualityInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

//�������ظ�Ʒ��������Ϣ
struct SAEquipQualityInfo : public SEquipQualityInfo
{
	DWORD	dnextqualityitem;	//����Ʒ�ʳɹ�����һ�׵ĵ���
	DWORD	dmoneycost;//�������һ���
	WORD		bQualitysuccessrate; //����Ʒ�ʳɹ���
	WORD      wneedLevel;//��Ҫ�ȼ�
	BYTE			bmaterial[512];	//������Ҫ itemid + num
};

//���߳���
DECLARE_MSG(SItemSale, SItemBaseMsg, SItemBaseMsg::EPRO__ITEM_SALE)
struct SQItemSale : public SItemSale
{
	BYTE bIndex;		//	�����е�����
};

struct SAItemSale : public SItemSale
{
	enum
	{
		ITEMSALE_OK,			//�ɹ�
		ITEMSALE_FAILE,		//ʧ�ܿ��ܴ�������״̬�����ߵ�ǰ���߲�������۵ȴ�
	};
	BYTE bResult;//���۽��
};

//���ǿ����ȴʱ�� 
DECLARE_MSG(SCleanColdtime, SItemBaseMsg, SItemBaseMsg::EPRO_CLEAN_COLDTIME)
struct SQCleanColdtime : public SCleanColdtime
{
};

struct SACleanColdtime : public SCleanColdtime
{
	BYTE bresult;
};

//�������� 
DECLARE_MSG(SRiseStarInfo, SItemBaseMsg, SItemBaseMsg::EPRO_RISE_INFO)
struct SQRiseStarInfo : public SRiseStarInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

struct SARiseStarInfo : public SRiseStarInfo
{
	WORD starpropertyid;//���Ǻ��װ������ID
	WORD starvalue;//���Ǻ������ֵ
	WORD		bsuccessrate; //���ǳɹ���
	WORD dmaterialnum;//��������
	DWORD dmaterialid; //���ϵ���ID
	DWORD dmoney;//�������������
};

//����ʼ������ӡ����
DECLARE_MSG(SBeginKeYin, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYIN)
struct SQBeginKeYin : public SBeginKeYin
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bPropertyIndex;	//��ӡ��������
};
struct SABeginKeYin : public SBeginKeYin
{
	enum KEYINRESULT
	{
		KEYIN_SUCCESS,  //��ӡ�ɹ�
		KEYIN_FAILE,  //��ӡʧ��
	};
	BYTE bResult;		//	���
};


//�����ӡ��������
DECLARE_MSG(SKeYinInfo, SItemBaseMsg, SItemBaseMsg::EPRO_KEYIN_INFO)
struct SQKeYinInfo : public SKeYinInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bPropertyIndex;	//��ӡ��������
};

struct SAKeYinInfo : public SKeYinInfo
{
	WORD		bsuccessrate; //��ӡ�ɹ���
	WORD dmaterialnum;//��������
	DWORD dmaterialid; //���ϵ���ID
	DWORD dmoney;//�������������
};

////�����ӡ����ת��
DECLARE_MSG(SKeYinChange, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYINCHANGE)
struct SQKeYinChange : public SKeYinChange
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bPropertyIndex;	//׼�����ĵĿ�ӡ��������
	BYTE bTargetIndex;	//Ŀ���ӡ��������
	BYTE	 bChangeType;//ת������ 0Ϊʹ�õ���ת�� 1Ϊʹ������ת��
};

////�����ӡ����ת����Ϣ
DECLARE_MSG(SKeYinChangeInfo, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_KEYINCHANGEINFO)
struct SQKeYinChangeInfo : public SKeYinChangeInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bPropertyIndex;	//׼�����ĵĿ�ӡ��������
};
struct SAKeYinChange : public SKeYinChangeInfo
{
	WORD dmaterialnum;//��������
	DWORD dmaterialid; //���ϵ���ID
	DWORD dmoney;//�������������
	BYTE bPropertyIndex[3];	//׼�����ĵĿ�ӡ����
};


////����װ������
DECLARE_MSG(SEquipJDing, SItemBaseMsg, SItemBaseMsg::EPRO_BEGIN_JDING)
struct SQEquipJDing : public SEquipJDing
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bPropertyIndex;	//׼����������������
	BYTE bIslock;//�Ƿ�������ǰ�ļ�������
};

struct SAEquipJDing : public SEquipJDing
{
	BYTE bPropertyIndex;	//׼����������������
	BYTE bPropertyID;//����ID
	WORD wvalue;//����ֵ
	BYTE	 bColor; //������ļ�������ʾ����ɫ
};

//����װ��������Ϣ
DECLARE_MSG(SEquipJDingInfo, SItemBaseMsg, SItemBaseMsg::EPRO_JDING_INFO)
struct SQEquipJDingInfo : public SEquipJDingInfo
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

struct SAEquipJDingInfo : public SEquipJDingInfo
{
	WORD dmaterialnum;//������������
	WORD dlockmaterialnum;//����������������
	DWORD dmaterialid; //���ϵ���ID
	DWORD dlockmaterialid; //�������ϵ���ID
};
#define COLORNUM 6	//��ɫ����
//ͬ��װ����������ɫ��Ϣ
DECLARE_MSG(SEquipJDingColor, SItemBaseMsg, SItemBaseMsg::EPRO_JDING_COLOR)
struct SQEquipJDingColor : public SEquipJDingColor
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

struct SAEquipJDingColor : public SEquipJDingColor
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
	BYTE bcolorIndex[COLORNUM];		//	����
};

///2014.2.20 Add
//ÿ���������Ӧ������ 
DECLARE_MSG(SQiDaoInfo, SItemBaseMsg, SItemBaseMsg::EPRO_EVERYDAY_ALLQIDAO)
struct SQQiDaoInfo : public SQiDaoInfo
{
};

struct SAQiDaoInfo : public SQiDaoInfo
{
	WORD wSilerCoinGetCount; //ÿ�ι������ҿɻ�õ�������
	WORD wSilerCoinPrice; //���ҵĵ���
	WORD wSilerCoinCanNum; //ÿ����Ի�ȡ���ҵĴ���
	WORD wSilerCoinUsedNum;	//���������Ѿ���õĴ�����ÿ��24Сʱ�󣬸ô�������Ϊ0
	WORD wAnimaGetCount;
	WORD wAnimaPrice;
	WORD wAnimaCanNum;
	WORD wAnimaUsedNum;
	WORD wExpGetCount;
	WORD wExpPrice;
	WORD wExpCanNum;
	WORD wExpUsedNum;
	WORD wNextVipCanNum;		//��һ��VIP���Ի�ȡ���ܴ���
};

//���������ص��������ҡ������;��飩
DECLARE_MSG(SAddQiDaoInfo, SItemBaseMsg, SItemBaseMsg::EPRO_EVERYDAY_ADDQIDAO)
struct SQAddQiDaoInfo : public SAddQiDaoInfo
{
	BYTE bQiDaoType;		//0��Ϊ����  1��Ϊ����  2��Ϊ����
};

struct SAAddQiDaoInfo : public SAddQiDaoInfo
{
	char cOptResult;	////-1: ��ʾ��������  0: ��ʾ��Ҳ���  1����ʾ�ɹ�
};

//2014.2.21 Add
//Ԫ�������������ݽṹ
DECLARE_MSG(SGiftPacketMsg, SItemBaseMsg, SItemBaseMsg::EPRO_GET_YUANBAOGIFTINF)
struct SQGiftPacketMsg : public SGiftPacketMsg
{
	DWORD dwRqItemPos;	//���������λ��
};

typedef struct _GiftPacketData
{
	unsigned int CurIDItemCount;	//��ǰ���ߵ�����

	unsigned int ItemID;	//���ߵ�ID
}GiftPacketData, *LPGiftPacketData;

struct SAGiftPacketMsg : public SGiftPacketMsg
{
	BYTE Isbuyed;	//�Ƿ��Ѿ�����1Ϊ����0Ϊû�й���

	BYTE TypeCount;	//�������͵�����

	DWORD IsExtraCell;	//�Ƿ��ж���ĵھŸ�����,�������Ϊ�������ID�ţ�Ϊ0��ʾû�еھŸ�����

	LPGiftPacketData lpGiftPacketData;

	SAGiftPacketMsg(BYTE ucTypeCount)
	{
		TypeCount = ucTypeCount;
		lpGiftPacketData = new GiftPacketData[TypeCount];
	}
	~SAGiftPacketMsg()
	{
		delete[] lpGiftPacketData;
	}
};

//����Ԫ�������������ݽṹ
DECLARE_MSG(SBuyGiftPacketMsg, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_ALLYUANBAOGIFT)
struct SQBuyGiftPacketMsg : public SBuyGiftPacketMsg
{
};

struct SABuyGiftPacketMsg : public SBuyGiftPacketMsg
{
	BYTE bResult;	//0�� ��Ҳ���  1�� ��������   2�� �ɹ�
};

//������װ����
DECLARE_MSG(SSuitcondition, SItemBaseMsg, SItemBaseMsg::EPRO_SUIT_CONDITION)
struct SQSuitcondition : public SSuitcondition
{
	BYTE btype;//��װ���� 1 Ʒ������ 2 �Ǽ����� 3��ӡ����
};

struct SASuitcondition : public SSuitcondition
{
	BYTE btype;//��װ���� 1 Ʒ������ 2 �Ǽ����� 3��ӡ����
	BYTE benough[EQUIP_P_MAX];//�Ƿ񵽴ﵱǰ����
	int nenoughproperty[3];//�����������ӵ�����ֵ
	int ncurrentproperty[3];//��ǰ���ӵ�����ֵ
	WORD wcurrentlevel;//�ﵽֵ��������
	WORD  wepropertyindex[3]; //�����������ӵ�����ID
	WORD  wcepropertyindex[3]; //��ǰ�������ӵ�����ID
};

DECLARE_MSG(SCellCountDown, SItemBaseMsg, SItemBaseMsg::EPRO_CELL_COUNTDOWN)
struct SQCellCountDown : public SCellCountDown
{
};

struct SACellCountDown : public SCellCountDown
{
	DWORD m_CountDown; //Ϊ��ǰ���ӵĵ���ʱʱ��
};

//������Ӧװ��ս����
DECLARE_MSG(SEquipPower, SItemBaseMsg, SItemBaseMsg::EPRO_EQUIPT_POWER)
struct SQEquipPower : public SEquipPower
{
	BYTE bIndex;		//	����
	BYTE bType;		// 0 Ϊ���װ����  1 ��ұ���
};

struct SAEquipPower : public SEquipPower
{
	DWORD dequippower;
	BYTE bIndex;		//	����
};

// ��ҹ�������
DECLARE_MSG(SBuyPlayerTp, SItemBaseMsg, SItemBaseMsg::EPRO_BUY_PLAYERTP)
struct SQBuyPlayerTp : public SBuyPlayerTp
{
};
