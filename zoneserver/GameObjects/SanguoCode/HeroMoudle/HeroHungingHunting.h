// ----- CHeroHungingHunting.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   �ȶ��������佫�һ�Ѱ�����淨���߼�����
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;

class CBaseDataManager;
struct SHeroExtendsGamplayMsg;
struct HeroHungingHuntingMsg;
struct AnswerHeroHuntingApplyMsg;
struct AnswerSynSurprisePresentsMsg;
struct GameFeatureActivationConfig;
struct m_ptrSynSurprisePresentMsg;
struct HeroHungingHuntingCfg;

class CHeroHungingHunting
{
public:
	explicit CHeroHungingHunting(CBaseDataManager& baseDataManager);
	~CHeroHungingHunting();

	void DispachMsg(const SHeroExtendsGamplayMsg* pMsg);

private: 
	///@brief �ͻ������뿪ʼ�һ�Ѱ��
	void RequestHunging(const HeroHungingHuntingMsg* pMsg);
	///@brief �ͻ����������
	void RequestSurprisePresents(const HeroHungingHuntingMsg* pMsg);
	///@brief �ͻ�������һ�Ѱ�����������
	void RequestLiquidation(const HeroHungingHuntingMsg* pMsg);
	///@brief ��ʼ������䱦��
	void RandomSurprise(int heroID, const HeroHungingHuntingCfg* config);

	CBaseDataManager& m_baseDataMgr;
	const GameFeatureActivationConfig& m_gameFeatureConfig;
	unique_ptr<AnswerHeroHuntingApplyMsg> m_ptrAllowToHungingHuntingMsg;///ͬ��ͻ���ĳһ��Ӣ�ۿ�ʼ�һ�Ѱ������Ϣ
	unique_ptr<AnswerHeroHuntingApplyMsg> m_ptrEarningsLiquidationSucceedMsg;///֪ͨ�ͻ���Ѱ���������ɹ�
	unique_ptr<AnswerSynSurprisePresentsMsg> m_ptrSynSurprisePresentMsg;///֪ͨ�ͻ��˱���Ѱ���������Ʒ
};

