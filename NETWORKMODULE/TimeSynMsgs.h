#ifndef TIMESYNMSGS_H
#define TIMESYNMSGS_H

#include "NetModule.h"

DECLARE_MSG_MAP(STimeBaseMsg, SMessage, SMessage::EPRO_SYNTIME_MESSAGE)
	EPRO_TIMESYN_SYNTIME,			// 同步服务器的逻辑时间
	EPRO_TIMESYN_CHECKTIME,			// 进行时间校正
END_MSG_MAP_WITH_ROTOC()

// 消息定义
DECLARE_MSG(SSynTimeMsg, STimeBaseMsg, STimeBaseMsg::EPRO_TIMESYN_SYNTIME)
struct SQSynTimeMsg : public SSynTimeMsg
{
	DWORD	m_clientTime;			// 客户端的时间
};

struct SASynTimeMsg : public SSynTimeMsg
{
	DWORD	m_clientTime;			// 客户端的时间
	DWORD	m_serverTime;			// 服务器端的时间
};

DECLARE_MSG(SCheckTimeMsg, STimeBaseMsg, STimeBaseMsg::EPRO_TIMESYN_CHECKTIME)
struct SQCheckTimeMsg : public SCheckTimeMsg
{
	DWORD	m_clientTime;			// 为了及时重新校正
	DWORD	m_serverTime;			// 在客户端同步保持的服务器逻辑时间
	BYTE	m_phase;				// 校正的阶段：
									// 0：表示是在同步完成后马上进行的一次校正
};

struct SACheckTimeMsg : public SCheckTimeMsg
{
	INT32	m_delay;					// 校正后的网络延时（可正可负）
};

#endif // TIMESYNMSGS_H