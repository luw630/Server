#pragma once
#include "EventBase.h"

namespace Event
{
	// 事件中断类型
	enum TEventInterrupt
	{
		EEventInterrupt_Move = 0x0001, // 移动中断
		EEventInterrupt_CastItem = 0x0002, // 吟唱物品中断
		EEventInterrupt_CastSkill = 0x0004, // 吟唱技能中断
		EEventInterrupt_UseSkill = 0x0008, // 使用技能中断
		EEventInterrupt_Collect = 0x0010, // 采集中断
		EEventInterrupt_PreMount = 0x0020, // 骑乘读条中断
		EEventInterrupt_Jump = 0x0040, // 跳中断
		EEventInterrupt_Damaged = 0x0080, // 被攻击中断事件

		EEventInterrupt_MoveItem = 0x0100, // 移动包裹中物品中断事件


		EEventInterrupt_Logout = 0x8000, // 退出游戏中断

		// 组合中断- 所有的都将打断(比较常用)
		EEventInterrupt_Combinate_Base = 
			(EEventInterrupt_Move|EEventInterrupt_CastItem|EEventInterrupt_CastSkill
			|EEventInterrupt_UseSkill|EEventInterrupt_Collect|EEventInterrupt_PreMount
			|EEventInterrupt_Jump|EEventInterrupt_Damaged|EEventInterrupt_Logout),
		EEventInterrupt_Combinate_CastItem = (EEventInterrupt_Combinate_Base|EEventInterrupt_MoveItem),

		EEventInterrupt_Combinate_ALL = 0xFFFF, // 必定终止

	};

	class ReadyEvent : public EventBase
	{
	public:
		ReadyEvent() { m_dwInterruptType = EEventInterrupt_Combinate_Base; }
		ReadyEvent(DWORD dwInterruptType) { m_dwInterruptType = dwInterruptType; }

		BOOL InterruptReadyEvent(BYTE _rate = 100)
		{
			if ((rand()%100)+1<=(int)_rate)
			{
				Interrupt();
				return TRUE;
			}
			return FALSE;
		}
		BOOL InterruptReadyEvent(DWORD dwInterruptType)
		{
			if ((dwInterruptType&m_dwInterruptType)!=0)
			{
				Interrupt();
				return TRUE;
			}
			return FALSE;
		}

		void SetInterrupType(DWORD dwInterruptType) { m_dwInterruptType = dwInterruptType; }
		DWORD GetInterrupType() { return m_dwInterruptType; }
		
		void SetEventName(const char* pName) 
		{ 
			if (pName) 
				m_EventName = pName; 
			else m_EventName = ""; 
		}

		bool IsEvent(const char* pName)
		{ 
			if (!pName) 
				return false; 
			return m_EventName.compare(pName) == 0; 
		}

	private:
		DWORD			m_dwInterruptType;	//中断类型
		std::string		m_EventName;
	};
}
