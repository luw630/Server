#pragma once
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include "stdint.h"
#include "..\PUB\ConstValue.h"
#include "..\NETWORKMODULE\HeroTypeDef.h"
using namespace std;

enum MissionType;

enum HeroTrainingType
{
	/// <summary>����ѵ��</summary>
	Normal = 1,
	/// <summary>����ѵ��</summary>
	Fast,
	/// <summary>����ѵ��</summary>
	Extreme,
};

struct GoodsInfoSG
{
	int itype; //���� GoodsType ���պ���  AddGoods_SG(const int32_t iType, const int32_t id=0, int32_t num = 0);
	int id; //����ΪͭǮ��ʯ��ʱ�˲�������,ֻ�������Ϳ�����
	int num;
};

///@ ȫ������
struct CGlobalConfig
{
	int32_t PhysicalLimit;///<������������
	int32_t PhysicalRecoverTime;///ÿ�������ָ�ʱ��(��)	    
	int32_t BuyPhysical;///ÿ�ι����õ�����ֵ	    
	int32_t SkillCostMoney;///ÿ�μ����������Ľ�Ǯ����   
	int32_t LotteryOnceMoney;///����ͭǮ�齱�۸�	        
	int32_t LotteryTenTimesMoney;///ʮ��ͭǮ�齱�۸�	        
	int32_t LotteryOnceGold;///����Ԫ���齱�۸�	
	int32_t LotteryTenTimesGold;///ʮ��Ԫ���齱�۸�	
	int32_t NotVipBuyPhysicalTimes;///��VIP������������
	int32_t BlessFreeTime;///Ԫ������Ѽ��ʱ��(��)	
	int32_t ChangeNameConsumption;//�������ĵ���ʯ
	int32_t ChangeNameConsumptionFreeTime;//��Ѹ����Ĵ���
	float	GetHeroForCashParam1;///Ԫ���н�����1	
	float	GetHeroForCashParam2;///Ԫ���н�����2
	int32_t LearnSkillLevel;	///����ѧϰ���������ȼ�
	int32_t SkillPointRecoverTime;
	int32_t SkillPointLimit;
	int32_t BattleLimitedTime;
	int32_t CrusadeHeroLevelLimit;
	int32_t CrusadeInitMana;
	int32_t BlessFreeTimeForMoney;
	int32_t FristBlessHero;
	int32_t SecondBlessHero;
	int32_t DiamondSweepCost;
	float	HeroPiecesPriceRatio;
	float	GreenEquipForRestore; ///<��ɫװ�������׹����ж��쾭��ֵ�ķ�������
	float	BluenEquipForRestore;	///<��ɫװ�������׹����ж��쾭��ֵ�ķ�������
	float	PurpleEquipForRestore;///<��ɫװ�������׹����ж��쾭��ֵ�ķ�������
	///<��Ե���˿��ż���
	float	miracleMerchantOpenProbability;
	///< ��Ե���˿�������VIP�ȼ�����
	int32_t MiracleMerchantVIPLevelLimit;
	///< ��Ե���˿������������ȼ�����
	int32_t MiracleMerchantMasterLevelLimit;
	///< �䱦���˿��ż���
	float	gemMerchantOpenProbability;
	///< �䱦���˿�������VIP�ȼ�����
	int32_t GemMerchantVIPLevelLimit;
	///< �䱦���˿������������ȼ�����
	int32_t GemMerchantMasterLevelLimit;
	int32_t ForgeExpRestoreItem;	///<װ�������׹����ж��쾭��ֵ�᷵������ƷID
	int32_t ForgeMoneyCostScale;	///<������Ʒ��ʱ��Ľ�Ǯ���ĵı���ֵ
	int32_t FirstTimeGetHeroForMoney;
	int32_t FirstTimeGetHeroForCash;
	int32_t OneStarHeroTransform;
	int32_t TwoStarHeroTransform;
	int32_t ThreeStarHeroTransform;
	int32_t BuySkillPointCost;
	int32_t LuckyTimeGap;///<����ʱ��Σ��߻���˵�ĺ�ҹʱ�䣩�ļ��ʱ�䣬�����ò���Ч
	int32_t LuckyTimePersistentTime;///<����ʱ��εĳ���ʱ��
	int32_t MaxRandomAchievementAccomplishTimes;///<������ɵĿɴ�ɵĳɾ͵����Ŀ���ս�Ĵ���
	int32_t ChaseThiefDurationTime;	///<׷��С͵�ĳ���ʱ��
	int32_t ChaseThiefOpenLevel;///<׷���������ŵȼ�
	float ChaseThiefActivateRate;///<׷�����������

	int	PlayerDonateMaxNum;//����ÿ�վ�������
	int	GemAndPrestigeRatio;//���ű�ʯ������ת��ֵ
	int	MoenyAndPrestigeRatio;//���Ž�Һ�����ת��ֵ
	int	CreateFactionNeedMoney;//���Ŵ���������
	int	FactionDayGetMaxExp;//����ÿ�վ������� 

	int32_t LogMoneyNum;//��־ ͭǮ��
	int32_t LogItemNum;//��־ ����������
	int32_t LogItemIdBengin1;//��־ ����id��ʼֵ1
	int32_t LogItemIdEnd1;//��־ ����id����ֵ1
	int32_t LogItemIdBengin2;//��־ ����id��ʼֵ2
	int32_t LogItemIdEnd2;//��־ ����id����ֵ2
	int32_t LogItemIdBengin3;//��־ ����id��ʼֵ3
	int32_t LogItemIdEnd3;//��־ ����id����ֵ3
	float MovingTime;			//��֤�ƶ�ʱ��
	float FightingCapacityFaultTolerant;	//��֤ս���ݴ���
	float AttackingFaultTolerant;	//��֤�����ݴ���
	float GethitFaultTolerant;		//��֤�ֻ��ݴ���

	int HeroTrainingCountLimit;//�佫ѵ����������
	int HeroTrainingNeedTime;//�佫ѵ������ʱ��(��)
	int HeroTrainingUnitTime;//�佫ѵ������ֵ����ĵ�λʱ��
	int HeroHuntingCountLimit;//�佫Ѱ����������
	int HeroHuntingLevelLimit;//�佫Ѱ���ȼ����ƣ���͵ȼ����ƣ�
	int HeroHuntingTime;//�佫Ѱ��ʱ��
	int HeroHuntingGoldClearingUnitTime;//�佫Ѱ������������ʱ�䵥λ
	int HeroHuntingSurpriseUnitTime;//�佫Ѱ���������ʱ�䵥λ

	///<�ӻ���ˢ������
	vector<int32_t> vecVarietyShopRefreshCost;
	///<����һ��̵�ˢ������
	vector<int32_t> vecSoulExchangeRefreshCost;
	///<�������̵�ˢ������
	vector<int32_t> vecArenaShopRefreshCost;
	///<Զ���̵�ˢ������
	vector<int32_t> vecExpeditionShopRefreshCost;
	///<��Ե�̵�ˢ������
	vector<int32_t> vecMiracleMerchantRefreshCost;
	///<�䱦�̵�ˢ������
	vector<int32_t> vecGemMerchantRefreshCost;
	///<�����̵�ˢ������
	vector<int32_t> vecLegionShopRefreshCost;
	///<��ս�̵�ˢ������
	vector<int32_t> vecWarOfLeagueShopRefreshCost;
	///<�ӻ�����Ʒˢ��ʱ���б�
	vector<int32_t> vecVarietyShopRefreshClock;
	///<����һ��̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecSoulExchangeRefreshClock;
	///<�������̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecArenaShopRefreshClock;
	///<Զ���̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecExpeditionShopRefreshClock;
	///<��Ե�̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecMiracleMerchantRefreshClock;
	///<�䱦������Ʒˢ��ʱ���б�
	vector<int32_t> vecGemMerchantRefreshClock;
	///<�����̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecLegionShopRefreshClock;
	///<��ս�̵���Ʒˢ��ʱ���б�
	vector<int32_t> vecWarOfLeagueShopRefreshClock;
	///<��ļ�佫����
	vector<int32_t> vecHeroSummonCost;
	///<�佫��������
	vector<int32_t> vecHeroEvolutionCost;
	//std::string GMHeroID;
};

///@brief ��Ϸ�淨���ܿ�������
struct GameFeatureActivationConfig
{
	//int skillUpgradeLevelLimit;///<���������Ŀ��ŵȼ�
	int shoppingLevelLimit;///<������Ʒ�Ŀ��ŵȼ�
	int eliteDungeonLevelLimit;///<��Ӣ�����Ŀ��ŵȼ�
	int midasLevelLimit; ///<����ֵȼ���������
	int treasureHuntingDungeonLevelLimit;///<�ؾ�Ѱ�������Ŀ��ŵȼ�
	int arenaLevelLimit;///<�������Ŀ��ŵȼ�
	int leagueOfLegendPalaceLevelLimit;///<����˵�Ŀ��ŵȼ�
	int expeditionDungeonLevelLimit;///<Զ���Ŀ��ŵȼ�
	int forgeShopLevelLimit;///<�����̵Ŀ��ŵȼ�
	int teamSiegeLevelLimit;///<���ŵĿ��ŵȼ�
	int reinforcementsLevelLimit;///<Ԯ���Ŀ��ŵȼ�
	int mopUpTollgateLevelLimit;///<�ؿ�ɨ���Ŀ��ŵȼ�
	int MoneyHand;///<�����
	int Relationship;///< Ե��ϵͳ���ŵȼ�
	int HeroTrainingLimit;///<Ӣ���佫ѵ��
	int HeroHungingHuntingLimit;///<Ӣ�۹һ�Ѱ��
	int soulExchangeLimit;///<����һ�
};

///@brief Ԫ���һ���Ǯ��������Ϣ
struct ExchangeGoldConfig{
	/// <summary>
	/// ����
	/// </summary>
	int m_count;
	/// <summary>
	/// �һ����
	/// </summary>
	int m_getGoldCount;
	/// <summary>
	/// ��ʯ����
	/// </summary>
	int m_useDiamondCount;
	/// <summary>
	/// ����������min
	/// </summary>
	int m_goldRandomMin;
	/// <summary>
	/// ����������max
	/// </summary>
	int m_goldRandomMax;
	/// <summary>
	/// ����min
	/// </summary>
	int m_critMin;
	/// <summary>
	/// ����max
	/// </summary>
	int m_critMax;
	/// <summary>
	/// ��������
	/// </summary>
	int m_critOdds;
};

///@brief С�����������ļ�
struct DessertConfig
{
	int DessertType;
	int DessertProbability;
	vector<int> DessertNums;///<ÿһ�����͵�С��������Ӧ�Ļά���ĸ���
	vector<int> DessertWeightPerUnit;///<ÿһ�����͵�С����������Ӧ�Ļά���ĸ����ļ���

	DessertConfig()
	{
		DessertType = 0;
		DessertProbability = 0;
		DessertNums.clear();
		DessertWeightPerUnit.clear();
	}

	DessertConfig(DessertConfig&& other)
		:DessertNums(std::move(other.DessertNums)),
		DessertWeightPerUnit(std::move(other.DessertWeightPerUnit))
	{
		DessertType = other.DessertType;
		DessertProbability = other.DessertProbability;
	}

	DessertConfig& operator = (DessertConfig&& other)
	{
		if (this != &other)
		{
			DessertType = other.DessertType;
			DessertProbability = other.DessertProbability;
			DessertNums = std::move(other.DessertNums);
			DessertWeightPerUnit = std::move(other.DessertWeightPerUnit);
		}

		return *this;
	}
};

///@brief �������ýṹ//
struct ItemConfig
{
	///< ID //
	int32_t ItemID;
	///< Ч��ֵ //
	int32_t Ability;
	///< �ϳ���Ʒ�� //
	int32_t ComposeItemID;
	///< �ϳ�����//
	int32_t ComposeNum;
	///< ��������ֵ //
	int32_t RefiningExp;
	///< ��ҹ���۸� //
	int32_t BuyGoldPrice;
	///< ��ʯ����۸� //
	int32_t BuyDiamondPrice;
	//�ֽ�õ��Ļ������
	int32_t ResolveSoulPoints;
	//���򱾵��ߵû��ѵĻ������
	int32_t BuySoulPointsCost;
	///< ��������۸� //
	int32_t BuyHonorPrice;
	///< ��������۸� //
	int32_t BuyExploitPrice;
	///< ��������۸� //
	int32_t BuyPrestigePrice;
	///< �������۸�
	int32_t BuyTokenPrice;
	///< ���ۼ۸�(����ֻ��ȡ���) //
	int32_t SellPrice;
	///< ����(����Ʒ������ʯ...) //
	BYTE Type;
	///< Ʒ�� (���ְ�װ ��װ ��װ ��װ)
	BYTE Quality;
};

struct MissionConfig
{
	int id; //����ID
	MissionType missionType;//���������
	int missionAvaliableLevel;//����Ŀ��ŵ��㼱
	int rewardsExp; //��������
	int rewardsGold; //�������
	int rewardsItemID; //������ƷID
	int rewardsItemAmount; //������Ʒ����
	int needCompleteTime; //��Ҫ��ɴ���
	int rewardsType;//����������
};

struct AchievementConfig
{
	int id; //�ɾ�ID
	int achieveType;//�ɾ͵�����
	int param1; //��ɳɾ���������1
	int param2; //��ɳɾ���������2
	int rewardsType;//�ɾͽ���������
	int rewardID; //������ƷID
	int rewardAmount; //������Ʒ����
};

struct RandomAchievementConfig : public AchievementConfig
{
	int accessibleLevel;///<������ɵĿɴ�ɵĳɾ͵Ŀ��ŵȼ�
};

///��ͨ��������Ӣ����������˵�������ؾ�Ѱ���������½�����
struct ChapterConfig
{
	int chapterID;        ///<�½�ID
	int chapterType;		///<��������
	int ticketNum;			///<��������ս������Ϊ-1��������ս����
	set<int> battleIdList;  ///���½��������Ĺؿ�ID�б�

	ChapterConfig()
	{
		chapterID = 0;
		chapterType = 0;
		ticketNum = 0;
		battleIdList.clear();
	}

	ChapterConfig(ChapterConfig&& other)
	{
		chapterID = other.chapterID;
		chapterType = other.chapterType;
		ticketNum = other.ticketNum;
		battleIdList = std::move(other.battleIdList);
	}

	ChapterConfig& operator = (ChapterConfig&& other)
	{
		if (this != &other)
		{
			chapterID = other.chapterID;
			chapterType = other.chapterType;
			ticketNum = other.ticketNum;
			battleIdList = std::move(other.battleIdList);
		}

		return *this;
	}
};

struct HeroTrainingSpeedUpInfor
{
	int consumeType;///Ӣ��ѵ��������Ʒ������
	HeroTrainingType trainingType;
	float bonusScale;///Ӣ��ѵ����ѵ��ģʽ��Ӧ�ľ���ֵ�ӳ�
	int payment; ///ѵ��Ӣ�����ĵ�ͭ�һ���Ԫ������

	HeroTrainingSpeedUpInfor()
	{
		bonusScale = 0;
		trainingType = HeroTrainingType::Normal;
		consumeType = 1;
		payment = 0;
	}
};

///Ӣ��ѵ���������ļ�
struct HeroTrainingCfg
{
	int masterLevel;
	float expProceeds;///ÿһ��ʱ�䵥λ��̶��ջ�Ľ�Ǯ
	float costOfSpeedUp;
	unordered_map<HeroTrainingType, HeroTrainingSpeedUpInfor> gainsAndCost;

	HeroTrainingCfg()
	{
		masterLevel = 0;
		expProceeds = 0;
		costOfSpeedUp = 0;
		gainsAndCost.clear();
	}
};

///Ӣ�۹һ�Ѱ����������Ϣ
struct HeroHungingHuntingCfg
{
	int masterLevel;
	float fixedProceeds;	///ÿһ��ʱ�䵥λ��̶��ջ�Ľ�Ǯ
	float fLuckyOdds;		///����������Ʒ�ļ���
	int goldProceeds;		///�������Ľ�Ǯ������
	float goldWeight;		///�����������Ǯ��Ȩ��
	int diamondProceeds;	///��������Ԫ��������
	float diamondWeight;	
	float propsWeight;
	float equipWeight;
	float debrisWeight;
	float totalWeight;
	vector<int> extraPropsProceeds;///��������ĵ�����Ʒ
	vector<int> extraEqupProceeds;///���������װ��
	vector<int> extraDebrisProceeds;///�����������Ƭ

	HeroHungingHuntingCfg()
	{
		masterLevel = 0;
		fixedProceeds = 0;
		fLuckyOdds = 0;
		goldProceeds = 0;
		goldWeight = 0;
		diamondProceeds = 0;
		diamondWeight = 0;
		propsWeight = 0;
		equipWeight = 0;
		debrisWeight = 0;
		extraPropsProceeds.clear();
		extraEqupProceeds.clear();
		extraDebrisProceeds.clear();
	}
};

///��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ�����
struct BattleLevelConfig
{
	/// <summary>
	/// ���
	/// </summary>
	int ID;
	/// <summary>
	/// ���Ƶȼ�
	/// </summary>
	int LevelLimit;
	/// <summary>
	/// �ؿ�Ӣ�۾���
	/// </summary>
	int Exp;
	/// <summary>
	/// ��������
	/// </summary>
	int Cost;
	/// <summary>
	/// ��ս����
	/// </summary>
	int TicketNum;
	/// <summary>
	/// �ؿ��еĹ���
	/// </summary>
	unordered_set < int > LevelMonsters;

	BattleLevelConfig()
	{
		ID = 0;
		LevelLimit = 0;
		Exp = 0;
		Cost = 0;
		TicketNum = 0;
		LevelMonsters.clear();
	}

	BattleLevelConfig(BattleLevelConfig&& other)
		:LevelMonsters(std::move(other.LevelMonsters))
	{
		ID = other.ID;
		LevelLimit = other.LevelLimit;
		Exp = other.Exp;
		Cost = other.Cost;
		TicketNum = other.TicketNum;
	}

	BattleLevelConfig& operator = (BattleLevelConfig&& other)
	{
		ID = other.ID;
		LevelLimit = other.LevelLimit;
		Exp = other.Exp;
		Cost = other.Cost;
		TicketNum = other.TicketNum;
		LevelMonsters = std::move(other.LevelMonsters);
	}
};

///��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ���һ�ε�������
struct BattleFirstDropConfig
{
	int ID;
	int Money;
	int BossItem1;
	int DropNum1;
	int BossItem2;
	int DropNum2;
};

///��ͨ��������Ӣ����������˵�������ؾ�Ѱ�������Ĺؿ���������
struct BattleDropConfig
{
	int ID;              ///<�ؿ�ID
	int Money;           ///<�ؿ���Ǯ
	int MinNumber;       ///<��С������
	int MaxNumber;       ///<��������
	int WhiteItemWeight;     ///<��ɫ��Ʒ����Ȩ��
	int GreenItemWeight;     ///<��ɫ��Ʒ����Ȩ��
	int BlueItemWeight;      ///<��ɫ��Ʒ����Ȩ��
	int PurpleItemWeight;    ///<��ɫ��Ʒ����Ȩ��
	int BossItem1;
	int BossItemNumber1;
	int BossItemDropPercent1;
	int BossItem2;
	int BossItemNumber2;
	int BossItemDropPercent2;
	int BossItem3;
	int BossItemNumber3;
	int BossItemDropPercent3;
	vector<int> WhiteItemList;
	vector<int> GreenItemList;
	vector<int> BlueItemList;
	vector<int> PurpleItemList;
	vector<int> PropID; ///<ɨ���������ID
	vector<int> PropNumber; ///<ɨ�������������

	BattleDropConfig()
	{
		ID = 0;
		Money = 0;
		MinNumber = 0;
		MaxNumber = 0;
		WhiteItemWeight = 0;
		GreenItemWeight = 0;
		BlueItemWeight = 0;
		PurpleItemWeight = 0;
		BossItem1 = 0;
		BossItemNumber1 = 0;
		BossItemDropPercent1 = 0;
		BossItem2 = 0;
		BossItemNumber2 = 0;
		BossItemDropPercent2 = 0;
		BossItem3 = 0;
		BossItemNumber3 = 0;
		BossItemDropPercent3 = 0;
		WhiteItemList.clear();
		GreenItemList.clear();
		BlueItemList.clear();
		PurpleItemList.clear();
		PropID.clear();
		PropNumber.clear();
	}

	BattleDropConfig(BattleDropConfig&& other)
	{
		ID = other.ID;
		Money = other.Money;
		MinNumber = other.MinNumber;
		MaxNumber = other.MaxNumber;
		WhiteItemWeight = other.WhiteItemWeight;
		GreenItemWeight = other.GreenItemWeight;
		BlueItemWeight = other.BlueItemWeight;
		PurpleItemWeight = other.PurpleItemWeight;
		BossItem1 = other.BossItem1;
		BossItemNumber1 = other.BossItemNumber1;
		BossItemDropPercent1 = other.BossItemDropPercent1;
		BossItem2 = other.BossItem2;
		BossItemNumber2 = other.BossItemNumber2;
		BossItemDropPercent2 = other.BossItemDropPercent2;
		BossItem3 = other.BossItem3;
		BossItemNumber3 = other.BossItemNumber3;
		BossItemDropPercent3 = other.BossItemDropPercent3;
		WhiteItemList = std::move(other.WhiteItemList);
		GreenItemList = std::move(other.GreenItemList);
		BlueItemList = std::move(other.BlueItemList);
		PurpleItemList = std::move(other.PurpleItemList);
		PropID = std::move(other.PropID);
		PropNumber = std::move(other.PropNumber);
	}

	BattleDropConfig& operator = (BattleDropConfig&& other)
	{
		if (this != &other)
		{
			ID = other.ID;
			Money = other.Money;
			MinNumber = other.MinNumber;
			MaxNumber = other.MaxNumber;
			WhiteItemWeight = other.WhiteItemWeight;
			GreenItemWeight = other.GreenItemWeight;
			BlueItemWeight = other.BlueItemWeight;
			PurpleItemWeight = other.PurpleItemWeight;
			BossItem1 = other.BossItem1;
			BossItemNumber1 = other.BossItemNumber1;
			BossItemDropPercent1 = other.BossItemDropPercent1;
			BossItem2 = other.BossItem2;
			BossItemNumber2 = other.BossItemNumber2;
			BossItemDropPercent2 = other.BossItemDropPercent2;
			BossItem3 = other.BossItem3;
			BossItemNumber3 = other.BossItemNumber3;
			BossItemDropPercent3 = other.BossItemDropPercent3;
			WhiteItemList = std::move(other.WhiteItemList);
			GreenItemList = std::move(other.GreenItemList);
			BlueItemList = std::move(other.BlueItemList);
			PurpleItemList = std::move(other.PurpleItemList);
			PropID = std::move(other.PropID);
			PropNumber = std::move(other.PropNumber);
		}

		return *this;
	}
};

/// <summary>
/// ������Ӧ�ȼ��ľ��顢�ȼ���Ϣ
/// </summary>
struct MasterLevelInfor
{
	/// <summary>
	/// ��Ӧ�ȼ�����Ҫ�ľ���ֵ
	/// </summary>
	int LevelNeededExp;
	/// <summary>
	/// ��Ӧ�����ȼ����佫�ȼ�����
	/// </summary>
	int CharactorLevelLimit;
	/// <summary>
	/// ������Ӧ�ȼ�����������
	/// </summary>
	int CharactorEnduranceLimit;
	/// <summary>
	/// ������ǰ�ȼ��ظ�����ֵ
	/// </summary>
	int CharactorActionRecover;
};

/// <summary>
/// ���ڷ���˼�¼��ǰԶ���ؿ���Ϣ��
/// </summary>
struct ExpeditionInstanceInfor
{
	/// <summary>
	/// �ؿ�ID
	/// </summary>
	int32_t levelID;
	/// <summary>
	/// ��С�Ŀ��ŵȼ�
	/// </summary>
	int32_t minLevel;
	/// <summary>
	/// ���Ŀ��ŵȼ�
	/// </summary>
	int32_t maxLevel;
	/// <summary>
	/// �ؿ��Ľ�Ǯ����
	/// </summary>
	int32_t moneyProceeds;
	/// <summary>
	/// �ؿ��ľ�������
	/// </summary>
	int32_t exploitProceeds;
	/// <summary>
	/// ��ɫ��Ʒ���伸��
	/// </summary>
	int32_t whiteItemWeight;
	/// <summary>
	/// ��ɫ��Ʒ���伸��
	/// </summary>
	int32_t greenItemWeight;
	/// <summary>
	/// ��ɫ��Ʒ���伸��
	/// </summary>
	int32_t blueItemWeight;
	/// <summary>
	/// ��ɫ��Ʒ���伸��
	/// </summary>
	int32_t purpleItemWeight;
	/// <summary>
	/// ��ͨӢ�۵��伸��
	/// </summary>
	int32_t normalHeroWeight;
	/// <summary>
	/// ����Ӣ�۵��伸��
	/// </summary>
	int32_t superiorHeroWeight;
	/// <summary>
	/// ϡ��Ӣ�۵��伸��
	/// </summary>
	int32_t rareHeroWeight;
	/// <summary>
	/// �ؿ��Ѷ�
	/// </summary>
	float levelFactor;
	/// <summary>
	/// �ؿ��ܵ������ͨӢ���б�
	/// </summary>
	vector<int> normalHeroList;
	/// <summary>
	/// �ؿ������ľ���Ӣ���б�
	/// </summary>
	vector<int> superiorHeroList;
	/// <summary>
	/// �ؿ�������ϡ��Ӣ���б�
	/// </summary>
	vector<int> rareHeroList;
	/// <summary>
	/// �ؿ������İ�ɫ��Ʒ�б�
	/// </summary>
	vector<int> whiteItemList;
	/// <summary>
	/// �ؿ���������ɫ��Ʒ�б�
	/// </summary>
	vector<int> greenItemList;
	/// <summary>
	/// �ؿ���������ɫ��Ʒ�б�
	/// </summary>
	vector<int> blueItemList;
	/// <summary>
	/// �ؿ���������ɫ��Ʒ�б�
	/// </summary>
	vector<int> purpleItemList;

	ExpeditionInstanceInfor()
	{
		levelID = 0;
		minLevel = 0;
		maxLevel = 0;
		moneyProceeds = 0;
		exploitProceeds = 0;
		whiteItemWeight = 0;
		greenItemWeight = 0;
		blueItemWeight = 0;
		purpleItemWeight = 0;
		normalHeroWeight = 0;
		superiorHeroWeight = 0;
		rareHeroWeight = 0;
		levelFactor = 0.0f;
		normalHeroList.clear();
		superiorHeroList.clear();
		rareHeroList.clear();
		whiteItemList.clear();
		greenItemList.clear();
		blueItemList.clear();
		purpleItemList.clear();
	}

	ExpeditionInstanceInfor(ExpeditionInstanceInfor&& other)
		:normalHeroList(std::move(other.normalHeroList))
		, superiorHeroList(std::move(other.superiorHeroList))
		, rareHeroList(std::move(other.rareHeroList))
		, whiteItemList(std::move(other.whiteItemList))
		, greenItemList(std::move(other.greenItemList))
		, blueItemList(std::move(other.blueItemList))
		, purpleItemList(std::move(other.purpleItemList))
	{
		levelID = other.levelID;
		minLevel = other.minLevel;
		maxLevel = other.maxLevel;
		moneyProceeds = other.moneyProceeds;
		exploitProceeds = other.exploitProceeds;
		whiteItemWeight = other.whiteItemWeight;
		greenItemWeight = other.greenItemWeight;
		blueItemWeight = other.blueItemWeight;
		purpleItemWeight = other.purpleItemWeight;
		normalHeroWeight = other.normalHeroWeight;
		superiorHeroWeight = other.superiorHeroWeight;
		rareHeroWeight = other.rareHeroWeight;
		levelFactor = other.levelFactor;
	}

	ExpeditionInstanceInfor& operator = (ExpeditionInstanceInfor&& other)
	{
		if (this != &other)
		{
			levelID = other.levelID;
			minLevel = other.minLevel;
			maxLevel = other.maxLevel;
			moneyProceeds = other.moneyProceeds;
			exploitProceeds = other.exploitProceeds;
			whiteItemWeight = other.whiteItemWeight;
			greenItemWeight = other.greenItemWeight;
			blueItemWeight = other.blueItemWeight;
			purpleItemWeight = other.purpleItemWeight;
			normalHeroWeight = other.normalHeroWeight;
			superiorHeroWeight = other.superiorHeroWeight;
			rareHeroWeight = other.rareHeroWeight;
			levelFactor = other.levelFactor;
			normalHeroList = std::move(other.normalHeroList);
			superiorHeroList = std::move(other.superiorHeroList);
			rareHeroList = std::move(other.rareHeroList);
			whiteItemList = std::move(other.whiteItemList);
			greenItemList = std::move(other.greenItemList);
			blueItemList = std::move(other.blueItemList);
			purpleItemList = std::move(other.purpleItemList);
		}
		return *this;
	}
};

///@breif �佫��������
struct HeroRankData
{
	float m_Power; ///����	����
	float m_Agility; ///��ʼ��������		
	float m_Intelligence; ///��������	
	std::vector<int> m_EquipIDs;
};

///@breif �佫�������ýṹ
struct HeroRankConfig
{
	map<int, HeroRankData> m_RankData;
public:
	HeroRankConfig()
	{
		m_RankData.clear();
	}

	HeroRankConfig(int rankLevel, HeroRankData&& data)
	{
		m_RankData.clear();
		m_RankData.insert(make_pair(rankLevel, std::move(data)));
	}

	HeroRankConfig(HeroRankConfig&& other)
	{
		m_RankData = std::move(other.m_RankData);
	}

	HeroRankConfig& operator = (HeroRankConfig&& other)
	{
		if (this != &other)
		{
			m_RankData = std::move(other.m_RankData);
		}

		return *this;
	}

	const HeroRankData* GetRankData(int rankLevel) const
	{
		auto iter = m_RankData.find(rankLevel);
		return iter == m_RankData.end() ? nullptr : &iter->second;
	}
};

struct AttrGrowth
{
	float PowerGrowth1;
	float AgilityGrowth;
	float IntelligenceGrowth;
	int Item;
	int Number;
	AttrGrowth()
	{
		PowerGrowth1 = 0;
		AgilityGrowth = 0;
		IntelligenceGrowth = 0;
		Item = 0;
		Number = 0;
	}
};

///@brief �佫���ýṹ
struct HeroConfig
{
	int32_t HeroId;
	int32_t Skill1;
	int32_t Skill2;
	int32_t Skill3;
	int32_t Skill4;
	int32_t Type;
	int32_t Star;
	int32_t Rank;
	float AttackCD;
	string URL;
	CharacterAttrData baseAttribute;///<��������

	HeroConfig()
		:baseAttribute()
	{
		HeroId = 0;
		Skill1 = 0;
		Skill2 = 0;
		Skill3 = 0;
		Skill4 = 0;
		Type = 0;
		Star = 0;
		Rank = 0;
		AttackCD = 0.0f;
		URL = "";
	}

	HeroConfig(HeroConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		HeroId = other.HeroId;
		Skill1 = other.Skill1;
		Skill2 = other.Skill2;
		Skill3 = other.Skill3;
		Skill4 = other.Skill4;
		Type = other.Type;
		Star = other.Star;
		Rank = other.Rank;
		AttackCD = other.AttackCD;
		URL = other.URL;
	}

	HeroConfig& operator = (HeroConfig&& other)
	{
		if (this != &other)
		{
			HeroId = other.HeroId;
			Skill1 = other.Skill1;
			Skill2 = other.Skill2;
			Skill3 = other.Skill3;
			Skill4 = other.Skill4;
			Type = other.Type;
			Star = other.Star;
			Rank = other.Rank;
			AttackCD = other.AttackCD;
			URL = other.URL;
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};

struct HeroStarConfig
{
	int HeroID;
	map<int, AttrGrowth> StarData;

public:
	HeroStarConfig()
	{
		HeroID = 0;
		StarData.clear();
	}

	HeroStarConfig(int heroID, int starLevel, AttrGrowth&& attr)
	{
		HeroID = heroID;
		StarData.clear();
		StarData.insert(make_pair(starLevel, std::move(attr)));
	}

	bool AddStarAttrData(int starLevel, AttrGrowth&& attr)
	{
		if (StarData.find(starLevel) != StarData.end())
			return false;

		StarData.insert(make_pair(starLevel, std::move(attr)));
		return true;
	}

	HeroStarConfig(HeroStarConfig&& other)
		:StarData(std::move(other.StarData))
	{
		HeroID = other.HeroID;
	}

	HeroStarConfig& operator = (HeroStarConfig&& other)
	{
		if (this != &other)
		{
			HeroID = other.HeroID;
			StarData = std::move(other.StarData);
		}
		
		return *this;
	}

	///@brief ��ȡ��Ӣ�������Ǽ�
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetMaxStarLevel()
	{
		if (StarData.size() == 0)
			return -1;

		auto endItor = StarData.end();
		--endItor;
		return endItor->first;
	}

	const AttrGrowth* GetHeroStarAttrGrowth(int starLevel) const
	{
		auto findResult = StarData.find(starLevel);
		if (findResult == StarData.end())
			return nullptr;

		return &(findResult->second);
	}
};

struct MonsterConfig
{
	int32_t monsterID;
	float attackCD;
	float attackRange;
	CharacterAttrData baseAttribute;///<��������

	MonsterConfig()
		:baseAttribute()
	{
		monsterID = 0;
		attackCD = 0;
		attackRange = 0;
	}

	MonsterConfig(MonsterConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		monsterID = other.monsterID;
		attackCD = other.attackCD;
		attackRange = other.attackRange;
	}

	MonsterConfig& operator = (MonsterConfig&& other)
	{
		monsterID = other.monsterID;
		attackCD = other.attackCD;
		attackRange = other.attackRange;
		baseAttribute = std::move(other.baseAttribute);
	}
};

///@breif װ������
struct EquipmentConfig
{
	int32_t m_EquipmentID; ///װ��ID
	int32_t m_EquipmentType; ///װ����λ
	int32_t levelLimit;	///�ȼ�����
	int32_t refiningExp; //��������
	///< ��ҹ���۸� //
	int32_t buyGoldPrice;
	///< ��ʯ����۸� //
	int32_t buyDiamondPrice;
	///< ��������۸� //
	int32_t buyHonorPrice;
	///< ��������۸� //
	int32_t buyExploitPrice;
	//���򱾵��ߵû��ѵĻ������
	int32_t BuySoulPointsCost;
	//�ֽ�õ��Ļ������
	int32_t ResolveSoulPoints;
	///< ��������۸� //
	int32_t buyPrestigePrice;
	///< �������۸�
	int32_t buyTokenPrice;
	///< ���ۼ۸�(����ֻ��ȡ���) //
	int32_t sellPrice;
	///< �ϳ�װ���������ƬID //
	int32_t pieceID;
	int32_t Quality;///װ���ĳ�ɫ�������ɫװ������ɫװ������ɫװ����
	CharacterAttrData baseAttribute;///<��������

	EquipmentConfig()
		:baseAttribute()
	{
		m_EquipmentID = 0;
		m_EquipmentType = 0;
		levelLimit = 0;
		refiningExp = 0;
		buyGoldPrice = 0;
		buyDiamondPrice = 0;
		buyHonorPrice = 0;
		buyExploitPrice = 0;
		buyPrestigePrice = 0;
		ResolveSoulPoints = 0;
		BuySoulPointsCost = 0;
		buyTokenPrice = 0;
		sellPrice = 0;
		pieceID = 0;
		Quality = 0;
	}

	EquipmentConfig(EquipmentConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
	{
		m_EquipmentID = other.m_EquipmentID;
		m_EquipmentType = other.m_EquipmentType;
		levelLimit = other.levelLimit;
		refiningExp = other.refiningExp;
		buyGoldPrice = other.buyGoldPrice;
		buyDiamondPrice = other.buyDiamondPrice;
		buyHonorPrice = other.buyHonorPrice;
		buyExploitPrice = other.buyExploitPrice;
		buyPrestigePrice = other.buyPrestigePrice;
		ResolveSoulPoints = other.ResolveSoulPoints;
		BuySoulPointsCost = other.BuySoulPointsCost;
		buyTokenPrice = other.buyTokenPrice;
		sellPrice = other.sellPrice;
		pieceID = other.pieceID;
		Quality = other.Quality;
	}

	EquipmentConfig& operator = (EquipmentConfig&& other)
	{
		if (this != &other)
		{
			m_EquipmentID = other.m_EquipmentID;
			m_EquipmentType = other.m_EquipmentType;
			levelLimit = other.levelLimit;
			refiningExp = other.refiningExp;
			buyGoldPrice = other.buyGoldPrice;
			buyDiamondPrice = other.buyDiamondPrice;
			buyHonorPrice = other.buyHonorPrice;
			buyExploitPrice = other.buyExploitPrice;
			buyPrestigePrice = other.buyPrestigePrice;
			ResolveSoulPoints = other.ResolveSoulPoints;
			BuySoulPointsCost = other.BuySoulPointsCost;
			buyTokenPrice = other.buyTokenPrice;
			sellPrice = other.sellPrice;
			pieceID = other.pieceID;
			Quality = other.Quality;
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};

///@brief װ���������ã���ʱû��ĳһ�Ǽ���װ���Ķ�Ӧ�����Լӳ�
struct ForgingConfig
{
	int id;
	int refiningTime;
	std::map<int, int> levelExp;
	CharacterAttrData baseAttribute;///<��������

	ForgingConfig()
		:baseAttribute()
	{
		id = 0;
		refiningTime = 0;
		levelExp.clear();
	}

	ForgingConfig(ForgingConfig&& other)
		:baseAttribute(std::move(other.baseAttribute))
		, levelExp(std::move(other.levelExp))
	{
		id = other.id;
		refiningTime = other.refiningTime;
	}

	ForgingConfig& operator = (ForgingConfig&& other)
	{
		if (this != &other)
		{
			id = other.id;
			refiningTime = other.refiningTime;
			levelExp = std::move(other.levelExp);
			baseAttribute = std::move(other.baseAttribute);
		}

		return *this;
	}
};


/**@brief �̳���Ʒ����*/
struct CommodityConfig
{
	int32_t minLimitLevel;
	int32_t maxLimitLevel;
	int32_t articleShelf;    //��Ʒ��//
	int32_t minNumber;
	int32_t maxNumber;
	vector<int32_t> vecItem1;
	int percent1;
	vector<int32_t> vecItem2;
	int percent2;
	vector<int32_t> vecItem3;
	int percent3;
	vector<int32_t> vecItem4;
	int percent4;
	vector<int32_t> vecItem5;
	int percent5;

	bool InLevelRange(int32_t level) const
	{
		return (minLimitLevel <= level && level <= maxLimitLevel) ? true : false;
	}
};

///@wk 30��ǩ������
struct CheckInConfig
{
	int32_t times;//ǩ������
	int32_t doubleNeedVip;//˫����Ҫ��vip�ȼ�
	GoodsInfoSG Goods[2];//2������
};
///@wk 7��ǩ������
struct CheckInday7Config
{
	int32_t SignInDay;//ǩ������
	GoodsInfoSG Goods[2];//1Ϊ�״�ǩ��;2Ϊ��ͨ����
};
struct ArenaRewardConfig {

	 int rankUp;
	 int rankFloor;
	 int Gold;
	 int  Gem;
	 int RefinedMaterial;
	 int RefinedMaterialNumber;
	 int Medicine;
	 int MedicineNumber;
	 int Honor;
	 float BestRecord;
};

struct ArenaRobotLevelInfo
{
	int uper;
	int lower;
	int heroLevel;
	int rankLevel;
	int starLevel;
	int skillLevel;
	int level;
};

///@brief ��������������Ϣ
struct NewbieGuideConfig
{
	int guideType;				///<��������
	int startIndex;				///<��ʼ����
	int nextTollgateID;			///<��ȡ�佫�����Ĺؿ�ID
	int rewardID;				///<��ȡ���佫���佫ID
	vector<int> vecOperateIndex;///<������������Ĳ�������
};

///@brief �佫Ե�����ýṹ
struct FateConfig
{
	int heroID;							///<Ե�����佫
	vector<int> vecRelationshipHero;	///<Ե���佫
	vector<int> vecAttributeType;		///<Ե�ּӳ���������
	vector<float> vecAttributePercent;	///<Ե�ּӳ����Ա���
};

///@brief �佫�������ýṹ
struct HeroSkillConfig
{
	int id;				///< ����ID
	int type;			///< ��������
	int buffID1;		///< ��һ��buffID
	int buffID2;		///< �ڶ���buffID
	int selfBuffID;		///< ����ӳ�buffID
	int effective;		///< ������Ч��ʼ�ȼ�
	float damage;		///< ���ܸ��ӳ�ʼ�˺�
	float addDamage;	///< ���ܸ��������˺�
	float extraDamage;	///< ����ֵ
	float factor;		///< �����˺�ϵ��
};

///@brief buff���ñ�
struct BuffConfig
{
	int id;
	int style;			///< ����(0:״̬�� 1:������)
	float initiaValue;	///< ��ʼֵ
	float addValue;		///< �����ӳ�
	int EffectiveStyle; ///< ���÷�ʽ
	string name;		///< ����
};