// ----- CMissionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的任务的相关数据的管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <functional>
#include <memory>
#include <unordered_map>
typedef std::function<void(int)> FunctionPtr;

///@brief 任务的数据管理类
struct SAMissionAccomplished;
struct SAUpdateMissionState;
class TimerEvent_SG;
enum MissionRewardsType;
enum MissionType;

class CMissionDataManager :
	public CExtendedDataManager
{
public:
	CMissionDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CMissionDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 当lua环境已经运行，且玩家数据还未发送到客户端的时候的操作
	void OnLuaInitialed();
	///@brief 设置任务完成时候的回调
	void SetMissionAccomplishedCallBack(FunctionPtr& fun);
	///@brief 设置任务的完成次数
	///@param MissionType 任务对应的类型
	///@param times 次数
	void SetMissionCompleteTimes(MissionType type, DWORD times, bool synMsg = true);
	///@brief 设置某一个任务完成
	///@param missionID 任务对应的ID
	void SetMissionAccompulished(DWORD missionID);
	///@brief 重置任务的相关数据
	///@param sendMsgFlag 为0代表不发消息， 非0代表发消息
	void ResetMissionData(int sendMsgFlag = 0);
	///@brief 体力任务的判断模块，主要为午餐的任务的显示时间段的判断
	void LunchMissionVisible();
	///@brief 体力任务的判断模块，主要为晚餐的任务的显示时间段的判断
	void DinnerMissionVisible();
	///@brief 体力任务的判断模块，主要为夜宵的任务的显示时间段的判断
	void SupperMissionVisible();
	///@brief 体力任务的判断模块，主要为午餐的任务的可领取时间段的判断
	void LunchMissionAccessible();
	///@brief 体力任务的判断模块，主要为晚餐的任务的可领取时间段的判断
	void DinnerMissionAccessible();
	///@brief 体力任务的判断模块，主要为夜宵的任务的可领取时间段的判断
	void SupperMissionAccessible();
	///@brief 夜宵任务的关闭处理
	void SupperMissionDisable();

private:
	enum MissionDataType
	{
		complistedTiems = 1, //任务的完成次数
		visible, //任务的可见的状况
		accomplished, //任务的完成状况
	};

	///@brief 初始的时候的体力礼包任务的判断
	void EnduranceMissionInitJudgement();
	///@brief 体力任务的处理模块，暂时为午餐、晚餐、夜宵任务的跟客户端打交道的逻辑处理
	///@param type 体力礼包任务的具体类型，是午餐还是晚餐还是夜宵任务
	///@param accessible 任务是否可接
	///@param visible 体力礼包任务是否可见
	///@param conflitProcess 是否处理体力礼包任务互斥的情况
	///@param sendMsg 是否发送相关消息到客户端
	void EnduranceMissionProcess(MissionType type, bool visible, bool accessible, bool conflitProcess, bool sendMsg = true);
	///@brief 设置任务的完成次数
	///@param missionID 任务对应的ID
	///@param times 次数
	void SetMissionCompleteTimes(int missionID, DWORD times, bool synMsg = true);

	bool m_bInitResetMission;
	FunctionPtr m_operationOnMissionAccomplished;///某一个任务完成后会做的事情
	shared_ptr<SAUpdateMissionState> m_ptrUpdateMissionStateMsg;
	shared_ptr<SAMissionAccomplished> m_ptrMissionAccomplishedMsg;
};