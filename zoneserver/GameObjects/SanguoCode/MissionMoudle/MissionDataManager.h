// ----- CMissionDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������������������ݵĹ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <functional>
#include <memory>
#include <unordered_map>
typedef std::function<void(int)> FunctionPtr;

///@brief ��������ݹ�����
struct SAMissionAccomplished;
struct SAUpdateMissionState;
class TimerEvent_SG;
enum MissionRewardsType;
enum MissionType;

class CMissionDataManager :
	public CExtendedDataManager
{
public:
	CMissionDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CMissionDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief ��lua�����Ѿ����У���������ݻ�δ���͵��ͻ��˵�ʱ��Ĳ���
	void OnLuaInitialed();
	///@brief �����������ʱ��Ļص�
	void SetMissionAccomplishedCallBack(FunctionPtr& fun);
	///@brief �����������ɴ���
	///@param MissionType �����Ӧ������
	///@param times ����
	void SetMissionCompleteTimes(MissionType type, DWORD times, bool synMsg = true);
	///@brief ����ĳһ���������
	///@param missionID �����Ӧ��ID
	void SetMissionAccompulished(DWORD missionID);
	///@brief ����������������
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	void ResetMissionData(int sendMsgFlag = 0);
	///@brief ����������ж�ģ�飬��ҪΪ��͵��������ʾʱ��ε��ж�
	void LunchMissionVisible();
	///@brief ����������ж�ģ�飬��ҪΪ��͵��������ʾʱ��ε��ж�
	void DinnerMissionVisible();
	///@brief ����������ж�ģ�飬��ҪΪҹ�����������ʾʱ��ε��ж�
	void SupperMissionVisible();
	///@brief ����������ж�ģ�飬��ҪΪ��͵�����Ŀ���ȡʱ��ε��ж�
	void LunchMissionAccessible();
	///@brief ����������ж�ģ�飬��ҪΪ��͵�����Ŀ���ȡʱ��ε��ж�
	void DinnerMissionAccessible();
	///@brief ����������ж�ģ�飬��ҪΪҹ��������Ŀ���ȡʱ��ε��ж�
	void SupperMissionAccessible();
	///@brief ҹ������Ĺرմ���
	void SupperMissionDisable();

private:
	enum MissionDataType
	{
		complistedTiems = 1, //�������ɴ���
		visible, //����Ŀɼ���״��
		accomplished, //��������״��
	};

	///@brief ��ʼ��ʱ����������������ж�
	void EnduranceMissionInitJudgement();
	///@brief ��������Ĵ���ģ�飬��ʱΪ��͡���͡�ҹ������ĸ��ͻ��˴򽻵����߼�����
	///@param type �����������ľ������ͣ�����ͻ�����ͻ���ҹ������
	///@param accessible �����Ƿ�ɽ�
	///@param visible ������������Ƿ�ɼ�
	///@param conflitProcess �Ƿ�������������񻥳�����
	///@param sendMsg �Ƿ��������Ϣ���ͻ���
	void EnduranceMissionProcess(MissionType type, bool visible, bool accessible, bool conflitProcess, bool sendMsg = true);
	///@brief �����������ɴ���
	///@param missionID �����Ӧ��ID
	///@param times ����
	void SetMissionCompleteTimes(int missionID, DWORD times, bool synMsg = true);

	bool m_bInitResetMission;
	FunctionPtr m_operationOnMissionAccomplished;///ĳһ��������ɺ����������
	shared_ptr<SAUpdateMissionState> m_ptrUpdateMissionStateMsg;
	shared_ptr<SAMissionAccomplished> m_ptrMissionAccomplishedMsg;
};