// EthernetMac.cpp: implementation of the CEthernetMac class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "EthernetMac.h"

//typedef struct _MIBICMPSTATS_EX
//{
//    DWORD       dwMsgs;
//    DWORD       dwErrors;
//    DWORD       rgdwTypeCount[256];
//} MIBICMPSTATS_EX, *PMIBICMPSTATS_EX;
//
//typedef struct _MIB_ICMP_EX
//{
//    MIBICMPSTATS_EX icmpInStats;
//    MIBICMPSTATS_EX icmpOutStats;
//} MIB_ICMP_EX,*PMIB_ICMP_EX;

#include <Iphlpapi.h>

#pragma comment(lib,"Iphlpapi.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEthernetMac::CEthernetMac()
{

}

CEthernetMac::~CEthernetMac()
{

}

BOOL CEthernetMac::GetFirstMac(BYTE macid[/*6*/])
{
	DWORD dwBufLen=sizeof(m_AdapterInfo);//Save memory size of buffer

	DWORD dwStatus=GetAdaptersInfo(m_AdapterInfo,&dwBufLen);
	if(dwStatus!=ERROR_SUCCESS)
		return FALSE;

	m_pAdapterInfo=m_AdapterInfo;

	return GetNextMac(macid);
}

BOOL CEthernetMac::GetNextMac(BYTE macid[/*6*/])
{
	if(NULL!=m_pAdapterInfo)
	{
		//把网卡MAC地址格式化成常用的16进制形式,如0010-A4E4-5802
		/*sprintf(szMac,"%02X-%02X-%02X-%02X-%02X-%02X",
			m_pAdapterInfo->Address[0],
			m_pAdapterInfo->Address[1],
			m_pAdapterInfo->Address[2],
			m_pAdapterInfo->Address[3],
			m_pAdapterInfo->Address[4],
			m_pAdapterInfo->Address[5]);*/
		memcpy(macid, m_pAdapterInfo->Address, 6);

		m_pAdapterInfo=m_pAdapterInfo->Next;//Progress through linked list

		return TRUE;
	}
	else
		return FALSE;
}
