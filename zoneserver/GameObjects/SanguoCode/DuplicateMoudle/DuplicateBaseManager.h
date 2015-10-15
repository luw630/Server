// ----- CDuplicateBaseManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的普通故事副本、精英副本、将神传说、秘境寻宝副本的管理类的共同模块
// --------------------------------------------------------------------
//   --  这里将分发萌斗三国那四个副本相关的数据的管理类抽象出共同的一个模块并封装起来，
//   --  管理某一类型的副本中的验证逻辑、结算逻辑等
//---------------------------------------------------------------------   
#pragma once
#include <memory>
#include <map>
#include <set>
#include <unordered_set>
#include <time.h>
using namespace std;

struct SAChallengePermissionInfor;
struct SATollgateEarningClearedInfor;
struct SMessage;
struct SDuplicateMsg;
struct SAActivateDuplicate;
struct SAResetSpecifyTollgate;
class CDuplicateDataManager;
class CExtendedDataManager;
class CBaseDataManager;
class CMissionUpdate;
class CAchieveUpdate;
class CRandomAchieveUpdate;
class CDuplicate;
enum InstanceType;

class CDuplicateBaseManager
{
public:
	CDuplicateBaseManager(CExtendedDataManager& dataMgr);
	virtual ~CDuplicateBaseManager();

	virtual void InitMgr() = 0;
	virtual void Update() = 0;
	virtual void OnRecvMsg(const SMessage *pMsg) final;
	///@brief 激活本类型的指定的副本
	///@param duplicateIDs 要激活的一系列的副本ID
	///@param IDNum 要激活的副本的个数
	///@param bSendMsg 激活副本后是否往客户端发送激活消息
	virtual void ActiveSpecifyDuplicate(int duplicateIDs[], int IDNum, bool bSendMsg = false)const;
	///@brief 此次挑战所耗费的时间
	float BattleSpendSeconds() const;
protected:
	virtual void InitProgress() final;
	virtual void DispatchMsg(const SDuplicateMsg* pMsg) = 0;
	///@brief 当客户端申请挑战某一个关卡的时候，子类会做的一些操作
	virtual void OnAskToEnterTollgate();
	///@brief 申请挑战某一个关卡
	virtual void AskToEnterTollgate(const SDuplicateMsg *pMsg) final;
	///@brief 关卡结算的逻辑操作
	virtual void EarningClearProcess(int duplicateID,  int tollgateID, int combatGrade, bool sendMsg = true) final;
	///@brief 计算操作的额外操作,暂时从EarningClearLogicProcess中独立出来
	virtual void EarningClearingExtraProgress();
	///@brief 当客户端准备进入战斗场景刷怪的时候，要将对应关卡的掉落信息给发给客户端
	virtual void OnStartToChallenge();
	///@brief 客户端再一次请求关卡掉落的物品
	virtual void ReRequestTollgateDropedItem(const SDuplicateMsg *pMsg) final;
	///@brief 激活本类型副本所有的副本
	virtual void ActiveAllDuplicate() final;
	///@brief 判断某一个副本ID是否属于当前的副本类型
	virtual bool IsDuplicateExist(int duplicateID) final;
	///@brief 批量加当前选择的英雄的经验值
	virtual void PlusHeroExpBatch() final;
	///@brief 重置某一个关卡的挑战次数
	virtual void ResetSpecifyTollgateChallegedTimes(const SDuplicateMsg *pMsg) final;
	///@brief "重置某一个关卡的挑战次数"功能的元宝花费获取
	///@return 成功放回对应的值，失败返回-1
	virtual int GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID);
	///@brief 设置激活某一系列的副本的消息的排他性状态，该消息的排他性状态是指指定的某类型的所有副本只有本消息中指定的可开放的副本才能开放
	virtual void GetActiveDuplicateExclusivityState(bool& exclusivityState) const;

protected:
	bool m_bInitFlag;///<是否已经初始化了
	bool m_bPermissionGot;///<是否允许挑战
	bool m_bEarningCleared;///<是否结算结束了，用于处理结算结束后，发送给客户端的消息客户端没有收到的情况
	int m_iCurChallengeTimes;///<当前的挑战次数，每次客户端询问是否可以挑战关卡时，如果判断可以挑战，将请求的挑战次数记录下来
	InstanceType m_DuplicateType; ///<当前的副本类型
	const set<int>* m_ptrDuplicateIDList;	///<当前副本类型的所有副本ID
	CAchieveUpdate* m_ptrAcievementUpdate; ///<更新关卡副本的相关终身成就逻辑
	CMissionUpdate* m_ptrMissionUpdate;		///<更新关卡副本的相关任务
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator;///<更新副本的相关随机成就的逻辑
	shared_ptr<CDuplicate> m_ptrCurDuplicate;///<当前用户正在打的副本
	CBaseDataManager& m_BaseDataManager;		///当前的通用数据管理模块
	CDuplicateDataManager& m_duplicateDataMgr;///<玩家的普通副本（又名故事副本）、精英副本、将神传说副本、秘境传说副本的数据管理总类
	map<int, shared_ptr<CDuplicate>> m_duplicateContainer;///<存了该类副本所有的副本实例
	unique_ptr<SAChallengePermissionInfor> m_msgEnterPermissionAnswer;	///<关卡挑战询问的结果
	unique_ptr<SAResetSpecifyTollgate> m_msgResetSpecifyTollgate; ///<重置某一个关卡的挑战次数的结果
	shared_ptr<SAActivateDuplicate> m_msgActivateDuplciate;	///<激活一系列的副本
	shared_ptr<SATollgateEarningClearedInfor> m_msgTollgateEarningClearedAnswer; ///<关卡结算成功后往客户端发送的消息
	__time64_t m_tBattleStartTime;	///<战斗开始时间
};

