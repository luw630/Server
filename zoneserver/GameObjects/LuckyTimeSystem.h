// ----- CLuckyTimeSystem.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   �ȶ������ġ���ҹϵͳ������߼�
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <functional>
typedef std::function<void(int)> FunctionPtr;

class TimerEvent_SG;

class CLuckyTimeSystem
{
public:
	CLuckyTimeSystem();
	~CLuckyTimeSystem();

	///@brief �ж��Ƿ��ǡ���ҹ��ʱ�䣬������ʱ��
	bool IsLuckyTime() const;
	///@brief ���غ�ҹʱ������µ�ʱ�䣬������ں�ҹʱ���򷵻ػ��ж೤ʱ�䵽���ҹʱ��
	int LuckyTimeRemaining() const;

private:
	///@brief ��ҹ����
	void LuckyTimeStart(int fieldHolder);
	///@brief ��ҹ��ɢ
	void LuckyTimeEnd(int fieldHolder);

	bool m_bIsLuckyTime;
	time_t m_LuckyTimePoint;		///<��ҹ���ٵ�ʱ���
	FunctionPtr m_funcLuckyTimeStart;
	FunctionPtr m_funcLuckyTimeEnd;
	TimerEvent_SG* m_ptrLuckyTimeStartEventHandler;
	TimerEvent_SG* m_ptrLuckyTimeEndEventHandler;
};

