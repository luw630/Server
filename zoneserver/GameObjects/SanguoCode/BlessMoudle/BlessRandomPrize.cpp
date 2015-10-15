#include "stdafx.h"
#include "BlessRandomPrize.h"
#include "..\Common\TabReader.h"
#include "Random.h"
#include "..\Common\ConfigManager.h"
CBlessRandomPrize::CBlessRandomPrize(const float param1, const float param2)
	:m_FirstFactor(param1),
	m_SecondFactor(param2)
{
	_InitMoneyBlessData(); ///��ʼ����Ǯ����������
	_InitDiamondBlessData();
}


CBlessRandomPrize::~CBlessRandomPrize()
{
}


bool CBlessRandomPrize::_InitMoneyBlessData()
{
	string fileName = "ServerConfig\\TongQianQiFu.txt"; //��ʱ����д
	CTabTableReader fileReader;
	if (fileReader.OpenTabTable(fileName) == false)
	{
		rfalse("can not find file %s��������", fileName.c_str());
		return false;
	}

	//White_Equip = 0, Green_Equip = 1,Blue_Equip,Purple_Equip,Hero_Weight, Max

	m_MoneyBlessConfig.weightSum = 0; //Ȩ���ܺ���0
	int tempValue;
	while (fileReader.ReadLine())
	{
		//////////////////��ɫװ������������
		string tempStr = "";
		m_MoneyBlessConfig.whiteEquipList.clear();
		tempStr = fileReader.GetStrValue("WhiteEquipForMoney"); //��ȡ��ɫװ��ID�ַ���
		_GetIntArray(tempStr, m_MoneyBlessConfig.whiteEquipList);//�����ַ�����������ýṹ��
		m_MoneyBlessConfig.whiteEquipWeight = fileReader.GetIntValue("WhiteEquipWeight"); //��ȡ��ɫװ��Ȩ��
		m_MoneyBlessConfig.weightSum += m_MoneyBlessConfig.whiteEquipWeight; //Ȩ���ܺ��ۼ�

		//////////////////��ɫװ������������
		tempStr = "";
		m_MoneyBlessConfig.greenEquipList.clear();
		tempStr = fileReader.GetStrValue("GreenEquipForMoney");
		_GetIntArray(tempStr, m_MoneyBlessConfig.greenEquipList);//�����ַ�����������ýṹ��
		tempValue = fileReader.GetIntValue("GreenEquipWeight");
		m_MoneyBlessConfig.weightSum += tempValue;
		m_MoneyBlessConfig.greenEquipWeight = m_MoneyBlessConfig.weightSum;

		//////////////////��ɫװ������������
		tempStr = "";
		m_MoneyBlessConfig.blueEquipList.clear();
		tempStr = fileReader.GetStrValue("BlueEquipForMoney");
		_GetIntArray(tempStr, m_MoneyBlessConfig.blueEquipList);//�����ַ�����������ýṹ��
		tempValue = fileReader.GetIntValue("BlueEquipWeight");
		m_MoneyBlessConfig.weightSum += tempValue;
		m_MoneyBlessConfig.blueEquipWeight = m_MoneyBlessConfig.weightSum;

		//////////////////��ɫװ������������
		tempStr = "";
		m_MoneyBlessConfig.purpleEquipList.clear();
		tempStr = fileReader.GetStrValue("PurpleEquipForMoney");
		_GetIntArray(tempStr, m_MoneyBlessConfig.purpleEquipList);//�����ַ�����������ýṹ��
		tempValue = fileReader.GetIntValue("PurpleEquipWeight");
		m_MoneyBlessConfig.weightSum += tempValue;
		m_MoneyBlessConfig.purpleEquipWeight = m_MoneyBlessConfig.weightSum;

		//////////////////�佫����������
		tempStr = "";
		m_MoneyBlessConfig.heroIDList.clear();
		tempStr = fileReader.GetStrValue("HeroForMoney");
		_GetIntArray(tempStr, m_MoneyBlessConfig.heroIDList);//�����ַ�����������ýṹ��
		tempValue = fileReader.GetIntValue("HeroWeight");
		m_MoneyBlessConfig.weightSum += tempValue;
		m_MoneyBlessConfig.heroWeight = m_MoneyBlessConfig.weightSum;

	}

	return true;
}


bool CBlessRandomPrize::_InitDiamondBlessData()
{
	string tempStr = "";
	int32_t tempValue = -1;

	///// ��ȡԪ����Ʒ
	string equipFileName = "ServerConfig\\YuanBaoQiFu.txt"; //��ʱ����д
	CTabTableReader equipFileReader;
	if (equipFileReader.OpenTabTable(equipFileName) == false)
	{
		rfalse("can not find file %s��������", equipFileName.c_str());
		return false;
	}
		
	m_DiamondBlessConfig.equipWeightSum = 0;
	while (equipFileReader.ReadLine())
	{
		//��ɫ��Ʒ
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("WhiteEquipForCash");
		_GetIntArray(tempStr, m_DiamondBlessConfig.WhiteEquip);//�����ַ�����������ýṹ��
		m_DiamondBlessConfig.WhiteEquipWeight = equipFileReader.GetIntValue("WhiteEquipWeight");
		m_DiamondBlessConfig.equipWeightSum += m_DiamondBlessConfig.WhiteEquipWeight;

		//��ɫ��Ʒ
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("GreenEquipForCash");
		_GetIntArray(tempStr, m_DiamondBlessConfig.GreenEquip);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("GreenEquipWeight");
		m_DiamondBlessConfig.equipWeightSum += tempValue;
		m_DiamondBlessConfig.GreenEquipWeight = m_DiamondBlessConfig.equipWeightSum;

		//��ɫ��Ʒ
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("BlueEquipForCash");
		_GetIntArray(tempStr, m_DiamondBlessConfig.BlueEquip);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("BlueEquipWeight");
		m_DiamondBlessConfig.equipWeightSum += tempValue;
		m_DiamondBlessConfig.BlueEquipWeight = m_DiamondBlessConfig.equipWeightSum;

		//��ɫ��Ʒ
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("PurpleEquipForCash");
		_GetIntArray(tempStr, m_DiamondBlessConfig.PurpleEquip);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("PurpleEquipWeight");
		m_DiamondBlessConfig.equipWeightSum += tempValue;
		m_DiamondBlessConfig.PurpleEquipWeight = m_DiamondBlessConfig.equipWeightSum;

		m_DiamondBlessConfig.heroWeightSum = 0;
		//һ���佫
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("StageId1");
		_GetIntArray(tempStr, m_DiamondBlessConfig.OneStarHeroID);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("StageWeight1");
		m_DiamondBlessConfig.heroWeightSum += tempValue;
		m_DiamondBlessConfig.OneStarHeroWeight = m_DiamondBlessConfig.heroWeightSum;

		//�����佫
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("StageId2");
		_GetIntArray(tempStr, m_DiamondBlessConfig.TwoStarHeroID);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("StageWeight2");
		m_DiamondBlessConfig.heroWeightSum += tempValue;
		m_DiamondBlessConfig.TwoStarHeroWeight = m_DiamondBlessConfig.heroWeightSum;


		//�����佫
		tempValue = -1;
		tempStr = equipFileReader.GetStrValue("StageId3");
		_GetIntArray(tempStr, m_DiamondBlessConfig.ThreeStarHeroID);//�����ַ�����������ýṹ��
		tempValue = equipFileReader.GetIntValue("StageWeight3");
		m_DiamondBlessConfig.heroWeightSum += tempValue;
		m_DiamondBlessConfig.ThreeStarHeroWeight = m_DiamondBlessConfig.heroWeightSum;
	}

	return true;
}


void CBlessRandomPrize::_GetIntArray(const string& strIn, vector<int32_t> &IntArrayOut)
{
	vector<string> tempVector;
	tempVector.clear();
	int32_t element = 0;
	CTabTableReader::StringSplit(strIn, "|", &tempVector);
	for each (string var in tempVector) //ѭ����ֵ
	{
		element = atoi(var.c_str());
		IntArrayOut.push_back(element);
	}

}

void CBlessRandomPrize::_GetStrArray(const std::string& strIn, std::vector<std::string> &strArrayOut)
{
	vector<string> tempVector;
	tempVector.clear();
	CTabTableReader::StringSplit(strIn, "|", &tempVector);
	for each (string var in tempVector) //ѭ����ֵ
		strArrayOut.push_back(var);
}

 BlessObjectType CBlessRandomPrize::MoneyBless(const int32_t blessPar, int32_t & prizeIDOut)
{
	BlessObjectType objectType = BlessObjectType::None;
	int32_t lowerNum = 0;
	lowerNum = (blessPar / MUTIPL_BLEES) * m_MoneyBlessConfig.greenEquipWeight;
	int32_t randomObjectType = CRandom::RandRange(lowerNum, m_MoneyBlessConfig.weightSum); //���������Ʒ����
	int32_t tempNum = 0;
	if (randomObjectType <= m_MoneyBlessConfig.whiteEquipWeight)
	{
		tempNum = CRandom::RandRange(0, m_MoneyBlessConfig.whiteEquipList.size() - 1);
		prizeIDOut = m_MoneyBlessConfig.whiteEquipList[tempNum];
		objectType = BlessObjectType::White_Equip;
	}
	else if (randomObjectType <= m_MoneyBlessConfig.greenEquipWeight)
	{
		tempNum = CRandom::RandRange(0, m_MoneyBlessConfig.greenEquipList.size() - 1);
		prizeIDOut = m_MoneyBlessConfig.greenEquipList[tempNum];
		objectType = BlessObjectType::Green_Equip;
	}
	else if (randomObjectType <= m_MoneyBlessConfig.blueEquipWeight)
	{
		tempNum = CRandom::RandRange(0, m_MoneyBlessConfig.blueEquipList.size() - 1);
		prizeIDOut = m_MoneyBlessConfig.blueEquipList[tempNum];
		objectType = BlessObjectType::Blue_Equip;
	}
	else if (randomObjectType <= m_MoneyBlessConfig.purpleEquipWeight)
	{
		tempNum = CRandom::RandRange(0, m_MoneyBlessConfig.purpleEquipList.size() - 1);
		prizeIDOut = m_MoneyBlessConfig.purpleEquipList[tempNum];
		objectType = BlessObjectType::Purple_Equip;
	}
	else if (randomObjectType <= m_MoneyBlessConfig.heroWeight)
	{
		tempNum = CRandom::RandRange(0, m_MoneyBlessConfig.heroIDList.size() - 1);
		prizeIDOut = m_MoneyBlessConfig.heroIDList[tempNum];
		objectType = BlessObjectType::Hero;
	}
	
	return objectType;
}

 BlessObjectType CBlessRandomPrize::DiamondBless(const int32_t blessPar, int32_t & prizeIDOut, bool cancelBlessHero/* = false*/)
 {
	 BlessObjectType objectType = BlessObjectType::Hero;
	 int32_t equipIndex = -1;
	 if (cancelBlessHero || _DiamondHeroBless(blessPar, prizeIDOut) == false) //����佫���ʧ�ܣ��������Ʒ���
	 {
		int32_t objectTypeWeight = CRandom::RandRange(1, m_DiamondBlessConfig.equipWeightSum);
		if (objectTypeWeight <= m_DiamondBlessConfig.WhiteEquipWeight)
		{
			equipIndex = CRandom::RandRange(0, m_DiamondBlessConfig.WhiteEquip.size() -1);
			prizeIDOut = m_DiamondBlessConfig.WhiteEquip[equipIndex];
			objectType = BlessObjectType::White_Equip;
		}
		else if (objectTypeWeight <= m_DiamondBlessConfig.GreenEquipWeight)
		{
			equipIndex = CRandom::RandRange(0, m_DiamondBlessConfig.GreenEquip.size() - 1);
			prizeIDOut = m_DiamondBlessConfig.GreenEquip[equipIndex];
			objectType = BlessObjectType::Green_Equip;
		}
		else if (objectTypeWeight <= m_DiamondBlessConfig.BlueEquipWeight)
		{
			equipIndex = CRandom::RandRange(0, m_DiamondBlessConfig.BlueEquip.size() -1);
			prizeIDOut = m_DiamondBlessConfig.BlueEquip[equipIndex];
			objectType = BlessObjectType::Blue_Equip;
		}
		else
		{
			equipIndex = CRandom::RandRange(0, m_DiamondBlessConfig.PurpleEquip.size() -1);
			prizeIDOut = m_DiamondBlessConfig.PurpleEquip[equipIndex];
			objectType = BlessObjectType::Purple_Equip;
		}

	 }


	 return objectType;
 }

 bool  CBlessRandomPrize::_DiamondHeroBless(const int32_t powerValue,  int32_t &heorIDOut)
 {
	 ///��ļ���� = X^(�齱����-1) * Y 
	 float result = pow(m_FirstFactor, powerValue);
	 result = result  * m_SecondFactor;
	 
	 if ((CRandom::RandRange(0, 100) / 100.0f) - result <= 0)
	 {
		  int32_t heorTypeWeight = CRandom::RandRange(0, m_DiamondBlessConfig.heroWeightSum);
		  int32_t heroIndex = -1;
		  if (heorTypeWeight <= m_DiamondBlessConfig.OneStarHeroWeight) //һ���佫
		  {
			  heroIndex = CRandom::RandRange(0, m_DiamondBlessConfig.OneStarHeroID.size()-1);
			  heorIDOut = m_DiamondBlessConfig.OneStarHeroID[heroIndex];
		  }
		  else if (heorTypeWeight <= m_DiamondBlessConfig.TwoStarHeroWeight) //�����佫
		  {
			  heroIndex = CRandom::RandRange(0, m_DiamondBlessConfig.TwoStarHeroID.size() -1);
			  heorIDOut = m_DiamondBlessConfig.TwoStarHeroID[heroIndex];
		  }
		  else //�����佫
		  {
			  heroIndex = CRandom::RandRange(0, m_DiamondBlessConfig.ThreeStarHeroID.size() -1);
			  heorIDOut = m_DiamondBlessConfig.ThreeStarHeroID[heroIndex];
		  }
		 return true;
	 }

	 return false;
 }


 int32_t  CBlessRandomPrize::RandomThreeStarsHero()
 {
	 int heroIndex = CRandom::RandRange(0, m_DiamondBlessConfig.ThreeStarHeroID.size() - 1);
	 return m_DiamondBlessConfig.ThreeStarHeroID[heroIndex];
 }

 int32_t  CBlessRandomPrize::RandomTwoStarsHero()
 {
	 int heroIndex = CRandom::RandRange(0, m_DiamondBlessConfig.TwoStarHeroID.size() - 1);
	 return m_DiamondBlessConfig.TwoStarHeroID[heroIndex];
 }

