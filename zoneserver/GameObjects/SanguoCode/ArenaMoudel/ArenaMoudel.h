#pragma once
#include <vector>
#include <map>
#include <functional>
#include "stdint.h"
#include"../PUB/Singleton.h"
#include"../../NETWORKMODULE\DyArenaMsg.h"

using namespace std;
#define MAX_ROBOT_NUM 10000 //������������
//#define ARENA_ID(arenaID) (MAX_ROBOT_NUM | arenaID)
typedef std::function<void(int)> FunctionPtr;


struct CGlobalConfig;
class CBaseDataManager;
class RobotNameConfig;
class TimerEvent_SG;
///@brief �������������ʱ���������ݣ������ڹ��캯���е���memset��������麯�������Բ���ʹ���κ��麯��
struct SArenaCompetitor : public SBaseArenaCompetitor
{
	int m_curOpponentID; ///@brief ��ǰ����
	__int64 m_LastChallengeTime; ///@brief ��һ����ս������ʱ��
	//SBaseArenaCompetitor m_BaseData; ///@brief ��������
	TimerEvent_SG * m_EventPtr; ///@brief ��ʱ���¼�
	DWORD m_ChallengeCount; ///@��������ս����
	BYTE m_IsLocked; ///@brief ����״̬
	WORD m_BuyedNum; //�������
	DWORD m_ResetNum; //���ô���

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

///@brief ������ģ��
class CArenaMoudel : public CSingleton<CArenaMoudel>
{
public:
	CArenaMoudel();
	~CArenaMoudel();
	void OnDispatchArenaMsg(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	bool InitArenaData();
	bool Release();
	void BackupArenaRankData();
	///@breif �������ʱ����Ҫ���ô˽ӿڱ����Լ��ľ��������ݵ��й�������
	bool SetPlayerManagedArenaData(CBaseDataManager* pDataMgr);
	///@breif ���ݴ���ľ�����ID�������䵱ǰ����������
	int GetArenaRank(const int arenaID);
	///@brief �����������������״̬
	SArenaCompetitor*  UnlockCompetitor(const int arenaID);
	///@brief ���뾺����
	bool DebutArena(DNID dnidClient, CBaseDataManager *baseDataMgr);
	///@ ����ͬ����Ҿ���������
	void SynLoginArenaData(DNID dnidClient, CBaseDataManager *pBaseDataMgr);
	///@ ������һ����ս��ʱ��
	void SendLastChallengTime(DNID dnidClient, int sid);
	///@ ����SID��ȡ�ϴ���ս��� 
	bool GetLastChallengTimespan(const int sid, DWORD &timespanOut);
	///@ ����SID��ȡ���������Ĵ���
	bool GetBuyChallengeCount(const int sid, DWORD &BuyNum);
	///@ ����SID��ȡ������ս�Ĵ���
	bool GetReSetCountDownCount(const int sid, DWORD &RestNum);
	///@ ����SID��ȡ��ս����
	bool GetArenaChallengCount(const int sid, DWORD &countOut);
	///@brief ����ǰ50���а�
	bool UpdateTop50th();
	///@brief ÿ������
	void ArenaRoutine();
	///@��ȡ�������
	DWORD GetBestRank(const int sid);
	//ͬ�����������а�����
	void ChangeNametoArena(int sid, char* name);
	///@brief ��ȡ����������й�����
	const SBaseArenaCompetitor* GetManagedArenaData(int arenaID);
private:
	///@brief ������ս���� 
	void _BuyChallengeNum(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief �����ھ���������ս
	void _OnRecvChallengeBegin(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief ������������
	void _OnRecvChallengeOver(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief ����滻����
	void _OnRandomReplacement(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief ��ȡǰ50�������
	void _OnGetTop50thCompetitors(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief ���÷�������
	void _OnSetDefensiveTeam(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief ��ȡ������Ҿ���������
	void _GetOnlineArenaData(CPlayer *pPlayer, SBaseArenaCompetitor & competitor);
	///@brief �������ʱ����ȡ�й�����
	SArenaCompetitor* _GetManagedArenaData(int arenaID);
	///@brief ��ȡ���а��ϵ������Ϣ
	void _GetTop50CompetitorDetail(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	///@brief �¿����������ɻ��������а�ǰ10000��
	void _GenerateRank();
	///@brief ����������˾�������Ϣ
	void _RandomRobotInfo(int level, int heroLevel, int rank, int star, int skillLevel, SArenaCompetitor& competitor, int arenaRank);

	///@brief ��ս��ʱ
	void _Timeout(DWORD arenaID);
	//���þ�������սʱ��
	void _ResetCountDown(DNID dnidClient, SSGArenaMsg *pSArenaMsg, CPlayer *pPlayer);
	const CGlobalConfig &globalconfig;
	int m_RobotID;
	map<int, SArenaCompetitor> m_ArenaManagedData; //�й����ݣ�keyֵΪ������������valueΪ��Ҿ���������
	map<int, int> m_ArenaIDRankMapping; //KeyֵΪ������ID��Value��������ӳ���
	SArenaPlayerReducedData* m_pTop50th; //���а�ǰ50��
	RobotNameConfig& m_RobotNameConfig;
	FunctionPtr m_UpdtateTop50thFucn;
	int m_defensiveTeam[5];
	int64_t m_lastRoutineTime;
};

