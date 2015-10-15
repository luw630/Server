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

	///@brief �佫���Եĵȼ��ӳ�
	///@param heroID ����Ҫ������һ��Ӣ�۵ĵȼ����Լӳ�
	///@param heroLevel ����Ҫ�����Ӣ�۵ĵȼ�
	///@param heroStarLevel ����Ҫ�����Ӣ�۵��Ǽ�
	///@param attr Ҫ�����Ӣ�۵ȼ������Լӳɵĳ�ʼֵ���������֮������
	void CalculateHeroLevelAttribute(int heroID, int heroLevel, int heroStarLevel, OUT CharacterAttrData& attr);
	///@brief �佫���Ե�װ���ӳɡ�ÿ��װ���ľ��������ھ���ʱ���
	///@param heroID ����Ҫ������һ��Ӣ�۵�װ�����Լӳ�
	///@param equipment Ӣ����װ����װ������
	///@param attr Ҫ�����װ�����Լӳɵĳ�ʼֵ���������֮������
	void CalculateHeroEquipmentAttribute(int heroID, const SHeroEquipment* equipment, OUT CharacterAttrData& attr);
	///@brief Ʒ�׼ӳɼ���
	///@param heroID ����Ҫ������һ��Ӣ�۵Ľ������Լӳ�
	///@param rankGrade ����Ӣ�۵�Ʒ�׵ȼ�
	///@param attr Ҫ�����Ʒ�����Լӳɵĳ�ʼֵ���������֮������
	void CalculateHeroRankAttribute(int heroID, int rankGrade, OUT CharacterAttrData& attr);
	///@brief ����ĳһ��װ�������Լӳ�
	///@param attr Ҫ�����װ���Ķ�Ӧ�Ļ������Լӳɵ�����ֵ��������ɺ󷵻�
	///@param equipment Ҫ�����Ӣ�۵Ķ�Ӧ�����е�װ��
	void CalculateItemEquipmentAttribute(OUT CharacterAttrData& attr, const SHeroEquipment& equipment);
	///@brief �������������������ݽ������Լӳ�
	///@param attribute Ҫ����Ļ�������
	///@param mainAttribute ���佫�Ķ�Ӧ��������
	void CalculateAttributeEffect(OUT CharacterAttrData& attribute, HeroType mainAttribute);
	///@brief �����Ѿ���õ��佫�Ļ�������������ĳһ���佫��ս����
	///@param heroSkillData Ҫ������佫�ļ��ܵĻ�������
	///@param pHeroAttr �Ѿ���ø������Ե��佫�����ṹ��ָ��
	///@return ���ؼ���õ�ս����ֵ
	int CalculateCombatPower(const SHeroSkillInfo* heroSkillData, SHeroAttr* pHeroAttr);
	///@brief ֻ�����佫�Ļ������ݣ����������佫��ս��������������У�����ݻ������ݣ�������佫�Ļ���������Ϣ��������佫��ս����
	///@param heroDataҪ������佫�Ļ�������
	///@param fateAttrArray Ե����������
	///@return ���ؼ���õ�ս����ֵ
	int CalculateSpecifyHeroComabtPower(const SHeroData* heroData, const int(&fateAttrArray)[4]);
	///@brief �佫Ե�����Լ���
	///@param heroID ������Ե�����Ե��佫ID
	///@param attr �����������õ�������
	///@param pHeroDataManager ����佫������ ���ڻ�ȡԵ���佫��Ϣ
	void CalculateHeroFateAttribute(int heroID, OUT CharacterAttrData& attr, const CHeroDataManager* pHeroDataManager);
	///@brief ͨ���������Ե������
	void ParseHeroFateAttribute(const int(&attrArray)[4], OUT CharacterAttrData& fateAttr);
};

