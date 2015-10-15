#pragma  once

#include "NetModule.h"
#include "PlayerTypedef.h"
#include <map>
//DECLARE_MSG_MAP(SSectionMsg, SMessage, SMessage::ERPO_DEVIDE_MESSAGE)
//END_MSG_MAP()
struct SSectionMsg : public SMessage
{
	SSectionMsg()
	{
		SMessage::_protocol = ERPO_SECTION_MESSAGE;
	}
	const static WORD MAX_SIZE = 30000;
	BYTE byIndex;				//消息序号
	BYTE byCount;				//消息总数
	WORD wSize;					//数据大小
	BYTE byData[MAX_SIZE];		//数据
	WORD wId;					//消息ID
	DNID dnidClient;
	QWORD qwSerialNo;
};

class SectionMessageManager
{
public:
	struct RecvData
	{
		BYTE* data;
		BYTE  count;
	};
	enum
	{
		DBSERVER = 1,
		LOGINSERVER = 2,
		REGIONSERVER = 3,
		CLIENT = 4
	};
	typedef std::map<WORD,RecvData> RecvDataMap;
	int evaluateDevidedAmount(DWORD size)
	{
		int sum = size/SSectionMsg::MAX_SIZE;
		if (sum*SSectionMsg::MAX_SIZE == size)
		{
			return sum;
		}
		return sum+1;
	}
	void popMessage(WORD id)
	{
		RecvDataMap::iterator it = m_recvDataMap.find(id);
		if (it != m_recvDataMap.end())
		{
			delete[] it->second.data;
			m_recvDataMap.erase(it);
		}
	}
	void popMessage(SMessage* msg)
	{
		for (RecvDataMap::iterator it = m_recvDataMap.begin();it != m_recvDataMap.end();it++)
		{
			if (it->second.data == ((BYTE*)msg))
			{
				delete[] it->second.data;
				m_recvDataMap.erase(it);
				break;
			}
		}
	}
	SMessage* recvMessageWithSection(SSectionMsg* msg)
	{

		std::pair<RecvDataMap::iterator,bool> ret = m_recvDataMap.insert(RecvDataMap::value_type(msg->wId,RecvData()));
		if (ret.second)
		{
			ret.first->second.data = new BYTE[msg->byCount*SSectionMsg::MAX_SIZE];
			ret.first->second.count = msg->byCount;
		}
		memcpy(ret.first->second.data+msg->byIndex*SSectionMsg::MAX_SIZE,msg->byData,msg->wSize);
		ret.first->second.count--;
		if (!ret.first->second.count)
		{
			return (SMessage*)ret.first->second.data;
		}
		return 0;
	}
	void sendMessageWithSection(DNID dnidClient,SMessage* msg,int size)
	{
		sendMessageWithSection(dnidClient,0,0,msg,size);
	}
	void sendMessageWithSection(SMessage* msg,int size)
	{
		sendMessageWithSection(0,0,0,msg,size);
	}
	void sendMessageWithSection(DNID dnidClient,DNID msgDnid,QWORD msgSerialNo,SMessage* msg,int size)
	{
		int num = evaluateDevidedAmount(size);
		WORD index = 0;
		BYTE* srcMsg = (BYTE*)msg;
		for(int i = 0;i < num;i++)
		{
			SSectionMsg sMsg;			
			index = devideMessage(i,num,&sMsg,msg,size,index);
			if (!index)
			{
				return;
			}
			sMsg.dnidClient = msgDnid;
			sMsg.qwSerialNo = msgSerialNo;
			sendSectionMessage(dnidClient,&sMsg);
		}		
	}
	WORD devideMessage(int index,int total,SSectionMsg* smsg,SMessage* msg,DWORD size,WORD id = 0)
	{
		if(!id)
		{
			(*((BYTE*)&id)) = m_id;
			if (!m_sendCount)
			{
				m_sendCount = 1;
			}
			(*(((BYTE*)&id)+1)) = m_sendCount++;
		}
		BYTE* srcMsg = (BYTE*)msg;		
		DWORD tempSize = size - index*SSectionMsg::MAX_SIZE;
		if (tempSize<=0)
		{
			return 0;
		}
		if (tempSize > SSectionMsg::MAX_SIZE)
		{				
			smsg->wSize = SSectionMsg::MAX_SIZE;
		}
		else
		{
			smsg->wSize = tempSize;
		}
		memcpy(smsg->byData,srcMsg+index*SSectionMsg::MAX_SIZE,smsg->wSize);
		smsg->byCount = total;
		smsg->byIndex = index;
		smsg->wId = id;
		return id;
	}
	static SectionMessageManager& getInstance()
	{
		static SectionMessageManager smm;
		return smm;
	}
	virtual void sendSectionMessage(DNID dnidClient,SSectionMsg* msg);
private:
	SectionMessageManager();
	SectionMessageManager(SectionMessageManager &){}
	SectionMessageManager& operator=(SectionMessageManager &){}
	RecvDataMap m_recvDataMap;
	BYTE m_id;
	BYTE m_sendCount;
};