#ifndef _TIMER_ENTRY_H_
#define _TIMER_ENTRY_H_
/*

//----------------------战斗中冷却时间的功能代码-------------------------------------------
struct Timer_Par
{
	INT32 gid;
	INT32 skillID;
	BYTE  skillType;
};

typedef void (*Timer_Function)(Timer_Par*);

class TimerEntry
{
public:
	TimerEntry()
	{
		isActive = true;
		tick = 0;
		interval = 0;
		
		callBack = NULL;
		par.gid = -1;
		par.skillID = -1;
	}

	~TimerEntry()
	{
	}

public:
	 void SetCallBack(Timer_Function f,Timer_Par* newPar)
	{
		callBack = f;
		par = *newPar;
	}

	 void Start()
	{
		tick = 0;
		isActive = false;
		if(callBack)
		{	
			callBack(&par);
		}

		Process(0);
	}

	 void Stop()
	{
		isActive = true;
	}

	 bool IsActive()
	{
		return isActive;
	}

	 void SetInterval(INT32 newInterval)
	{
		interval = newInterval;
	}

	 INT32 GetInterval()
	{
		return interval;
	}

	 void Process(INT32 elapse)
	{
		if (!isActive)
		{
			tick += elapse;
			if (tick >= interval)
			{
				Stop();
			}
		}
	}

private:
	bool isActive;
	INT32 tick;
	INT32 interval;

	Timer_Function callBack;
	Timer_Par par;
};
*/
#endif

