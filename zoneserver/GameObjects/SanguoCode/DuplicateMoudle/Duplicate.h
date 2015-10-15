// ----- CDuplicate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的基本副本类
// --------------------------------------------------------------------
//   --  管理副本中的验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include <memory>
#include <map>
using namespace std;

class CTollgate;
class CDuplicateAstrict;
class CBaseDataManager;
class CDuplicateDataManager;
struct SATollgateDropedItem;
struct ChapterConfig;
enum InstanceType;
enum CheckResult;

class CDuplicate
{
public:
	CDuplicate();
	~CDuplicate();

	void Init(CDuplicateDataManager* pDuplicateDataManage, int chapterID);
	void Update();
	///@brief 玩家要进入战斗场景后会做的一些操作，比如计算挑战关卡的物品掉落，扣除挑战消耗，重置挑战CD等操作
	///@param challengeTime 主要用于扫荡功能中的多次掉落计算，扣除对应的挑战消耗等
	///@param bMopUpOperation 是否为扫荡操作
	void ActionWhileEnterBattle(int challengeTime = 1, bool bMopUpOperation = false);
	///@brief 收益结算，比如加钱、加经验、加装备等
	///@param SQDuplicateEarningClear 客户端发来的请求关卡结算的消息
	///@param challengeTimes 标示该关卡一次性挑战了多少次
	void EarningClearing(int duplicateID, int tollgateID, int combatGrade, int challengeTimes = 1);
	///@brief 刷新此副本的挑战次数
	void RefreshTicket();
	///@brief 重置某一个指定关卡的票数
	void ResetSpecifyTollgateTicket(int tolllgateID);
	///@brief 检测指定的关卡是否允许玩家挑战，或者扫荡
	///@param tollgateID 标示要检查哪一个关卡的是否挑战
	///@param challengeTime 挑战次数，默认为1，如果是扫荡的话就会大于1
	///@param mopUpOperation 默认为false，标示是否为扫荡操作
	CheckResult Check(int tollgateID, int challengeTimes = 1, bool mopUpOperation = false);
	///@brief 激活本关卡
	void Activate();
	///@breif 关闭本副本
	void Deactivate();
	///@brief 激活本副本的所有关卡
	void ActiveAllTollgate();
	///@brief 将本副本的最新能挑战的关卡激活到tollgateID指定的关卡
	void ActivateTo(int tollgateID);
	///@brief 判断某一个关卡是否在本副本中
	bool IsTollgateExist(int tollgateID);

	///@brief 设置当前副本的下一个副本ID，好在通关本副本后激活下一个副本
	void SetNextDuplciateID(int duplicateID);

	///@brief 获取当前关卡是否已经挑战成功
	bool GetCompletedState();
	///@brief 获取当前关卡是否已经激活
	bool GetActiveState();
	///@brief 获取当前的副本类型
	InstanceType GetCurInstanceType();
	///@brief 获取副本的ID
	int GetDuplicateID();
	///@brief 获取当前选择的关卡ID
	///@note 失败返回-1，成功放回对应的值
	int GetCurTollgateID();
	///@brief 获取当前副本的下一个副本的ID
	int GetNextDuplicateID();
	///@brief 将已经计算好的要掉落的物品再一次发给客户端
	void ReSendCaculatedDropItem();

private:

	bool m_bActivete;
	bool m_bInitFlag;
	int m_iCurDuplciateID;
	int m_iNextDuplicateID;
	int m_iDefaultActivateTollgateID;
	shared_ptr<SATollgateDropedItem> m_ptrDropedItemMsg;///<先保存，将来如果客户端没有收到掉落物品的消息可以发给他
	shared_ptr<CTollgate> m_ptrCurSelectedTollgate;
	unique_ptr<CDuplicateAstrict> m_ptrDuplicateAstrict;
	InstanceType m_curInstanceType;///< 当前副本的类型
	map<int, shared_ptr<CTollgate>> m_mapTollgateContainer;
	CBaseDataManager* m_ptrDataManager;		///当前的通用数据管理模块
	CDuplicateDataManager* m_ptrDuplicateDataManager;///<当前的副本数据管理模块
	const ChapterConfig* m_ptrChapterConfig;
};

