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
			/// ֪ͨ�ͻ��˶���  ///
			

		}

		virtual void OnActive()
		{
			if (m_pUser)
				m_pUser->OnShareExpWhenJingMaiOpenVenSuccess(m_wXueID);
		}

		virtual void OnCancel()
		{
			//if (m_pUser) m_pUser->OnShareExpWhenJingMaiOpenVenSuccess(m_wXueID);
			/// ֪ͨ�ͻ���ȡ������ ///
		}

		virtual void SelfDestory()
		{
			if (m_pUser)
				m_pUser->m_BatchReadyEvent.OnCloseReadyEvent(this);
			delete this;
		}

	private:
		CPlayer*	m_pUser;		
		WORD		m_wXueID; //ѨλID
	};

}