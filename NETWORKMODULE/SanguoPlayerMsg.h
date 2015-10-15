#pragma once
#include "NetModule.h"
#include "../networkmodule/playertypedef.h"


#define MAX_ADDR_HTTP     512  //http���� ��󳤶�
#define MAX_ARENACHALLENGE_HERONUM 5	//������ս�����Ӣ����
#define MAX_War_HERONUM 5	//��ս���Ӣ����
#define  MAX_WAR_NAME_LIST 20 // ��ս��ս�б�,ÿ��ȡ20��
#define  MAX_WAR_HERO_INFO 27 //16��������������(1-16)+8����������(17-24)+2��Ѫ��ŭ��(25-26)+1�佫id(27)
#define  MAX_Rank_List 50 // ���а������
#define  MaxExchangeData 10 //�һ���������

//======================================================================
DECLARE_MSG_MAP(SSGPlayerMsg, SMessage, SMessage::EPRO_SANGUOPLAYER_MESSAGE)
EPRO_SANGUO_ATTR = 0, ///<�����佫����
EPRO_SANGUO_NEWGUIDE, ///<����ָ������
EPRO_SANGUO_BLESS, ///<�����
EPRO_SANGUO_DUPLICATE,	///<��������
EPRO_SANGUO_FORGING,	///<������Ϣ
EPRO_EXPEDITION_MESSAGE,///<Զ����Ϣ
EPRO_ACHIEVEMENT_MESSAGE,///<�ɾ���Ϣ
EPRO_MISSION_MESSAGE,///<������Ϣ
EPRO_SANGUO_HEROUPGRADE, ///�佫�������
EPRO_SANGUO_STORAGE,	///<��������
EPRO_SANGUO_ADDGOODS_SYN,	///<ͬ�����,�õ�����,��ҪΪaddgoods���,��ͭǮԪ����
EPRO_SANGUO_DECGOODS_SYN,	///<ͬ�����,�۳�����,��ҪΪDecgoods���,��ͭǮԪ����
EPRO_SANGUO_MALL,	///<�̳�����
EPRO_SANGUO_CHECKIN,	///<ǩ��
EPRO_SANGUO_DIAMONDEXCHANGE, ///<��ʯ�һ������������Ϣ
EPRO_SANGUO_SKILLUPGRADE, ///<��������
EPRO_SANGUO_SCRIPT, ///<�ű���Ϣ���
EPRO_SANGUO_RANDOMAVHIEVEMNT_MESSAGE, ///<������ɵĿɴ�ɳɾ͵������Ϣ
EPRO_SANGUO_DESSERT_MESSAGE,	///<С�ͽ���
EPRO_SANGUO_CHASETHIEF,	///<׷������
EPRO_SANGUO_CHANGENAME,	///<�������
EPRO_SANGUO_ACTIVITY,///<����
EPRO_SANGUO_RECHARGE,///<��ֵ
EPRO_SANGUO_HEROEXTENDSGAMEPLAY,///�佫Ӣ�۵���չ�淨������һ��ý�ң��һ�ѵ��
EPRO_SANGUO_fIRSTRECHARGE,//�׳��콱
EPRO_SANGUO_REWARDSTATE,///����״̬
EPRO_SANGUO_WeekReMoney,//����
EPRO_SANGUO_War,//��ս
EPRO_SANGUO_Rank,//���а�
EPRO_SANGUO_Excheange,//�һ��

END_MSG_MAP()





///@brief Ԫ���һ������������Ϣ
DECLARE_MSG_MAP(SDiamondExchangeMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DIAMONDEXCHANGE)
SANGUO_EXCHANGEGOLD = 0, ///<Ԫ���һ���Ǯ��Ϣ
SANGUO_EXCHANGEENDURANCE, ///<Ԫ���һ���������Ϣ
END_MSG_MAP()

///@brief Ԫ���һ���Ǯ����Ϣ
DECLARE_MSG_MAP(SDiamondExchangeGoldMsg, SDiamondExchangeMsg, SDiamondExchangeMsg::SANGUO_EXCHANGEGOLD)
REQUEST_EXCHANGEGOLD_ONCE = 0,///<�����ñ�ʯ�һ�һ�ν�Ǯ
REQUEST_EXCHANGEGOLD_CONTINUOUS,///<��������ʹ�ñ�ʯ���һ���Ǯ
ANSWER_EXCHANGEGOLD_INFOR,///<�ñ�ʯ�һ���Ǯ�Ľ����Ϣ
ANSWER_REFRESH,			///<ˢ�¿ɶһ��Ĵ���
END_MSG_MAP()

///@brief �����ñ�ʯ�һ���Ǯ����Ϣ
struct SAExchangeGoldInfor : SDiamondExchangeGoldMsg
{
	DWORD diamondCostEachTime;	///<ÿһ�����ı�ʯ������
	DWORD exchangedTimes;		///<�һ��Ĵ���
	DWORD exchangedInfors[MAX_ITEM_NUM_SYN_TO_CLIENT];	///<��СΪexchangedTimes��2����ǰexchangedTimesλΪ��õĽ��������exchangedTimesλΪ���εı�������

	SAExchangeGoldInfor()
	{
		diamondCostEachTime = 0;
		exchangedTimes = 0;
		_protocol = SDiamondExchangeGoldMsg::ANSWER_EXCHANGEGOLD_INFOR;
	}
};



///@brief Ԫ���һ���������Ϣ
DECLARE_MSG_MAP(SDiamondExchangeEnduranceMsg, SDiamondExchangeMsg, SDiamondExchangeMsg::SANGUO_EXCHANGEENDURANCE)
REQUEST_EXCHANGEENDURANCE = 0,
ANSWER_EXCHANGEENDURANCE_INFOR,///<�ñ�ʯ�һ������Ľ����Ϣ
ANSWER_REFRESH,			///<ˢ�¿ɶһ��Ĵ���
END_MSG_MAP()

///@brief Ԫ���һ������ɹ�������Ϣ
struct SAExchangeEnduranceInfor : SDiamondExchangeEnduranceMsg
{
	bool bSuccess;

	SAExchangeEnduranceInfor()
	{
		bSuccess = false;
		_protocol = SDiamondExchangeEnduranceMsg::ANSWER_EXCHANGEENDURANCE_INFOR;
	}
};

///С�ͽ�������ص�һЩ��Ϣ
DECLARE_MSG_MAP(SDessertMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DESSERT_MESSAGE)
REQUEST_DESSERT = 0,
REQUEST_CLAIM_DESSERT,
ANSWER_DESSERT_BAKED_INFOR,
ANSWER_DESSERT_REFRESH,
END_MSG_MAP()

struct SADessertBakedInfor : SDessertMsg
{
	bool bDessertAvaliable;
	byte dessertType;
	int dessertNum;

	SADessertBakedInfor()
	{
		bDessertAvaliable = false;
		dessertType = 0;
		dessertNum = 0;
	}
};

DECLARE_MSG_MAP(SSetAttrMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_ATTR)
EPRO_ATTR_NONE = 0,
//=================================
EPRO_ATTR_STORYPLAYED,/// StoryPlayed
EPRO_ATTR_UPDATEPLAYERICON,///�������ͷ��ͼ��
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
//EPRO_ATTR_
EPRO_ATTR_MAX, 
END_MSG_MAP()


struct SQSetAttrMsg : public SSetAttrMsg
{
	DWORD m_AttrValue;
	BYTE m_AttrEnum;
	SQSetAttrMsg()
	{
		m_AttrValue = 0;
		m_AttrEnum = 0;
	}
};

//==============�ֿ���Ϣ��ʱ����

DECLARE_MSG(SStorageMsg, SMessage, SMessage::EPRO_STORAGE_MESSAGE)
struct SQStroageMsg : public SStorageMsg
{
	SSanguoItem m_ItmeInfo;
	SQStroageMsg()
	{
		m_ItmeInfo.m_dwCount = 0;
		m_ItmeInfo.m_dwItemID = 0;
	}
};

DECLARE_MSG_MAP(SBlessMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_BLESS)
MONEY_FREE_BLESS,
MONEY_SINGLE_BLESS,
MONEY_MUTIPLE_BLESS,
DIAMOND_FREE_BLESS,
DIAMOND_SINGLE_BLESS,
DIAMOND_MUTIPLE_BLESS
END_MSG_MAP()

struct SQRequstBless : public SBlessMsg
{

	SQRequstBless()
	{
		SBlessMsg::_protocol = MONEY_FREE_BLESS;
	}

};

struct SASingleRequstBless : public SBlessMsg
{
	DWORD m_dwPrizeID;
	DWORD m_dwProperty;
	SASingleRequstBless(const BYTE blessType)
	{
		SBlessMsg::_protocol = blessType;
		m_dwProperty = 0;
		m_dwPrizeID = 0;
	}

};


struct SARequstMutipleBless : public SBlessMsg
{
	DWORD m_dwPrizeIDs[10];
	DWORD m_dwProperty;
	SARequstMutipleBless(const BYTE blessType)
	{
		SBlessMsg::_protocol = blessType;
		m_dwProperty = 0;
		for (int i = 0; i < 10; i++)
		{
			m_dwPrizeIDs[i] = 0;
		}
	}
};

DECLARE_MSG_MAP(SForgingMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_FORGING)
REQUEST_MONEY_FORGING = 0,
REQUEST_DIAMOND_FORGING,
ANSWER_MONEY_FORGING_PERMITTED,
ANSWER_DIAMOND_FORGING_PERMITTED,
END_MSG_MAP()

struct SQAskToForging : public SForgingMsg
{
	int heroID;///<Ҫ����װ����Ӣ�۵�ID
	int forgingEquipID;///<Ҫ�����װ��
	SQAskToForging()
	{
		_protocol = SForgingMsg::REQUEST_MONEY_FORGING;
		forgingEquipID = 0;
	}
};

struct SQAskToMoneyForging : public SQAskToForging
{
	int materialTypeNum;///<�ж����ֲ���
	int materialInfor[];///<ǰmaterialTypeNumλΪID����materialTypeNumλΪnum
	SQAskToMoneyForging()
	{
		_protocol = SForgingMsg::REQUEST_MONEY_FORGING;
		heroID = 0;
		forgingEquipID = 0;
		materialTypeNum = 1;
	}
};

struct SAForgingPermission : public SForgingMsg
{
	bool permitted;
	int heroID;
	int forgedEquipID;
	int curEquipStarLevel;
	SAForgingPermission()
	{
		permitted = false;
		heroID = 0;
		forgedEquipID = 0;
		curEquipStarLevel = 0;
		_protocol = SForgingMsg::ANSWER_MONEY_FORGING_PERMITTED;
	}
};

DECLARE_MSG_MAP(SMissionMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_MISSION_MESSAGE)
REQUEST_ACCOMPLISH_MISSION,
ANSWER_REFRESH_MISSION_DATA,
ANSWER_MISSION_ACCOMPLISHED,
ANSWER_UPDATE_MISSION_STATE,
END_MSG_MAP()

struct SQMissionAccomplished : public SMissionMsg
{
	int missionID;

	SQMissionAccomplished()
	{
		missionID = 0;
		_protocol = SMissionMsg::REQUEST_ACCOMPLISH_MISSION;
	}
};

struct SARefreshMissionData : public SMissionMsg
{
	SARefreshMissionData()
	{
		_protocol = SMissionMsg::ANSWER_REFRESH_MISSION_DATA;
	}
};

struct SAMissionAccomplished : public SMissionMsg
{
	bool bAccomplished;
	int missionID;
	SAMissionAccomplished()
	{
		missionID = 0;
		bAccomplished = false;
		_protocol = SMissionMsg::ANSWER_MISSION_ACCOMPLISHED;
	}
};

///@brief ˢ�°�ʱ���ʱ�������Ƿ���ʾ�������״̬
struct SAUpdateMissionState : public SMissionMsg
{
	bool visible;
	int missionID;
	int completeTimes;
	SAUpdateMissionState()
	{
		_protocol = SMissionMsg::ANSWER_UPDATE_MISSION_STATE;
		visible = false;
		completeTimes = 0;
	}
};

DECLARE_MSG_MAP(SAchievementMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_ACHIEVEMENT_MESSAGE)
REQUESET_CLAIM_ACHIEVMENT = 0,
ANSWER_UPDATE_ACHIEVEMENT_STATE,
ANSWER_ACHIEVEMENT_CLAIM_CONFORMED,
END_MSG_MAP()

///@brief �ͻ��˷�����Ϣ��ȡ�ɾ�
struct SQClaimAchievement : public SAchievementMsg
{
	DWORD achievementID;

	SQClaimAchievement()
	{
		achievementID = 0;
		_protocol = SAchievementMsg::REQUESET_CLAIM_ACHIEVMENT;
	}
};

///@brief ������׼���Ƿ������ȡĳһ���ɾ�
struct SAAchievementClaimConformed : public SAchievementMsg
{
	bool bConformed;///<�Ƿ�������ȡ
	bool bAchievementGroupAccomplished;///<����ɾ��Ƿ�ȫ�����
	byte achievementType;	///<�����ĳ���ɾͿ�����ȡ
	DWORD achievementID;	///<�ĸ��ɾͿ�����ȡ
	DWORD newsetAchievementID;
	SAAchievementClaimConformed()
	{
		newsetAchievementID = 0;
		achievementID = 0;
		bAchievementGroupAccomplished = false;
		bConformed = false;
		achievementType = (byte)AchievementType::Battle0;
		_protocol = SAchievementMsg::ANSWER_ACHIEVEMENT_CLAIM_CONFORMED;
	}
};

///@brief ����ĳһ���ɾ͵����״̬
struct SAUpdateAchievementState : public SAchievementMsg
{
	DWORD achievementID;
	DWORD completedTimes;
	SAUpdateAchievementState()
	{
		achievementID = 0;
		_protocol = SAchievementMsg::ANSWER_UPDATE_ACHIEVEMENT_STATE;
	}
};

DECLARE_MSG_MAP(SRandomAchievementMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_RANDOMAVHIEVEMNT_MESSAGE)
REQUESET_RANDOM_NEW_ACHIEVEMENT = 0,
REQUESET_CLAIM_ACHIEVMENT,
ANSWER_REFRESH_RANDOM_ACHIEVEMENT,
ANSWER_NEW_AHIEVEMENT_GOT,
ANSWER_UPDATE_ACHIEVEMENT_STATE,
ANSWER_ACHIEVEMENT_CLAIM_CONFORMED,
END_MSG_MAP()

///@brief �ͻ��˷�����Ϣ��ȡ�ɾ�
struct SQClaimRandomAchievement : public SRandomAchievementMsg
{
	DWORD achievementID;

	SQClaimRandomAchievement()
	{
		achievementID = 0;
		_protocol = SRandomAchievementMsg::REQUESET_CLAIM_ACHIEVMENT;
	}
};

///@brief ��ͻ��˷��͵ĵ��µ�����õĳɾ�
struct SANewRandomAchievementGot : public SRandomAchievementMsg
{
	bool achievementGot;
	DWORD achievementPersistentTimes;
	DWORD achievementID;

	SANewRandomAchievementGot()
	{
		achievementGot = false;
		achievementPersistentTimes = 0;
		_protocol = SRandomAchievementMsg::ANSWER_NEW_AHIEVEMENT_GOT;
	}
};

///@brief ������׼���Ƿ������ȡĳһ���ɾ�
struct SARandomAchievementClaimConformed : public SRandomAchievementMsg
{
	bool bConformed;///<�Ƿ�������ȡ
	DWORD achievementID;	///<�ĸ��ɾͿ�����ȡ
	SARandomAchievementClaimConformed()
	{
		achievementID = 0;
		bConformed = false;
		_protocol = SRandomAchievementMsg::ANSWER_ACHIEVEMENT_CLAIM_CONFORMED;
	}
};

///@brief ����ĳһ���ɾ͵����״̬
struct SAUpdateRandomAchievementState : public SRandomAchievementMsg
{
	DWORD achievementID;
	DWORD completedTimes;
	SAUpdateRandomAchievementState()
	{
		achievementID = 0;
		_protocol = SRandomAchievementMsg::ANSWER_UPDATE_ACHIEVEMENT_STATE;
	}
};

DECLARE_MSG_MAP(SExpeditionMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_EXPEDITION_MESSAGE)
REQUEST_BATTLE_RESULT_RESOLVES = 0,
//REQUEST_NEXT_ENEMY_INFOR,
REQUEST_EXIT_BATTLE,
REQUEST_RECEIVE_REWARDS,
REQUEST_ENTER_BATTLE,
REQUEST_UPDATE_SELECTEDCHARACTER_INFOR,
REQUEST_UPDATE_ENEMYCHARACTER_INFOR,
REQUEST_COST_A_TICKET,
REQUEST_ACTIVE_EXPEDITION,///<��ʱ�ɿͻ�����֪ͨ����Զ���ؿ�
ANSWER_BATTLE_RESOLVES_RESULT,
ANSWER_EXPEDITION_CONQURED,
ANSWER_PERMISSION_INFOR,
ANSWER_HOSTILEPLAYER_INFOR,
ANSWER_REFRESH_TICKET,
END_MSG_MAP()

struct SQExpeditionRequest : public SExpeditionMsg
{
	SQExpeditionRequest()
	{
		SQExpeditionRequest::_protocol = REQUEST_BATTLE_RESULT_RESOLVES;
	}
};

struct SQExpeditionPermissionRequest : public SExpeditionMsg
{
	DWORD selectedHero[g_iExpeditionCharacterLimit];

	SQExpeditionPermissionRequest()
	{
		SExpeditionMsg::_protocol = REQUEST_ENTER_BATTLE;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			selectedHero[i] = 0;
		}
	}
};

struct SQUpdateSelectedHeroData : public SExpeditionMsg
{
	DWORD selectedHeroMap[g_iExpeditionCharacterLimit];///<�洢����Ҳ�ս�佫��ID
	float heroHealthScale[g_iExpeditionCharacterLimit];
	float heroManaScale[g_iExpeditionCharacterLimit];

	SQUpdateSelectedHeroData()
	{
		SExpeditionMsg::_protocol = REQUEST_UPDATE_SELECTEDCHARACTER_INFOR;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			selectedHeroMap[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			heroHealthScale[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			heroManaScale[i] = 0;
		}
	}
};

struct SQUpdateEnemyData : public SExpeditionMsg
{
	DWORD machedEnemyMap[g_iExpeditionCharacterLimit];///<�洢�˵з���ս�佫��ID
	float enemyHealthScale[g_iExpeditionCharacterLimit];
	float enemyManaScale[g_iExpeditionCharacterLimit];

	SQUpdateEnemyData()
	{
		SExpeditionMsg::_protocol = REQUEST_UPDATE_ENEMYCHARACTER_INFOR;
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			machedEnemyMap[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			enemyHealthScale[i] = 0;
		}
		for (int i = 0; i < g_iExpeditionCharacterLimit; ++i)
		{
			enemyManaScale[i] = 0;
		}
	}
};

struct SABattleResolve : public SExpeditionMsg
{
	bool bIsHero;
	DWORD obtainedItemID;
	SABattleResolve()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_BATTLE_RESOLVES_RESULT;
		bIsHero = false;
		obtainedItemID = 0;
	}
};

struct SAPermissionInfor : public SExpeditionMsg
{
	BYTE checkResult;

	SAPermissionInfor()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_PERMISSION_INFOR;
		checkResult = 0;
	}
};

struct SAHostileEnemyInfor : public SExpeditionMsg
{
	float levelFactor;///�ؿ����Ѷ�ϵ��
	DWORD curLevelInstance;
	SHeroData enmeyInfo[g_iExpeditionCharacterLimit];
	SAHostileEnemyInfor()
	{
		levelFactor = 1;
		curLevelInstance = 0;
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_HOSTILEPLAYER_INFOR;
	}
};

struct SAConqured : public SExpeditionMsg
{
	SAConqured()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_EXPEDITION_CONQURED;
	}
};

struct SAExpeditionRefresh : public SExpeditionMsg
{
	SAExpeditionRefresh()
	{
		SExpeditionMsg::_protocol = SExpeditionMsg::ANSWER_REFRESH_TICKET;
	}
};

///<@brief �ȶ���������ͨ�������������¸���������Ӣ����������˵�������ؾ�Ѱ����������Ϣ����
DECLARE_MSG_MAP(SDuplicateMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_DUPLICATE)
REQUEST_ENTER_SPECIFY_TOLLGATE = 0,
REQUEST_MOP_UP_TOLLGATE,
REQUEST_START_CHALLENGE,
REQUEST_EARNING_CLEAR,
REQUEST_RESET_TOLLGATE,
REREQUEST_TOLLGATE_DROPED_ITEM, ///<���»�ȡ�ؿ��ĵ�����Ʒ
ANSWER_REFRESH_ALL_TICKET,
ANSWER_ACTIVATE_DUPLICATE,
ANSWER_TOLLGATE_DROPED_ITEM,
ANSWER_RESET_SPECIFY_TOLLGATE,
ANSWER_TOLLGATE_EARNING_CLEARED,
ANSWER_ENTER_TOLLGATE_PERMISSION_INFOR,
END_MSG_MAP()

///@brief ��������һ����������Ϣ
struct SDuplicateTypeMsg : public SDuplicateMsg
{
	byte duplicateType;

	SDuplicateTypeMsg()
	{
		duplicateType = InstanceType::Story;
	}
};

///@brief ������������һ�������е���һ���ؿ�
struct SSpecifyTollgateMsg : public SDuplicateTypeMsg 
{
	int duplicateID;
	int tollgateID;

	SSpecifyTollgateMsg()
	{
		duplicateID = 0;
		tollgateID = 0;
	}
};

///@brief �ͻ��˷������������ĳһ���ؿ�����Ϣ
struct SQEnterSpecifyTollgate : public SSpecifyTollgateMsg
{
	int selectedHeroNum;
	int monsterCachingNum;
	int requestData[];

	SQEnterSpecifyTollgate()
	{
		_protocol = REQUEST_ENTER_SPECIFY_TOLLGATE;
		selectedHeroNum = 0;
	}
};

///@brief �ͻ��˷�����Ϣ����ɨ��ĳһ���ؿ�
struct SQMopUpTollgate : public SSpecifyTollgateMsg
{
	int sweepTicketID;///<ɨ������ID
	int challengeTimes;

	SQMopUpTollgate()
	{
		_protocol = REQUEST_MOP_UP_TOLLGATE;
		challengeTimes = 0;
		sweepTicketID = 0;
	}
};

///@brief �ͻ��˿�ʼ��ս��Ӧ�ĸ����ؿ�
struct SQStartToChallengeDuplicate : public SDuplicateTypeMsg
{
	SQStartToChallengeDuplicate()
	{
		_protocol = REQUEST_START_CHALLENGE;
	}
};

///@brief �ͻ�������ؿ�����
struct SQDuplicateEarningClear : public SSpecifyTollgateMsg
{
	int combatGrade;///<ս������

	SQDuplicateEarningClear()
	{
		_protocol = REQUEST_EARNING_CLEAR;
		combatGrade = 0;
	}
};

///@brief �ͻ�����������ĳһ���ؿ�
struct SQResetTollgate : public SSpecifyTollgateMsg
{
	SQResetTollgate()
	{
		_protocol = REQUEST_RESET_TOLLGATE;
	}
};

struct SAResetSpecifyTollgate : public SSpecifyTollgateMsg
{
	bool bPermited;

	SAResetSpecifyTollgate()
	{
		bPermited = false;
		_protocol = ANSWER_RESET_SPECIFY_TOLLGATE;
	}
};

///@brief ���߿ͻ����Ƿ������ս��ָ���Ĺؿ�
struct SAChallengePermissionInfor : public SDuplicateTypeMsg
{
	bool permitted;
	bool bMopUpOperation;
	int monsterNum;
	MonsterData monsterData[MAX_TOLLGATE_MONSTER_NUM];
	SAChallengePermissionInfor()
	{
		permitted = false;
		bMopUpOperation = false;
		monsterNum = 0;
		memset(monsterData, 0, sizeof(monsterData));
		_protocol = ANSWER_ENTER_TOLLGATE_PERMISSION_INFOR;
	}
};

///@brief �����ͻ�����ˢ���ȶ���������ͨ�������������¸���������Ӣ����������˵�������ؾ�Ѱ����������ս��������Ϣ
struct SARefreshDuplicateAllTicket : public SDuplicateMsg
{
	SARefreshDuplicateAllTicket()
	{
		_protocol = SDuplicateMsg::ANSWER_REFRESH_ALL_TICKET;
	}
};

///@brief �����ͻ�����������ĳһ����������ʱ���������Ӣ����û�е��ô���Ϣ���ͻ�������Ӧ�ļ��Ӣ�������߼�����ʱ�����������
struct SAActivateDuplicate : public SDuplicateTypeMsg
{
	bool bExclusivity; ///<�Ƿ��������Եģ���ֻ��duplicateIDs����ָ���ĸ����ż���
	int duplicateNum;
	int duplicateIDs[MAX_LEAGUEOFLEGEND_DUPILICATE_NUM];

	SAActivateDuplicate()
	{
		bExclusivity = true;
		duplicateNum = 0;
		_protocol = SDuplicateMsg::ANSWER_ACTIVATE_DUPLICATE;
	}
};

///@brief ���߿ͻ��˱��ιؿ�Ҫ�������Ʒ
struct SATollgateDropedItem : public SSpecifyTollgateMsg
{
	bool bMopUpOperation;
	int commonItemNum;///��ͨ��Ʒ�ж�������
	int bossItemNum;///boss�������Ʒ�ж�������
	int arrayBossItemID[MAX_BOSS_DROPCOUNT];///<��ʱд��boss��������Ʒ��
	int arrayBossItemCount[MAX_BOSS_DROPCOUNT];///<��ʱд��boss��������Ʒ��
	int arrayCommonItemList[MAX_ITEM_NUM_SYN_TO_CLIENT];
	SATollgateDropedItem()
		:SSpecifyTollgateMsg()
	{
		commonItemNum = 0;
		bossItemNum = 0;
		bMopUpOperation = false;
		_protocol = SDuplicateMsg::ANSWER_TOLLGATE_DROPED_ITEM;
		memset(arrayBossItemID, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayBossItemCount, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayCommonItemList, 0, sizeof(int) * MAX_ITEM_NUM_SYN_TO_CLIENT);
	}
};

struct SHeroExpInfor
{
	DWORD m_dwHeroID;///>�佫����
	DWORD m_dwHeroExp;
	DWORD m_dwIncreasedExp;
	DWORD m_dwHeroLevel;
};

///���߿ͻ��ˣ����������Ѿ�����
struct SATollgateEarningClearedInfor : public SSpecifyTollgateMsg
{
	bool bcleared;
	bool bMopUpOperation;
	int heroNum;
	SHeroExpInfor heroExpInfor[MAX_TOLLGATE_HERO_NUM];
	SATollgateEarningClearedInfor()
		:SSpecifyTollgateMsg()
	{
		bcleared = false;
		bMopUpOperation = false;
		heroNum = 0;
		_protocol = SDuplicateMsg::ANSWER_TOLLGATE_EARNING_CLEARED;
	}
};

//======================�佫����չ�����淨����Ϣ����
DECLARE_MSG_MAP(SHeroExtendsGamplayMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_HEROEXTENDSGAMEPLAY)
HERO_HUNGING,///Ӣ�۹һ�Ѱ��
HERO_TRAINING,///Ӣ��ѵ��
END_MSG_MAP()

/// ����佫����չ���淨����Ϣ����
struct HeroHungingHuntingMsg : SHeroExtendsGamplayMsg
{
	HeroHungingHuntingMsg()
	{
		SHeroExtendsGamplayMsg::_protocol = SHeroExtendsGamplayMsg::HERO_HUNGING;
		heroID = 0;
	}

	enum
	{
		REQUEST_START_HUNTING, ///�һ�Ѱ��������
		REQUEST_HUNTING_LIQUIDATION,///������������
		REQUEST_SURPRISE_PRESENTS,///�������
		ANSWER_HUNTING_PERMISSION,///�Ƿ�����һ�Ѱ������Ϣ
		ANSWER_HUNTING_LIQUIDATION_RESULT,///Ѱ����������Ƿ�ɹ�����Ϣ
		ANSWER_SURPRISE_PRESENT,///Ѱ�������е����⽱����ͬ��
	};

	BYTE _protocol;
	int heroID;
};

///�Կͻ��˷������佫Ӣ�۹һ�Ѱ����һЩ����ļ򵥻ظ�
struct AnswerHeroHuntingApplyMsg : HeroHungingHuntingMsg
{
	bool bSucceed;

	explicit AnswerHeroHuntingApplyMsg(BYTE protocolType)
	{
		bSucceed = false;
		HeroHungingHuntingMsg::_protocol = protocolType;
	}
};

struct AnswerSynSurprisePresentsMsg : HeroHungingHuntingMsg
{
	int itemType; ///�����ͻ��˵���Ʒ�����ͣ���AddGoods��ص���Ʒ�����ϸ�һ��
	int itemID; ///�����ͻ��˵���Ʒ��ID�������Ԫ������Ǯ�ȣ���IDΪ0
	int itemNum; ///�����ͻ��˵���Ʒ������

	AnswerSynSurprisePresentsMsg()
	{
		HeroHungingHuntingMsg::_protocol = HeroHungingHuntingMsg::ANSWER_SURPRISE_PRESENT;
		itemType = 0;
		itemID = 0;
		itemNum = 0;
	}
};

/// ���ѵ������佫�ĵ������Ϣ����
struct HeroTrainingMsg : SHeroExtendsGamplayMsg
{
	HeroTrainingMsg()
	{
		SHeroExtendsGamplayMsg::_protocol = SHeroExtendsGamplayMsg::HERO_TRAINING;
		heroID = 0;
		trainingFlag = 0;
	}

	enum
	{
		REQUEST_START_TRAINING,///����ѵ���佫
		REQUEST_TRAINING_LIQUIDATION,///�������ѵ������Ľ���
		ANSWER_TRAINING_PERMISSION,///����ѵ���佫
		ANSWER_TRAINING_LIQUIDATION_RESULT,///ѵ��������Ľ����Ƿ�ɹ�����Ϣ
	};

	BYTE _protocol;
	int heroID;
	BYTE trainingFlag;
};

//======================�佫�������Э�鶨��
DECLARE_MSG_MAP(SHeroUpgrade, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_HEROUPGRADE)
HeroUpgrade_AttachEquipment,
HeroUpgrade_RankRise,
HeroUpgrade_StarLevelRise,
HeroUpgrade_ConscribeHero,
HeroUpgrade_UpgradeLevel,
END_MSG_MAP()

struct  SBaseHeroUpgrade : public SHeroUpgrade
{
	DWORD m_HeroID;
	SBaseHeroUpgrade()
	{
		m_HeroID = 0;
	}
};


struct SQHeroAttachEquipment : public SBaseHeroUpgrade
{
	DWORD m_EquipmentNum;
	DWORD m_EquipmentID[];
	SQHeroAttachEquipment()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_AttachEquipment;
		m_EquipmentNum = 0;
	}
}; 


struct SAHeroAttachEquipment : public SBaseHeroUpgrade
{
	BYTE m_SuccessFlag;
	DWORD m_EquipmentNum;
	DWORD m_EquipmentIDs[EQUIP_MAX];
	SAHeroAttachEquipment()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_AttachEquipment;
		m_EquipmentNum = 0;
		m_SuccessFlag = 0;
	}
};

struct SQHeroRankRiseRequest : public SBaseHeroUpgrade
{
	DWORD m_CurRank;
	SQHeroRankRiseRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_RankRise;
		m_CurRank = 0;
	}
};

struct SAHeroRankRiseResult : public SBaseHeroUpgrade
{
	BYTE m_SuccessFlag;
	SAHeroRankRiseResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_RankRise;
		m_SuccessFlag = 0;
	}
};

struct SQHeroStarLevelRiseRequest : public SBaseHeroUpgrade
{
	DWORD m_CurStarLevel;
	SQHeroStarLevelRiseRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_StarLevelRise;
		m_CurStarLevel = 0;
	}
};

struct SAHeroStarLevelRiseResult : public SBaseHeroUpgrade
{
	DWORD dwMoney;
	bool bSuccess;
	SAHeroStarLevelRiseResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_StarLevelRise;
		dwMoney = 0;
		bSuccess = false;
	}
};

///<�ͻ�����ļ�佫����//
struct SQConscribeHeroRequest : public SBaseHeroUpgrade
{
	SQConscribeHeroRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_ConscribeHero;
	}
};

///<������ļ�佫���//
struct SAConscribeHeroResult : public SBaseHeroUpgrade
{
	DWORD dwSoulStoneID;
	DWORD dwSoulStoneNum;
	DWORD dwMoney;
	SAConscribeHeroResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_ConscribeHero;
	}
};

struct SQHeroUpgradeLevelRequest : public SBaseHeroUpgrade
{
	DWORD dwExpPotionID;
	DWORD dwExpPotionNum;
	SQHeroUpgradeLevelRequest()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_UpgradeLevel;
	}
};

struct SAHeroUpgradeLevelResult : public SBaseHeroUpgrade
{
	DWORD dwLevel;
	DWORD dwExperienceAmount;
	SAHeroUpgradeLevelResult()
	{
		SHeroUpgrade::_protocol = HeroUpgrade_UpgradeLevel;
	}
};

DECLARE_MSG_MAP(SStorageBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_STORAGE)
SStorage_None,
SStorage_SellItem,
REQUEST_USECONSUMABLE,///����ʹ������Ʒ
ANWSER_USECONSUMABLE,///ʹ������Ʒ�Ļظ�
REQUEST_SYNTHETIZE,
ANWSER_SYNTHETIZEINFOR,
REQUEST_RESOLVEITEM,///����ֽ���Ʒ
ANSWER_RESOLVEITEM,///�ֽ���Ʒ�Ļظ�
END_MSG_MAP()

///@brief �ͻ��˷�������ϳ�װ��
struct SQSynthetizeItem : SStorageBaseMsg
{
	DWORD synthetizeTimes;
	DWORD debrisID;

	SQSynthetizeItem()
	{
		synthetizeTimes = 0;
		debrisID = 0;
		_protocol = SStorageBaseMsg::REQUEST_SYNTHETIZE;
	}
};

///@brief װ���ϳ��Ƿ�ɹ�����Ϣ
struct SASynthetizeItemInfor : SStorageBaseMsg
{
	bool bsucceed;

	SASynthetizeItemInfor()
	{
		bsucceed = false;
		_protocol = SStorageBaseMsg::ANWSER_SYNTHETIZEINFOR;
	}
};

///@brief ������Ʒ�Ĵ�����Ϣ
struct SStorageItemProcess : SStorageBaseMsg
{
	DWORD m_dwItemID;
	DWORD m_dwProcessCount;

	SStorageItemProcess()
	{
		m_dwItemID = 0;
		m_dwProcessCount = 0;
	}
};

struct SUseConsumableAnswer : SStorageBaseMsg
{
	DWORD itemNum;
	DWORD itemList[MAX_ITEM_NUM_GAINED_FROM_CHEST];///����3������Ϣ��ǰitemNumλΪ��Ʒ�����ͣ���itemNumλΪ��Ʒ��ID����itemNumλΪ��Ʒ������

	SUseConsumableAnswer()
	{
		itemNum = 0;
		memset(itemList, 0, sizeof(DWORD) * MAX_ITEM_NUM_GAINED_FROM_CHEST);
		_protocol = SStorageBaseMsg::ANWSER_USECONSUMABLE;
	}
};

//<@brief ���տͻ������������Ʒ��Ϣ//
struct SRequstSellItem : SStorageItemProcess
{
	SRequstSellItem()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::SStorage_SellItem;
		m_dwItemID = 0;
		m_dwProcessCount = 0;
	}
};

//<@brief ���������Ʒ����Ľ��//
struct SSellItemResultMsg : SStorageBaseMsg
{
	BYTE m_CurrencyType;	//<�������//
	DWORD m_dwProperty;		//<��������//
	DWORD m_dwItemID;		//<��ƷID//
	DWORD m_dwSellCount;	//<���۸���//
	SSellItemResultMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::SStorage_SellItem;
		m_CurrencyType = 0;
		m_dwItemID = 0;
		m_dwSellCount = 0;
		m_dwProperty = 0;
	}
};

///@brief ����ֽ���Ʒ
struct SRequestResolveItemMsg : SStorageBaseMsg
{
	int itemArrayNum;
	int resolveItemArray[];

	SRequestResolveItemMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::REQUEST_RESOLVEITEM;
		itemArrayNum = 0;
	}
};

///@brief ����ֽ���Ʒ��Ļظ�
struct SAnswerResolveItemMsg : SStorageBaseMsg
{
	bool bSuccess;

	SAnswerResolveItemMsg()
	{
		SStorageBaseMsg::_protocol = SStorageBaseMsg::ANSWER_RESOLVEITEM;
		bSuccess = false;
	}
};

DECLARE_MSG_MAP(SMallBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_MALL)
MALL_BUYCOMMODITY,			///< ������Ʒ
MALL_REFRESH,				///< �ֶ�ˢ����Ʒ
MALL_AUTOREFRESH,			///< �Զ�ˢ����Ʒ
MALL_OPENMALL,				///< �����̳�
END_MSG_MAP()

struct SMallMsgInfo : SMallBaseMsg
{
	BYTE m_MallType;	///<�̵�����/
	SMallMsgInfo()
	{
		m_MallType = 0;
	}
};

struct SBuyCommodityRequest : SMallMsgInfo
{
	DWORD m_dwIndex;	///<��Ʒ����/
	SBuyCommodityRequest()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_BUYCOMMODITY;
		m_MallType = 0;
		m_dwIndex = 0;
	}
};

struct SBuyCommodityResult : SMallMsgInfo
{
	BYTE m_CurrencyType;	///<��������/
	DWORD m_dwProperty;		///<��������/
	DWORD m_dwIndex;		///<��Ʒ����/
	SBuyCommodityResult()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_BUYCOMMODITY;
		m_CurrencyType = 0;
		m_dwProperty = 0;
		m_dwIndex = 0;
	}
};

struct SRefreshCommodityRequest : SMallMsgInfo
{
	SRefreshCommodityRequest()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_REFRESH;
	}
};

struct SRefreshCommodityResult : SMallMsgInfo
{
	DWORD m_dwProperty;
	DWORD m_dwNextRefreshTime;
	DWORD m_dwRefreshedCount;
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	SRefreshCommodityResult()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_REFRESH;
		m_dwProperty = 0;
		m_dwNextRefreshTime = 0;
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
	}
};

struct SAutoRefreshCommodityMsg : SMallMsgInfo
{
	DWORD m_dwNextRefreshTime;
	DWORD m_dwNextRefreshRemainingSeconds;
	SSanguoCommodity m_arrCommodity[MALL_COMMODITY_NUM];
	SAutoRefreshCommodityMsg()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_AUTOREFRESH;
		m_dwNextRefreshTime = 0;
		m_dwNextRefreshRemainingSeconds = 0;
		memset(m_arrCommodity, 0, sizeof(SSanguoCommodity) * MALL_COMMODITY_NUM);
	}
};

///�����̳���Ϣ
struct SOpenMallMsg : SMallMsgInfo
{
	SOpenMallMsg()
	{
		SMallBaseMsg::_protocol = SMallBaseMsg::MALL_OPENMALL;
	}
};

DECLARE_MSG_MAP(SNewbieGuideMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_NEWGUIDE)
Guide_SetInfo,		///����������Ϣ
Guide_ChangeScene,	///ս����������л�����
Guide_NextStep,		///֪ͨ�ͻ���ָ����һ��
END_MSG_MAP()

struct SQSetNewbieGuideInfo : SNewbieGuideMsg
{
	BYTE index;
	SNewbieGuideDInfo info;
	SQSetNewbieGuideInfo()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_SetInfo;
		index = 0;
	}
};

///@����ս����� �л�����
struct SChangeScene : SNewbieGuideMsg
{
	SChangeScene()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_ChangeScene;
	}
};

///@֪ͨ�ͻ��� ָ��ǰ����һ��
struct SAGoNextStep : SNewbieGuideMsg
{
	SAGoNextStep()
	{
		SNewbieGuideMsg::_protocol = SNewbieGuideMsg::Guide_NextStep;
	}
};

///<ͬ�����,�õ�����,��̨������ǰ̨,��ҪΪaddgoods���,��ͭǮԪ����
struct SAAddGoodsSYNMsg : SSGPlayerMsg
{
	bool bIDIndex;		///Ҫͬ������Ʒ���Ƿ�����ID�������ģ����Ϊ�棬��ôitemList�оͷֳ��������֣����Ϊ�٣���itemList��Ϊָ����Goods������
	DWORD iType;
	DWORD itemGroupNum;	///Ҫͬ������Ʒ�����͵�����
	DWORD itemList[MAX_ITEM_NUM_SYN_TO_CLIENT];///ÿ����Ʒ��Ӧ��ID�Ͷ�Ӧ�ĸ�����ǰitemGroupNumԪ��ΪID�������ŵ�itemGroupNumΪ��Ӧ�ĸ�������Ϊnullptr,���֧��64����Ʒ��ͬʱ���
	SAAddGoodsSYNMsg()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ADDGOODS_SYN;
		iType = 0;
		itemGroupNum = 0;
	}
};

///<ͬ�����,�۳�����,��̨������ǰ̨,��ҪΪDecgoods���,��ͭǮԪ����
struct SADecGoodsSYNMsg : SSGPlayerMsg
{
	bool bIDIndex;		///Ҫͬ������Ʒ���Ƿ�����ID�������ģ����Ϊ�棬��ôitemList�оͷֳ��������֣����Ϊ�٣���itemList��Ϊָ����Goods������
	DWORD iType;
	DWORD itemGroupNum;	///Ҫͬ������Ʒ�����͵�����
	DWORD itemList[128];///ÿ����Ʒ��Ӧ��ID�Ͷ�Ӧ�ĸ�����ǰitemGroupNumԪ��ΪID�������ŵ�itemGroupNumΪ��Ӧ�ĸ�������Ϊnullptr,���֧��64����Ʒ��ͬʱɾ��
	SADecGoodsSYNMsg()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_DECGOODS_SYN;
		iType = 0;
		itemGroupNum = 0;
	}
};

//=====================�佫��������(�������ܵ�Ŀ۳� �����)==================
DECLARE_MSG_MAP(SSkillUpgrade, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_SKILLUPGRADE)
SkillUpgrade_LearnSkill,
SkillUpgrade_BuyPoint,
SkillUpgrade_CheckPoint,
END_MSG_MAP()

struct  SQUpgradeHeroSkill : public SSkillUpgrade
{
	DWORD dwHeroID;
	//DWORD dwValidNum;
	//DWORD dwSkillID[MAX_SKILLNUM];
	//DWORD dwSkillLevel[MAX_SKILLNUM];
	DWORD dwSkillID;
	DWORD dwSkillLevel;
	SQUpgradeHeroSkill()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_LearnSkill;
		dwHeroID = 0;
		//dwValidNum = 0;
		dwSkillID = 0;
		dwSkillLevel = 0;
	}
};

///@breif �����佫��������
struct  SAUpgradeHeroSkillResult : public SSkillUpgrade
{
	DWORD dwHeroID;
	DWORD dwValidNum;
	DWORD dwSkillPointCost;
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	DWORD dwSkillID[MAX_SKILLNUM];
	DWORD dwSkillLevel[MAX_SKILLNUM];
	DWORD dwMoney;
	bool bResult;
	SAUpgradeHeroSkillResult()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_LearnSkill;
		dwHeroID = 0;
		dwValidNum = 0;
		dwSkillPointCost = 0;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
		memset(dwSkillID, 0, sizeof(DWORD) * MAX_SKILLNUM);
		memset(dwSkillLevel, 0, sizeof(DWORD) * MAX_SKILLNUM);
		dwMoney = 0;
		bResult = false;
	}
};

///<���չ����ܵ�ṹ
struct SQBuySkillPoint : public SSkillUpgrade
{
	SQBuySkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_BuyPoint;
	}
};

///<�������ܵ㹺��ṹ
struct SABuySkillPointResult : public SSkillUpgrade
{
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	DWORD dwPurchasedTimes;
	DWORD dwDiamond;
	bool bResult;
	SABuySkillPointResult()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_BuyPoint;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
		dwPurchasedTimes = 0;
		dwDiamond = 0;
		bResult = false;
	}
};

struct SQCheckSkillPoint : public SSkillUpgrade
{
	SQCheckSkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_CheckPoint;
	}
};

struct SACheckSkillPoint : public SSkillUpgrade
{
	DWORD dwSkillPoint;
	DWORD dwRemainingTime;
	SACheckSkillPoint()
	{
		SSkillUpgrade::_protocol = SkillUpgrade_CheckPoint;
		dwSkillPoint = 0;
		dwRemainingTime = 0;
	}
};

//ǩ�����
DECLARE_MSG_MAP(SCHECKINBaseMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_CHECKIN)
CHECKIN_Quest,
CHECKIN_DATASYN,//����ͬ��
END_MSG_MAP()


struct SQCHECKIN_Quest : SCHECKINBaseMsg
{
	BYTE itype; //1ÿ�����,2��ǩ
	SQCHECKIN_Quest()
	{
		SCHECKINBaseMsg::_protocol = SCHECKINBaseMsg::CHECKIN_Quest;
	}
};

//����ͬ��,��½ʱǩ����ʹ��
struct SACHECKIN_DATASYN : SCHECKINBaseMsg
{
	DWORD beginTime;
	DWORD lastTime;
	DWORD lastCount;
	DWORD addCount;
	/*DWORD turn;
	DWORD days;*/
	SACHECKIN_DATASYN()
	{
		SCHECKINBaseMsg::_protocol = SCHECKINBaseMsg::CHECKIN_DATASYN;
	}
};

//�ű���Ϣ
DECLARE_MSG_MAP(SSCRIPTMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_SCRIPT)
DATASYN_FIRST,//�״ε�½����ͬ��
DATASYN_NOTIFICATION,//��½��ʱ��ͬ��ϵͳ����֪ͨ������
DATASYN_MISSION,///��·�ζ�ʱ��ͬ��ÿ����������ݵ��ͻ���
DATASYN_HERO_EXTENDS_DATAS,///<Ӣ�۶�������ݣ������佫�һ�Ѱ�������ݣ��佫ѵ��������
END_MSG_MAP()
struct SADATASYN_FIRST : SSCRIPTMsg
{
	DWORD lenth;
	DWORD sdata[MAX_SYN_SCRIPT_LENTH];
	SADATASYN_FIRST()
	{
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_FIRST;
	}
};

/// <summary>
/// ֪ͨ���ݽṹ��
/// </summary>
struct SNotificationData
{
	/// <summary>
	/// ֪ͨ��ID
	/// </summary>
	int notificationID;
	/// <summary>
	/// ֪ͨ�¼��ἤ��ľ���ʱ�䣬Ϊ����������������1970-1-1 00��00��00��ʼ����
	/// </summary>
	int alarmTime;
	/// <summary>
	/// ֪ͨ����
	/// </summary>
	char notificationTitle[64];
	/// <summary>
	/// ֪ͨ����
	/// </summary>
	char notificationContents[128];

	SNotificationData()
	{
		alarmTime = 0;
		notificationID = 0;
		memset(notificationTitle, 0, sizeof(notificationTitle));
		memset(notificationContents, 0, sizeof(notificationContents));
	}
};

///ͬ��ϵͳ����֪ͨ���ݵ��ͻ���
struct SDATASYN_NOTIFICATION : SSCRIPTMsg
{
	int length;
	SNotificationData datas[MAX_SYN_NOTIFICATION_LENGTH];

	SDATASYN_NOTIFICATION()
	{
		length = 0;
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_NOTIFICATION;
	}
};

///ͬ���������ݵ��ͻ���
struct SDATASYN_MISSION : SSCRIPTMsg
{
	int length;
	int datas[MISSION_NUM];

	SDATASYN_MISSION()
	{
		length = 0;
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_MISSION;
	}
};

///ͬ��Ӣ�۵���չ���淨���ݣ�����Ӣ�۵Ĺһ�Ѱ�������ݣ�ѵ��������
struct SDATASYN_HEROEXTENDSGAMEPLAY : SSCRIPTMsg
{
	int length;
	int datas[MAX_HERO_NUM * 2];///ǰlengthλΪ����佫��չ�淨�Ļ�����Ϣ����lengthλΪ����佫����ĳ����չ���淨��ʱ��

	SDATASYN_HEROEXTENDSGAMEPLAY()
	{
		length = 0;
		memset(datas, 0, sizeof(datas));
		SSCRIPTMsg::_protocol = SSCRIPTMsg::DATASYN_HERO_EXTENDS_DATAS;
	}
};

DECLARE_MSG_MAP(SChaseThiefMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_CHASETHIEF)
CHASETHIEF_ACTIVATE,	//����׷������
CHASETHIEF_PERPATIONCHANLLENGE,	//��ս׼��
CHASETHIEF_STARTCHANLLENGE, //��ʼս��
CHASETHIEF_REFUSALENTER,	//�ܾ�����ս��
CHASETHIEF_CLEARING,	//����
CHASETHIEF_REWARD,		//ս������
END_MSG_MAP()

struct SThiefActivate : SChaseThiefMsg
{
	DWORD dwRemainingTime;
	SThiefActivate()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_ACTIVATE;
		dwRemainingTime = 0;
	}
};

struct SThiefRequestPerpationChanllenge : SChaseThiefMsg
{
	SThiefRequestPerpationChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_PERPATIONCHANLLENGE;
	}
};

struct SThiefAnswerPerpationChanllenge : SChaseThiefMsg
{
	bool permission;
	int tollgateID;
	SThiefAnswerPerpationChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_PERPATIONCHANLLENGE;
		permission = false;
		tollgateID = 0;
	}
};

struct SThiefRequestStartChanllenge : SChaseThiefMsg
{
	int heroes[MAX_THIEF_HERO_LIMIT];
	SThiefRequestStartChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_STARTCHANLLENGE;
		memset(heroes, 0, sizeof(int) * MAX_THIEF_HERO_LIMIT);
	}
};

struct SThiefAnswerStartChanllenge : SChaseThiefMsg
{
	int commonItemNum;///��ͨ��Ʒ�ж�������
	int bossItemNum;///boss�������Ʒ�ж�������
	int arrayBossItemID[MAX_BOSS_DROPCOUNT];///<��ʱд��boss��������Ʒ��
	int arrayBossItemCount[MAX_BOSS_DROPCOUNT];///<��ʱд��boss��������Ʒ��
	int arrayCommonItemList[MAX_ITEM_NUM_SYN_TO_CLIENT];
	SThiefAnswerStartChanllenge()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_STARTCHANLLENGE;
		commonItemNum = 0;
		bossItemNum = 0;
		memset(arrayBossItemID, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayBossItemCount, 0, sizeof(int) * MAX_BOSS_DROPCOUNT);
		memset(arrayCommonItemList, 0, sizeof(int) * MAX_ITEM_NUM_SYN_TO_CLIENT);
	}
};

struct SThiefRefusalEnter : SChaseThiefMsg
{
	bool loseEffectiveness;
	SThiefRefusalEnter()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_REFUSALENTER;
		loseEffectiveness = false;
	}
};

struct SThiefRequestClearing : SChaseThiefMsg
{
	bool win;
	SThiefRequestClearing()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_CLEARING;
		win = false;
	}
};

struct SThiefAnswerClearing : SChaseThiefMsg
{
	bool complete;
	SThiefAnswerClearing()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_CLEARING;
		complete = false;
	}
};

struct SThiefFightRewardMsg : SChaseThiefMsg
{
	int rewardItemNum;
	int items[MAX_THIEF_REWARD_NUM];
	SThiefFightRewardMsg()
	{
		SChaseThiefMsg::_protocol = SChaseThiefMsg::CHASETHIEF_REWARD;
		rewardItemNum = 0;
		memset(items, 0, sizeof(int) * MAX_THIEF_REWARD_NUM);
	}
};


///@�ͻ������������Ϣ
struct  SQChangePlayerName : SSGPlayerMsg
{	 
	char name[CONST_USERNAME];
	SQChangePlayerName()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_CHANGENAME;
	}
	
};
///@�ͻ��˸�����Ϣ�ظ�
struct  SAChangeName : SSGPlayerMsg
{
	SAChangeName()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_CHANGENAME;
	}
	bool falg;
	int ChangeNameCount;

};
///@breif �ͻ��������ȡ�����
struct SQGetActReward : SSGPlayerMsg
{
	int actType;///<�����
	int reawrdIndex; ///<��������
	SQGetActReward()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ACTIVITY;
		actType = 0;
		reawrdIndex = 0;
	}
};


///@breif �ͻ��˷�����ȡ��������
struct SAGetActReward : SSGPlayerMsg
{
	int actType; ///<�����
	int reawrdIndex;	 ///��������
	byte result;			///<��ȡ��ɹ�����1��ʧ�ܷ���0
	SAGetActReward()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_ACTIVITY;
		result = 0;
		reawrdIndex = 0;
	}
};


///С�ͽ�������ص�һЩ��Ϣ
DECLARE_MSG_MAP(SRewardState, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_REWARDSTATE)
CHARGESUM,
END_MSG_MAP()

///@breif ��ȡ�ۼƳ�ֵ�����������
struct SARewardState : SRewardState
{
	char rewardState[20]; //'0'Ϊδ��ȡ�� '1'Ϊ����ȡ
	DWORD chargeSum; //��ұ��λ��ֵ����
	SARewardState()
	{
		SRewardState::_protocol = SRewardState::CHARGESUM;
		memset(rewardState, 0, sizeof(char) * 20);
		chargeSum = 0;
	}
};

///@brief Ԫ���һ������������Ϣ
DECLARE_MSG_MAP(SRecharge, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_RECHARGE)
SANGUO_RECHARGE,
SANGUO_RECHARGE_REQUEST,
SANGUO_RECHARGE_RESET,
SANGUO_MONTHLY_RECHARGE_RESULT,
END_MSG_MAP()


///@breif �ͻ��˷��ͳ�ֵ��Ϣ
struct SQRecharge : SRecharge
{
	DWORD dwNowPoint;//��������
	WORD wPlatfrom;//ƽ̨ 1 ΢��ƽ̨ 2 QQƽ̨
	char addr[MAX_ADDR_HTTP];
	SQRecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE;
	}
};


///@breif ����˷�����ֵ��Ϣ
struct SARecharge : SRecharge
{
	int ret;//��ֵУ����:1�ɹ�,2ʧ��
	unsigned int nowPoint;//��������
	SARecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE;
	}
};

///@breif ����˷��������¿�
struct SABuyMembership : SRecharge
{
	BYTE byType;//��ֵУ����:1�ɹ�,-1ʧ��, 0ͬ��
	DWORD memberShipData;//�¿�����ʱ��
	SABuyMembership()
	{
		byType = 0;
		memberShipData = 0;
		SRecharge::_protocol = SRecharge::SANGUO_MONTHLY_RECHARGE_RESULT;
	}
};

struct SQResetRecharge : SRecharge //���ó�ֵ
{
	SQResetRecharge()
	{
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_RESET;
	}
};

///@breif ��ֵ����
struct SQRechargeRequst : SRecharge
{
	BYTE byType; //  1 ������Ϸ�� ��2 �����¿�
	DWORD dwDiamond; //����Ԫ����
	SQRechargeRequst()
	{
		byType = -1;
		dwDiamond = 0;
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_REQUEST;
	}
	
};

///@breif ��ֵ����
struct SARechargeRequst : SRecharge
{
	bool bResult;
	BYTE byType; //  1 ������Ϸ�� ��2 �����¿�
	DWORD dwDiamond; //����Ԫ����
	SARechargeRequst()
	{
		bResult = false;
		byType = -1;
		SRecharge::_protocol = SRecharge::SANGUO_RECHARGE_REQUEST;
		dwDiamond = 0;
	}

};


///@brief  ս����֤����������
struct PhysicAttackData
{
	int targetID;
	float damage;
	bool crit;
};

///@brief ս����֤���ܹ�������
struct SkillAttackData
{
	int targetID;
	int skillID;
	float damage;
	bool crit;
};

///@brief ��������ս��ս����֤����
struct ArenaFighterVerifyData
{
	int heroID;
	float maxHealth;
	int physicAttackDataNum;
	int skillAttackDataNum;
	PhysicAttackData physicAttackData[MAX_ARENACHALLENGE_HERONUM];
	SkillAttackData skillAttackData[MAX_ARENACHALLENGE_HERONUM];
	ArenaFighterVerifyData()
	{
		heroID = 0;
		maxHealth = 0.0f;
		physicAttackDataNum = 0;
		skillAttackDataNum = 0;
		memset(physicAttackData, 0, sizeof(PhysicAttackData) * MAX_ARENACHALLENGE_HERONUM);
		memset(skillAttackData, 0, sizeof(SkillAttackData) * MAX_ARENACHALLENGE_HERONUM);
	}
};

///@brief ������ս����֤����
struct ArenaBattleVerifyData
{
	int competitorArenaID;
	int heroID;
	int AttackTimes;
	int myFighterNum;
	int competitorFighterNum;
	ArenaFighterVerifyData myFighterVerifyDatas[MAX_ARENACHALLENGE_HERONUM];
	ArenaFighterVerifyData competitorFighterVerifyDatas[MAX_ARENACHALLENGE_HERONUM];
	ArenaBattleVerifyData()
	{
		competitorArenaID = 0;
		myFighterNum = 0;
		competitorFighterNum = 0;
		memset(myFighterVerifyDatas, 0, sizeof(ArenaFighterVerifyData) * MAX_ARENACHALLENGE_HERONUM);
		memset(competitorFighterVerifyDatas, 0, sizeof(ArenaFighterVerifyData) * MAX_ARENACHALLENGE_HERONUM);
	}
};


///��ȡ�׳影��
struct SQFirstRecharge : SSGPlayerMsg
{
	SQFirstRecharge()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_fIRSTRECHARGE;
	}
};
///�ظ���ȡ�׳影��
struct SAFirstRecharge : SSGPlayerMsg
{
	int res;// 1�ɹ�,2����ȡ,3δ��ֵ
	SAFirstRecharge()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_fIRSTRECHARGE;
	}
};
///@breif ����ص���Ϣ
struct SAWeekRemoney : SSGPlayerMsg
{
	int state;//�������� 0 δ����
	int day;
	int res[7];

	SAWeekRemoney()
	{
		SSGPlayerMsg::_protocol = SSGPlayerMsg::EPRO_SANGUO_WeekReMoney;
		state = 0;
		memset(res, 0, sizeof(res));
	}
};
///������ȡ����s
struct SQWeekRemoney : SSGPlayerMsg
{
	int day;
};

///@��ս
DECLARE_MSG_MAP(SWarMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_War)
	war_join,			//����
	war_teamList,//ȡ��Ӫ��ս��Ա�б�
	war_fight_start,	//ƥ�俪��
	war_fight_res,		//ս�����
	war_fight_res_broadcast,		//ս������㲥
	war_all_end,		//��Ӫ��ʤ��������
	war_exit,			//����˳�
	war_isjoin,			//�Ƿ���
	war_rank,			//���а�
	war_end,			//��ս����
END_MSG_MAP()
//����
struct SQWarJoin : SWarMsg
{
	DWORD dwHeroId[MAX_War_HERONUM];
	SQWarJoin()
	{
		SQWarJoin::_protocol = SWarMsg::war_join;
		memset(dwHeroId, 0, sizeof(DWORD)*MAX_War_HERONUM);
	}
};
//�������
struct SAWarJoin : SWarMsg
{
	DWORD res;//1�ɹ�  ,2Ϊ�Ѿ�����
	DWORD teamNum;//���䵽�ĸ���Ӫ(1,2)
	SAWarJoin()
	{
		SAWarJoin::_protocol = SWarMsg::war_join;
	}
};
//ȡ��Ӫ��ս��Ա�б�
struct SQwar_teamList : SWarMsg
{
	DWORD teamNum;//ȡ�Ķ�
	DWORD beginNum;//�����λ������ȡ20��
	SQwar_teamList()
	{
		SQwar_teamList::_protocol = SWarMsg::war_teamList;
	}
};
//ȡ��Ӫ��ս��Ա�б�
struct SAwar_teamList : SWarMsg
{
	DWORD teamNum;//�Ķ�
	DWORD beginNum;//�����λ������ȡ20��
	char name_all[CONST_USERNAME*MAX_WAR_NAME_LIST];
	SAwar_teamList()
	{
		SAwar_teamList::_protocol = SWarMsg::war_teamList;
		memset(name_all, 0, CONST_USERNAME*MAX_WAR_NAME_LIST);
	}
};
//ƥ�俪��
struct SAwar_fight_start : SWarMsg
{
	char name_other[CONST_USERNAME];//��������
	DWORD heroInfo_other[MAX_WAR_HERO_INFO*MAX_War_HERONUM];//16��������������(1-16)+8����������(17-24)+2��Ѫ��ŭ��(25-26)+1�佫id(27)
	DWORD heroInfo[MAX_War_HERONUM*3];//�Լ�5���佫��Ѫ��,ŭ��,id
	DWORD score1;//1�����
	DWORD score2;//2�����
	DWORD winAll;//������ʤ
	SAwar_fight_start()
	{
		SAwar_fight_start::_protocol = SWarMsg::war_fight_start;
		memset(name_other, 0, CONST_USERNAME);
		memset(heroInfo_other, 0, sizeof(DWORD)*MAX_WAR_HERO_INFO*MAX_War_HERONUM);
		memset(heroInfo, 0, sizeof(DWORD) * 3 * MAX_War_HERONUM);
	}
};

//ս�����
struct SQwar_fight_res : SWarMsg
{
	DWORD res;//1Ӯ,2��  
	DWORD heroInfo[3 * MAX_War_HERONUM];//�Լ��佫��id,Ѫ��,ŭ��
	SQwar_fight_res()
	{
		SQwar_fight_res::_protocol = SWarMsg::war_fight_res;
		memset(heroInfo, 0, sizeof(DWORD) * 3 * MAX_War_HERONUM);
	}
};
//ս�������������
struct SAwar_fight_res : SWarMsg
{

	DWORD res;//1Ӯ,2�� 100 ������
	DWORD winInfo[6];//��ʤ��Ϣ --ʤ������,ʧ�ܳ���,��ǰ��ʤ����,�����ʤ����,��������,ͭǮ����
	SAwar_fight_res()
	{
		SAwar_fight_res::_protocol = SWarMsg::war_fight_res;
		memset(winInfo, 0, sizeof(DWORD)*4);
	}
};

//ս������㲥
struct SAwar_fight_res_broadcast : SWarMsg
{
	char name_our[CONST_USERNAME];//������Ӫ�������
	char name_other[CONST_USERNAME];//�Է���Ӫ�������
	DWORD res;//1Ӯ,2��
	DWORD winNum;//��ʤ����

	SAwar_fight_res_broadcast()
	{
		SAwar_fight_res_broadcast::_protocol = SWarMsg::war_fight_res_broadcast;
		memset(name_our, 0, CONST_USERNAME);
		memset(name_other, 0, CONST_USERNAME);
	}
};
//����˳�
struct SQwar_exit : SWarMsg
{
	SQwar_exit()
	{
		SQwar_exit::_protocol = SWarMsg::war_exit;
	}
};
//����˳�����
struct SAwar_exit : SWarMsg
{
	SAwar_exit()
	{
		SAwar_exit::_protocol = SWarMsg::war_exit;
	}
};

//��������Ƿ���
struct SQwar_isjoin : SWarMsg
{
	SQwar_isjoin()
	{
		SQwar_isjoin::_protocol = SWarMsg::war_isjoin;
	}
};
//����˳�����
struct SAwar_isjoin : SWarMsg
{
	DWORD isjoin;//1 �Ѿ��μ�;2 δ�μ�
	DWORD team;//������
	SAwar_isjoin()
	{
		SAwar_isjoin::_protocol = SWarMsg::war_isjoin;
	}
};
//��ս����
struct SAWarEnd : SWarMsg
{
	DWORD teamNum;//Ӯ����Ӫ
	DWORD score1;//1�����
	DWORD score2;//2�����
	SAWarEnd()
	{
		SAWarEnd::_protocol = SWarMsg::war_end;
	}
};


//����������а�
struct SQwar_rank : SWarMsg
{
	SQwar_rank()
	{
		SQwar_rank::_protocol = SWarMsg::war_rank;
	}
};

//���а����
struct SAwar_rank : SWarMsg
{
	char name[CONST_USERNAME];
	DWORD win;//��ʤ��
	DWORD lv;
	DWORD icon;
	DWORD team;

	char name2[CONST_USERNAME];
	DWORD win2;//��ʤ��
	DWORD lv2;
	DWORD icon2;
	DWORD team2;
	SAwar_rank()
	{
		SAwar_rank::_protocol = SWarMsg::war_rank;
		memset(name, 0, CONST_USERNAME);
		memset(name2, 0, CONST_USERNAME);
	}
};

///@���а�
DECLARE_MSG_MAP(SRankMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_Rank)
rank_get,			//ȡ���а�
rank_common,//��ͨ���а�
rank_hero,	//�������а�
END_MSG_MAP()
//ȡ���а�
struct SQRank : SRankMsg
{
	DWORD itype;//���а�����
	SQRank()
	{
		SQRank::_protocol = SRankMsg::rank_get;
	}
};
///�ظ�ȡ���а�,��ͨ
struct SARank_common : SRankMsg
{
	DWORD itype;//���а�����
	RankList_SG list[MAX_Rank_List];
	SARank_common()
	{
		SARank_common::_protocol = SRankMsg::rank_common;
	}
};
///�ظ�ȡ���а�,����
struct SARank_hero : SRankMsg
{
	DWORD itype;//���а�����
	RankList_hero_SG list[MAX_Rank_List];
	SARank_hero()
	{
		SARank_hero::_protocol = SRankMsg::rank_hero;
	}
};


///@�һ��
DECLARE_MSG_MAP(SExcheangeMsg, SSGPlayerMsg, SSGPlayerMsg::EPRO_SANGUO_Excheange)
getExchange,			//ȡ����,�콱
exchangeBack,//�콱����
exchangeData,	//����ȫ������
END_MSG_MAP()

///�һ�� ��ȡ
struct SQGetExcheange : SExcheangeMsg
{
	DWORD itype; //1Ϊȡ�һ����� ,2�һ�����
	DWORD index;//�ڼ���
	DWORD num;//�һ�����
	SQGetExcheange()
	{
		SQGetExcheange::_protocol = SExcheangeMsg::getExchange;
	}
};
///�һ�� �һ��󷵻���Ϣ
struct SAexchangeBack : SExcheangeMsg
{
	DWORD res; //������� 1�ɹ�,2��������,3��Ʒ����,4�δ����
	DWORD index; //�ڼ����ӻ
	DWORD value; //�������ֵ
	SAexchangeBack()
	{
		SAexchangeBack::_protocol = SExcheangeMsg::exchangeBack;
	}
};
///�һ�� ����ͬ��
struct SAexchangeData : SExcheangeMsg
{
	DWORD exData[MaxExchangeData];//��Ҷһ������
	SAexchangeData()
	{
		SAexchangeData::_protocol = SExcheangeMsg::exchangeData;
		memset(exData, 0, MaxExchangeData*sizeof(DWORD));
	}
};

