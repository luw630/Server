// ----- CExpeditionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的过关斩将的相关逻辑的管理
// --------------------------------------------------------------------
//   --  管理过关斩将中的重要逻辑判断，比如是否可以挑战、敌人的匹配、挑战成功的结算等等，也管理与客户端之间的消息的收发
//---------------------------------------------------------------------  
#pragma once
#include "Networkmodule\SanguoPlayerMsg.h"
#include <unordered_set>
#include"../PUB/Singleton.h"

struct SHeroData;
struct CGlobalConfig;
class CBaseDataManager;
class CExtendedDataManager;
class CExpeditionDataManager;
struct ExpeditionInstanceInfor;

class CExpeditionManager : public CSingleton<CExpeditionManager>
{
public:
	CExpeditionManager();
	~CExpeditionManager();

	void DispatchExpeditionMsg(const SMessage *pMsg, CExtendedDataManager *pExpeditionDataMgr);

private:
	///@brief 更新远征通关的关卡数
	void _UpdateExpeditionTimes();
	void _AskToCostATicket();
	void _ActiveExpedition();
	void _BattleResultResolves();
	void _GetNextEnmyInfor();
	void _RewardsReceived();
	void _ExitBattle();
	///@brief 检测是否能进入到副本中
	void _CheckEnterBattlePermission(const SMessage *pMsg);
	///@brief 由客户端同步参战武将的血量等信息过来
	void _SynSelectedHeroInfor(const SMessage *pMsg);
	///@brief 由客户端同步参战地方英雄的血量等信息过来
	void _SynHostileEnmyInfor(const SMessage *pMsg);
	void _MatchEnemy();
	void _GenerateEnemy(const SHeroData* refHeroInfo, int enemyID, int enemySrialNumber);

	int m_iFirstExpeditionLevelID;///<第一个远征关卡的ID
	SABattleResolve m_msgResolveInfo;
	SAConqured m_msgExpeditionConqured;				///<远征副本通关
	SAPermissionInfor m_msgPermissionInfor;
	SAHostileEnemyInfor m_msgHostileEnmeyInfor;
	CExpeditionDataManager * m_ptrDataMgr;
	const CGlobalConfig& globalConfig;
	const SHeroData* m_excellentHeroInfor;
	const ExpeditionInstanceInfor* m_ptrCurLevelInstanceInfor;
};

