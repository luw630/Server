// ----- CBaseTollgate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的关卡基本类
// --------------------------------------------------------------------
//   --  管理关卡中的验证逻辑、结算逻辑等。类中的第一次关卡掉落只有在普通副本（又名故事副本）才有，扫荡功能也是只有在普通副本、精英副本中才有，将神传说副本、秘境寻宝副本中均没有
//   --  出于不分出很多零散的类的考虑，暂时不将该类的相关逻辑分离，都集成在此，如果将来该类的功能差异化更加明显、类功能更加庞大，请最好分离相关逻辑
//---------------------------------------------------------------------   
#pragma once
#include <unordered_map>
#include <memory>
using namespace std;

enum InstanceType;
enum CheckResult;
struct BattleLevelConfig;
class CBaseDataManager;
class CTollgateDrop;
class CDuplicateDataManager;

class CTollgate
{
public:
	CTollgate();
	~CTollgate();

	void Init(CDuplicateDataManager* dataManager, const BattleLevelConfig* config, InstanceType type);
	///@brief 主要用来更新CD，但CD暂时没有用
	void Update();
	///@brief 玩家要进入战斗场景后会做的一些操作，比如计算本关卡的物品掉落，扣除挑战消耗，重置挑战CD等操作
	///@param challengeTime 主要用于扫荡功能中的多次掉落计算，扣除对应的挑战消耗等
	void ActionWhileEnterBattle(int challengeTime = 1);
	///@brief 收益结算，比如加钱、加经验、加装备等
	///@param challengeTime 主要用于扫荡功能中的多次掉落计算，扣除对应的挑战消耗等
	void EarningClearing(int challengeTime = 1);
	///@brief 刷新此关卡的挑战次数
	void RefreshTicket();
	///@brief 检测当前关卡是否允许玩家挑战本关卡，或者扫荡本关卡
	///@param challengeTime 挑战次数，默认为1，如果是扫荡的话就会大于1
	///@param mopUpOperation 默认为false，标示是否为扫荡操作
	CheckResult Check(int challengeTime = 1, bool mopUpOperation = false);
	///@brief 进行征服此关卡的一些数据的设置
	void Conquer();
	///@brief 激活本关卡
	void Activate();
	///@brief 关闭本关卡
	void Deactivate();
	///@brief 当激活的本关卡的时候初始化本关卡的已挑战次数、挑战分数等信息
	void ArrangeDataWhileActivated();

	///@brief 设置当前关卡的下一个关卡，以便当前关卡挑战成功后激活下一个关卡
	void SetNextTollgateID(int tollgateID);
	///@brief 设置关卡挑战评分
	void SetBattleGrade(int grade);

	///@brief 获取当前关卡是否已经挑战成功
	bool GetCompletedState();
	///@brief 获取当前关卡是否已经激活
	bool GetActiveState();
	///@brief 获取当前关卡的ID
	int GetTollgateID();
	///@brief 获取下一个关卡的ID
	int GetNextTollgateID();
	///@brief 获取当前关卡掉落的普通物品信息
	const unordered_map<int, int>* GetCommonDropOutItemList();
	///@brief 获取当前关卡Boss掉落的物品信息
	const unordered_map<int, int>* GetBossDropItemList();

protected:
	///@brief 主要重置上一次关卡掉落的信息已经结算的信息
	void Reset();
	///@brief 计算物品的掉落
	///@param challengeTime 大于1的话就会将多次的掉落结果存到对应的容器中，主供扫荡功能使用
	///@param bMopUpOperation 是否为扫荡操作
	void CalculateDropOutItem(int challengeTime = 1, bool bMopUpOperation = false);
	///@brief 添加挑战成功后会收获的物品,不支持加负数个数的Item
	void AddEarningItem(int itemID, int itemNum);

private:
	bool m_bCompleted;
	bool m_bActivated;
	bool m_bFirstChallenge;
	bool m_bMopUpOperation;
	int m_iTollgateId;     ///<关卡ID//
	int m_iNextTollgateID;	///<下一个关卡ID
	int m_iCurBattleGrade; ///< 当前的战斗评价
	int m_iTeamExp;     //战队经验//
	int m_iGeneralsExp; //武将经验//
	//int m_iGloryValue; //荣誉值//
	//int m_iSalaryValue; //俸禄值//
	int m_iMoney;
	float m_fCurCD;		///<挑战的CD，暂时无用
	int m_iTeamMinimumLevel;    ///<战队最低等级//
	int m_iChanllengedTimes;		///<挑战了多少次
	int m_iOneDayChanllengeCount;   ///<每日可挑战的次数 <0表示无限制//
	int m_iChallengeStaminaCost;	///<挑战的体力消耗
	InstanceType m_curInstanceType; ///<当前副本的类型
	unordered_map<int, int> m_mapFirstDropItems;///< 如果是普通副本（又名故事副本）的话首次掉落的物品ID和数量,key为ID，value为数量
	unordered_map<int, int> m_listItem;///收获的物品列表，key值为itemID，value值为itemID对应的个数
	unique_ptr<CTollgateDrop> m_ptrDropOut;        ///<掉落物品//
	const BattleLevelConfig* m_ptrConfig;
	CBaseDataManager* m_ptrDataManager;		///当前的通用数据管理模块
	CDuplicateDataManager* m_ptrDuplicateDataManager;///<当前的副本数据管理模块
};

