// EthernetMac.h: interface for the CEthernetMac class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ETHERNETMAC_H__5089E27B_AA24_4467_9736_A926216DF761__INCLUDED_)
#define AFX_ETHERNETMAC_H__5089E27B_AA24_4467_9736_A926216DF761__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IpTypes.h>

class CEthernetMac  
{
public:
	CEthernetMac();
	virtual ~CEthernetMac();

	BOOL GetFirstMac(BYTE macid[/*6*/]);
	BOOL GetNextMac(BYTE macid[/*6*/]);

private:
	IP_ADAPTER_INFO m_AdapterInfo[16];//Allocate information
	PIP_ADAPTER_INFO m_pAdapterInfo;//Contains pointer to
};

#endif // !defined(AFX_ETHERNETMAC_H__5089E27B_AA24_4467_9736_A926216DF761__INCLUDED_)
