#pragma  once

/*
#include "../../xydclient/CtrlModule/XMLite.h"

#define  MAX_BUFF_LEVEL    10			//buff 最大等级
#define  MAX_BUFF_ACTION   10			//buff 最大事件数
#define  MAX_BUFF_TRIGGER  10		//buff 最大触发器数

enum
{
	BP_NUM_LEVEL = 1,
	BP_NUM_TRIGGER,
	BP_NUM_ACTION,

	BP_NUM_MAX,
};
enum
{
	BP_STEP_BUFF = 1,
	BP_STEP_LEVEL,
	BP_STEP_TRIGGER,
	BP_STEP_ACTION,
};
struct __BuffBaseAction
{
	WORD m_wActionType;		   //Buff 事件类型
	WORD m_wActionID;		   //Buff 事件ID
	int  m_wActionValue;	   //Buff 发生值基数	
	float m_fValueQuotiety;	   //Buff 发生值系数	 
};

struct __BuffBaseTrigger
{
	WORD m_wTriggerType;	   //Buff 触发器类型
	WORD m_wInterval;		   //Buff 触发间隔时间
	WORD  m_wActionNum;
	__BuffBaseAction m_stBaseAction[MAX_BUFF_ACTION];
};

struct __BuffBaseLevel
{
	int   m_BuffLevel;			//Buff 等级（>=0：对玩家 <0：对怪物）
	int   m_wDuration;		    //Buff 延迟时间基数
	float m_fDurQuotiety;		//Buff 延迟时间系数
	//char  m_sMutexName[256];	//Buff 互斥类型
	char* m_sRemoveStr;			//Buff 移除列表
	char* m_sRejectStr;			//Buff 拒绝列表	
	WORD  m_wTriggerNum;
	__BuffBaseTrigger m_stBaseTrigger[MAX_BUFF_TRIGGER];
};

struct __BuffBaseData
{
	__BuffBaseData()
	{
		m_wBuffID = 0;
		m_wBuffProperty = 0;
		m_wBuffObject   = 0;
		m_wBuffDelType  = 0;
	}
	WORD m_wBuffID;			   //Buff ID
	char m_sBuffName[256];     //buff 名字
	WORD m_wBuffProperty;      //buff 属性
	WORD m_wBuffObject;		   //buff 添加到的目标（0，被攻击对象，1，攻击者自身）
	WORD m_wBuffDelType;	   //buff 删除类型
							   //	0：	普通状态，时间到，死亡消失，任何情况下线消失
							   // 	1： 死亡不消失
							   // 	2： 下线不消失，但是时间为绝对时间，即下线时间继续算
							   // 	3： 下线不消失，并且保存当前时间，下次上线后继续		
	WORD  m_wLevelNum;			
	__BuffBaseLevel m_stBaseLevel[MAX_BUFF_LEVEL];
};

class BuffBaseData
{
public:
	typedef std::vector< __BuffBaseData* > BUFFBASEDATAVEXTOR;

	BuffBaseData();
	void Init();
	void Destroy();
	void AddBuff(__BuffBaseData* buff);
	static BuffBaseData& getSingle();
public:
	__BuffBaseData* GetBuffDataFromIndex(WORD id);
	char*	GetBuffNameFromIndex(WORD id);
private:
	BUFFBASEDATAVEXTOR m_BuffBaseDataVector;
	__BuffBaseData* m_OneBuff;		//单独的一个buff

	WORD m_wCurStep;				//解析步骤
private:	
	bool ParseBuffXML(LPCSTR szXmlData, DWORD size);
	bool ResolutionElement(XNode &Node);
	void SetAttribute(XNode &Node);
	void SetSubElement(XNode &Node);
	bool SetParameter(LPCSTR szParameterName, LPARAM lParam);
	int  GetParameterInt(LPARAM lParam, WORD wIndex);

	int  GetBuffPerNum(WORD index);
	bool AddBuffPerNum(WORD index,int val);
};
//*/