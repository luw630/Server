// ----- CExpeditionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ĺ���ն����������ݵĹ���
// --------------------------------------------------------------------
//   --  ͨ������洢�����ݿ��еĹ���ն��������ݣ���Ϊ����ն������ص��߼�����ʵ��ҵ��������ݲ�ķ���
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>

struct SExpeditionData;
struct SHeroData;
struct CGlobalConfig;
struct ExpeditionInstanceInfor;
class TimerEvent_SG;

class CExpeditionDataManager :
	public CExtendedDataManager
{
public:
	CExpeditionDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CExpeditionDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();
	
	///@brief �������Ƿ�ȫ������
	bool CheckCompletelyAnnihilated();

	///@brief ����Զ���ؿ��Ƿ�ͨ�ص�״̬
	void SetConquredState(bool state);
	///@brief �����Ƿ������佫��״̬
	void SetHeroRewardState(bool state);
	///@brief ���ý����Ƿ���ȡ��״̬
	void SetRewardsUnclaimedState(bool state);
	///@brief ����Զ�������״̬
	void SetExpeditionActiveState(bool state);
	///@brief ���õ�ǰԶ���ؿ���ID
	void SetCurExpeditionInstanceID(DWORD ID);
	///@brief ���õ�ǰ�ؿ����Ѷ�ϵ��
	void SetCurLevelFactor(float factor);
	///@brief ����һ����Ʊ
	void CostATicket();
	///@brief ���õ�ǰ��������Ʒ��ID
	void SetCurItemObtained(DWORD itemID);
	///@brief ����Զ�����˵�������Ϣ
	void ResetEnemyInfor();
	///@brief �ж��佫ID�Ƿ����Ѿ���¼�ĵط��佫�б���
	bool IsEnemyExist(DWORD heroID);
	///@brief ��ƥ��õĵ��˵�IDѹ�뵽������
	void CacheMarchedEnmey();
	///@brief �����ѡ����佫��¼����
	///@param selectedHeroΪѡ�е�Ӣ�۵����飬����Ϊ5
	void CacheSelectedHero(const DWORD* selectedHero);
	///@brief ���õ��˵�Ѫ������ֵ����������ֵ��Ϣ
	void SetEnemyInfor(DWORD heroID, float healthScale, float manaScale);
	///@brief �����Ƿ�׼���˽������ؿ�
	void SetPermissionGotState(bool state);
	///@brief ���õ����Ƿ���ƥ���״̬
	void SetEnemyMarchingState(bool state);

	///��ȡ���֤״̬
	bool GetPermissionState();
	///@brief ��ȡ����ƥ���״̬���Ƿ���ƥ���У�
	bool GetEnemyMarchingState();
	///@brief ��ȡԶ���ؿ�ͨ�ص����
	bool GetConquredState();
	///@brief ��ȡԶ���ؿ��Ƿ�����Ӣ�۵�״̬
	bool GetHeroRewardState();
	///@brief ��ȡ�����Ƿ���ȡ��״̬
	bool GetRewardsUnclaimedState();
	///@brief ��ȡԶ���Ƿ񼤻��״̬
	bool GetExpeditionActiveState();
	///@brief ��ȡԶ���ؿ��ĵ�ǰID
	///@param data Ҫ��ȡ��������Ϣ
	///@return �Ƿ�ɹ�ȡ����Ҫ��ֵ
	bool GetCurExpeditionInstanceID(OUT DWORD& data);
	///@brief ��ȡ��ǰԶ���ؿ���Ʊ��
	///@param data Ҫ��ȡ��������Ϣ
	///@return �Ƿ�ɹ�ȡ����Ҫ��ֵ
	bool GetCurTicketsNum(OUT DWORD& data);
	///@brief ��ȡ��ǰ�Ļ�õ���Ʒ������ID
	///@param data Ҫ��ȡ��������Ϣ
	///@return �Ƿ�ɹ�ȡ����Ҫ��ֵ
	bool GetCurItemObtained(OUT DWORD& data);
	///@brief ��ȡ��ǰ�ж���ҵ��佫������
	///@param arraySize ����Ĵ�С
	SHeroData* GetCurHostileCharacterInfor();
	///@brief ���ĳһ��Ӣ��ID�Ƿ�����Ҹո�ѡ���˵�
	bool IsHeroSelected(DWORD heroID);
	///@brief ��ȡ��ǰ��VIP�ȼ���Ӧ��Ч����������
	void CacheCurProceedsRiseState();
	///@brief ��õ�ǰ�Ļ��õĽ�Ǯ����
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷŻ�-1
	int GetCurExpectedMoneyProceeds();
	///@brief ��õ�ǰ�Ļ��ý�Ǯ����
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷŻ�-1
	int GetCurExpectedExploitProceeds();

	///@brief ����ˢ��Զ���ؿ���Ʊ��
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	void RefreshTicket(int sendMsg = 0);

private:
	bool _checkExpeditionDataAvaliable();

	bool m_bPermissionGot;
	bool m_bEnemyMarching;
	int m_iProceedsRiseState; ///<���ջ��������ɵı���
	int m_dwCurMaxAvaliableTickets; ///<��ǰ��������ö��ٴ�Զ��
	DWORD m_dwAvaliableHostileCharacterNum;
	SExpeditionData* m_ptrData;
	const CGlobalConfig& globalConfig;
	unordered_set<int> m_selectedHeroID;
	unordered_map<DWORD, SHeroData*> m_hostileCharacterList;
};

