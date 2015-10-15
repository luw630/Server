#pragma  once

/*
#include "../../xydclient/CtrlModule/XMLite.h"

#define  MAX_BUFF_LEVEL    10			//buff ���ȼ�
#define  MAX_BUFF_ACTION   10			//buff ����¼���
#define  MAX_BUFF_TRIGGER  10		//buff ��󴥷�����

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
	WORD m_wActionType;		   //Buff �¼�����
	WORD m_wActionID;		   //Buff �¼�ID
	int  m_wActionValue;	   //Buff ����ֵ����	
	float m_fValueQuotiety;	   //Buff ����ֵϵ��	 
};

struct __BuffBaseTrigger
{
	WORD m_wTriggerType;	   //Buff ����������
	WORD m_wInterval;		   //Buff �������ʱ��
	WORD  m_wActionNum;
	__BuffBaseAction m_stBaseAction[MAX_BUFF_ACTION];
};

struct __BuffBaseLevel
{
	int   m_BuffLevel;			//Buff �ȼ���>=0������� <0���Թ��
	int   m_wDuration;		    //Buff �ӳ�ʱ�����
	float m_fDurQuotiety;		//Buff �ӳ�ʱ��ϵ��
	//char  m_sMutexName[256];	//Buff ��������
	char* m_sRemoveStr;			//Buff �Ƴ��б�
	char* m_sRejectStr;			//Buff �ܾ��б�	
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
	char m_sBuffName[256];     //buff ����
	WORD m_wBuffProperty;      //buff ����
	WORD m_wBuffObject;		   //buff ��ӵ���Ŀ�꣨0������������1������������
	WORD m_wBuffDelType;	   //buff ɾ������
							   //	0��	��ͨ״̬��ʱ�䵽��������ʧ���κ����������ʧ
							   // 	1�� ��������ʧ
							   // 	2�� ���߲���ʧ������ʱ��Ϊ����ʱ�䣬������ʱ�������
							   // 	3�� ���߲���ʧ�����ұ��浱ǰʱ�䣬�´����ߺ����		
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
	__BuffBaseData* m_OneBuff;		//������һ��buff

	WORD m_wCurStep;				//��������
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