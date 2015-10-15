// ----- CLuckyTimeSystem.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   萌斗三国的“黑夜系统”相关逻辑
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

	///@brief 判断是否是“黑夜”时间，即幸运时间
	bool IsLuckyTime() const;
	///@brief 返回黑夜时间的余下的时间，如果不在黑夜时间则返回还有多长时间到达黑夜时间
	int LuckyTimeRemaining() const;

private:
	///@brief 黑夜降临
	void LuckyTimeStart(int fieldHolder);
	///@brief 黑夜退散
	void LuckyTimeEnd(int fieldHolder);

	bool m_bIsLuckyTime;
	time_t m_LuckyTimePoint;		///<黑夜降临的时间点
	FunctionPtr m_funcLuckyTimeStart;
	FunctionPtr m_funcLuckyTimeEnd;
	TimerEvent_SG* m_ptrLuckyTimeStartEventHandler;
	TimerEvent_SG* m_ptrLuckyTimeEndEventHandler;
};

