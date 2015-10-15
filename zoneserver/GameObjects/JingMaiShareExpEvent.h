#pragma once
#include "./Player.h"
#include "./EventTimer.h"

namespace JingMaiShareExp
{
	class ReadyJingMaiEvent : public Event::ReadyEvent
	{
	public:
		ReadyJingMaiEvent(CPlayer *pUser, WORD wXueID, DWORD dwEventInterrupType)
			: Event::ReadyEvent(dwEventInterrupType)
			, m_pUser(pUser)
			, m_wXueID(wXueID)
		{	
			/// 通知客户端读数  ///
			

		}

		virtual void OnActive()
		{
			if (m_pUser)
				m_pUser->OnShareExpWhenJingMaiOpenVenSuccess(m_wXueID);
		}

		virtual void OnCancel()
		{
			//if (m_pUser) m_pUser->OnShareExpWhenJingMaiOpenVenSuccess(m_wXueID);
			/// 通知客户端取消读数 ///
		}

		virtual void SelfDestory()
		{
			if (m_pUser)
				m_pUser->m_BatchReadyEvent.OnCloseReadyEvent(this);
			delete this;
		}

	private:
		CPlayer*	m_pUser;		
		WORD		m_wXueID; //穴位ID
	};

}