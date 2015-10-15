// ----- CHeroTraining.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   �ȶ��������佫ѵ���淨���߼�����
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;

class CBaseDataManager;
struct SHeroExtendsGamplayMsg;
struct HeroTrainingMsg;
struct GameFeatureActivationConfig;

class CHeroTraining
{
public:
	explicit CHeroTraining(CBaseDataManager& baseDataManager);
	~CHeroTraining();

	void DispachMsg(const SHeroExtendsGamplayMsg* pMsg);

private:
	///@brief �ͻ����������ĳһ��Ӣ�۵�ѵ��
	void RequestStartTraining(const HeroTrainingMsg* pMsg);
	///@brief �ͻ����������ѵ�����������
	void RequestLiquidation(const HeroTrainingMsg* pMsg);

	CBaseDataManager& m_baseDataMgr;
	const GameFeatureActivationConfig& m_gameFeatureConfig;
	unique_ptr<HeroTrainingMsg> m_ptrAllowStartTrainingMsg;
	unique_ptr<HeroTrainingMsg> m_ptrEarningsLiquidationMsg;

	bool bTrainedFlag; ///�Ƿ����佫ѵ����
};

