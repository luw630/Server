#include "Stdafx.h"
#include "BuffBaseData.h"
//#include "../../xydclient/CtrlModule/XMLite.cpp"

/*
#define ISENTITYROOT(a) if(!a)return false;

BuffBaseData& BuffBaseData::getSingle()
{
	static BuffBaseData m_BuffBaseData;
	return m_BuffBaseData;
}
BuffBaseData::BuffBaseData()
{
	m_OneBuff = 0;
	m_wCurStep = 0;
}

void BuffBaseData::Init()
{
	//处理载XML表的事情
	PakFile pf;
	if (!pf.Open("Data/buff数据配置表.xml"))
	{
		rfalse(0, 0, "读buff数据文件出错!");
		return;
	}

	DWORD size = pf.Size();
	char *buffer = new char [size];
	pf.Read(buffer, size);

	if(!ParseBuffXML(buffer, size))
		rfalse(0, 0, "初始化buff数据文件出错!");

	delete[] buffer;
}

void BuffBaseData::AddBuff(__BuffBaseData* buff)
{
	
 	if (buff)
 		m_BuffBaseDataVector.push_back( buff );
// 
// 	delete []m_OneBuff;
	m_OneBuff = 0;
}

void BuffBaseData::Destroy()
{
	for (WORD i = 0;i< m_BuffBaseDataVector.size();++i)
	{
		__BuffBaseData* buff = m_BuffBaseDataVector[i];
		for (WORD j = 0;j < buff->m_wLevelNum;++j)
		{
			if(buff->m_stBaseLevel[j].m_sRejectStr)
				delete buff->m_stBaseLevel[j].m_sRejectStr;

			if(buff->m_stBaseLevel[j].m_sRemoveStr)
				delete buff->m_stBaseLevel[j].m_sRemoveStr;
		}
		delete buff;
	}
	m_BuffBaseDataVector.clear();
}

bool BuffBaseData::ParseBuffXML( LPCSTR szXmlData, DWORD size )
{
	if (szXmlData == NULL)
		return false;

	DWORD s = size;
	LPCSTR b = szXmlData;

	XNode xml;
	PARSEINFO pi;

	pi.escape_value = 0;

	LPCSTR ret = xml.Load(b, &pi);

	if (ret == NULL)
	{
		if( pi.erorr_occur ) // is error_occur?
		{
			MessageBoxW( 0, _bstr_t( pi.error_string ), 0, 0 );
			MessageBoxW( 0, _bstr_t( xml.GetXML() ), 0, 0 );
		}

		return false;
	}

	return ResolutionElement(xml);;
}

bool BuffBaseData::ResolutionElement( XNode &Node )
{
	// 首先处理该元素的类型（MainFrame、Include、Window、Button、ListBox……）

	CString NordName = Node.name;


	// 根据元素属性设置节点
	SetAttribute(Node);

	// 处理子节点
	SetSubElement(Node);

	return true;
}

void BuffBaseData::SetAttribute( XNode &Node )
{
	int i = 0;

	while (LPXAttr pAttr = Node.GetAttr(i++))
	{
		//没详细属性
		if (pAttr == NULL)
		{
			break;
		}
		// 设定指定属性

		SetParameter(pAttr->name, (LPARAM)(LPCSTR)pAttr->value);
	}
}

void BuffBaseData::SetSubElement( XNode &Node )
{
	LPXNode pd = NULL;
	int i = 0;

	while (true)
	{
		LPXNode pNode = Node.GetChild(i++);

		pd = pNode;

		//没子节点了
		if (pNode == NULL)
			break;
		
		if (!ResolutionElement(*pNode)){
			continue;
		}
	}

	if (strcmp(Node.name, "Buff") == 0)
		m_wCurStep = BP_STEP_BUFF;
	else if(strcmp(Node.name,"Level") == 0)
		m_wCurStep = BP_STEP_LEVEL;
	else if(strcmp(Node.name,"Trigger") == 0)
		m_wCurStep = BP_STEP_TRIGGER;
	else if(strcmp(Node.name,"Action") == 0)
		m_wCurStep = BP_STEP_ACTION;

	if (pd == NULL)
	{
		switch ( m_wCurStep )
		{
		case BP_STEP_BUFF:
			AddBuff(m_OneBuff);	// 把buff加入链表
			break;
		case BP_STEP_LEVEL:
			AddBuffPerNum(BP_NUM_LEVEL,1);
			break;
		case BP_STEP_TRIGGER:
			AddBuffPerNum(BP_NUM_TRIGGER,1);
			break;
		case BP_STEP_ACTION:
			AddBuffPerNum(BP_NUM_ACTION,1);
			break;
		default:
			break;
		}
	}
}
bool BuffBaseData::SetParameter( LPCSTR szParameterName, LPARAM lParam )
{
	WORD nL = 0,nT = 0,nA = 0;

	if(strcmp(szParameterName, "ID") != 0)
	{
		ISENTITYROOT(m_OneBuff);

		nL = GetBuffPerNum(BP_NUM_LEVEL);
		nT = GetBuffPerNum(BP_NUM_TRIGGER);
		nA = GetBuffPerNum(BP_NUM_ACTION);

		if(nL == -1 || nT == -1 || nA == -1)
			return false;
	}

	if (strcmp(szParameterName, "ID") == 0)
	{
		m_OneBuff = new(__BuffBaseData);//BUFF
		memset(m_OneBuff,0,sizeof(__BuffBaseData));
		m_OneBuff->m_wBuffID = GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "Name") == 0)
	{
		dwt::strcpy(m_OneBuff->m_sBuffName, (LPSTR)lParam, 256);
	}
	else if (strcmp(szParameterName, "Property") == 0)
	{
		m_OneBuff->m_wBuffProperty = GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "Object") == 0)
	{
		m_OneBuff->m_wBuffObject = GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "DelType") == 0)
	{
		m_OneBuff->m_wBuffDelType = GetParameterInt(lParam, 1);
	}
	else if(strcmp(szParameterName, "Lv") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_sRejectStr = new char();
		m_OneBuff->m_stBaseLevel[nL].m_sRemoveStr = new char();
		m_OneBuff->m_stBaseLevel[nL].m_BuffLevel = GetParameterInt(lParam, 1);
	}
	else if(strcmp(szParameterName, "Duration") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_wDuration = GetParameterInt(lParam, 1);	
	}
	else if (strcmp(szParameterName, "DurQuotiety") == 0)
	{
		char tmp[256] = "";
		dwt::strcpy(tmp, (LPSTR)lParam, 256);
		m_OneBuff->m_stBaseLevel[nL].m_fDurQuotiety = (float)atof(tmp);
	}
	else if (strcmp(szParameterName, "RemoveList") == 0)
	{
		if (m_OneBuff->m_stBaseLevel[nL].m_sRemoveStr)
			delete m_OneBuff->m_stBaseLevel[nL].m_sRemoveStr;
		WORD len = (WORD)strlen((LPSTR)lParam) + 1;
		m_OneBuff->m_stBaseLevel[nL].m_sRemoveStr = new char[len];
		dwt::strcpy(m_OneBuff->m_stBaseLevel[nL].m_sRemoveStr, (LPSTR)lParam, len);
	}
	else if (strcmp(szParameterName, "RejectList") == 0)
	{
		if (m_OneBuff->m_stBaseLevel[nL].m_sRejectStr)
			delete m_OneBuff->m_stBaseLevel[nL].m_sRejectStr;
		WORD len = (WORD)strlen((LPSTR)lParam) + 1;
		m_OneBuff->m_stBaseLevel[nL].m_sRejectStr = new char[len];
		dwt::strcpy(m_OneBuff->m_stBaseLevel[nL].m_sRejectStr, (LPSTR)lParam, len);
	}
	else if (strcmp(szParameterName, "TriggerType") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_wTriggerType
			= GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "Interval") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_wInterval
			= GetParameterInt(lParam, 1);	
	}
	else if (strcmp(szParameterName, "ActionType") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_stBaseAction[nA].m_wActionType
			= GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "AbilityID") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_stBaseAction[nA].m_wActionID
			= GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "Amount") == 0)
	{
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_stBaseAction[nA].m_wActionValue
			= GetParameterInt(lParam, 1);
	}
	else if (strcmp(szParameterName, "ValueQuotiety") == 0)
	{
		char tmp[256] = "";
		dwt::strcpy(tmp, (LPSTR)lParam, 256);
		m_OneBuff->m_stBaseLevel[nL].m_stBaseTrigger[nT].m_stBaseAction[nA].m_fValueQuotiety 
			= (float)atof(tmp);
	}
	else 
		return false;
	return true;
}
int BuffBaseData::GetParameterInt(LPARAM lParam, WORD wIndex)// wIndex从1开始
{
	char sParam[256] = {0};
	WORD j = 1;
	short nCut[20] = {-1};
	char* Param = (char*)lParam;
	int nLen = dwt::strlen(Param, 256);
	for(int i = 0; i < nLen; i ++)
		if(Param[i] == ',')
			nCut[j++] = i;

	nCut[j] = nLen;

	if(wIndex > j) return 0;

	memcpy(sParam, Param + nCut[wIndex - 1] + 1, nCut[wIndex] - nCut[wIndex - 1]);
	sParam[ nCut[wIndex] - nCut[wIndex - 1] - 1 ] = 0;

	return atoi(sParam);
}

int BuffBaseData::GetBuffPerNum( WORD index )
{
	ISENTITYROOT(m_OneBuff);
	WORD indexL = m_OneBuff->m_wLevelNum;
	WORD indexT = m_OneBuff->m_stBaseLevel[indexL].m_wTriggerNum;
	switch (index)
	{
	case BP_NUM_LEVEL:
		return m_OneBuff->m_wLevelNum;
	case BP_NUM_TRIGGER:
		return m_OneBuff->m_stBaseLevel[indexL].m_wTriggerNum;
	case BP_NUM_ACTION:
		return m_OneBuff->m_stBaseLevel[indexL].m_stBaseTrigger[indexT].m_wActionNum;
	default:
		break;
	}
	return -1;
}

bool BuffBaseData::AddBuffPerNum( WORD index ,int val)
{
	ISENTITYROOT(m_OneBuff);
	WORD indexL= m_OneBuff->m_wLevelNum;
	WORD indexT = m_OneBuff->m_stBaseLevel[indexL].m_wTriggerNum;
	switch (index)
	{
	case BP_NUM_LEVEL:
		m_OneBuff->m_wLevelNum += val;
		return true;
	case BP_NUM_TRIGGER:
		m_OneBuff->m_stBaseLevel[indexL].m_wTriggerNum += val;
		return true;
	case BP_NUM_ACTION:
		m_OneBuff->m_stBaseLevel[indexL].m_stBaseTrigger[indexT].m_wActionNum += val;
		return true;
	default:
		return false;
	}
}

__BuffBaseData* BuffBaseData::GetBuffDataFromIndex( WORD id )
{
	__BuffBaseData* buff;
	for (WORD i = 0;i < m_BuffBaseDataVector.size();++i)
	{
		buff = m_BuffBaseDataVector[i];
		if(buff->m_wBuffID == id)
		{
			return buff;
		}
	}
	return 0;
}

char* BuffBaseData::GetBuffNameFromIndex( WORD id )
{
	__BuffBaseData* buff;
	for (WORD i = 0;i < m_BuffBaseDataVector.size();++i)
	{
		buff = m_BuffBaseDataVector[i];
		if(buff->m_wBuffID == id)
		{
			return buff->m_sBuffName;
		}
	}
	return NULL;
}
//*/