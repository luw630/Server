#pragma once
#include <map>
#include "NetAccountCheckModule.h"


class AccountModuleManage
{
public:
	BOOL RegisterALLACC();
	void Execution(void);
	CNetAccountCheckModule * Find(LPCSTR lpSvrName);
	BOOL DispatchAll( LPCVOID data, size_t size );
	BOOL Dispatch( LPCVOID data, size_t size, LPCSTR server );
	std::string GetStatus();
	BOOL Reconnect();
	void Clear();

public:
	AccountModuleManage(void);
	~AccountModuleManage(void);

private:
    typedef std::map< std::string, CNetAccountCheckModule* > ACCSvrMap;
    ACCSvrMap m_ACCSvrList; 
    std::string defaultServer;
};
