#pragma once
#include "stdint.h"
struct SNewbieGudieData;
struct SNewbieGuideDInfo;
enum GuideType;

/*引导触发依赖的模块*/
enum FunctionMoudleType
{
	Function_Bless,			//祈福模块
	Function_HeroUpgrade,	//英雄提升模块
	Function_Achievement,	//成就模块
	Function_SkillUpgrade,	//技能升级模块
	Function_Duplicate,		//副本模块
};

class CGuideManager
{
public:
	CGuideManager();
	~CGuideManager();
	bool InitGuideData(SNewbieGudieData* pData);
	/*brief 设置引导数据
	@param [in] 引导类型(数组索引)
	@param [in] 引导信息
	*/
	bool SetGuideData(BYTE type, SNewbieGuideDInfo& info);
	/*@brief 查询引导是否完成
	@param [in] type 待查询的引导类型
	*/
	bool IsCompleted(GuideType type) const;
	/*@brief 查询引导是否已被触发
	@param [in] type 待查询的引导类型
	*/
	bool BeTriggered(GuideType type) const;
	/*@brief 触发获取英雄引导
	@param [in] 最新解锁的关卡ID
	*/
	bool TriggerRequireHero(int latestTollgateID);
	/*@brief 触发穿齐装备进阶
	*/
	bool TriggerCollectEquipment();
	/*模块操作处理*/
	bool ProcessOperation(FunctionMoudleType functionType, int rewardID = 0);
private:
	void _Trigger(GuideType type);
private:
	SNewbieGudieData *m_pGuideData;
	GuideType m_eCurGuideType;
};

