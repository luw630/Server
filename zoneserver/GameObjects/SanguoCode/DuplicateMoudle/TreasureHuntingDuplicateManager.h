// ----- CTreasureHuntingDuplicateManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ��������ؾ�Ѱ�������Ĺ�����
// --------------------------------------------------------------------
//   --  �������ĳ�ʼ���Լ���֤�߼��������߼���
//---------------------------------------------------------------------   
#pragma once
#include "DuplicateBaseManager.h"

class CTreasureHuntingDuplicateManager : public CDuplicateBaseManager
{
public:
	CTreasureHuntingDuplicateManager(CExtendedDataManager& dataMgr);
	virtual ~CTreasureHuntingDuplicateManager();

	virtual void InitMgr();
	virtual void Update();

protected:
	///@brief ����ͻ��˷�������Ϣ
	virtual void DispatchMsg(const SDuplicateMsg* pMsg);

private:
};

