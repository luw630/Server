// ----- SkillUpgradeManager.h ------
//	 --  Author : LiuWeiWei
//	 --  Date : 3/4/15 加注时间
//   --  Desc : 处理武将技能的升级 技能点的购买
#pragma once
#include "../ExtendedDataManager.h"
struct SSkillPointData;
class CBaseDataManager;
class CMissionUpdate;
struct CGlobalConfig;

class CSkillUpgradeManager
	: public CExtendedDataManager
{
public:
	CSkillUpgradeManager(CBaseDataManager& BaseDataMgr);
	virtual ~CSkillUpgradeManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	/**@brief 计算距离上次下线或首次登陆所回复的技能点*/
	bool InitSkillPointData();
	/**<@brief 升级技能
	@note 技能等级等于或大于武将等级
	@param [in] heroID 武将ID
	@param [in] skillID 技能ID
	@param [in] skillLevel 技能等级
	*/
	bool UpgradeSkill(const int heroID, const int skillID, const int skillLevel);
	bool MultiUpgradeSkill(const int heroID, const int skillID, const int destLevel, DWORD& skillPointCost);
	/**@brief 传递技能ID和目标等级数组升级若干个技能
	@param [in] heroID 英雄ID
	@param [in] skillIDs 技能ID数组
	@param [in] destLevel 目标等级数组
	@param [in] 数组有效元素个数
	@param [out] skillPointCost 消耗技能点数
	*/
	bool MultiUpgradeSkill(const int heroID, DWORD skillIDs[], DWORD destLevel[], DWORD validNum, DWORD& skillPointCost);
	///@breif 吃技能点药剂等加技能点
	///@param Num 添加的技能点数
	///NOTE:此接口仅供已有的使用技能点药剂的lua逻辑调用（因为里面有往服务器发消息），走的是通用的使用消耗品的逻辑。由于现在客户端是有自己回技能点的逻辑,
	///改成走AddGoods那一边要费点时间，暂时提供这么一个接口暴露出去，重构@jonson
	void AddSkillPoint(int Num);
	///<@brief 购买技能点
	bool BuySkillPoint();
	///<@brief 可否学习技能
	bool CanLearnSkill();
	///<@brief 已购买过技能点的次数
	DWORD PurchasedSkillPointTimes() const;
	///<@brief 剩余的技能点数
	DWORD RemainingSkillPoint() const;
	///<@brief 剩余回复时间
	DWORD RecoverRemainingTime() const;
	///<@brief 设置已学习技能标志(用于客户端的技能引导判定)
	void SetLearnedSkill();
	/**@brief VIP等级提升时改变技能点数据 
	@param [in] vipLevel 当前VIP等级
	*/
	bool SetSkillPointForVIP(int vipLevel);
private:
	void _CalculateSkillPoint(const __time64_t& lastTime, const __time64_t& currentTime);
private:
	SSkillPointData* m_pSkillPointData;
	CGlobalConfig& m_GlobalConfig;
	__time64_t m_LastCalcSkillPointTime;
	__time64_t m_CurrentTime;
	int m_iSkillPointLimit;	///<技能点上限(VIP玩家可扩充)//
	CMissionUpdate* m_ptrMissionUpdator;
};

