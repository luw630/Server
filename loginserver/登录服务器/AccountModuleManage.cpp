#include "StdAfx.h"
#include ".\accountmodulemanage.h"
#include <assert.h>
#include "pub/stlini.h"

AccountModuleManage::AccountModuleManage(void)
{
}

AccountModuleManage::~AccountModuleManage(void)
{
    Clear();
}

void AccountModuleManage::Clear()
{
    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin() ; it != m_ACCSvrList.end(); ++it )
    {
        it->second->Destroy();
        delete it->second;
    }

    m_ACCSvrList.clear();
}

BOOL AccountModuleManage::RegisterALLACC()
{
    IniFiles inifile;
    if ( !inifile.open( "ACCServerList.ini" ) )
    {
        // 特殊处理，如果当前目录下没有ACCServerList.ini这个文件，则使用原有的服务器配置！
        if ( !defaultServer.empty() )
            return false;

        defaultServer = "default";
        if ( !Find( defaultServer.c_str() ) ) 
        {
            LPCSTR ip = NULL, port = NULL;
            Globals::GetAccountIPPORT( ip, port );
            CNetAccountCheckModule *accmodule = new CNetAccountCheckModule();
            if ( accmodule->Connect( ip, port ) )
            {
                Clear();
                m_ACCSvrList[ defaultServer ] = accmodule;
                return true;
            }

            return false;
        }
    }

    // 如果之前的服务器是通过默认设置创建的，则需要重新设置
    if ( Find( "default" ) ) 
        Clear();

    int nSvrCount= 0;
    nSvrCount = inifile.read_int( "ServerInfo", "Count", 0 );
    defaultServer = inifile.read( "ServerInfo", "default", "" );
    char tmpStr[32] = {0};

    BOOL result = TRUE;
    for ( int i= 0; i< nSvrCount; ++i)
    {
        _snprintf( tmpStr, sizeof (tmpStr), "Server%d", i);
        std::string svrName = inifile.read( tmpStr, "Name", "");
        std::string svrIP = inifile.read( tmpStr, "IP", "");
        std::string svrPort = inifile.read( tmpStr, "Port","");

        assert( svrName.c_str());
        assert( svrIP.c_str());
        assert( svrPort.c_str());

        if ( !Find( svrName.c_str() ) ) 
        {
            CNetAccountCheckModule *accmodule = new CNetAccountCheckModule();

            // 设置服务器前缀名，如果这个帐号服务器就是默认的服务器，则将前缀名取消！
            if ( defaultServer != svrName )
                accmodule->accountPrefix = svrName;

            if ( !accmodule->Connect( svrIP.c_str(), svrPort.c_str() ) )
            {
                rfalse(2, 1, "帐号服务器[%s]连接失败！",svrIP.c_str());
                rfalse(1, 1, "帐号服务器[%s]连接失败！",svrIP.c_str());
                result = FALSE;
            }
            else
            {
                m_ACCSvrList[ svrName ] = accmodule;
            }
        }
    }

    return result;
}

void AccountModuleManage::Execution()
{
    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin() ; it != m_ACCSvrList.end(); ++it )
        it->second->Execution();
}

CNetAccountCheckModule* AccountModuleManage::Find( LPCSTR lpSvrName )
{
    ACCSvrMap::iterator it = m_ACCSvrList.find( lpSvrName );
    if ( it != m_ACCSvrList.end() )
        return it->second; 

    return NULL;
}

BOOL AccountModuleManage::DispatchAll( LPCVOID data, size_t size )
{
    BOOL ret = TRUE;
    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin(); it != m_ACCSvrList.end(); ++it )
        ret &= it->second->client.SendMessage( data, static_cast< WORD >( size ) );

    return ret;
}

BOOL AccountModuleManage::Dispatch( LPCVOID data, size_t size, LPCSTR server )
{
    if ( server == NULL )
        server = defaultServer.c_str();

    // 正常没有前缀的账号，默认情况下用第一个账号链接[default]
    if ( CNetAccountCheckModule *accmodule = Find( server ) )
        return accmodule->client.SendMessage( data, static_cast<WORD>(size) );

    return rfalse( 2, 1, "指定前缀的账号[%s]没有找到相应的账号服务器！", server ), FALSE;
}

std::string AccountModuleManage::GetStatus()
{
    std::string result;

    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin() ; it != m_ACCSvrList.end(); ++it )
    {
        result += it->first;
        result += it->second->client.IsConnected() ?"|正常 ":"|没联上 ";
    }

    return result;
}

BOOL AccountModuleManage::Reconnect()
{
    BOOL ret = TRUE;
    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin(); it != m_ACCSvrList.end(); ++it )
        ret &= it->second->Reconnect();

    return ret;
}
