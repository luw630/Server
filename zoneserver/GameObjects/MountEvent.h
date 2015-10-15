#pragma once
#include "./Player.h"
#include "./EventTimer.h"

namespace Mounts
{
	// ���������¼�
	class ReadyMountEvent : public Event::ReadyEvent
	{
	public:
		ReadyMountEvent(CPlayer *pUser, int mountIndex, DWORD dwEventInterrupType) 
			: ReadyEvent(dwEventInterrupType)
		{ 
			m_pUser		 = pUser; 
			m_MountIndex = mountIndex; 
			
			// ֪ͨ�ͻ��˶���
			SAPreUseItemMsg msg;
			msg.time		= 2000;
			msg.operateType = SAPreUseItemMsg::OT_RIDE;

			if (m_pUser)
			{
				g_StoreMessage(m_pUser->m_ClientIndex, &msg, sizeof(SAPreUseItemMsg));
				m_pUser->SendSynPlayMount(pUser->m_Property.mount[mountIndex].ID, pUser->m_Property.mount[mountIndex].Level, SAMountAction::MA_READY);
			}
		}

		virtual void OnActive()
		{
			if (m_pUser)
				m_pUser->UpMounts(m_MountIndex);
		}

		virtual void OnCancel()
		{
			// ֪ͨ�ͻ���ȡ������
			SAPreUseItemMsg msg;
			msg.time		= 0;
			msg.operateType = SAPreUseItemMsg::OT_RIDE_CANCEL;

			if (m_pUser)
			{
				g_StoreMessage( m_pUser->m_ClientIndex, &msg, sizeof(SAPreUseItemMsg));
				m_pUser->SendSynPlayMount(m_pUser->m_Property.mount[m_MountIndex].ID, m_pUser->m_Property.mount[m_MountIndex].Level, SAMountAction::MA_CANCEL_READY);
				m_pUser->m_InRidingevens = 0;
			}
		}

		virtual void SelfDestory() 
		{
			if (m_pUser) 
				m_pUser->m_BatchReadyEvent.OnCloseReadyEvent(this); 
			delete this;
		};
	
	private:
		CPlayer* m_pUser;
		int		 m_MountIndex;
	};


	// ���＼����ȴ
// 	class MountSkillCD : public EventMgr::EventBase
// 	{
// 	public:
// 		MountSkillCD(){ m_SkillIndex = 0; m_MountIndex = 0; m_bActive = false; }
// 		void Init(int mountindex,int skillindex){ m_MountIndex=mountindex; m_SkillIndex = skillindex; }
// 
// 		// �¼�����������
// 		virtual void OnActive(EventMgr *mgr){ m_bActive = false; }
// 		virtual void OnCancel(EventMgr *mgr){ m_bActive = false; }
// 		// ������ȴʱ��
// 		void		Start( CPlayer* pPlayer, DWORD time)
// 		{
// 			EventMgr::singleton().SetEvent(this, time);
// 			m_bActive = true;
// 			SAMountSkillColdTimer cdmsg;
// 			cdmsg.MountIndex = (BYTE)m_MountIndex;
// 			cdmsg.SkillIndex = (BYTE)m_SkillIndex;
// 			cdmsg.ColdeTime = (INT32)time;
// 			g_StoreMessage( pPlayer->m_ClientIndex, &cdmsg, sizeof(SAMountSkillColdTimer) );
// 		}
// 		// �Ƿ��Ѿ�������ȴ״̬. ����true��ʾ�ѽ�����ȴ״̬.
// 		bool		IsActive( ){ return m_bActive; }
// 		int			GetMountIndex() { return m_MountIndex; }
// 
// 	private:
// 		int		m_SkillIndex;
// 		int		m_MountIndex;
// 		bool	m_bActive;
// 	};
};
