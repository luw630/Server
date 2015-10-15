// ----- CDiamondExchangeModule.h -----
//
//   --  Author: Jonson
//   --  Date:   15/03/13
//   --  Desc:   �ȶ�������Ԫ���һ����������ģ���࣬������ʱ������Ϣ�ķַ�
// --------------------------------------------------------------------
//	 --  ����������Ϊ���ھ��жһ���Ǯ���һ�������ģ�飬�������ܻ�������ģ�飬�����ӽ���������Ϣ����EPRO_SANGUO_DIAMONDEXCHANGE֮�£��������SSGPlayerMsg����Ϣ���ڷ���
//---------------------------------------------------------------------  
#pragma once
#include <unordered_map>
#include <memory>

class CDiamondExchangeProcess;

class CDiamondExchangeModule
{
public:
	CDiamondExchangeModule();
	~CDiamondExchangeModule();

	///@brief ���Ԫ���һ���������Ĳ���
	void AddProcesser(int processerType, shared_ptr<CDiamondExchangeProcess> processer);
	///@brief ��ˢ�µ���ˢ������
	void RefreshData();
	void RecvMsg(const SMessage *pMsg);

private:
	unordered_map<int, shared_ptr<CDiamondExchangeProcess>> m_ProcesserList;///<��ǰ�ı�ʯ�һ������ļ���
};

