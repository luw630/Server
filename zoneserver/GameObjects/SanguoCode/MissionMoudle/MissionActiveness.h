// ----- CMissionActiveness.h -----
//
//   --  Author: Jonson
//   --  Date:   15/10/16
//   --  Desc:   萌斗三国的任务活跃度的相关数据的更新管理与奖励的分发
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"


class CMissionActiveness :
	public CExtendedDataManager
{
public:
	CMissionActiveness(CBaseDataManager& baseDataMgr);
	virtual ~CMissionActiveness();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 当lua环境已经运行，且玩家数据还未发送到客户端的时候的操作
	void OnLuaInitialed();
	///@brief 当某一个任务领取成功后的操作
	void OnMissionAccomplished(int missionID);
	///@brief 领取某一个活跃度任务的
	void RequestClaimRewards(int activenessLevel);
	///@brief 重置任务活跃度
	void ResetMissionActivenessData(int sendMsgFlag = 0);

private:
	bool m_bInitResetMissionActiveness;
	DWORD* m_dwMissionActiveness; ///<玩家积累的任务的活跃度
};

