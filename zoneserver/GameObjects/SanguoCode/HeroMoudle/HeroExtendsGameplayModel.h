// ----- CHeroExtendsGameplayModel.h -----
//
//   --  Author: Jonson
//   --  Date:   15/07/04
//   --  Desc:   萌斗三国的武将英雄的扩展性玩法的管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
using namespace std;
struct SSGPlayerMsg;
class CHeroHungingHunting;
class CHeroTraining;
class CBaseDataManager;

///标明lua层中的武将的扩展性玩法的数据中的数据位代表的“信息”
enum HeroExDataType
{
	trainingState = 3, //千分位
	trainingType = 4, //万分位
	hungingHuntingState = 5, //十万分位
	surprisedTimes = 6, //百万分位，代表已经申请掉宝的次数
	cachedLevel = 7, //千万分位及其以上代表的是开始这一玩法的时候武将的等级
};

///@brief 现在对两个子系统武将挂机寻宝、武将训练进行封装，避免SanguoPlayerBase“知道”的太多
class CHeroExtendsGameplayModel
{
public:
	explicit CHeroExtendsGameplayModel(CBaseDataManager& baseDataManager);
	~CHeroExtendsGameplayModel();

	///@brief 主管消息的分发
	void OnRecvMsg(SSGPlayerMsg* pMsg);

private:
	unique_ptr<CHeroTraining> m_ptrHeroTrainingMgr;
	unique_ptr<CHeroHungingHunting> m_ptrHeroHuntingMgr;
};

