// ----- CExpeditionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ĺ���ն��������߼��Ĺ���
// --------------------------------------------------------------------
//   --  �������ն���е���Ҫ�߼��жϣ������Ƿ������ս�����˵�ƥ�䡢��ս�ɹ��Ľ���ȵȣ�Ҳ������ͻ���֮�����Ϣ���շ�
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
	///@brief ����Զ��ͨ�صĹؿ���
	void _UpdateExpeditionTimes();
	void _AskToCostATicket();
	void _ActiveExpedition();
	void _BattleResultResolves();
	void _GetNextEnmyInfor();
	void _RewardsReceived();
	void _ExitBattle();
	///@brief ����Ƿ��ܽ��뵽������
	void _CheckEnterBattlePermission(const SMessage *pMsg);
	///@brief �ɿͻ���ͬ����ս�佫��Ѫ������Ϣ����
	void _SynSelectedHeroInfor(const SMessage *pMsg);
	///@brief �ɿͻ���ͬ����ս�ط�Ӣ�۵�Ѫ������Ϣ����
	void _SynHostileEnmyInfor(const SMessage *pMsg);
	void _MatchEnemy();
	void _GenerateEnemy(const SHeroData* refHeroInfo, int enemyID, int enemySrialNumber);

	int m_iFirstExpeditionLevelID;///<��һ��Զ���ؿ���ID
	SABattleResolve m_msgResolveInfo;
	SAConqured m_msgExpeditionConqured;				///<Զ������ͨ��
	SAPermissionInfor m_msgPermissionInfor;
	SAHostileEnemyInfor m_msgHostileEnmeyInfor;
	CExpeditionDataManager * m_ptrDataMgr;
	const CGlobalConfig& globalConfig;
	const SHeroData* m_excellentHeroInfor;
	const ExpeditionInstanceInfor* m_ptrCurLevelInstanceInfor;
};

