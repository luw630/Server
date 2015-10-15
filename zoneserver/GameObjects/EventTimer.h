#pragma once
#include "EventBase.h"

namespace Event
{
	// �¼��ж�����
	enum TEventInterrupt
	{
		EEventInterrupt_Move = 0x0001, // �ƶ��ж�
		EEventInterrupt_CastItem = 0x0002, // ������Ʒ�ж�
		EEventInterrupt_CastSkill = 0x0004, // ���������ж�
		EEventInterrupt_UseSkill = 0x0008, // ʹ�ü����ж�
		EEventInterrupt_Collect = 0x0010, // �ɼ��ж�
		EEventInterrupt_PreMount = 0x0020, // ��˶����ж�
		EEventInterrupt_Jump = 0x0040, // ���ж�
		EEventInterrupt_Damaged = 0x0080, // �������ж��¼�

		EEventInterrupt_MoveItem = 0x0100, // �ƶ���������Ʒ�ж��¼�


		EEventInterrupt_Logout = 0x8000, // �˳���Ϸ�ж�

		// ����ж�- ���еĶ������(�Ƚϳ���)
		EEventInterrupt_Combinate_Base = 
			(EEventInterrupt_Move|EEventInterrupt_CastItem|EEventInterrupt_CastSkill
			|EEventInterrupt_UseSkill|EEventInterrupt_Collect|EEventInterrupt_PreMount
			|EEventInterrupt_Jump|EEventInterrupt_Damaged|EEventInterrupt_Logout),
		EEventInterrupt_Combinate_CastItem = (EEventInterrupt_Combinate_Base|EEventInterrupt_MoveItem),

		EEventInterrupt_Combinate_ALL = 0xFFFF, // �ض���ֹ

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
		DWORD			m_dwInterruptType;	//�ж�����
		std::string		m_EventName;
	};
}
