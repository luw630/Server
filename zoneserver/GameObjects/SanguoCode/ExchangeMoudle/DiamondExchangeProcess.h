// ----- CDiamondExchangeProcess.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   �ȶ�������Ԫ���һ���������Ĵ��������
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once

struct SDiamondExchangeMsg;
class CBaseDataManager;

class CDiamondExchangeProcess
{
public:
	CDiamondExchangeProcess(){};
	virtual ~CDiamondExchangeProcess(){};

	///@brief ��ʼ���������߼������ݵ�����
	virtual void Init(void* data) = 0;
	virtual void RecvMsg(const SDiamondExchangeMsg *pMsg) = 0;
	///@brief ����Ԫ���û�����������������
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	virtual void ResetData(int sendMsg = 0) = 0;
};

