// ----- CAchieveUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/05
//   --  Desc:   萌斗三国的成就更新逻辑管理抽象类
// --------------------------------------------------------------------
//   --  用于抽象出成就更新的部分接口，让其他模块只依赖于该层抽象
//---------------------------------------------------------------------   
#pragma once
class CAchieveUpdate
{
public:
	CAchieveUpdate(){}
	virtual ~CAchieveUpdate(){}

	///@brief 更新普通副本、精英副本、将神传说、秘境寻宝的成就相关的信息
	///@param curConquredTollgateID 当前征服的副本关卡ID
	///@param completedTimes 挑战完成的次数
	virtual void UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes = 1) = 0;
	///@brief 更新武将收集的成就
	virtual void UpdateHeroCollectionAchieve() = 0;
	///@brief 更新武将进阶的成就
	///@param curHeroRank 当前武将进阶到的等级
	virtual void  UpdateHeroRankRiseAchieve(int curHeroRank) = 0;
	///@brief 更新君主升级的成就
	///@param curMasterLevel 当前的君主等级
	///@param levelUpgradeAmount 君主上升的等级数，一般来说为1，也有特殊情况一次升几级的
	virtual void UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount) = 0;
};

