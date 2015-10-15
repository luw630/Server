#pragma once
#include "../ExtendedDataManager.h"
#include <memory>
#include <functional>
//#include <vector>

class CBaseDataManager;
struct SChaseThiefData;
struct SThiefActivate;

typedef std::function<void()> EmptyFunction;

class CChaseThiefDataManager : public CExtendedDataManager
{
public:
	CChaseThiefDataManager(CBaseDataManager& baseDataMgr);
	~CChaseThiefDataManager();
	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();
	/**@brief ������Ч����*/
	void TakeEffect();
	/**@brief ����ʧЧ����*/
	void LoseEffectiveness();
	/**@brief �����Ч��*/
	bool CheckEffectiveness();
	/**@brief ����׷���������佫*/
	void SetChasingHeroes(int heroArr[], int num);
	/**@brief ׷��������սӢ��*/
	const std::vector<int>& GetChasingHeroes() const;
	EmptyFunction& GetActivateFunc() { return m_funcActivate; }
	/**@brief ����׷��С͵��־*/
	void SetChasing(bool flag) { m_bChaseStarted = flag; }
	/**@brief ��ȡ׷��С͵��־*/
	bool GetChaseFlag() { return m_bChaseStarted; }
	/**@brief ���ս������*/
	void RandomReward();
	/**@brief ��佱������
	@param [in] win ս�����
	@param [out] itemID ������ƷID
	@param [out] itemNum ������Ʒ����
	*/
	void FillReward(bool win, OUT int& itemID, OUT int& itemNum);
private:
	bool _ChaseThiefDataIsValid() const;
private:
	static DWORD m_dwChaseThiefDurationTime;	///<׷����������ʱ��
	static DWORD m_dwChaseThiefOpenLevel;	///<׷���������ŵȼ�
	static float m_fChaseThiefActivateRate;	///<׷��������������
	SChaseThiefData *m_pChaseThiefData;
	__time64_t m_tEffectiveTime;	///<׷��������Чʱ��
	shared_ptr<SThiefActivate> m_pActivateMsg;	///<������Ϣ
	EmptyFunction m_funcActivate;
	bool m_bChaseStarted;	///<׷����ʼ��־
	std::vector<int> m_vecChasingHeroes;	///<׷���������佫
	int m_iRewardID;	///<����ID
	int m_iRewardNum;  ///<��������
};

