// ----- CDuplicate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ļ���������
// --------------------------------------------------------------------
//   --  �������е���֤�߼��������߼���
//---------------------------------------------------------------------   
#pragma once
#include <memory>
#include <map>
using namespace std;

class CTollgate;
class CDuplicateAstrict;
class CBaseDataManager;
class CDuplicateDataManager;
struct SATollgateDropedItem;
struct ChapterConfig;
enum InstanceType;
enum CheckResult;

class CDuplicate
{
public:
	CDuplicate();
	~CDuplicate();

	void Init(CDuplicateDataManager* pDuplicateDataManage, int chapterID);
	void Update();
	///@brief ���Ҫ����ս�������������һЩ���������������ս�ؿ�����Ʒ���䣬�۳���ս���ģ�������սCD�Ȳ���
	///@param challengeTime ��Ҫ����ɨ�������еĶ�ε�����㣬�۳���Ӧ����ս���ĵ�
	///@param bMopUpOperation �Ƿ�Ϊɨ������
	void ActionWhileEnterBattle(int challengeTime = 1, bool bMopUpOperation = false);
	///@brief ������㣬�����Ǯ���Ӿ��顢��װ����
	///@param SQDuplicateEarningClear �ͻ��˷���������ؿ��������Ϣ
	///@param challengeTimes ��ʾ�ùؿ�һ������ս�˶��ٴ�
	void EarningClearing(int duplicateID, int tollgateID, int combatGrade, int challengeTimes = 1);
	///@brief ˢ�´˸�������ս����
	void RefreshTicket();
	///@brief ����ĳһ��ָ���ؿ���Ʊ��
	void ResetSpecifyTollgateTicket(int tolllgateID);
	///@brief ���ָ���Ĺؿ��Ƿ����������ս������ɨ��
	///@param tollgateID ��ʾҪ�����һ���ؿ����Ƿ���ս
	///@param challengeTime ��ս������Ĭ��Ϊ1�������ɨ���Ļ��ͻ����1
	///@param mopUpOperation Ĭ��Ϊfalse����ʾ�Ƿ�Ϊɨ������
	CheckResult Check(int tollgateID, int challengeTimes = 1, bool mopUpOperation = false);
	///@brief ����ؿ�
	void Activate();
	///@breif �رձ�����
	void Deactivate();
	///@brief ������������йؿ�
	void ActiveAllTollgate();
	///@brief ������������������ս�Ĺؿ����tollgateIDָ���Ĺؿ�
	void ActivateTo(int tollgateID);
	///@brief �ж�ĳһ���ؿ��Ƿ��ڱ�������
	bool IsTollgateExist(int tollgateID);

	///@brief ���õ�ǰ��������һ������ID������ͨ�ر������󼤻���һ������
	void SetNextDuplciateID(int duplicateID);

	///@brief ��ȡ��ǰ�ؿ��Ƿ��Ѿ���ս�ɹ�
	bool GetCompletedState();
	///@brief ��ȡ��ǰ�ؿ��Ƿ��Ѿ�����
	bool GetActiveState();
	///@brief ��ȡ��ǰ�ĸ�������
	InstanceType GetCurInstanceType();
	///@brief ��ȡ������ID
	int GetDuplicateID();
	///@brief ��ȡ��ǰѡ��Ĺؿ�ID
	///@note ʧ�ܷ���-1���ɹ��Żض�Ӧ��ֵ
	int GetCurTollgateID();
	///@brief ��ȡ��ǰ��������һ��������ID
	int GetNextDuplicateID();
	///@brief ���Ѿ�����õ�Ҫ�������Ʒ��һ�η����ͻ���
	void ReSendCaculatedDropItem();

private:

	bool m_bActivete;
	bool m_bInitFlag;
	int m_iCurDuplciateID;
	int m_iNextDuplicateID;
	int m_iDefaultActivateTollgateID;
	shared_ptr<SATollgateDropedItem> m_ptrDropedItemMsg;///<�ȱ��棬��������ͻ���û���յ�������Ʒ����Ϣ���Է�����
	shared_ptr<CTollgate> m_ptrCurSelectedTollgate;
	unique_ptr<CDuplicateAstrict> m_ptrDuplicateAstrict;
	InstanceType m_curInstanceType;///< ��ǰ����������
	map<int, shared_ptr<CTollgate>> m_mapTollgateContainer;
	CBaseDataManager* m_ptrDataManager;		///��ǰ��ͨ�����ݹ���ģ��
	CDuplicateDataManager* m_ptrDuplicateDataManager;///<��ǰ�ĸ������ݹ���ģ��
	const ChapterConfig* m_ptrChapterConfig;
};

