//////////////////////////////////////////////////////////////////////
/*
	Created:	2012/03/06
	Filename: 	CenterModule.h
	Author:		TonyJJ
	Commnet:	����������ķ���socket����
*/
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _H_CenterModule_H_
#define _H_CenterModule_H_

#include "networkmodule\netconsumermodule\netconsumermodule.h"

class CenterModule : public CNetConsumerModule
{
public:
	CenterModule(void);
	~CenterModule(void);

public:
	int Execution(void);

	bool Entry( void* data, size_t size );

	BOOL SendToCenterServer( void* pMsg, WORD iSize);

	BOOL  OnCheckWhiteList(struct SACheckWhiteList  *pMsg);
	bool OnLogout();
};



#endif
