#pragma once
#include "..\PUB\Singleton.h"
#include"..\Common\ConfigManager.h"

enum HeroType;
struct SHeroEquipment;
struct SHeroSkillInfo;
class CHeroDataManager;

class CHeroAttributeCalculator : public CSingleton<CHeroAttributeCalculator>
{
public:
	CHeroAttributeCalculator();
	~CHeroAttributeCalculator();

	///@brief 武将属性的等级加成
	///@param heroID 标明要计算哪一个英雄的等级属性加成
	///@param heroLevel 标明要计算的英雄的等级
	///@param heroStarLevel 标明要计算的英雄的星级
	///@param attr 要计算的英雄等级的属性加成的初始值，计算好了之后会输出
	void CalculateHeroLevelAttribute(int heroID, int heroLevel, int heroStarLevel, OUT CharacterAttrData& attr);
	///@brief 武将属性的装备加成。每个装备的精炼属性在精炼时算好
	///@param heroID 标明要计算哪一个英雄的装备属性加成
	///@param equipment 英雄已装备的装备集合
	///@param attr 要计算的装备属性加成的初始值，计算好了之后会输出
	void CalculateHeroEquipmentAttribute(int heroID, const SHeroEquipment* equipment, OUT CharacterAttrData& attr);
	///@brief 品阶加成计算
	///@param heroID 标明要计算哪一个英雄的进阶属性加成
	///@param rankGrade 标明英雄的品阶等级
	///@param attr 要计算的品阶属性加成的初始值，计算好了之后会输出
	void CalculateHeroRankAttribute(int heroID, int rankGrade, OUT CharacterAttrData& attr);
	///@brief 计算某一个装备的属性加成
	///@param attr 要计算的装备的对应的基本属性加成的已有值，计算完成后返回
	///@param equipment 要计算的英雄的对应的现有的装备
	void CalculateItemEquipmentAttribute(OUT CharacterAttrData& attr, const SHeroEquipment& equipment);
	///@brief 根据力量、智力和敏捷进行属性加成
	///@param attribute 要计算的基础属性
	///@param mainAttribute 该武将的对应的主属性
	void CalculateAttributeEffect(OUT CharacterAttrData& attribute, HeroType mainAttribute);
	///@brief 根据已经算好的武将的基础属性来计算某一个武将的战斗力
	///@param heroSkillData 要计算的武将的技能的基本数据
	///@param pHeroAttr 已经算好各项属性的武将无数结构体指针
	///@return 返回计算好的战斗力值
	int CalculateCombatPower(const SHeroSkillInfo* heroSkillData, SHeroAttr* pHeroAttr);
	///@brief 只根据武将的基本数据，来算出这个武将的战斗力，计算过程中，会根据基本数据，来算出武将的基本属性信息，再算出武将的战斗力
	///@param heroData要计算的武将的基本数据
	///@param fateAttrArray 缘分属性数组
	///@return 返回计算好的战斗力值
	int CalculateSpecifyHeroComabtPower(const SHeroData* heroData, const int(&fateAttrArray)[4]);
	///@brief 武将缘分属性计算
	///@param heroID 待计算缘分属性的武将ID
	///@param attr 将计算结果设置到该属性
	///@param pHeroDataManager 玩家武将管理类 用于获取缘分武将信息
	void CalculateHeroFateAttribute(int heroID, OUT CharacterAttrData& attr, const CHeroDataManager* pHeroDataManager);
	///@brief 通过数组解析缘分属性
	void ParseHeroFateAttribute(const int(&attrArray)[4], OUT CharacterAttrData& fateAttr);
};

