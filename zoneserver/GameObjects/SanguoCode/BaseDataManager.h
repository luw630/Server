#pragma once
#include "stdint.h"
#include <memory>
#include <functional>
#include"HeroMoudle\HeroDataManager.h"
//#include "player.h"

class CPlayer;
class CDManager;
struct SFixData;
class CStorageManager;
class SSGPlayerMsg;
class CAchieveUpdate;
class CRandomAchieveUpdate;
struct STimeData;
class CEnduranceManager;
class CMissionUpdate;
class CGuideManager;
enum FunctionMoudleType;

//@brief 此类管理玩家最基本通用的数据
class CBaseDataManager : public CHeroDataManager
{
public:
	CBaseDataManager(DNID clientIndex, SFixData* pPlayerData, CPlayer * _cplayer);
	~CBaseDataManager();

	void Init(bool bGMFlag);
	/**@brief 在玩家数据发给客户端之前，进行一些数据的初始化，现在主要用于跟lua交互的部分的数据的初始化*/
	void InitDataOfLogin();
	///@brief 设置成就的更新逻辑模块
	void SetAchievementUpdator(CAchieveUpdate* pAchievement);
	///@brief 设置随机成就的更新逻辑模块
	void SetRandomAchievementUpdator(CRandomAchieveUpdate* updator);
	///@brief 设置任务的更新逻辑模块
	void SetMissionUpdator(CMissionUpdate* updator);
	int32_t GetMasterLevel();
	///@返回点数的个数
	DWORD GetGoods(int32_t iType, int32_t id = 0);
	///@return 返回应该发送给客户端的“数量”，对于武将、物品而言是新加的个数，对体力、金钱、宝石而言是加了指定个数之后的总个数
	DWORD AddGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0, int32_t detailType = 0, bool IsSynToClient = true);
	///@brief 批量添加某一类的Goods，再统一发消息到客户端
	///@param iType 要添加的Goods的类型
	///@param IDList 要添加的物品的ID列表，为一个动态数组
	///@param IDNum 要添加的物品的ID的个数
	///@param numList 要添加的不同的物品对应的个数
	///@note 请注意IDList 和numList的个数一定要为IDNum(前提是IDList不为空)
	bool AddGoods_SG(int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] = nullptr, int32_t detailType = 0);
	///@return 返回应该发送给客户端的“数量”，对于武将、物品而言是要减去的个数，对体力、金钱、宝石而言是减了指定个数之后的总个数
	DWORD DecGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0, int32_t detailType = 0, bool IsSynToClient = true);
	///@brief 批量删除某一类的Goods，再统一发消息到客户端
	///@param iType 要删除的Goods的类型
	///@param IDList 要删除的物品的ID列表，为一个动态数组
	///@param IDNum 要删除的物品的ID的个数
	///@param numList 要删除的不同的物品对应的个数
	///@note 请注意IDList 和numList的个数一定要为IDNum(前提是IDList不为空)
	bool DecGoods_SG(const int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] = nullptr, int32_t detailType = 0);
	bool CheckGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0);

	int32_t GetMoney() const;
	int32_t GetEndurance() const;
	int32_t GetDiamond() const;
	uint32_t GetMaxRecharge() const;
	int32_t GetHonor() const;
	int32_t GetExploit() const;
	int32_t GetPrestige() const;
	DNID GetDNID(){return m_ClientIndex; }
	void SetDNID(DNID newID) { m_ClientIndex = newID; }
	DWORD GetSID() const;
	std::string GetName();
	
	//检查名称唯一性
	int CheckGameName(char* newname,DWORD SID);

	//变更内存名称
	int SetGameName(char* newname);

	//int GetChangeNameCount();
	bool Release();

	///@brief 金钱加上传入参数
	DWORD PlusMoney(const int32_t value);

	///@brief 增加军功
	int PlusExploit(int32_t value);

	///@brief 钻石加上传入参数
	DWORD PlusDiamond(const int32_t value);

	///@增加中充值数额
	DWORD AddMaxRechage(const uint32_t value);

	///@brief 增加荣誉
	int PlusHonor(int32_t value);

	///@brief 增加声望
	int PlusPrestige(int32_t value);

	///@brief 修改祈福会产出的祈福点数
	int ModifyBlessPoints(int value);

	///@brief 修改将魂分解会产生的魂点
	int ModifySoulPoints(int value);

	///@brief 修改军团会产出的军令
	int ModifyToken(int value);

	///@brief 用于增加君主经验且不往客户端发消息,不支持加负数
	void PlusExp(int32_t value);

	///@brief 添加新物品
	DWORD AddItem(const int32_t item, const int32_t itemCount = 1);

	///@brief 是否是新账号
	bool IsNewPlayer();
	
	///@brief 往“背包”添加新的英雄，暂时只是做的对CHeroDataManager的一层封装，添加了下更新英雄收集成就的逻辑
	///@TODO 重构@Joson
	virtual bool AddHero(const int32_t heroID);
	///@brief 当防守阵容变了的时候
	virtual void OnArenaDefenseTeamChanged();

	///@breif 装备武将
	bool EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType, const int32_t equipLevelLimit);
	///@brief 增加指定英雄的经验值，不支持增加 负数的经验值
	///@param heroID 要操作的英雄的ID
	///@param value[int/out] 要增加的经验值的数量,增加成功后返回增加后的值
	///@param level[int/out] 放回增加经验后的等级
	///@param levelLimitted[out] 玩家武将等级是否冲顶了
	bool PlusHeroExp(int heroID, OUT int& value, OUT int& level, OUT bool& levelLimitted);

	///@breif 升级武将技能
	//virtual bool UpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t skillLevel);

	///@brief 暂时暂时在此处理武将升阶逻辑，用于处理升阶过程中判断是否有装备锻造过，有锻造过就返回一定比例的玉石
	///@NOTE 重构@Jonson
	virtual bool HeroRankRise(const int32_t heroID, const DWORD curRank);
	virtual bool HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel);

	///@brief新的一天重置相关数据
	void ResetDataForNewDay();

	///@brief获取CD管理类
	CDManager &GetCDManager() { return (*m_upCdMgr); }

	///@breif 获取上一次技能升级时间
	//DWORD GetLastUpgradeHeroSkillTime();
	///@breif 设置上一次技能升级时间
	//void SetLastUpgradeHeroSkillTime(const DWORD timePar);

	///@breif 获取技能点
	//DWORD GetSkillPoint();
	///@breif 设置技能点
	//void SetSkillPoint(const DWORD skillPointNum);

	///@brief设置CD管理类
	//void SetCDManager(CDManager * pCDMgr){ m_pCdMgr = pCDMgr; }
	CStorageManager& GetStorageManager() { return *m_upStorageMgr; }
	///@brief 暂时丑陋的让三国的其他模块的终生成就相关的逻辑更新的模块都从这里获取成就更新的入口
	///@NOTE: 重构@Jonson
	CAchieveUpdate* GetLifeTimeAchievementUpdator();
	///@brief 暂时丑陋的让三国的其他模块的随机成就相关的逻辑更新的模块都从这里获取成就更新的入口
	///@NOTE: 重构@Jonson
	CRandomAchieveUpdate* GetRandomAchievementUpdator();
	///@brief 暂时丑陋的让三国的其他模块的任务相关的逻辑更新的模块都从这里获取任务更新的入口
	///@NOTE: 重构@Jonson
	CMissionUpdate* GetMissionUpdator();
	///@brief 背包的逻辑暂时放到CBaseDataManager中，将来要不要独立出去@刘为为
	void OnRecvStorageOperation(SSGPlayerMsg* pMsg);
	/*@brief 处理新手引导消息*/
	void OnRecvGuideOperation(SSGPlayerMsg* pMsg);
	/*@指引交互步骤完成判断前往下一步 因需要发送消息 故在此处理*/
	void ProcessOperationOfGuide(FunctionMoudleType functionType, int rewardID = 0);
	
	const int64_t GetLogoutTime() { return m_LogoutTime; }
	const int64_t GetLoginTime() { return m_LoginTime; }
	DWORD GetPlayerLevel() const;
	const STimeData& GetTimeData() const;
	//临时的GM标记，用于测试以后需要修改
	bool GetGMFlag() const { return m_bTempGMFlag; }
	void SetGMFlag(bool flag){ m_bTempGMFlag = flag; }
	// 获取新手引导管理类对象
	CGuideManager& GetGuideManager() { return *m_upGuideMgr; }
	/// 判断是否有武将可进阶 
	void JudgeHeroUpgradeRank(int itemID);
	/// 判断是否可以计算缘分属性
	virtual bool CanCalculateFateAttribute();
	
	///@brief 获取iconID
	DWORD GetIconID();
	///@brief VIP等级提升式更新相关数据
	void UpdateDateForVIP(int vipLevel);
protected:
	///计算了战斗力之后会做的事
	virtual void ProcessAfterCombatPowerCalculated(int heroID, int combatPower);

private:
	bool _checkSFixData();

	SFixData* m_pPlayerData;
	int64_t m_LoginTime;
	int64_t m_LogoutTime;
	DNID m_ClientIndex;
	int32_t m_CurItemNum;
	CAchieveUpdate* m_ptrAcievementUpdate; ///<更新武将升阶、武将收集的相关“终生”成就逻辑 TODO:暂时该数据类访问成就的逻辑类,将来看看要不要重构
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdate;///<更新武将升阶、武将收集等随机达成的成就的逻辑
	CMissionUpdate* m_ptrMissionUpdate;	///<任务的进度更新逻辑
	std::unique_ptr<CDManager> m_upCdMgr;
	std::unique_ptr<CStorageManager> m_upStorageMgr;
	std::unique_ptr<CEnduranceManager> m_upEnduranceMgr;
	std::unique_ptr<CGuideManager> m_upGuideMgr;
	bool m_bTempGMFlag;
	std::function<void(int)> m_funcJudgeUpgradeRank;
	CPlayer*  cPlayer;
};

