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


	///@brief ͭǮ��, ����Ĳ���Խ�󣬻�ýϸ�Ʒ�ʵ���Ʒ����Խ��, ����const �˺������޸������Ա����
	BlessObjectType MoneyBless(const int32_t blessParIn, int32_t & prizeIDOut);

	///@brief ��ʯ��, ����Ĳ���Խ�󣬻�ýϸ�Ʒ�ʵ���Ʒ����Խ��, ����const �˺������޸������Ա����
	BlessObjectType DiamondBless(const int32_t blessParIn, int32_t & prizeIDOut, bool cancelBlessHero = false);
	///@��������佫
	int32_t RandomThreeStarsHero();
	///@��������佫
	int32_t  RandomTwoStarsHero();

	///@�����Ʒ
	///int32_t CBlessRandomItem();
private:

	bool _InitMoneyBlessData();
	bool _InitDiamondBlessData();
	bool _DiamondHeroBless(const int32_t blessCount, int32_t &heorIDOut);
	///@brief ͭǮ�����ýṹ��
	struct MoneyBlessConfig
	{
		int32_t whiteEquipWeight; ///��ɫ��ƷȨ��
		int32_t greenEquipWeight; ///��ɫ��ƷȨ��
		int32_t blueEquipWeight; ///��ɫ��ƷȨ��
		int32_t purpleEquipWeight; ///��ɫ��ƷȨ��
		int32_t heroWeight; ///�佫Ȩ��
		int32_t weightSum;

		std::vector<int32_t> whiteEquipList;	 ///��ɫ��ƷID
		std::vector<int32_t> greenEquipList;	///��ɫ��ƷID
		std::vector<int32_t> blueEquipList;		///��ɫ��ƷID
		std::vector<int32_t> purpleEquipList;	///��ɫ��ƷID
		std::vector<int32_t> heroIDList; ///�佫ID

		//int BlessObjectWeight[BlessObjectType::Max];
		//std::vector<int> HeroID[BlessObjectType::Max];
	};



	///@brief ��ʯ�����ýṹ��
	struct DiamondBlessConfig
	{
		int32_t WhiteEquipWeight; ///��ɫ��ƷȨ��
		int32_t GreenEquipWeight; ///��ɫ��ƷȨ��
		int32_t BlueEquipWeight; ///��ɫ��ƷȨ��
		int32_t PurpleEquipWeight; ///��ɫ��ƷȨ��
		int32_t equipWeightSum;

		std::vector<int32_t> WhiteEquip;	 ///��ɫ��ƷID
		std::vector<int32_t> GreenEquip;	///��ɫ��ƷID
		std::vector<int32_t> BlueEquip;		///��ɫ��ƷID
		std::vector<int32_t> PurpleEquip;	///��ɫ��ƷID

		int32_t OneStarHeroWeight; ///һ���佫Ȩ��
		int32_t TwoStarHeroWeight; ///�����佫Ȩ��
		int32_t ThreeStarHeroWeight; ///�����佫Ȩ��
		int32_t heroWeightSum;

		std::vector<int32_t> OneStarHeroID; ///һ���佫ID
		std::vector<int32_t> TwoStarHeroID; ///�����佫ID
		std::vector<int32_t> ThreeStarHeroID; ///�����佫ID

	};

	///@brief �Ӵ����strIn�����У���"|"��Ϊ�ָ��� ����ȡint ����
	void _GetIntArray(const std::string& strIn, std::vector<int32_t> &IntArrayOut);

	///@brief �Ӵ����strIn�����У���"|"��Ϊ�ָ��� ����ȡstr����
	void _GetStrArray(const std::string& strIn, std::vector<std::string> &strArrayOut);

	MoneyBlessConfig m_MoneyBlessConfig; ///��Ǯ������
	DiamondBlessConfig m_DiamondBlessConfig; ///Ԫ�������� 
	const float m_FirstFactor;
	const float m_SecondFactor;
};

