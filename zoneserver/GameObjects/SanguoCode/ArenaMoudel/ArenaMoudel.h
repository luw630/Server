#pragma once
#include <vector>
#include <map>
#include <functional>
#include "stdint.h"
#include"../PUB/Singleton.h"
#include"../../NETWORKMODULE\DyArenaMsg.h"

using namespace std;
#define MAX_ROBOT_NUM 10000 //最大机器人数量
//#define ARENA_ID(arenaID) (MAX_ROBOT_NUM | arenaID)
typedef std::function<void(int)> FunctionPtr;


struct CGlobalConfig;
class CBaseDataManager;
class RobotNameConfig;
class TimerEvent_SG;
///@brief 竞技场玩家在线时的完整数据，这里在构造函数中调用memset会清理掉虚函数表，所以不能使用任何虚函数
struct SArenaCompetitor : public SBaseArenaCompetitor
{
	int m_curOpponentID; ///@brief 当前对手
	__int64 m_LastChallengeTime; ///@brief 上一次挑战竞技场时间
	//SBaseArenaCompetitor m_BaseData; ///@brief 基础属性
	TimerEvent_SG * m_EventPtr; ///@brief 定时器事件
	DWORD m_ChallengeCount; ///@竞技场挑战次数
	BYTE m_IsLocked; ///@brief 锁定状态
	WORD m_BuyedNum; //已买次数
	DWORD m_ResetNum; //重置次数

	SArenaCompetitor(SBaseArenaCompetitor & baseComp)
		:SBaseArenaCompetitor(baseComp)
	{
		Init();
	}

	SArenaCompetitor()
	{
		memset(this, 0, sizeof(SArenaCompetitor));
		Init();
	}

	void Init()
	{
		m_IsLocked = false;
		m_curOpponentID = -1;
		m_EventPtr = nullptr;
		_time64(&m_LastChallengeTime);
		m_LastChallengeTime -= 1 * 3600;
		m_ChallengeCount = 5;
		m_BuyedNum = 0;
		m_ResetNum = 0;
	}
};

///@brief 竞技场模块
class CArenaMoudel : public CSingleton<CArenaMoudel>
{
public:
	CArenaMoudel();
	~CArenaMoudel();
	void OnDispatchArenaMsg(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	bool InitArenaData();
	bool Release();
	void BackupArenaRankData();
	///@breif 玩家下线时，需要调用此接口保存自己的竞技场数据到托管数据中
	bool SetPlayerManagedArenaData(CBaseDataManager* pDataMgr);
	///@breif 根据传入的竞技场ID，返回其当前竞技场排名
	int GetArenaRank(const int arenaID);
	///@brief 解锁竞技场玩家锁定状态
	SArenaCompetitor*  UnlockCompetitor(const int arenaID);
	///@brief 初入竞技场
	bool DebutArena(DNID dnidClient, CBaseDataManager *baseDataMgr);
	///@ 上线同步玩家竞技场数据
	void SynLoginArenaData(DNID dnidClient, CBaseDataManager *pBaseDataMgr);
	///@ 发送上一次挑战的时间
	void SendLastChallengTime(DNID dnidClient, int sid);
	///@ 根据SID获取上次挑战间隔 
	bool GetLastChallengTimespan(const int sid, DWORD &timespanOut);
	///@ 根据SID获取购买体力的次数
	bool GetBuyChallengeCount(const int sid, DWORD &BuyNum);
	///@ 根据SID获取重置挑战的次数
	bool GetReSetCountDownCount(const int sid, DWORD &RestNum);
	///@ 根据SID获取挑战次数
	bool GetArenaChallengCount(const int sid, DWORD &countOut);
	///@brief 更新前50排行榜
	bool UpdateTop50th();
	///@brief 每日历程
	void ArenaRoutine();
	///@获取最佳排名
	DWORD GetBestRank(const int sid);
	//同步竞技场排行榜名称
	void ChangeNametoArena(int sid, char* name);
	///@brief 获取竞技场玩家托管数据
	const SBaseArenaCompetitor* GetManagedArenaData(int arenaID);
private:
	///@brief 购买挑战次数 
	void _BuyChallengeNum(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 处理在竞技场中挑战
	void _OnRecvChallengeBegin(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 处理竞技场结束
	void _OnRecvChallengeOver(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 随机替换对手
	void _OnRandomReplacement(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 获取前50排名情况
	void _OnGetTop50thCompetitors(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 设置防守阵容
	void _OnSetDefensiveTeam(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 获取在线玩家竞技场数据
	void _GetOnlineArenaData(CPlayer *pPlayer, SBaseArenaCompetitor & competitor);
	///@brief 玩家下线时，获取托管数据
	SArenaCompetitor* _GetManagedArenaData(int arenaID);
	///@brief 获取排行榜上的玩家信息
	void _GetTop50CompetitorDetail(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief 新开服务器生成机器人排行榜前10000名
	void _GenerateRank();
	///@brief 随机出机器人竞技场信息
	void _RandomRobotInfo(int level, int heroLevel, int rank, int star, int skillLevel, SArenaCompetitor& competitor, int arenaRank);

	///@brief 挑战超时
	void _Timeout(DWORD arenaID);
	//重置竞技场挑战时间
	void _ResetCountDown(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	const CGlobalConfig &globalconfig;
	int m_RobotID;
	map<int, SArenaCompetitor> m_ArenaManagedData; //托管数据，key值为竞技场排名，value为玩家竞技场数据
	map<int, int> m_ArenaIDRankMapping; //Key值为竞技场ID，Value是排名的映射表
	SArenaPlayerReducedData* m_pTop50th; //排行榜前50名
	RobotNameConfig& m_RobotNameConfig;
	FunctionPtr m_UpdtateTop50thFucn;
	int m_defensiveTeam[5];
	int64_t m_lastRoutineTime;
};

