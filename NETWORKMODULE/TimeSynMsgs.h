#ifndef TIMESYNMSGS_H
#define TIMESYNMSGS_H

#include "NetModule.h"

DECLARE_MSG_MAP(STimeBaseMsg, SMessage, SMessage::EPRO_SYNTIME_MESSAGE)
	EPRO_TIMESYN_SYNTIME,			// ͬ�����������߼�ʱ��
	EPRO_TIMESYN_CHECKTIME,			// ����ʱ��У��
END_MSG_MAP_WITH_ROTOC()

// ��Ϣ����
DECLARE_MSG(SSynTimeMsg, STimeBaseMsg, STimeBaseMsg::EPRO_TIMESYN_SYNTIME)
struct SQSynTimeMsg : public SSynTimeMsg
{
	DWORD	m_clientTime;			// �ͻ��˵�ʱ��
};

struct SASynTimeMsg : public SSynTimeMsg
{
	DWORD	m_clientTime;			// �ͻ��˵�ʱ��
	DWORD	m_serverTime;			// �������˵�ʱ��
};

DECLARE_MSG(SCheckTimeMsg, STimeBaseMsg, STimeBaseMsg::EPRO_TIMESYN_CHECKTIME)
struct SQCheckTimeMsg : public SCheckTimeMsg
{
	DWORD	m_clientTime;			// Ϊ�˼�ʱ����У��
	DWORD	m_serverTime;			// �ڿͻ���ͬ�����ֵķ������߼�ʱ��
	BYTE	m_phase;				// У���Ľ׶Σ�
									// 0����ʾ����ͬ����ɺ����Ͻ��е�һ��У��
};

struct SACheckTimeMsg : public SCheckTimeMsg
{
	INT32	m_delay;					// У�����������ʱ�������ɸ���
};

#endif // TIMESYNMSGS_H