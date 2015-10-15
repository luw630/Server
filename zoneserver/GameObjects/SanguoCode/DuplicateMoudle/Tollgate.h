// ----- CBaseTollgate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ĺؿ�������
// --------------------------------------------------------------------
//   --  ����ؿ��е���֤�߼��������߼��ȡ����еĵ�һ�ιؿ�����ֻ������ͨ�������������¸��������У�ɨ������Ҳ��ֻ������ͨ��������Ӣ�����в��У�����˵�������ؾ�Ѱ�������о�û��
//   --  ���ڲ��ֳ��ܶ���ɢ����Ŀ��ǣ���ʱ�������������߼����룬�������ڴˣ������������Ĺ��ܲ��컯�������ԡ��๦�ܸ����Ӵ�����÷�������߼�
//---------------------------------------------------------------------   
#pragma once
#include <unordered_map>
#include <memory>
using namespace std;

enum InstanceType;
enum CheckResult;
struct BattleLevelConfig;
class CBaseDataManager;
class CTollgateDrop;
class CDuplicateDataManager;

class CTollgate
{
public:
	CTollgate();
	~CTollgate();

	void Init(CDuplicateDataManager* dataManager, const BattleLevelConfig* config, InstanceType type);
	///@brief ��Ҫ��������CD����CD��ʱû����
	void Update();
	///@brief ���Ҫ����ս�������������һЩ������������㱾�ؿ�����Ʒ���䣬�۳���ս���ģ�������սCD�Ȳ���
	///@param challengeTime ��Ҫ����ɨ�������еĶ�ε�����㣬�۳���Ӧ����ս���ĵ�
	void ActionWhileEnterBattle(int challengeTime = 1);
	///@brief ������㣬�����Ǯ���Ӿ��顢��װ����
	///@param challengeTime ��Ҫ����ɨ�������еĶ�ε�����㣬�۳���Ӧ����ս���ĵ�
	void EarningClearing(int challengeTime = 1);
	///@brief ˢ�´˹ؿ�����ս����
	void RefreshTicket();
	///@brief ��⵱ǰ�ؿ��Ƿ����������ս���ؿ�������ɨ�����ؿ�
	///@param challengeTime ��ս������Ĭ��Ϊ1�������ɨ���Ļ��ͻ����1
	///@param mopUpOperation Ĭ��Ϊfalse����ʾ�Ƿ�Ϊɨ������
	CheckResult Check(int challengeTime = 1, bool mopUpOperation = false);
	///@brief ���������˹ؿ���һЩ���ݵ�����
	void Conquer();
	///@brief ����ؿ�
	void Activate();
	///@brief �رձ��ؿ�
	void Deactivate();
	///@brief ������ı��ؿ���ʱ���ʼ�����ؿ�������ս��������ս��������Ϣ
	void ArrangeDataWhileActivated();

	///@brief ���õ�ǰ�ؿ�����һ���ؿ����Ա㵱ǰ�ؿ���ս�ɹ��󼤻���һ���ؿ�
	void SetNextTollgateID(int tollgateID);
	///@brief ���ùؿ���ս����
	void SetBattleGrade(int grade);

	///@brief ��ȡ��ǰ�ؿ��Ƿ��Ѿ���ս�ɹ�
	bool GetCompletedState();
	///@brief ��ȡ��ǰ�ؿ��Ƿ��Ѿ�����
	bool GetActiveState();
	///@brief ��ȡ��ǰ�ؿ���ID
	int GetTollgateID();
	///@brief ��ȡ��һ���ؿ���ID
	int GetNextTollgateID();
	///@brief ��ȡ��ǰ�ؿ��������ͨ��Ʒ��Ϣ
	const unordered_map<int, int>* GetCommonDropOutItemList();
	///@brief ��ȡ��ǰ�ؿ�Boss�������Ʒ��Ϣ
	const unordered_map<int, int>* GetBossDropItemList();

protected:
	///@brief ��Ҫ������һ�ιؿ��������Ϣ�Ѿ��������Ϣ
	void Reset();
	///@brief ������Ʒ�ĵ���
	///@param challengeTime ����1�Ļ��ͻὫ��εĵ������浽��Ӧ�������У�����ɨ������ʹ��
	///@param bMopUpOperation �Ƿ�Ϊɨ������
	void CalculateDropOutItem(int challengeTime = 1, bool bMopUpOperation = false);
	///@brief �����ս�ɹ�����ջ����Ʒ,��֧�ּӸ���������Item
	void AddEarningItem(int itemID, int itemNum);

private:
	bool m_bCompleted;
	bool m_bActivated;
	bool m_bFirstChallenge;
	bool m_bMopUpOperation;
	int m_iTollgateId;     ///<�ؿ�ID//
	int m_iNextTollgateID;	///<��һ���ؿ�ID
	int m_iCurBattleGrade; ///< ��ǰ��ս������
	int m_iTeamExp;     //ս�Ӿ���//
	int m_iGeneralsExp; //�佫����//
	//int m_iGloryValue; //����ֵ//
	//int m_iSalaryValue; //ٺ»ֵ//
	int m_iMoney;
	float m_fCurCD;		///<��ս��CD����ʱ����
	int m_iTeamMinimumLevel;    ///<ս����͵ȼ�//
	int m_iChanllengedTimes;		///<��ս�˶��ٴ�
	int m_iOneDayChanllengeCount;   ///<ÿ�տ���ս�Ĵ��� <0��ʾ������//
	int m_iChallengeStaminaCost;	///<��ս����������
	InstanceType m_curInstanceType; ///<��ǰ����������
	unordered_map<int, int> m_mapFirstDropItems;///< �������ͨ�������������¸������Ļ��״ε������ƷID������,keyΪID��valueΪ����
	unordered_map<int, int> m_listItem;///�ջ����Ʒ�б�keyֵΪitemID��valueֵΪitemID��Ӧ�ĸ���
	unique_ptr<CTollgateDrop> m_ptrDropOut;        ///<������Ʒ//
	const BattleLevelConfig* m_ptrConfig;
	CBaseDataManager* m_ptrDataManager;		///��ǰ��ͨ�����ݹ���ģ��
	CDuplicateDataManager* m_ptrDuplicateDataManager;///<��ǰ�ĸ������ݹ���ģ��
};

