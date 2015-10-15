// ----- CHeroExtendsGameplayModel.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   �ȶ��������佫Ӣ�۵���չ���淨�Ĺ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;
struct SSGPlayerMsg;
class CHeroHungingHunting;
class CHeroTraining;
class CBaseDataManager;

///����lua���е��佫����չ���淨�������е�����λ����ġ���Ϣ��
enum HeroExDataType
{
	trainingState = 3, //ǧ��λ
	trainingType = 4, //���λ
	hungingHuntingState = 5, //ʮ���λ
	surprisedTimes = 6, //�����λ�������Ѿ���������Ĵ���
	cachedLevel = 7, //ǧ���λ�������ϴ�����ǿ�ʼ��һ�淨��ʱ���佫�ĵȼ�
};

///@brief ���ڶ�������ϵͳ�佫�һ�Ѱ�����佫ѵ�����з�װ������SanguoPlayerBase��֪������̫��
class CHeroExtendsGameplayModel
{
public:
	explicit CHeroExtendsGameplayModel(CBaseDataManager& baseDataManager);
	~CHeroExtendsGameplayModel();

	///@brief ������Ϣ�ķַ�
	void OnRecvMsg(SSGPlayerMsg* pMsg);

private:
	unique_ptr<CHeroTraining> m_ptrHeroTrainingMgr;
	unique_ptr<CHeroHungingHunting> m_ptrHeroHuntingMgr;
};

