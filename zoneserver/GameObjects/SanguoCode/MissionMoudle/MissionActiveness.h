// ----- CMissionActiveness.h -----
//
//   --  Author: Jonson
//   --  Date:   15/10/16
//   --  Desc:   �ȶ������������Ծ�ȵ�������ݵĸ��¹����뽱���ķַ�
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"


class CMissionActiveness :
	public CExtendedDataManager
{
public:
	CMissionActiveness(CBaseDataManager& baseDataMgr);
	virtual ~CMissionActiveness();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief ��lua�����Ѿ����У���������ݻ�δ���͵��ͻ��˵�ʱ��Ĳ���
	void OnLuaInitialed();
	///@brief ��ĳһ��������ȡ�ɹ���Ĳ���
	void OnMissionAccomplished(int missionID);
	///@brief ��ȡĳһ����Ծ�������
	void RequestClaimRewards(int activenessLevel);
	///@brief ���������Ծ��
	void ResetMissionActivenessData(int sendMsgFlag = 0);

private:
	bool m_bInitResetMissionActiveness;
	DWORD* m_dwMissionActiveness; ///<��һ��۵�����Ļ�Ծ��
};

