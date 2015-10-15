#pragma once
#include "stdint.h"
#define MUTIPL_BLEES 10

enum BlessObjectType
{
	None = -1,
	White_Equip = 0,
	Green_Equip = 1,
	Blue_Equip,
	Purple_Equip,
	Hero,
	Max
};
class CGlobalConfig;
class CBlessRandomPrize
{
public:
	CBlessRandomPrize(const float param1, const float param2);
	~CBlessRandomPrize();
	bool Release(){ return true; }


	///@brief 铜钱祈福, 传入的参数越大，获得较高品质的物品几率越大, 加上const 此函数不修改人物成员变量
	BlessObjectType MoneyBless(const int32_t blessParIn, int32_t & prizeIDOut);

	///@brief 宝石祈福, 传入的参数越大，获得较高品质的物品几率越大, 加上const 此函数不修改人物成员变量
	BlessObjectType DiamondBless(const int32_t blessParIn, int32_t & prizeIDOut, bool cancelBlessHero = false);
	///@随机三星武将
	int32_t RandomThreeStarsHero();
	///@随机二星武将
	int32_t  RandomTwoStarsHero();

	///@随机物品
	///int32_t CBlessRandomItem();
private:

	bool _InitMoneyBlessData();
	bool _InitDiamondBlessData();
	bool _DiamondHeroBless(const int32_t blessCount, int32_t &heorIDOut);
	///@brief 铜钱祈福配置结构体
	struct MoneyBlessConfig
	{
		int32_t whiteEquipWeight; ///白色物品权重
		int32_t greenEquipWeight; ///绿色物品权重
		int32_t blueEquipWeight; ///蓝色物品权重
		int32_t purpleEquipWeight; ///紫色物品权重
		int32_t heroWeight; ///武将权重
		int32_t weightSum;

		std::vector<int32_t> whiteEquipList;	 ///白色物品ID
		std::vector<int32_t> greenEquipList;	///绿色物品ID
		std::vector<int32_t> blueEquipList;		///蓝色物品ID
		std::vector<int32_t> purpleEquipList;	///紫色物品ID
		std::vector<int32_t> heroIDList; ///武将ID

		//int BlessObjectWeight[BlessObjectType::Max];
		//std::vector<int> HeroID[BlessObjectType::Max];
	};



	///@brief 宝石祈福配置结构体
	struct DiamondBlessConfig
	{
		int32_t WhiteEquipWeight; ///白色物品权重
		int32_t GreenEquipWeight; ///绿色物品权重
		int32_t BlueEquipWeight; ///蓝色物品权重
		int32_t PurpleEquipWeight; ///紫色物品权重
		int32_t equipWeightSum;

		std::vector<int32_t> WhiteEquip;	 ///白色物品ID
		std::vector<int32_t> GreenEquip;	///绿色物品ID
		std::vector<int32_t> BlueEquip;		///蓝色物品ID
		std::vector<int32_t> PurpleEquip;	///紫色物品ID

		int32_t OneStarHeroWeight; ///一档武将权重
		int32_t TwoStarHeroWeight; ///二档武将权重
		int32_t ThreeStarHeroWeight; ///三档武将权重
		int32_t heroWeightSum;

		std::vector<int32_t> OneStarHeroID; ///一档武将ID
		std::vector<int32_t> TwoStarHeroID; ///二档武将ID
		std::vector<int32_t> ThreeStarHeroID; ///三档武将ID

	};

	///@brief 从传入的strIn变量中，按"|"作为分隔符 ，获取int 数组
	void _GetIntArray(const std::string& strIn, std::vector<int32_t> &IntArrayOut);

	///@brief 从传入的strIn变量中，按"|"作为分隔符 ，获取str数组
	void _GetStrArray(const std::string& strIn, std::vector<std::string> &strArrayOut);

	MoneyBlessConfig m_MoneyBlessConfig; ///金钱祈福配置
	DiamondBlessConfig m_DiamondBlessConfig; ///元宝祈福配置 
	const float m_FirstFactor;
	const float m_SecondFactor;
};

