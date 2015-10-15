#pragma once
#include "../pub/ConstValue.h"
#include "HeroTypeDef.h"
#include <time.h>
#include "ConstValueSG.h"
//#include "PlayerTypedef.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////// 
///�����佫��������
///////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_HERO_NUM 100 ///<�佫���������
#define MAX_ITEM_NUM 400 ///< ��Ʒ�������
#define MAX_TOLLEGATE_NUM 160 ///<���Ĺؿ���
#define MAX_TREASUREFIND_TOLLGATE_NUM 6///<�����ؾ�Ѱ���ؿ���
#define MAX_LEAGUEOFLEGEND_TOLLGATE_NUM 18///<���Ľ���˵�ؿ���
#define MAX_LEAGUEOFLEGEND_DUPILICATE_NUM 3///<����˵�ĸ�����
//#define MONEY_BLESS_INDEX 0 ///<��Ǯ���±�
//#define DIAMOND_BLESS_INDEX 1 ///<Ԫ�����±�
#define  SWEEPTICKET_ID 20010	///<ɨ��ȯID
#define  MALL_COMMODITY_NUM 8	///<�̳���Ʒ����
#define  MISSION_NUM 15 ///<��ǰ����������
#define  NEWBIEGUIDE_NUM 20	///<��ǰָ������
#define  ACHIEVEMENT_NUM 168 ///<��ǰ�ĳɾ͵�����
#define  ACHIEVEMENT_GROUP_NUM 8///<��ǰ�ɾ����͵�����
#define  MAX_BOSS_DROPCOUNT 3///<boss �������Ʒ����������
#define	 MAX_ITEM_NUM_GAINED_FROM_CHEST 30 ///<����������ܻ�ö�����Ʒ
#define  MAX_ITEM_NUM_SYN_TO_CLIENT 128///<�ͻ��˸�������һ���Խ�������Ʒ����Ϣ�����������С����Ʒ�������Ļ�Ϊ64��һ���ID��һ������
#define  MAX_TOLLGATE_HERO_NUM 10 ///<�ؿ�������ж��ٸ��佫���Բ�ս
#define  MAX_TOLLGATE_MONSTER_NUM 16///<��ͨ��������Ӣ����������˵���ؾ�Ѱ���йؿ��Ĺ��������������
#define SecsOfDay 86400 ///<����һ�����ж�����
#define  EXTENDS_STATICDATAS_SIZE 5120 ///<��̬�ġ���չ�ԡ�C++�����ݣ�Ϊ�Ժ���¹�����������ռλ��
#define  MAX_SYN_SCRIPT_LENTH 256///<ͬ�����ͻ��˵�LUA����
#define  MAX_SYN_NOTIFICATION_LENGTH 16 ///<ͬ�����ͻ��˵�֪ͨ�����ݵ����ĳ���Ϊ16��
#define	 MAX_THIEF_REWARD_NUM 10	///<С͵��������Ϣ����
#define  MAX_THIEF_HERO_LIMIT 5		///<С͵��ս�����佫����
const int g_iExpeditionCharacterLimit = 5;///<Զ�������ϳ��佫����
const int g_iOneTypeEnmeyNumLimit = 3;///<Զ��ͬһ���Եĵ��˵���������

///@brief struct ������ҵ���Ʒ�ṹ�壬���Կ���ѹ����char[]���ѽṹ���С������4 byte����ƷID 8 bits, ��Ʒ����3bits �ָ���2bits
struct SSanguoItem
{
	DWORD m_dwItemID; ///< ��ƷID
	DWORD m_dwCount; ///<��Ʒ����
	SSanguoItem()
	{
		m_dwItemID = 0;
		m_dwCount = 0;
	}
};

///@brief struct ������
struct SBlessData
{
	//DWORD m_dwBlessCount; ///<������
	//DWORD m_dwBlessFreeCount; ///<���������
	//DWORD m_dwLastBlessDate; ///<�ϴ�������
	//DWORD m_dwIsFirstBless; ///<��һ�������
	DWORD m_dwDiamondblessCount; ///Ԫ���齱����
	DWORD m_dwMoneyBlessCount; ///ͭǮ�齱����
	DWORD m_dwDiamondFreeBlessCount = 1;  ///Ԫ������Ѵ���
	DWORD m_dwMoneyFreeBlessCount = 5; ///ͭǮ����Ѵ���
	DWORD m_dwLastDiamondFreeBlessDate; ///��һ�����Ԫ��ʱ��
	DWORD m_dwLastMoneyFreeBlessDate; ///��һ����ѽ�Ǯʱ��
	DWORD m_dwIsFirstMoneyBless; /// ��һ��ͭǮ�����
	DWORD m_dwIsFirstDiamondBless; /// ��һ��Ԫ�������
	SBlessData()
	{
		m_dwDiamondblessCount = 0; ///Ԫ���齱����
		m_dwMoneyBlessCount = 0; ///ͭǮ�齱����
		m_dwDiamondFreeBlessCount = 1;  ///Ԫ������Ѵ���
		m_dwMoneyFreeBlessCount = 5; ///ͭǮ����Ѵ���
		m_dwLastDiamondFreeBlessDate = 0; ///��һ�����Ԫ��ʱ��
		m_dwLastMoneyFreeBlessDate = 0; ///��һ����ѽ�Ǯʱ��
		m_dwIsFirstMoneyBless = 0; /// ��һ��ͭǮ�����
		m_dwIsFirstDiamondBless = 0; /// ��һ��Ԫ�������
	};
};

struct SSanguoTollgate
{
	DWORD m_dwStarLevel;
	DWORD m_dwChallengeTime;
	SSanguoTollgate()
	{
		m_dwStarLevel = 0;
		m_dwChallengeTime = 0;
	}
};

struct SSanguoStoryTollgate
{
	bool bFirstTime;
	SSanguoTollgate tollgateData;

	SSanguoStoryTollgate()
	{
		bFirstTime = true;
	}
};

struct SSanguoStoryEliteTollgate
{
	DWORD dwResetedTimes; ///�Ѿ������˶��ٴ�
	SSanguoTollgate tollgateData;

	SSanguoStoryEliteTollgate()
	{
		dwResetedTimes = 0;
	}
};

/**@brief �����̳ǵ���Ʒ����*/
struct SSanguoCommodity
{
	BYTE m_bSoldOut;
	DWORD m_dwID;
	DWORD m_dwCount;
	SSanguoCommodity()
	{
		m_dwID = 0;
		m_dwCount = 0;
		m_bSoldOut = FALSE;
	}

	void SetData(DWORD dwID, DWORD dwCount, BYTE bSoldOut)
	{
		m_dwID = dwID;
		m_dwCount = dwCount;
		m_bSoldOut = bSoldOut;
	}
};

/// <summary>
/// ��������
/// </summary>
enum InstanceType
{
	/// <summary>
	/// ���鸱��
	/// </summary>
	Story = 0,
	/// <summary>
	/// ���龫Ӣ����
	/// </summary>
	StoryElite,
	/// <summary>
	/// Զ������
	/// </summary>
	Expedition,
	/// <summary>
	/// ������
	/// </summary>
	Arena,
	/// <summary>
	/// ����˵
	/// </summary>
	LeagueOfLegends,
	/// <summary>
	/// �ؾ�Ѱ��
	/// </summary>
	TreasureHunting,
	/// <summary>
	/// ����ս
	/// </summary>
	TeamSige,
	/// <summary>
	/// ׷������
	/// </summary>
	ChaseThief,
	/// <summary>
	/// ��ս
	/// </summary>
	WarOfLeague,
};

///@brief ������������
enum SSanguoCurrencyType
{
	Currency_Money = 1,		//<���//
	Currency_Diamond = 2,	//<��ʯ//
	Currency_Honor = 3,		//<����//
	Currency_Exploit = 4,	//<����//
	Currency_Prestige = 5,	//<����//
	Currency_SoulPoints = 6,	//<���
	Currency_Token = 7,		//<������//
};

enum CheckResult     //������ƵĽ��//
{
	Failed = 0,                 //ʧ�� ����ԭ��//
	Pass,                   //ͨ��//
	NoEnoughStamina,        //��������//
	NoEnoughMoney,          //��Ǯ����//
	NoEnoughDiamond,        //��ʯ����//
	NoEnoughCD,             //CD����//
	NoEnoughTimes,          //��ս��������//
	NoEnoughTeamLevel,      //ս�ӵȼ�����//
};

//��ҵ�������
enum GoodsType
{
	money = 1, //ͭǮ
	diamond=2,  //��ʯ
	item=3, //����
	hero=4, //Ӣ��
	Exp=5,//����
	honor=6,//����
	exploit=7,//����
	endurance=8,//����
	level=9, ///�����ȼ�
	vipExp=10,//vip����
	Prestige=11,//��������ս����������
	blessPoints = 12,//��������������
	soulPoints = 13,//�ֽ⽫������Ļ��
	token = 14, //���Ų����ľ���
};
enum GoodsWay //��Ʒ��Դ�Լ���������,��־ʹ��,���ղ߻� "����������.xlsx"
{
	duplicate=1,	//����
	checkIn =2,		//ǩ��
	exploitMall=3,		//�����̳�,Զ���̵�
	honorMall=4,		//�����̳�,�������̵�
	findCowry = 5,		//�ؾ�Ѱ��	
	passCustoms = 6,	//����ն��	
	heroLegend= 7,		//����˵	
	practiceForce= 8,		//���䳡	(������)
	recharge = 9,		//��ֵ	
	task = 10,			//����
	achievement = 11,	//	�ɾ�	
	active = 12,			//�	
	mainCityActive = 13,	//���ǻ	(׷������)
	varietyMall = 14,	//�ӻ���	
	forgeMall = 15,		//������	
	bless = 16,			//��
	addskill = 17,		//�Ӽ���	
	buyskill = 18,		//�����ܵ���	
	luckMan = 19,		//��Ե����	
	gemMan = 20,		//�䱦����	
	goldenTouch= 21,	//�����	
	bugEndurance = 22,	//��������	
	cdtimeEndurance = 23,	//ʱ��ظ�(����)	
	itemUse = 24,		//����ʹ�û����
	LegionMall = 25,		//�����̵�,��������
	mail=26,//�ʼ���ȡ
	lvup=27,//��������
	fanctionWar = 28,//����ս
	giftVipexp = 29,		//����vip����ֵ
	hungingHunting = 30, //�佫�һ�Ѱ��
	heroTraining = 31, //�佫��ѵ��
	chargeRebate = 32, //��ֵ����
	changename = 33, //����
	warOfLeague = 34, ///��ս
	soulExchange = 35,//����һ�
	monthlyMembership = 36, //�¿�
	StaminaGift = 37, //������������
	exchangeACT = 38, //�һ��


};

///������һЩС�����ݵ����ͣ������Ӷ�Ӧ��lua���ݳ��л�ȡ��С������
enum SG_ExDataType
{
	MultiDiamondBlessCount = 1, //�����Ԫ��ʮ��������
	BlessMaxThreeStarHeroCount, //����ʮ����ص������佫��������
	ChargeClaimedFlag, //�׳���ȡ��ʶ
	TrainedFlag, //�״�ѵ���Ƿ���ɵı�ʶ��Ϊ0��ʾû��ʼ��Ϊ1��ʾ�Ѿ���ʼѵ����Ϊ2��ʾ�Ѿ���Ǯ�������ѵ����һ��
	NewbieGoldGainedFlag,///�״ν������Ƿ��������
	SendStaminaCount, ///������������
	GetStaminaCount, ///��ȡ��������
	ExpeditionTimes, ///<����ն��ͨ�عؿ�����
};

enum AchievementType
{
	Battle0 = 0,
	Battle1,
	Battle2,
	Battle3,
	Battle4,
	Advanced,
	Level,
	Collect,
};

/// <summary>
/// ������ɵĿɴ�ɵĳɾ͵�����
/// </summary>
enum RandomAchievementType
{
	/// <summary>
	/// ��
	/// </summary>
	Random_Blessing = 1,
	/// <summary>
	/// ����
	/// </summary>
	Random_Forging,
	/// <summary>
	/// ͨ�ظ���
	/// </summary>
	Random_Battle,
	/// <summary>
	/// ��������
	/// </summary>
	Random_SkillUpgrade,
	/// <summary>
	/// ����
	/// </summary>
	Random_Advanced,
	Random_Level,
	/// <summary>
	/// �ռ��佫
	/// </summary>
	Random_Collect,
};

enum AchievementRewardType
{
	ART_Hero = 0,
	ART_HeroDebris,
	ART_Diamond,
	ART_Equip,
	ART_Gold,
	ART_Property,
};

enum MissionRewardsType
{
	MR_DIAMOND, ///<������ʯ����vip�йҹ�
	MR_ENDURANCE, ///<��������
	MR_SWEEPTICKET, ///<����ɨ�����ʵ��MR_ITEM��һ����������������ĸ�����vip�ȼ��йҹ���ϵ�����ڴ˶�������
	MR_ITEM, ///<������Ʒ
	MR_COMMON, ///<����һЩͨ�õĶ���������˵��Ǯ�������
	MR_LUNCH, ///<��ͽ���
	MR_DINNER,///<��ͽ���
	MR_SUPPER,///<ҹ������
};

///���е���������
enum MissionType
{
	MT_MonthCard,	///<�¿�����
	MT_VIP,		///<ɨ��������
	MT_Lunch,		///<�������
	MT_Dinner,		///<�������
	MT_Supper,		///<ҹ������
	MT_Duplicate,	///<��������
	MT_EliteDuplicate,///<��Ӣ���¸�������
	MT_TreasureHunting,///<�ؾ�Ѱ����������
	MT_LeagueOfLegends,///<����˵��������
	MT_ArenaMission,		///<���䳡��������
	MT_SkillUpgrade,		///<������������
	MT_Forging,	///<��������
	MT_Blessing,	///<������
	MT_Expedition,	///<����ն����������
};

enum HeroType
{
	Power = 0,
	Intelligence,
	Agility,
};

///@brief С����������
enum DessertType
{
	Dessert_Gold = 1,
	DessertDiamond,
};

enum SSanguoItemType
{
	Item_None = -1,
	Item_ExpPotion,				//<����ҩˮ//
	Item_EnchantingMaterial,	//<��ħ����//
	Item_SellingGoods,			//<����Ʒ//
	Item_SweepTicket,			//<ɨ��ȯ//
	Item_TreasureChest,			///<����
	Item_EndurancePotion,		///<����ҩ��
	Item_SoulStone,				//<���ʯ//
	Item_Debris,				//<��Ƭ//
};

///@brief ��������
enum GuideType
{
	Newbie,				///<��������//
	UpgradeRank,		///<�佫����//
	SummonHero,			///<�ٻ�Ӣ��//
	LearnSkill,			///<���ܼӵ�//
	UnlockArena,		///<���ž�����//
	CollectEquipment,	///<Ӣ���ռ�����װ��//
	EliteDuplicate,		///<������Ӣ����//
	UnlockMall,				///<�����̵�//
	UnlockTreasureHunting,	///<�����ؾ�Ѱ��//
	UnlockLegend,				///<��������˵//
	UnlockForging,			///<����������(װ������)//
	UnlockExpedition,			///<��������ն��//
	AcquireGuanFeng,	///<��ȡ�ط�//
	AcquireXiaoQiao,	///<��ȡС��//
	AcquireChengYu,		///<��ȡ����//
	MaxCount = 20,			///<ָ�������������//
};

struct STimeData
{
	char m_LoginTime[8]; //�ϴ�����ʱ��
	char m_LogoutTime[8]; //�ϴ�����ʱ��
	char m_OnlineTime[8]; //����ʱ��
};
struct SExpeditionData
{
	/// <summary>
	/// �ɹ������������е�Զ������
	/// </summary>
	bool bSuccessToConqureWholeExpedtion;
	/// <summary>
	/// �齱�鵽Ӣ��
	/// </summary>
	bool bRewardedHero;
	/// <summary>
	/// �Ƿ��н���û��ȡ
	/// </summary>
	bool bRewardsUnclaimed;
	bool expeditionActived;
	/// <summary>
	/// ��ǰԶ��������ID
	/// </summary>
	DWORD curExpeditionInstanceID;
	/// <summary>
	/// ��ǰԶ����������ս����
	/// </summary>
	DWORD curChallengeTimes;
	/// <summary>
	/// ��ǰ�ջ����Ʒ
	/// </summary>
	DWORD curItemObtained;
	/// <summary>
	/// ��ǰ�Ĺؿ��Ѷ�ϵ��
	/// </summary>
	float levelFactor;
	/// <summary>
	/// ��ǰ�жԵ��佫��Ϣ
	/// </summary>
	SHeroData curHostileCharacterInfor[g_iExpeditionCharacterLimit];

	SExpeditionData()
	{
		bSuccessToConqureWholeExpedtion = false;
		bRewardedHero = false;
		bRewardsUnclaimed = false;
		expeditionActived = false;
		curExpeditionInstanceID = 0;
		curChallengeTimes = 0;
		curItemObtained = 0;
		levelFactor = 1;
	}
};

struct SSanguoMallData
{
	/// ��Ʒ����
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	/// �´�ˢ��ʱ���
	DWORD m_dwNextRefreshTime;
	/// �����ֶ�ˢ�´���
	DWORD m_dwRefreshedCount;
	/// �ϴ�ˢ��ʱ��
	char m_LastRefreshTime[8];
	/// �����´�ˢ��ʣ��ʱ��
	DWORD m_dwRefreshRemainingSeconds;
	
	SSanguoMallData()
	{
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
		m_dwNextRefreshTime = 0;
		m_dwRefreshedCount = 0;
		memset(m_LastRefreshTime, 0, sizeof(char) * 8);
		m_dwRefreshRemainingSeconds = 0;
	}
};

struct SSanguoSpecialMallData
{
	SSanguoMallData mallNormalData;
	/// ����ʱ��
	char activateTime[8];
	/// �����������ʱ��
	DWORD dwElapsedTime;
	/// �Ƿ���ʱ����
	bool bTemporaryOpened;
	/// �Ƿ�һֱ����
	bool bAlwaysOpened;

	SSanguoSpecialMallData()
	{
		//memset(&mallNormalData, 0, sizeof(SSanguoMallData));
		memset(activateTime, 0, sizeof(char) * 8);
		dwElapsedTime = 0;
		bTemporaryOpened = false;
		bAlwaysOpened = false;
	}
};

///@brief ���������ɾ͵���ɵ�����
struct SAchivementUnitData
{
	bool accompulished;///<�Ƿ����
	DWORD achievementID;///<�ɾ�ID
	DWORD completedTimes;///<��ɴ���
	DWORD groupType;///<�ɾ����ڵ����
};

///@brief ĳһ���ɾ͵�����
struct SAchivementGroupData
{
	bool accompulished;///<����ɾ��Ƿ��Ѿ����
	int GroupType;///<���������
	DWORD newestAchivementID;///<����ɾ����µĴ���ɵĳɾ�ID
};

struct SAchievementData
{
	SAchivementUnitData achievementsData[ACHIEVEMENT_NUM];
	SAchivementGroupData groupData[ACHIEVEMENT_GROUP_NUM];
};

struct SArenaData
{
	char m_LastChallengeTime[8];
	DWORD m_BestRank;
	DWORD defensiveTeam[5];
	DWORD attackingTeam[5];
	SArenaData()
	{
		memset(&defensiveTeam, 0, 5 * sizeof(DWORD));
		memset(&attackingTeam, 0, 5 * sizeof(DWORD));
	}
};


struct SNewbieGuideDInfo
{
	bool completed;
	BYTE stage;
	SNewbieGuideDInfo()
	{
		completed = false;
		stage = 0;
	}
};

struct SNewbieGudieData
{
	SNewbieGuideDInfo datas[NEWBIEGUIDE_NUM];
	DWORD curGuide;//�������� ǧλΪ��������,����Ϊ��һ��
	SNewbieGudieData()
	{
		memset(datas, 0, sizeof(SNewbieGuideDInfo) * NEWBIEGUIDE_NUM);
		curGuide = 0;
	}
};

struct SSkillPointData
{
	DWORD m_dwSkillPoint; ///<���ܵ�
	DWORD m_dwBuySkillPointTime;	///<�����ܵ����
	DWORD m_dwRemainingSeconds;	///<���㼼�ܵ�ʱ��ʣ��CD
	char m_LastCalculateSkillPointTime[8];	///<�ϴμ��㼼�ܵ��ʱ��
	bool learnedSkill;	///<ѧϰ�����ܵı�־(��������ѧϰ���ܵ��ж�)
};

struct SEnduranceData
{
	DWORD m_dwEndurance;	///<�������ʱ���»��������ʱ���������ֵ
	DWORD m_dwRemainingSeconds;	///<���������ʣ��Ļظ�����
	char m_latestCheckTime[8];	///<���¼���������ʱ��
};

struct SChaseThiefData
{
	char  activateTime[8];	///< ׷����������ʱ�� 
	DWORD remainningTime;	///< ʣ����Чʱ��
	bool valid;	///< �Ƿ���Ч
};

///@brief struct �������
struct SSanGuoPlayerFixData
{
	SHeroData m_HeroList[MAX_HERO_NUM]; ///<�佫�б�
	SSanguoItem m_ItemList[MAX_ITEM_NUM]; ///<��Ʒ�б�
	SBlessData m_BlessData; ///<��Ǯ����Ԫ��������
	SExpeditionData m_ExpeditionData; ///<Զ��������
	SAchievementData m_AchievementData;///<�����ɾ�����
	SAchivementUnitData m_RandomAchievementData; ///<������ɵĿɴ�ɵĳɾ͵�����
	SSanguoStoryTollgate m_TollgateData[MAX_TOLLEGATE_NUM]; ///<��ͨ�ؿ�
	SSanguoStoryEliteTollgate m_EliteTollgateData[MAX_TOLLEGATE_NUM]; ///<��Ӣ�ؿ�
	SSanguoTollgate m_TreasureTollgateData[MAX_TREASUREFIND_TOLLGATE_NUM];///<�ؾ�Ѱ���ؿ�
	SSanguoTollgate m_LeagueOfLegendTollgateData[MAX_LEAGUEOFLEGEND_TOLLGATE_NUM];///<����˵�ؿ�
	DWORD m_TreasureHuntingChallengeTimes;
	DWORD m_LeagueOfLegendChallengeTimes[MAX_LEAGUEOFLEGEND_DUPILICATE_NUM];
	STimeData m_TimeData;
	SSanguoMallData m_VarietyShopData;		///<�ӻ�������
	SSanguoMallData m_ArenaShopData;		///<�������̵�����
	SSanguoMallData m_ExpeditionShopData;	///<Զ���̵�����
	SSanguoSpecialMallData m_MiracleMerchantData;	///<��Ե�̵�����
	SSanguoSpecialMallData m_GemMerchantData;		///<�䱦�̵�����
	SSanguoMallData m_LegionShopData;		///<�����̵�����
	SSanguoMallData m_WarOfLeagueShopData; ///<��ս�̵�����
	SSanguoMallData m_SoulExchangeData;		///<�����̵�
	SSanguoMallData m_BlessPointsShop;		///<�������̳�
	SNewbieGudieData m_NewbieGuideData;	///< ��������
	SSkillPointData m_SkillPointData;	///< ���ܵ�����
	SEnduranceData m_EnduranceData;		///< ��������
	SChaseThiefData m_ChaseThiefData;	///< ׷����������
	DWORD m_ArenaDefenseTeam[5]; ///<��������������
	DWORD m_RandomAchievementRemainingTime;//����ɾͽ�����ʣ��ʱ�䣨�룩
	//SPlayerRelation		m_sPlayerRelation;	//��Һ�������
	DWORD		m_dwExp;	///< ��ǰ����
	DWORD		m_version; ///<�汾��
	DWORD		m_dwMoney;	///< ���
	DWORD		m_dwMaxRecharge; ///<�ܳ�ֵ����
	DWORD		m_dwDiamond; ///<��ʯ
	DWORD		m_dwLevel; ///<�ȼ�
	//======================== �������Խ��Ҫ���뿪�Ȿ�ṹ����
	DWORD	m_dwExploit; ///<����,����ն�����
	DWORD	m_dwHonor;		///<���������䳡���
	DWORD	m_dwPrestige;	///<��������ս���
	DWORD	m_dwSoulPoints;  ///<��㣬������Ƭ�ֽ���
	DWORD	m_dwToken;		///<������Ż��
	DWORD	m_dwBlessPoints;///<�������Ļ���
	DWORD m_dwLatestBattleFileID; ///<���½�������ͨ�ؿ�ID(Int)
	DWORD m_dwLatestEliteBattelFileID; ///<���½����ľ�Ӣ�ؿ�ID(Int)
	DWORD m_dwSiginCount; ///<�ۻ�ǩ������(Int)
	DWORD m_dwTodaySiginFlag; ///<ÿ��ǩ��״̬(BOOL)
	DWORD m_dwGoldExhcangedCount; ///<��ʯ�һ���Ǯ��Ĵ���
	DWORD m_dwEnduranceExchangeCount;///<��ʯ�һ�������Ĵ���
	DWORD m_dwRandomAchieveAccomplishedTimes;///<����ɴ�������Ѿ�ˢ�µĴ���
	DWORD m_dwIsFirstGoldBless; ///<��һ�ν�Ǯ�����
	DWORD m_dwIsFirstDiamondBless; ///<��һ��Ԫ�������
	DWORD m_dwIsFirstEnterArena; ///<��һ�ξ��������
	DWORD m_dwStoryPlayed; ///
	DWORD m_dwEnterZoneServerTime; ///zone server�ĵ�ǰʱ��
	DWORD m_dwZoneID; ///�����˺�ʱ��zone id
	DWORD m_PlayerIconID; //���ͷ��
	BYTE m_extendsBuffer[EXTENDS_STATICDATAS_SIZE]; ///��չ�Ե�C++�˾�̬���ݣ�ռλ��
	BYTE m_bNewPlayer; ///<����� 
	BYTE m_bLuckyTimeFall; ///<�Ƿ��ں�ҹʱ�䣨����ʱ�䣩
	char m_Name[CONST_USERNAME]; /// �������
	//DWORD m_dFactionID[8];//��ҹ���ID
	char m_FactionName[CONST_USERNAME];//����id,ֻ����ǰ4λ
	BYTE	m_bluaBuffer[LUABUFFERSIZE];	// ������lua �����ݵĴ洢


	SSanGuoPlayerFixData()
	{
		//memset(m_FactionName, 0, sizeof(m_FactionName));
		m_PlayerIconID = 108;
	}
};



///@brief struct �̶���������ԣ��ᱣ������������	
struct SFixData : SSanGuoPlayerFixData
{ 
	enum SAVESTATE		///< ��ɫ�������ϵ�״̬
	{
		ST_LOGIN = 1,	// �ոյ���
		ST_LOGOUT,		// �˳�
		ST_HANGUP,		// ���߹һ�
	};

	DWORD   m_dwStaticID;					///< ��ҶԷ�������ȺΨһ��̬��ID�������ݿ����������
	char    m_UPassword[CONST_USERPASS];///< ��Ҷ�������
	BYTE    m_byStoreFlag;					///< ��ҵĵ�ǰ״̬���������ֱ���ʱ����Ϣ
	static const int VERSIONID = 5;///<
	//����ṹ
	static DWORD GetVersion() { return (sizeof(SFixData) << 16 | VERSIONID); };
};
enum Expense
{
	EX_Zhan0,			//ռ0����
	Ex_Strength,		//�����������ı�ʯ��		

	Ex_Skill,			//�����ܵ�������		
	Ex_ExpertPass,		//���þ�Ӣ�ؿ����ı�ʯ		
	Ex_ArenaTicket,		//�������䳡��Ʊ����Ԫ��		
	Ex_ReSetCD,		//�������䳡ս��CD���ı�ʯ
};

enum VipLevelFactor
{
	VF_Zhan0,				//ռ0����
	VF_GiveATKTicket_Num,		//����ɨ��ȯ		
	VF_BuyStrength_Num,			//������������		
	VF_OneToK_Num,			//һ��ǧ�����		
	VF_ButSkill_Num,		//�����ܵ����		
	VF_ReSetPass_Num,			//���þ�Ӣ�ؿ�
	VF_BuyArenaTicket_Num,				//�������䳡��Ʊ����
	VF_Skill_Num,			//���ܵ���
	VF_ATKTen_Num,			//ɨ��ʮ��
	VF_ReSetArenaCD_Num,			//�������䳡ս��CD
	VF_OneKey_Num,			//һ������
	VF_ArtfulMan_Num,			//��Ե����
	VF_KillPass_Num,			//����ն������
	VF_GemMan_Num,			//�䱦����
	VF_KillPassAdd_Num,			//����ն����������
};


