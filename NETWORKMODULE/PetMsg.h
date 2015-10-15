#pragma once
#include "NetModule.h"
#include "../networkmodule/playertypedef.h"
#define MAX_PET_NEEDNUM 5
//=============================================================================================
DECLARE_MSG_MAP(SPetMsg, SMessage, SMessage::EPRO_FIGHTPET_MESSAGE)
//{{AFX
//2014/5/22 add by ly
EPRO_PET_EXTRACT,	//请求卡牌抽取宠物消息
EPRO_PET_CHIPCOMPOSE,	//请求碎片合成宠物消息
EPRO_PET_MERGER,	//宠物吞并消息
EPRO_PET_BREACH,	//宠物突破到下一个星级
EPRO_PET_RENAME,	//宠物重命名
EPRO_PET_SKILLUPGRADE,	//宠物技能升级
EPRO_PET_SUISHENFIGHT,	//宠物跟随玩家一起出战
EPRO_PET_SHAPESHIFTFIGHT,	//玩家变身宠物出战
EPRO_PET_GETCARDPETINFO,	//获取卡牌宠物列表信息
EPRO_PET_USEPETITEM,	//使用宠物道具获得宠物
EPRO_PET_GETPETCARDINFO,	//获取宠物卡牌信息
EPRO_PET_SKILLCONFIG,	//宠物技能配置
EPRO_PET_GETEXPINFO,	//获取宠物的相关经验信息
EPRO_PET_SYNDURABLE,	//同步宠物魂力消息
EPRO_PET_GLODBUY,	//金币直接购买宠物消息
EPRO_PET_RESUMEDURABLENEEDGOLD,		//恢复满宠物的魂力需要的金币消息
EPRO_PET_GOLDRESUMEDURABLE,	//金币恢复满宠物的魂力消息
EPRO_PET_STUDYSKILL,	//通过技能书学习宠物的新技能
EPRO_PET_OPENSTUDYSKILLPLAN,	//打开宠物技能学习面板
EPRO_PET_SYNPETSKILL,	//同步宠物技能
//}}AFX
END_MSG_MAP()

////////////////////////////////////////////////////////
//请求卡牌抽取宠物消息
DECLARE_MSG(SExtractPetMsg, SPetMsg, SPetMsg::EPRO_PET_EXTRACT)
struct SQExtractPetMsg : public SExtractPetMsg
{
	BYTE m_ExtractMode;		//抽取方式；1为免费抽取 2为付费抽取一次 3为付费抽取多次 4为赌博抽取
};

struct SAExtractPetMsg : public SExtractPetMsg
{
	SNewPetData m_ExtractPet;	//抽取到的宠物
};


//请求碎片合成宠物消息
DECLARE_MSG(SChipComposPetMsg, SPetMsg, SPetMsg::EPRO_PET_CHIPCOMPOSE)
struct SQChipComposPetMsg : public SChipComposPetMsg
{
	DWORD m_PetID;	//宠物ID
};

struct SAChipComposPetMsg : public SChipComposPetMsg
{
	SNewPetData m_ClipComposePet;	//合成的宠物
};

//宠物吞并消息
DECLARE_MSG(SMergerPetMsg, SPetMsg, SPetMsg::EPRO_PET_MERGER)
struct SQMergerPetMsg : public SMergerPetMsg
{
	BYTE m_SrcPetIndex;	//吞并宠物发起者的宠物索引
	BYTE m_DestPetIndex[MAX_PET_NEEDNUM];	//被吞并宠物索引数组
	DWORD m_ExpRateItemID;	//吞噬宠物时的经验转换比率道具ID
};

struct SAMergerPetMsg : public SMergerPetMsg
{
	BYTE m_PetLevel;	//吞并后的宠物等级， 为-1时表示失败
	DWORD m_PetExp;	//吞并后的宠物经验
	DWORD m_PetHp;	//吞并后的宠物血量
	DWORD m_PetMp;	//吞并后的宠物精力
	DWORD m_PetDurable;	//吞并后的宠物魂力
};

//宠物突破到下一个星级
DECLARE_MSG(SPetBreachStarMsg, SPetMsg, SPetMsg::EPRO_PET_BREACH)
struct SQPetBreachStarMsg : public SPetBreachStarMsg
{
	BYTE m_PetIndex;	//宠物索引
};

struct SAPetBreachStarMsg : public SPetBreachStarMsg
{
	WORD m_PetStarAndMaxLevel;	//宠物星级和最大等级数；高字节为星级、低字节为最大等级数
};

//宠物重命名
DECLARE_MSG(SRenamePetMsg, SPetMsg, SPetMsg::EPRO_PET_RENAME)
struct SQRenamePetMsg : public SRenamePetMsg
{
	char m_NewName[PETNAMEMAXLEN];		//新名字
	BYTE m_PetIndex;	//宠物索引
};

struct SARenamePetMsg : public SRenamePetMsg
{
	BYTE m_Result;	//成功与否 0表示失败 1表示成功
};

//宠物技能升级
DECLARE_MSG(SPetSkillUpgradeMsg, SPetMsg, SPetMsg::EPRO_PET_SKILLUPGRADE)
struct SQPetSkillUpgradeMsg : public SPetSkillUpgradeMsg
{
	BYTE m_PetIndex;	//宠物索引
	BYTE m_PetSkillIndex;	//宠物技能索引，为0 - 9
};

struct SAPetSkillUpgradeMsg : public SPetSkillUpgradeMsg
{
	BYTE m_Result;	//成功与否 0表示失败 1表示成功
};

//宠物跟随玩家一起出战
DECLARE_MSG(SSuiShenFightMsg, SPetMsg, SPetMsg::EPRO_PET_SUISHENFIGHT)
struct SQSuiShenFightMsg : public SSuiShenFightMsg
{
	BYTE m_PetIndex;	//宠物索引
};

struct SASuiShenFightMsg : public SSuiShenFightMsg
{
	BYTE m_Res;
};

//玩家变身宠物出战
DECLARE_MSG(SShapeshiftFightMsg, SPetMsg, SPetMsg::EPRO_PET_SHAPESHIFTFIGHT)
struct SQShapeshiftFightMsg : public SShapeshiftFightMsg
{
	BYTE m_PetIndex;	//宠物索引
};

struct SAShapeshiftFightMsg : public SShapeshiftFightMsg
{
	BYTE m_Res;
};

//获取卡牌宠物列表信息
DECLARE_MSG(SGetCardPetInf, SPetMsg, SPetMsg::EPRO_PET_GETCARDPETINFO)
struct SQGetCardPetInf : public SGetCardPetInf
{
	BYTE m_CardType;	//卡牌类型 1免费 2一次 3多次 4赌博
};

struct SAGetCardPetInf : public SGetCardPetInf
{
	BYTE m_PetCount;
	DWORD *m_pPetID;
};

//使用宠物道具获得宠物
DECLARE_MSG(SUsePetItem, SPetMsg, SPetMsg::EPRO_PET_USEPETITEM)
struct SAUsePetItem : public SUsePetItem
{
	SNewPetData m_GetPet;	//使用道具获得宠物
};


//获取宠物卡牌信息
DECLARE_MSG(SGetPetCardInf, SPetMsg, SPetMsg::EPRO_PET_GETPETCARDINFO)
struct SQGetPetCardInf : public SGetPetCardInf
{
};

struct SAGetPetCardInf : public SGetPetCardInf
{
	BYTE m_CardCount;	//卡牌数量
	BYTE *m_pCardCanExtractTimes;	//使用一次卡牌可以抽取的宠物的次数
	DWORD *m_pUsePrice;	//抽取价格
	DWORD *m_pCardCountDown;	//卡牌刷新倒计时 到倒计时不为0时，表示不可以抽取操作
};

//宠物技能配置
DECLARE_MSG(SPetSkillCfgInf, SPetMsg, SPetMsg::EPRO_PET_SKILLCONFIG)
struct SQPetSkillCfgInf : public SPetSkillCfgInf
{
	BYTE m_PetIndex;	//宠物索引
	BYTE m_SkillIndex[PETMAXUSESKILLNUM];	//技能配置数据
};

struct SAPetSkillCfgInf : public SPetSkillCfgInf
{
	BYTE m_Res;	//返回操作结果 0表示失败 1表示成功
};


//获取吞噬宠物可以获取的经验
DECLARE_MSG(SPetExpInfMsg, SPetMsg, SPetMsg::EPRO_PET_GETEXPINFO)
struct SQPetExpInfMsg : public SPetExpInfMsg
{
	BYTE m_DestPetIndex[MAX_PET_NEEDNUM];	//被吞并宠物索引数组
	DWORD m_ExpRateItemID;	//吞噬宠物时的经验转换比率道具ID
};

struct SAPetExpInfMsg : public SPetExpInfMsg
{
	DWORD m_PetAddExp;	//吞并后所增加的宠物经验
};


//同步宠物魂力消息
DECLARE_MSG(SPetSynDurableMsg, SPetMsg, SPetMsg::EPRO_PET_SYNDURABLE)
struct SAPetSynDurableMsg : public SPetSynDurableMsg
{
	BYTE m_PetIndex;	//宠物索引
	DWORD m_CurDurable;	//当前魂力
};

//金币直接购买宠物消息
DECLARE_MSG(SGlodDirectBuyPetMsg, SPetMsg, SPetMsg::EPRO_PET_GLODBUY)
struct SQGlodDirectBuyPetMsg : public SGlodDirectBuyPetMsg
{
	DWORD m_PetID;	//宠物ID
};

struct SAGlodDirectBuyPetMsg : public SGlodDirectBuyPetMsg
{
	SNewPetData m_BuyPet;	//购买的宠物
};

//恢复满宠物的魂力需要的金币消息
DECLARE_MSG(SResumeNdGoldMsg, SPetMsg, SPetMsg::EPRO_PET_RESUMEDURABLENEEDGOLD)
struct SQResumeNdGoldMsg : public SResumeNdGoldMsg
{
	BYTE m_PetIndex;	//宠物索引
};

struct SAResumeNdGoldMsg : public SResumeNdGoldMsg
{
	DWORD m_GoldNum;	//需要的金币数
};
//金币恢复满宠物的魂力消息
DECLARE_MSG(SResumePetDurableMsg, SPetMsg, SPetMsg::EPRO_PET_GOLDRESUMEDURABLE)
struct SQResumePetDurableMsg : public SResumePetDurableMsg
{
	BYTE m_PetIndex;	//宠物索引
};

struct SAResumePetDurableMsg : public SResumePetDurableMsg
{
	BYTE m_Result;	//购买的结果
};


//缓存变身前玩家的战斗属性
struct FightProperty
{
	// 对象属性
	DWORD		m_MaxHp;				// 最大生命
	DWORD		m_CurHp;				// 当前生命
	WORD		m_GongJi;				// 攻击
	WORD		m_FangYu;				// 防御
	WORD		m_BaoJi;				// 暴击
	WORD		m_Hit;					//命中
	WORD		m_ShanBi;				// 闪避
	WORD m_uncrit; //抗暴
 	WORD m_wreck;//破击
 	WORD m_unwreck;	//抗破
 	WORD m_puncture;	//穿刺
 	WORD m_unpuncture;	//抗穿
};


//通过技能书学习宠物的新技能
DECLARE_MSG(SPetStudySkillMsg, SPetMsg, SPetMsg::EPRO_PET_STUDYSKILL)
struct SQPetStudySkillMsg : public SPetStudySkillMsg
{
	BYTE m_PetIndex;	//宠物索引
	DWORD m_PetSkillBookID;	//宠物技能书ID
};

struct SAPetStudySkillMsg : public SPetStudySkillMsg
{
	BYTE m_Result;	//学习结果
};

//打开宠物技能学习面板
DECLARE_MSG(SOpenPetStudySkillPlanMsg, SPetMsg, SPetMsg::EPRO_PET_OPENSTUDYSKILLPLAN)
struct SQOpenPetStudySkillPlanMsg : public SOpenPetStudySkillPlanMsg
{
};

struct SAOpenPetStudySkillPlanMsg : public SOpenPetStudySkillPlanMsg
{
	BYTE m_Result;
};

//同步宠物技能
DECLARE_MSG(SSynPetSkillMsg, SPetMsg, SPetMsg::EPRO_PET_SYNPETSKILL)

struct SASynPetSkillMsg : public SSynPetSkillMsg
{
	BYTE m_PetIndex;	//宠物索引
	BYTE m_PetSkillIndex;	//宠物技能索引
	BYTE m_PetSkillLevel;	//宠物技能等级
	DWORD m_PetSkillID;	//宠物技能ID
};