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
        // ���⴦�������ǰĿ¼��û��ACCServerList.ini����ļ�����ʹ��ԭ�еķ��������ã�
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

    // ���֮ǰ�ķ�������ͨ��Ĭ�����ô����ģ�����Ҫ��������
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

            // ���÷�����ǰ׺�����������ʺŷ���������Ĭ�ϵķ���������ǰ׺��ȡ����
            if ( defaultServer != svrName )
                accmodule->accountPrefix = svrName;

            if ( !accmodule->Connect( svrIP.c_str(), svrPort.c_str() ) )
            {
                rfalse(2, 1, "�ʺŷ�����[%s]����ʧ�ܣ�",svrIP.c_str());
                rfalse(1, 1, "�ʺŷ�����[%s]����ʧ�ܣ�",svrIP.c_str());
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

    // ����û��ǰ׺���˺ţ�Ĭ��������õ�һ���˺�����[default]
    if ( CNetAccountCheckModule *accmodule = Find( server ) )
        return accmodule->client.SendMessage( data, static_cast<WORD>(size) );

    return rfalse( 2, 1, "ָ��ǰ׺���˺�[%s]û���ҵ���Ӧ���˺ŷ�������", server ), FALSE;
}

std::string AccountModuleManage::GetStatus()
{
    std::string result;

    for ( ACCSvrMap::iterator it = m_ACCSvrList.begin() ; it != m_ACCSvrList.end(); ++it )
    {
        result += it->first;
        result += it->second->client.IsConnected() ?"|���� ":"|û���� ";
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
