// ----- CDuplicateBaseManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ���������ͨ���¸�������Ӣ����������˵���ؾ�Ѱ�������Ĺ�����Ĺ�ͬģ��
// --------------------------------------------------------------------
//   --  ���ｫ�ַ��ȶ��������ĸ�������ص����ݵĹ�����������ͬ��һ��ģ�鲢��װ������
//   --  ����ĳһ���͵ĸ����е���֤�߼��������߼���
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
	///@brief ������͵�ָ���ĸ���
	///@param duplicateIDs Ҫ�����һϵ�еĸ���ID
	///@param IDNum Ҫ����ĸ����ĸ���
	///@param bSendMsg ��������Ƿ����ͻ��˷��ͼ�����Ϣ
	virtual void ActiveSpecifyDuplicate(int duplicateIDs[], int IDNum, bool bSendMsg = false)const;
	///@brief �˴���ս���ķѵ�ʱ��
	float BattleSpendSeconds() const;
protected:
	virtual void InitProgress() final;
	virtual void DispatchMsg(const SDuplicateMsg* pMsg) = 0;
	///@brief ���ͻ���������սĳһ���ؿ���ʱ�����������һЩ����
	virtual void OnAskToEnterTollgate();
	///@brief ������սĳһ���ؿ�
	virtual void AskToEnterTollgate(const SDuplicateMsg *pMsg) final;
	///@brief �ؿ�������߼�����
	virtual void EarningClearProcess(int duplicateID,  int tollgateID, int combatGrade, bool sendMsg = true) final;
	///@brief ��������Ķ������,��ʱ��EarningClearLogicProcess�ж�������
	virtual void EarningClearingExtraProgress();
	///@brief ���ͻ���׼������ս������ˢ�ֵ�ʱ��Ҫ����Ӧ�ؿ��ĵ�����Ϣ�������ͻ���
	virtual void OnStartToChallenge();
	///@brief �ͻ�����һ������ؿ��������Ʒ
	virtual void ReRequestTollgateDropedItem(const SDuplicateMsg *pMsg) final;
	///@brief ������͸������еĸ���
	virtual void ActiveAllDuplicate() final;
	///@brief �ж�ĳһ������ID�Ƿ����ڵ�ǰ�ĸ�������
	virtual bool IsDuplicateExist(int duplicateID) final;
	///@brief �����ӵ�ǰѡ���Ӣ�۵ľ���ֵ
	virtual void PlusHeroExpBatch() final;
	///@brief ����ĳһ���ؿ�����ս����
	virtual void ResetSpecifyTollgateChallegedTimes(const SDuplicateMsg *pMsg) final;
	///@brief "����ĳһ���ؿ�����ս����"���ܵ�Ԫ�����ѻ�ȡ
	///@return �ɹ��Żض�Ӧ��ֵ��ʧ�ܷ���-1
	virtual int GetCostOfResetSpecifyTollgateChallengedTimes(int tollgateID);
	///@brief ���ü���ĳһϵ�еĸ�������Ϣ��������״̬������Ϣ��������״̬��ָָ����ĳ���͵����и���ֻ�б���Ϣ��ָ���Ŀɿ��ŵĸ������ܿ���
	virtual void GetActiveDuplicateExclusivityState(bool& exclusivityState) const;

protected:
	bool m_bInitFlag;///<�Ƿ��Ѿ���ʼ����
	bool m_bPermissionGot;///<�Ƿ�������ս
	bool m_bEarningCleared;///<�Ƿ��������ˣ����ڴ����������󣬷��͸��ͻ��˵���Ϣ�ͻ���û���յ������
	int m_iCurChallengeTimes;///<��ǰ����ս������ÿ�οͻ���ѯ���Ƿ������ս�ؿ�ʱ������жϿ�����ս�����������ս������¼����
	InstanceType m_DuplicateType; ///<��ǰ�ĸ�������
	const set<int>* m_ptrDuplicateIDList;	///<��ǰ�������͵����и���ID
	CAchieveUpdate* m_ptrAcievementUpdate; ///<���¹ؿ��������������ɾ��߼�
	CMissionUpdate* m_ptrMissionUpdate;		///<���¹ؿ��������������
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdator;///<���¸������������ɾ͵��߼�
	shared_ptr<CDuplicate> m_ptrCurDuplicate;///<��ǰ�û����ڴ�ĸ���
	CBaseDataManager& m_BaseDataManager;		///��ǰ��ͨ�����ݹ���ģ��
	CDuplicateDataManager& m_duplicateDataMgr;///<��ҵ���ͨ�������������¸���������Ӣ����������˵�������ؾ���˵���������ݹ�������
	map<int, shared_ptr<CDuplicate>> m_duplicateContainer;///<���˸��ั�����еĸ���ʵ��
	unique_ptr<SAChallengePermissionInfor> m_msgEnterPermissionAnswer;	///<�ؿ���սѯ�ʵĽ��
	unique_ptr<SAResetSpecifyTollgate> m_msgResetSpecifyTollgate; ///<����ĳһ���ؿ�����ս�����Ľ��
	shared_ptr<SAActivateDuplicate> m_msgActivateDuplciate;	///<����һϵ�еĸ���
	shared_ptr<SATollgateEarningClearedInfor> m_msgTollgateEarningClearedAnswer; ///<�ؿ�����ɹ������ͻ��˷��͵���Ϣ
	__time64_t m_tBattleStartTime;	///<ս����ʼʱ��
};

