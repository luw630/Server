#pragma once
#include "stdint.h"
struct SNewbieGudieData;
struct SNewbieGuideDInfo;
enum GuideType;

/*��������������ģ��*/
enum FunctionMoudleType
{
	Function_Bless,			//��ģ��
	Function_HeroUpgrade,	//Ӣ������ģ��
	Function_Achievement,	//�ɾ�ģ��
	Function_SkillUpgrade,	//��������ģ��
	Function_Duplicate,		//����ģ��
};

class CGuideManager
{
public:
	CGuideManager();
	~CGuideManager();
	bool InitGuideData(SNewbieGudieData* pData);
	/*brief ������������
	@param [in] ��������(��������)
	@param [in] ������Ϣ
	*/
	bool SetGuideData(BYTE type, SNewbieGuideDInfo& info);
	/*@brief ��ѯ�����Ƿ����
	@param [in] type ����ѯ����������
	*/
	bool IsCompleted(GuideType type) const;
	/*@brief ��ѯ�����Ƿ��ѱ�����
	@param [in] type ����ѯ����������
	*/
	bool BeTriggered(GuideType type) const;
	/*@brief ������ȡӢ������
	@param [in] ���½����Ĺؿ�ID
	*/
	bool TriggerRequireHero(int latestTollgateID);
	/*@brief ��������װ������
	*/
	bool TriggerCollectEquipment();
	/*ģ���������*/
	bool ProcessOperation(FunctionMoudleType functionType, int rewardID = 0);
private:
	void _Trigger(GuideType type);
private:
	SNewbieGudieData *m_pGuideData;
	GuideType m_eCurGuideType;
};

