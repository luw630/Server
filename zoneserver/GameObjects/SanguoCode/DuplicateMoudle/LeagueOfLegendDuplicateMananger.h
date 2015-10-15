// ----- CLeagueOfLegendDuplicateMananger.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ľ���˵�����Ĺ�����
// --------------------------------------------------------------------
//   --  �������ĳ�ʼ���Լ���֤�߼��������߼���
//---------------------------------------------------------------------   
#pragma once
#include "DuplicateBaseManager.h"

class CLeagueOfLegendDuplicateMananger : public CDuplicateBaseManager
{
public:
	CLeagueOfLegendDuplicateMananger(CExtendedDataManager& dataMgr);
	virtual ~CLeagueOfLegendDuplicateMananger();

	virtual void InitMgr();
	virtual void Update();

	///�жϱ����͸�����ĳЩ�����ļ���״̬
	void JudgeDuplicateAcitiveState();

protected:
	///@brief ����ͻ��˷�������Ϣ
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);
	///@brief ���ü���ĳһϵ�еĸ�������Ϣ��������״̬������Ϣ��������״̬��ָָ����ĳ���͵����и���ֻ�б���Ϣ��ָ���Ŀɿ��ŵĸ������ܿ���
	virtual void GetActiveDuplicateExclusivityState(bool& exclusivityState) const;
};

