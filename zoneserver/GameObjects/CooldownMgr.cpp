#include "StdAfx.h"
#include "CooldownMgr.h"

static const DWORD nullTick = -1;

struct CheckOnlyCooler : Cooler {
    CheckOnlyCooler( std::string &type ) {
        type = type;
        segment = offset = 0;
        owner = 0; 
    }
    void OnActive( EventMgr *mgr ) {}
};

BOOL CooldownMgr::Active( std::string &type, DWORD remain, Cooler *cooler ) 
{
	if (!cooler)
	{
		rfalse(4, 1, "CooldownMgr.cpp - Active() - !cooler");
		return FALSE;
	}

    Cooler *dst = cooler;
    COOLDOWNMAP::iterator it = coolerMap.find( type );
    
	if ( it == coolerMap.end() ) 
	{
        if ( cooler == NULL ) 
            dst = new CheckOnlyCooler( type );
    
		coolerMap[type] = dst;
    } 
	else 
	{
        if ( dst == NULL ) 
            dst = it->second;
        else if ( cooler != it->second )        // 不允许重复注册冷却器
            return false;   
        if ( dst->segment != nullTick ) {    // 如果还处于冷却过程中，则操作失败
            int ticks = ( int )( dst->offset - EventMgr::singleton().ServerTime() );
            if ( ticks > 0 )
                return false;
        }
    }

    dst->segment = EventMgr::singleton().ServerTime();
    dst->offset = dst->segment + remain;
    if ( cooler ) 
        EventMgr::singleton().SetEvent( cooler, remain );
    
    return true;
}

DWORD CooldownMgr::Check( std::string &type ) {
    COOLDOWNMAP::iterator it = coolerMap.find( type );
    if ( it != coolerMap.end() && it->second->segment != nullTick ) {
        Cooler *dst = it->second;

		if (!dst)
		{	
			rfalse(4, 1, "CooldownMgr.cpp - Check() - !dst");
			return 0;
		}
     
        int ticks = ( int )( dst->offset - EventMgr::singleton().ServerTime() );
        if ( ticks > 0 )
            return ticks;
    }
    return 0;
}

BOOL CooldownMgr::Deactive( std::string &type ) {
    COOLDOWNMAP::iterator it = coolerMap.find( type );
    if ( it != coolerMap.end() && it->second->segment != nullTick ) {
        it->second->segment = nullTick;
        EventMgr::singleton().ResetEvent( it->second );
        return true;
    }
    return false;
}
