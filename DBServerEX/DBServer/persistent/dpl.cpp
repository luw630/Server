#include "stdafx.h"
#include "dpl.h"
#include "LITESERIALIZER/Lite.h"
#include "networkmodule/logtypedef.h"
#include "networkmodule/playertypedef.h"
#include "networkmodule/datamsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "time.h"
#include "../���������/GameObjects/CPlayerDefine.h"
#include "../���������/GameObjects/CPlayerService.h"

extern BOOL TryDecoding_NilBuffer( const void * src_data, size_t src_size, void *&dst_data, size_t &dst_size );

const static char *CONST_TITLES[] = { "���ֵ���", "���ҵ���", "�䵱����", "ʥ�����","���޵���" };

struct PlayerInitProperty
{
	int HP;
	int MP;
	int SP;
	int HPBottleIdx;		// ��Ѫҩ������
	int MPBottleIdx;		// ����ҩ������
};

static PlayerInitProperty _s_PlayerInitProperty;

BOOL LoadPlayerInitProperty()
{
	IniFiles IniFile;
    if (!IniFile.open("player_init_property.ini"))
        return rfalse(0, 0, "�޷��������ļ�player_init_property.ini");

    _s_PlayerInitProperty.HP = IniFile.read_int("BASE_PROPERTY", "HP", 1);
    _s_PlayerInitProperty.MP = IniFile.read_int("BASE_PROPERTY", "MP", 1);
    _s_PlayerInitProperty.SP = IniFile.read_int("BASE_PROPERTY", "SP", 1);
	_s_PlayerInitProperty.HPBottleIdx = IniFile.read_int("DEFAULT_SHORTCUT", "HP_BOTTLE", 1);
	_s_PlayerInitProperty.MPBottleIdx = IniFile.read_int("DEFAULT_SHORTCUT", "MP_BOTTLE", 11);

    return TRUE;
}


void LogMsg( LPCSTR filename, LPCSTR szLogMessage, int ex = -1 )
{
    try
    {
        SYSTEMTIME s;
        char szLogTime[128];

        GetLocalTime(&s);
        sprintf(szLogTime, "[%d-%d-%d %d:%d:%d][%d]", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond, ex );

        HANDLE hFile = ::CreateFile( filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, NULL, NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
            return;

        DWORD sizeA = (DWORD)strlen( szLogTime );
        DWORD sizeB = (DWORD)strlen( szLogMessage );

        SetFilePointer( hFile, 0, 0, FILE_END );
        WriteFile( hFile, szLogTime, sizeA, &sizeA, 0 );
        WriteFile( hFile, szLogMessage, sizeB, &sizeB, 0 );
        WriteFile( hFile, "\r\n", 2, &sizeA, 0 );

        CloseHandle( hFile);
    }
    catch ( ... )
    {
    }
}
#define LogXcptMsg( str ) LogMsg( "debugXcpt.txt", str )
SFixData staticData4Test;
extern BOOL SendMessage( DNID , LPCVOID , size_t );
BOOL DataPersistentLayer::InitDpl()
{
    totalSaveTicks = 0;
    totalSaveTimes = 0;
    totalSaveFailTimes = 0;
    maxSaveDuration = 0;
    minSaveDuration = 0x7fffffffffffffff;
    conf1 = conf2 = 0;

    quitEvent = NULL;
    threadHandle.param = this;
    threadHandle.threadHandle = reinterpret_cast< HANDLE >( _beginthreadex( NULL, 0, 
        ( ThreadHandle::FuncType )PersistentThread, &threadHandle, 0, &threadHandle.threadID ) );

    return true;
}

void DataPersistentLayer::Release( HANDLE quitEvent )
{
    threadHandle.Join();
}

BOOL DataPersistentLayer::SaveToDB( CacheUnit &unit )
{
    return false;
}

int DataPersistentLayer::PersistentThread( ThreadHandle *handle )
{
    // ��֤���߼���ǰ�᣺
    // updateQueue�еĶ���ɾ��ֻ�ᷢ���� PersistentThread �У����������������
    // updateQueue���Դ��������¼�����map�еĶ���ض���Ψһ���ڵ�
    // �����¼����Գ���������������ÿ������ض�����һ�������ϵĸ����¼�
    
    // ����У���ǣ���ʵҲ����ʱ���
    // ���ύ����ʱ��CacheMap�е�ʱ���������Ϊ�ύʱ��
    // �ڸ����߳��У������¼���ʱ���Ϊ���һ�θ���ʱ��
    // ��ǿ�Ƹ���״̬�£���CacheMap���޸ĵ�ͬʱ������⸽��һ�������¼���������ǰ�棬
    // �����߳�ֱ��ȡ�����µ��¼����жϣ�����ʱ�����ύʱ��֮��
    DataPersistentLayer *_this = ( DataPersistentLayer* )handle->param;

	// ��������趨������Ҫ��֤����������ᱻ���룡����
	static SFixData	basedata;
	static SWareHouses			warehouse;

    while ( ( handle->breakFlag == 0 && _this->quitEvent == NULL ) || _this->updateQueue.size() )
    {
        // ѭ���¼�ֻ�����������������
        if ( _this->quitEvent == 0 )
            _this->LoopService( handle );

///////////////////////////////////////////////////////////////////////////////////////////
        BOOL haveData = false;
        UpdateEvent ue = { 0, 0 };
		CacheUnit cu;

        // critcal scope
        {
            UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );
    __retry:
            // �Ӹ��¶�����ȡ�����ݶ���
            if ( !_this->updateQueue.empty() ) 
            {
                // �Ȼ�ȡ�����¼�������������
                ue = _this->updateQueue.front();
                _this->updateQueue.pop();

                // �Ƿ��Ǳ�����Ϣ��
                if ( _this->atBackup && ue.uniqueId == 0 && ue.serverId == 0 )
                {
                    // �������ݣ�����
                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().Backup( _this->path.c_str(), _this->title.c_str() );
                    *_this->atBackup = 0;
                    _this->atBackup = NULL;
                    goto __retry;
                }

                // ͨ��������uid��cacheMap�в��һ��浥Ԫ
                // �����п����Ҳ���Ŀ�꣬˵��Ŀ���Ѿ�����ǰ����
                // ���������������������沢���ٶ��������£�
                // �������¼���������ȡ��Ч����

                CacheMap::iterator it = _this->cacheMap.find( ue.uniqueId );
                if ( it == _this->cacheMap.end() ) 
                    goto __retry;

                // ���������Ч�����ȡ��ͬʱ���������ⱻ�����߳���ռ������ǰ����
                cu = it->second;

				// �ж����£�
				// ����ý�ɫ������Ҫ�����������ߣ���������رգ��򱸷�״̬�������Һ���һ�α���ʱ��Ƚ����̣ܶ�С��30���ӣ����򲻿��ǽ�һ�����棡
				int saveMargin = abs( ( int )( timeGetTime() - it->second.timeStamp ) );
                if ( !cu.removeMe && _this->quitEvent == NULL && _this->atBackup == 0 )
                    if ( !cu.modified || ( cu.modified && ( saveMargin < ( 1000 * 60 * 30 ) ) ) )
                        goto __retry;

				// �����ǰ�¼�������������ʱ���ӵ���ʱ�¼��������κδ���������ӻظ��¶����ײ�
                // ������ύ����ʱ�����Ѿ�������������״̬����˵���ڽ����Ѿ������˸��Ӹ����¼�
                // ������ʱ���ٶ��⸽���¼����������޸����ݼ��ɣ��ظ����ӻᵼ�´���2��������¼�������һ�����޷������٣�
                //if ( cu.immediate ) 
                //    it->second.immediate = false;
                //else
                //{
                //    ue.timeStamp = timeGetTime();
                //    _this->updateQueue.push( ue );
                //}

                // ����Ǹ��º����٣��������ﴦ����Ϊ�ᵼ�¹����������������ڸ�����Ϻ�����ٶ���
                // ����ύ����ʱ��������ڲ����Ѿ��Ǵ�������״̬����˵���ö���δ������ɾ��������ֻ��ȡ��removeMe����
				
				if ( cu.data && cu.modified )
				{
					it->second.timeStamp = timeGetTime();

					/*
					memcpy( &( SFixBaseData&    )basedata, ( SFixBaseData*    )cu.data, sizeof SFixBaseData    ); 
					memcpy( &( SFixPackage&     )basedata, ( SFixPackage*     )cu.data, sizeof SFixPackage     );
					memcpy( &( SPlayerTasks&    )basedata, ( SPlayerTasks*    )cu.data, sizeof SPlayerTasks    );
					memcpy( &( SPlayerGM&       )basedata, ( SPlayerGM*       )cu.data, sizeof SPlayerGM       );
					memcpy( &( SPlayerRelation& )basedata, ( SPlayerRelation* )cu.data, sizeof SPlayerRelation );
                    memcpy( &( SPlayerVenapointEx& )basedata, ( SPlayerVenapointEx* )cu.data, sizeof SPlayerVenapointEx );
                    memcpy( &( SPlayerDialUnit& )basedata, ( SPlayerDialUnit* )cu.data, sizeof SPlayerDialUnit );
					basedata.m_dwStaticID = cu.data->m_dwStaticID;

					memcpy( &warehouse.m_WareHouse[0], ( SFixStorage1* )cu.data, sizeof SFixStorage1 );
					memcpy( &warehouse.m_WareHouse[1], ( SFixStorage2* )cu.data, sizeof SFixStorage1 );
					memcpy( &warehouse.m_WareHouse[2], ( SFixStorage3* )cu.data, sizeof SFixStorage1 );
					*/

					basedata = *cu.data;

                    // �Ѿ���ȡ�������ڱ���,�޸�״̬
                    it->second.modified = false;
				}

                haveData = true;
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////

        // ���û��������Ҫ���£������ߺ��ٴγ���
        if ( !haveData ) { Sleep( 10 ); continue; }

        BOOL result = true;

        LARGE_INTEGER lib, lie;
        lib.QuadPart = 0;
        lie.QuadPart = 0;

        // ��������
        if ( !reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().KeepAlive( 0, "�־ò�" ) )
            result = false; // �����������ʧ�ܣ���ֻ�ܷ�������
        else
        {
        // �����ݵ�����£���Ҫ���±���
        if ( cu.data != NULL && cu.modified )
        {
            ::QueryPerformanceCounter( &lib );

		    result = reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().
			    SavePlayerData( cu.account.c_str(), ue.serverId, basedata, &warehouse );
            if ( !result )
            {
                // �������ʧ�ܣ���˵��ĳЩ�ط��������ˣ���ô��Ӧ�ü������ڻ���������ͷ�
            }
            else
            {
                ::QueryPerformanceCounter( &lie );
            }
        }

        // ����д����ֻ���е�ǰ��ôһ���̣߳����Բ��ü�����
        if ( lib.QuadPart != 0 )
        {
            _this->totalSaveTimes ++;
            if ( lie.QuadPart == 0 )
                _this->totalSaveFailTimes ++;
            else
            {
                __int64 duration = lie.QuadPart - lib.QuadPart;
                _this->totalSaveTicks += duration;
                if ( _this->maxSaveDuration < duration )
                    _this->maxSaveDuration = duration;
                if ( _this->minSaveDuration > duration )
                    _this->minSaveDuration = duration;
                }
            }
        }

///////////////////////////////////////////////////////////////////////////////////////////
        // ����ö�����Ҫ�����٣�����Ҫ�ٴδ���cacheMap
        if ( result && cu.removeMe )
        {
            UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

            CacheMap::iterator it = _this->cacheMap.find( ue.uniqueId );
            // ��Ҫ�ٴ�ȷ������δ���޸�,����ȷʵ��Ҫ����
            if ( it != _this->cacheMap.end() && it->second.removeMe && !it->second.modified ) 
			{
		        // ������˳���Ϸʱ�����걣��֮��Ҫ����ҽ������
                if ( it->second.data != NULL )
                {
		            if ( it->second.data->m_byStoreFlag != SFixData::ST_LOGOUT )
                        continue;

                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().
                        UpdateAccountLock( cu.account.c_str(), ue.serverId, FALSE );

                    // ������Ҫ�޸ĵ��ߵİ�״̬�� isCahced = false ��
                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->iuChecker.
                        AtRemovePlayerFromCache( *it->second.data );

				    delete it->second.data;
                }
                else
                {
                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().
                        UpdateAccountLock( cu.account.c_str(), ue.serverId, FALSE );
                }

                AccountMap::iterator it2 = _this->accountMap.find( cu.account );
                assert( it2 != _this->accountMap.end() );

                _this->cacheMap.erase( it );
		        _this->accountMap.erase( it2 );
            }
        }
        else if ( !result )
        {
            UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

            CacheMap::iterator it = _this->cacheMap.find( ue.uniqueId );
            if ( it != _this->cacheMap.end() ) 
			{
                // ����ʧ�ܣ���Ҫ���޸ı�ǸĻ���
                it->second.modified = true;
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////
    }

    {
        UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

        for ( DataPersistentLayer::CacheMap::iterator it = _this->cacheMap.begin(); it != _this->cacheMap.end(); it ++ )
        {
            assert( !it->second.modified );
            delete it->second.data;
        }

        assert( _this->accountMap.size() == _this->cacheMap.size() );
    }

    Sleep( 3000 );
    if ( _this->quitEvent != NULL )
        SetEvent( _this->quitEvent );

    return 0;
}

int DataPersistentLayer::LoopService( ThreadHandle *tHandle )
{
    return 0;
}

DWORD DataPersistentLayer::CheckAccountExist( std::string &account )
{
    UGE::CriticalLock::Barrier4ReadWrite barrier( lock );
    AccountMap::iterator it2 = accountMap.find( account );
    if ( it2 == accountMap.end() )
        return 0;

    return it2->second;
}

struct Query4QueryPlayerList : public DataPersistentLayer4XYD1::QueryBase
{
    Query4QueryPlayerList(DataPersistentLayer4XYD1 *a, DNID b, DWORD c, LPCSTR d, DNID Retdnid, _g::_p::Callback4QueryPlayerList e)
        : _this(a), dnidClient(b), serverId(c), account(d),RetDNID(Retdnid), callback(e) 
    { 
    }

    const std::string &GetKey() const { return account; }

    void Release() { delete this; }

    void Execute()
	{
		if (RetDNID == 0)
		{
            // ����ط�����һ���߼�©��
            // ������������Ϸ���У�����ͨ���쳣�ֶη�����һ������󶨵���Ϣ����
            // ��ҵ����ݾͱ�����ˣ�Ȼ���������»�ȡ��ɫ���ݵ�����������ô����ɵ����ݾͻḲ����Ϸ���µ�����

            // ���ڲ������˻��棬ֱ�Ӽ�������
	        UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

            DataPersistentLayer::AccountMap::iterator it1 = _this->accountMap.find( account );
            if ( it1 != _this->accountMap.end() )
            {
                DataPersistentLayer::CacheMap::iterator it2 = _this->cacheMap.find( it1->second );
                // ������һ�����⴦��Ҳ������Ϊ�������˽�ɫ���ݵľ�һ���Ǵ�����Ϸ�����е����ݣ����Բ������׽����
                // 2008-6-16 ���serverId���жϣ����ⲻ��Դ����������������ע�������½�ɫ�����쳣�Ĵ���
                if (it2 != _this->cacheMap.end() && it2->second.serverId == serverId) 
                {
	                it2->second.removeMe = true;
                    if ( it2->second.data )
                        it2->second.data->m_byStoreFlag = SFixData::ST_LOGOUT;

                    DataPersistentLayer::UpdateEvent ue = { it1->second, serverId };
	                _this->updateQueue.push( ue );
                }
            }
			return;
		}

		SAGetCharacterListMsg msg;
		memset(msg.CharListData, 0, sizeof (msg.CharListData));
		msg.dnidClient = RetDNID;		

        // �ڻ�ȡ��ɫ�б�����ʱ��������ʺ��Ѿ������ڻ����У����ܼ�������ִ��
        {
            // ���ڲ������˻��棬��ʹ��CheckAccountExist����ֱ�Ӽ�������
	        UGE::CriticalLock::Barrier4ReadWrite barrier(_this->lock);

            DataPersistentLayer::AccountMap::iterator it1 = _this->accountMap.find(account);
            if (it1 != _this->accountMap.end())
            {
                DataPersistentLayer::CacheMap::iterator it2 = _this->cacheMap.find( it1->second );
                assert(it2 != _this->cacheMap.end());

                if (it2 != _this->cacheMap.end()) 
                {
		            try
		            {
			            lite::Serializer lsm(msg.streamData, sizeof(msg.streamData));
			            lsm(account.c_str());
                        msg.byResult = (it2->second.serverId == serverId) ? 
                            SAGetCharacterListMsg::RET_LOCKEDSELF : SAGetCharacterListMsg::RET_LOCKEDOTHER;
			            SendMessage(dnidClient, &msg, sizeof(msg) - lsm.EndEdition());
                    }
		            catch( lite::Xcpt & )
		            {
		            }
                    return ; // �ý�ɫ����Ч����ʱ�����ȡ��ɫ�б�������������״������
                }
                OutputDebugString( "�ʺ��б�ͻ����б�ƥ�䣡����" );
            }
        }

		try
		{
			lite::Serializer lsm(msg.streamData, sizeof(msg.streamData));
			lsm(account.c_str());

			msg.byResult = _this->GetQuery().GetPlayerCharList(account.c_str(), serverId, msg.CharListData);

			SendMessage(dnidClient, &msg, sizeof(msg) - lsm.EndEdition());
		}
		catch(lite::Xcpt &)
		{
		}
    }

    DNID dnidClient;
    DataPersistentLayer4XYD1 *_this;
    DWORD serverId;
    std::string account;
    _g::_p::Callback4QueryPlayerList callback;
	DNID RetDNID;
};

//��ʼ��
bool DataPersistentLayer4XYD1::Init( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName)
{
	return m_Query.Init( UserName, PassWord, DBIP, DBName );
}

//ѭ����������
int DataPersistentLayer4XYD1::LoopService( ThreadHandle *tHandle )
{
    if ( !queryQueue.empty() )
    {
        Element element;
        // ���ﱣ֤���ȴ������е��ⲿ�ύ����
        while ( queryQueue.mtsafe_get_and_pop( element ) )
        {
            // ������ܴ��ڵ��ⲿ�ύ�����ֻ��4������ȡ�б��½���ɫ����ȡ��ɫ��ɾ����ɫ
            // �������ݿ�����ʧ�ܣ���ֱ�ӷ��غ��ˡ�
            if ( !m_Query.KeepAlive( 0, "�־ò���չ" ) )
                break;

            element.query->Execute();
            element.query->Release();
        }
    }

    return DataPersistentLayer::LoopService( tHandle );
}


struct Query4NewPlayer : public DataPersistentLayer4XYD1::QueryBase
{
	Query4NewPlayer(DataPersistentLayer4XYD1 *a, DNID b, DWORD c, LPCSTR d, DWORD e, SFixData &f, _g::_p::CallBack4CreatePlayer g)
        : _this(a), dnidClient(b), serverId(c), account(d), index(e), data(f), callback(g) 
    { 
    }

    const std::string &GetKey() const { return account; }

    void Release() { delete this; }

    void Execute()
	{	
        // �ڴ�����ɫ����ʱ��������ʺ��Ѿ������ڻ����У����ܼ�������ִ��
        if ( _this->CheckAccountExist(account))
            return;

		SACreatePlayerMsg Acreatemsg;					// ���ĳ��λ�Ľ�ɫ�����ڣ���ô����Ӧλ�ϵ����ݾ�Ϊ��
		
		Acreatemsg.qwSerialNo = 0;	
		//Acreatemsg.dnidClient = data.m_Exp;				// ��ȡGID
		//data.m_Exp = 0;									// �ظ�����
        data.m_version = SFixData::GetVersion();	// ���ð汾

		int retcode = _this->GetQuery().CreatePlayerData(account.c_str(), (WORD)index, serverId, data);

		Acreatemsg.byResult						= retcode;
		Acreatemsg.m_byIndex					= (BYTE)index;
		

		memcpy(Acreatemsg.PlayerData.m_szName, data.m_Name, sizeof(data.m_Name));
		
		SendMessage(dnidClient, &Acreatemsg, sizeof(SACreatePlayerMsg));
    }

    DNID dnidClient;
    DataPersistentLayer4XYD1 *_this;
    DWORD serverId;
    std::string account;
    _g::_p::CallBack4CreatePlayer callback;
    DWORD index;
	SFixData data;
};

struct Query4SavePlayerConfig : public DataPersistentLayer4XYD1::QueryBase
{
	Query4SavePlayerConfig( DataPersistentLayer4XYD1 *a, LPCSTR straccount,DWORD  dserverId,
		LPCSTR pconfig )
		: _this( a ), serverId( dserverId ), account( straccount ),strplayerconfig(pconfig)
	{ 
	}

	const std::string &GetKey() const { return account; }

	void Release() { delete this; }

	void Execute()
	{
// 		// �ڻ�ȡ��ɫ����ʱ��������ʺ��Ѿ������ڻ����У����ܼ�������ִ��
// 		if (_this->CheckAccountExist(account))
// 		{
// 			// ������ܻ���������ݴ������ɫ�Ѿ�����Ϸ���У�����ظ�����󣬻ᵼ�����ݱ����ǣ�
// 			return;
// 		}
		_this->GetQuery().SavePlayerConfig(account.c_str(), serverId, strplayerconfig.c_str());
	}

	DNID dnidClient;
	DataPersistentLayer4XYD1 *_this;
	DWORD serverId;
	std::string account;
	_g::_p::Callback4SavePlayerConfig callback;
	DNID retDnid;
	BYTE byIndex;
	std::string strplayerconfig;
};

struct Query4SelectPlayer : public DataPersistentLayer4XYD1::QueryBase
{
    Query4SelectPlayer( DataPersistentLayer4XYD1 *a, DNID b, DWORD c, LPCSTR d, 
        BYTE e, DNID f, _g::_p::Callback4QuerySelectPlayer g )
        : _this( a ), dnidClient( b ), serverId( c ), account( d ), byIndex(e), retDnid( f ), callback( g ) 
    { 
    }

    const std::string &GetKey() const { return account; }

    void Release() { delete this; }

	void Execute()
    {
		SAGetCharacterMsg msg;    

		msg.dnidClient = retDnid;
		lite::Serializer lsm(msg.streamData, sizeof(msg.streamData));
		lsm(account.c_str());

        // �ڻ�ȡ��ɫ����ʱ��������ʺ��Ѿ������ڻ����У����ܼ�������ִ��
        if (_this->CheckAccountExist(account))
        {
            // ������ܻ���������ݴ������ɫ�Ѿ�����Ϸ���У�����ظ�����󣬻ᵼ�����ݱ����ǣ�
            return;
        }

		// �����ݿ��ȡ���ݣ������µ�����
		memset(&msg.PlayerData, 0, sizeof(msg.PlayerData));

		msg.byResult = _this->GetQuery().GetPlayerData(account.c_str(), serverId, byIndex, msg.PlayerData);

		if (SAGetCharacterMsg::ERC_SUCCESS == msg.byResult)
			if (!_this->LoadIntoCache(serverId, account.c_str(), msg.PlayerData, false))
				msg.byResult = SAGetCharacterMsg::ERC_FAIL;

		msg.byResult = (msg.byResult == SAGetCharacterMsg::ERC_SUCCESS);
		SectionMessageManager::getInstance().sendMessageWithSection(dnidClient,&msg,sizeof(msg)-lsm.EndEdition());
		SendMessage(dnidClient, &msg, sizeof(msg) - lsm.EndEdition());
	}

    DNID dnidClient;
    DataPersistentLayer4XYD1 *_this;
    DWORD serverId;
    std::string account;
    _g::_p::Callback4QuerySelectPlayer callback;
    DNID retDnid;
	BYTE byIndex;
};

struct Query4DeletePlayer : public DataPersistentLayer4XYD1::QueryBase
{
    Query4DeletePlayer( DataPersistentLayer4XYD1 *a, DNID b, DWORD c, LPCSTR d, DWORD e, DNID f )
        : _this( a ), dnidClient( b ), serverId( c ), account( d ), playerId(e), retDnid( f ) {}

    const std::string &GetKey() const { return account; }

    void Release() { delete this; }

    void Execute()
    {
		// ��Ҫ��֤�����ɫ�������ڻ�����
		int deleteResult = 0;

        // ��ɾ����ɫ����ʱ��������ʺ��Ѿ������ڻ����У����ܼ�������ִ��
        if (!_this->CheckAccountExist(account))
        {
			UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock);
			DataPersistentLayer4XYD1::CacheMap::iterator it = _this->cacheMap.find(playerId);
			if (it != _this->cacheMap.end())
				return;

			// ������Ҫ��֤��ɾ�������У������������߼����ؽ�ɫ�����Ծͽ����ݿ����Ҳ�Ž��ٽ�����
			deleteResult = _this->GetQuery().DeletePlayerData(playerId, account.c_str(), serverId);
		}

	
		SADeleteCharacterMsg msg;
		msg.dwStaticID	= playerId;
		msg.dnidClient	= retDnid;
		msg.byResult	= deleteResult;
		SendMessage(dnidClient, &msg, sizeof msg);
	}

    DNID dnidClient;
    DataPersistentLayer4XYD1 *_this;
    DWORD serverId;
    std::string account;
    DNID retDnid;
	DWORD playerId;
};
struct QueryAllPlayerRanklist : public DataPersistentLayer4XYD1::QueryBase
{
	QueryAllPlayerRanklist(DataPersistentLayer4XYD1 * a,DNID dnidClient, DNID serverID, DWORD level):_this(a),account("&&"),m_dnidClient(dnidClient), m_serverID(serverID), m_level(level){}
	void Release() { delete this; }
	const std::string &GetKey() const { return account; }
	void Execute()
	{
		if (_this)
		{
			SADBGETRankList Msg;
			Msg.m_Num = 0;
			Msg.serverID = m_serverID;

			_this->GetQuery().GetAllPlayerRanklist(m_level,Msg.m_Num,&Msg.m_NewRankList[0]);
			
// 			// ����Ϣ,���͵���½��������Ȼ�����������ֱ�Ӵ�LogServer��ȡ����
// 			SendMessage(m_dnidClient, &Msg, sizeof Msg); 

			int num = SectionMessageManager::getInstance().evaluateDevidedAmount(sizeof(Msg));
			int id = 0;
			for (int i = 0;i < num;i++)
			{
				SSectionMsg sMsg;
				id = SectionMessageManager::getInstance().devideMessage(i,num,&sMsg,&Msg,sizeof(Msg),id);
				SendMessage(m_dnidClient,&sMsg,sizeof(SSectionMsg));
			}
		}
	}
	DataPersistentLayer4XYD1 *_this;
	DNID m_dnidClient;
	DNID m_serverID;
	std::string account;
	DWORD m_level;
};
// �����½�ɫ
void DataPersistentLayer4XYD1::NewPlayer(DNID dnidClient, DWORD serverId, LPCSTR account, DWORD index, SFixData &data, _g::_p::CallBack4CreatePlayer callback)
{
    queryQueue.mtsafe_push(new Query4NewPlayer(this, dnidClient, serverId, account, index, data, callback));
}

// ��ȡ�ʺŶ�Ӧ�Ľ�ɫ�б�
void DataPersistentLayer4XYD1::GetChList(DNID dnidClient, DWORD serverId, LPCSTR account, DNID RetDnid, _g::_p::Callback4QueryPlayerList callback)
{
    queryQueue.mtsafe_push(new Query4QueryPlayerList(this, dnidClient, serverId, account, RetDnid, callback));
}

// ��ȡ��ɫ����
void DataPersistentLayer4XYD1::GetPlayerData( DNID dnidClient, DWORD serverId, LPCSTR account,  BYTE byIndex, DNID RetDnid,  _g::_p::Callback4QuerySelectPlayer callback )
{
    queryQueue.mtsafe_push( new Query4SelectPlayer(this, dnidClient, serverId, account, byIndex, RetDnid, callback));
}

// ɾ����ɫ
void DataPersistentLayer4XYD1::DeletePlayer(DNID dnidClient, DWORD serverId, LPCSTR account, DWORD playerId, DNID retDnid)
{
    queryQueue.mtsafe_push(new Query4DeletePlayer(this, dnidClient, serverId, account, playerId, retDnid));
}

/*
// ���½�ɫ����
void DataPersistentLayer4XYD1::UpdateData( DWORD serverId, LPCSTR account )
{
    queryQueue.mtsafe_push( new Query4UpdatePlayer( this, dnidClient, serverId, account, byIndex, RetDnid, callback ) );
}
*/

// ���⴦������������չ
void DataPersistentLayer4XYD1::RPCStream()
{

}

void DataPersistentLayer4XYD1::SavePlayerConfig(LPCSTR account ,DWORD serverId, LPCSTR PlayerConfig )
{
	queryQueue.mtsafe_push( new Query4SavePlayerConfig(this, account,serverId, PlayerConfig));
}
void DataPersistentLayer4XYD1::GetAllPlayerRankList(DNID dnidClient, DNID serverID, DWORD level)
{
	queryQueue.mtsafe_push( new QueryAllPlayerRanklist(this,dnidClient, serverID, level));
}
BOOL DataPersistentLayer::LoadIntoCache(DWORD serverId, LPCSTR account, SFixData &data, BOOL fromGame)
{
	// ���ڲ������˻��棬ֱ�Ӽ�������
	UGE::CriticalLock::Barrier4ReadWrite barrier(lock);

	// ��ҪԤ�ȼ��ý�ɫ�Ƿ��Ѿ������ڻ������ˣ�����ǵĻ���˵��ĳ���ط��������쳣
	{
		CacheMap::iterator it = cacheMap.find(data.m_dwStaticID);
		if (it != cacheMap.end())
			return false;

        AccountMap::iterator it2 = accountMap.find(account);
		if (it2 != accountMap.end())
			return false;
	}

//	if (!iuChecker.AtLoadPlayerFromDatabase(data, fromGame))
//		return false;

	CacheUnit &cu = cacheMap[data.m_dwStaticID];
	accountMap[account] = data.m_dwStaticID;

	if (!cu.data)
	{
		cu.data		= new SFixData;
		cu.account	= account;
        cu.serverId = serverId;
	}

	*cu.data		= data;		
	cu.modified		= fromGame;		// ����Ǵ���Ϸ���������ݣ���Ӧ�ñ��棡
	cu.removeMe		= 0;
	cu.timeStamp	= timeGetTime();

	return true;
}

BOOL DataPersistentLayer::SaveIntoCache(DWORD serverId, SFixData &data, LPCSTR account)
{
    // �ο͵������ǲ���Ҫ����ģ���Ϊ�����acc������һ����Ч��buffer������ֱ��ʹ����ǰ4���ֽڽ��бȽϣ�
    if (*(LPDWORD)account == *(LPDWORD)"�ο�")
        return false;

	UGE::CriticalLock::Barrier4ReadWrite barrier(lock);

    commitTimes ++;

	// ��Ҫȷ������һ���Ѿ������������
    DataPersistentLayer::CacheMap::iterator it = cacheMap.find(data.m_dwStaticID);
    if (it == cacheMap.end())
    {
        // ������Ҫ���⴦������ݴ��߼������ݿ���������ܻᱻ��������رգ�
        // ע�⣬LoadIntoCache ����һ���ظ���
        if (!LoadIntoCache( serverId, account, data, true))
		    return false;

        it = cacheMap.find( data.m_dwStaticID );
        if ( it == cacheMap.end() )
            return false;
    }

    // �ų�δ�������ݾͳ��ֱ�����߼���
    if ( it->second.data == NULL )
        return false;

    CacheUnit &cu = it->second;
	if ( !iuChecker.AtSavePlayerIntoCache( data, cu.data ) )
    {
        conf1 ++;
		return false;
    }

	cu.modified = true;												    // ��ʶ�����Ѿ����޸�!
	cu.removeMe = ( data.m_byStoreFlag == SFixData::ST_LOGOUT );    // �Ƿ���Ҫ���챣������ݿⲢ�������������

	UpdateEvent ue = { data.m_dwStaticID, serverId };
	updateQueue.push( ue );

	return true;
}

BOOL DataPersistentLayer::DoubleSaveIntoCache( DWORD serverId, SFixData &d1, SFixData &d2, LPCSTR acc1, LPCSTR acc2 )
{
    // �ο͵������ǲ���Ҫ����ģ�����Ϊ�����acc������һ����Ч��buffer������ֱ��ʹ����ǰ4���ֽڽ��бȽϣ�
    if ( *( LPDWORD )acc1 == *( LPDWORD )"�ο�" )
        return SaveIntoCache( serverId, d2, acc2 );
    else if ( *( LPDWORD )acc2 == *( LPDWORD )"�ο�" )
        return SaveIntoCache( serverId, d1, acc1 );

	UGE::CriticalLock::Barrier4ReadWrite barrier( lock );

    commitTimes += 2;

    DataPersistentLayer::CacheMap::iterator it1 = cacheMap.find( d1.m_dwStaticID );
    DataPersistentLayer::CacheMap::iterator it2 = cacheMap.find( d2.m_dwStaticID );
    if ( it1 == cacheMap.end() || it2 == cacheMap.end() )
    {
        // �ݴ���
        if ( it1 == cacheMap.end() )
        {
            if ( !LoadIntoCache( serverId, acc1, d1, true ) )
		        return false;

            it1 = cacheMap.find( d1.m_dwStaticID );
            if ( it1 == cacheMap.end() )
                return false;
        }

        if ( it2 == cacheMap.end() )
        {
            if ( !LoadIntoCache( serverId, acc2, d2, true ) )
		        return false;

            it2 = cacheMap.find( d2.m_dwStaticID );
            if ( it2 == cacheMap.end() )
                return false;
        }
    }

    // �ų�δ�������ݾͳ��ֱ�����߼���
    if ( it1->second.data == NULL || it2->second.data == NULL )
        return false;

    CacheUnit &cu1 = it1->second;
    CacheUnit &cu2 = it2->second;

	if ( !iuChecker.AtDoubleSavePlayerIntoCache( d1, d2, cu1.data, cu2.data ) )
    {
        conf2 ++;
		return false;
    }

	cu1.modified = true;
	cu1.removeMe = d1.m_byStoreFlag == SFixData::ST_LOGOUT;
	cu2.modified = true;
	cu2.removeMe = d2.m_byStoreFlag == SFixData::ST_LOGOUT;	// �Ƿ���Ҫ���챣������ݿⲢ�������������

	UpdateEvent ue[2] = { { d1.m_dwStaticID, serverId }, { d2.m_dwStaticID, serverId } };
	updateQueue.push( ue[0] );
	updateQueue.push( ue[1] );

	return true;
}

namespace _g
{
    DataPersistentLayer4XYD1 dpLayer;

    DWORD GetServerID()
    {
        return 0;
    }

	void _p::_WriteRole()
	{
		dpLayer.GetQuery().LoadAllPlayerData();
	}
	BOOL _p::Initialize( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName )
    {
		return dpLayer.Init( UserName, PassWord, DBIP, DBName) && dpLayer.InitDpl();
    }

    int _p::Shutdown( HANDLE quitEvent )
    {
        int _SaveAll( HANDLE quitEvent );
        return _SaveAll( quitEvent );

        //if ( quitEvent != NULL )
        //    dpLayer.Release( quitEvent );
    }

    int _p::Backup( BOOL *atBackup, LPCSTR title, LPCSTR bakPath )
    {
        // �򱸷��¼���־
        if ( atBackup ) 
            *atBackup = 1;

        _g::dpLayer.title = title;
        _g::dpLayer.path = bakPath;
        _g::dpLayer.atBackup = atBackup;

        int _SaveAll( HANDLE quitEvent );
        int ck = _SaveAll( NULL );

        // ��ײ�֪ͨ�����¼���
        DataPersistentLayer::UpdateEvent ue = { 0, 0 };
	    _g::dpLayer.updateQueue.push( ue );

        return ck;
    }

    void _p::QueryPlayerList(DNID dnidClient, SQGetCharacterListMsg *pMsg)
    {  
		LPCSTR account = NULL;
		lite::Serialreader reader(pMsg->streamData);
		account = reader();
		if (account == NULL) 
			return;

		Callback4QueryPlayerList callback =NULL;

        // �������ύ���߳���ִ�У��������Խ������ʱ������������߼�����������
		dpLayer.GetChList(dnidClient, pMsg->dwServerId, account, pMsg->dnidClient, callback);
    }

	void _p::SetPlayerInitData(SFixData &fixproperty)
	{
		fixproperty.m_bNewPlayer = true;
		fixproperty.m_byStoreFlag = SFixData::ST_LOGIN;
		fixproperty.m_EnduranceData.m_dwEndurance = 90;
		fixproperty.m_dwLevel = 1;


		//�½��佫װ����ʼ��
		SHeroData tempHeroData;
		memset(&tempHeroData, 0, sizeof(SHeroData));
		SHeroEquipment tempEquip;
		memset(&tempEquip, 0, sizeof(SHeroEquipment));
		/*for (int i = 0; i < EQUIP_MAX; i++)
			memcpy(&tempHeroData.m_Equipments[i], &tempEquip, sizeof(SEquipment));*/
			
		//�½��佫���Գ�ʼ��
		for (int i = 0; i < MAX_HERO_NUM; i++)
			memcpy(&fixproperty.m_HeroList[i], &tempHeroData, sizeof(SEquipment));
	}

	void _p::QueryCreatePlayer(DNID dnidClient, SQCreatePlayerMsg *pMsg)
    {
		LPCSTR account = NULL;
		lite::Serialreader reader(pMsg->streamData);
		account = reader();
		if (!account) 
			return;

		SFixData fixData;	
		memset(&fixData, 0, sizeof(SFixData));
		memcpy(fixData.m_Name, pMsg->PlayerData.m_szName, sizeof(fixData.m_Name));
		fixData.m_dwStaticID = pMsg->PlayerData.m_dwStaticID;


		SetPlayerInitData(fixData);

		CallBack4CreatePlayer callback = NULL;
		dpLayer.NewPlayer(dnidClient, pMsg->dwServerId, account, pMsg->m_byIndex, fixData, callback);
    }

    void _p::QuerySelectPlayer(DNID dnidClient, SQGetCharacterMsg *pMsg)
    {
        LPCSTR account = NULL;
		lite::Serialreader reader(pMsg->streamData);
		account = reader();
		if (account == NULL) 
			return;
        Callback4QuerySelectPlayer callback = NULL;		
		dpLayer.GetPlayerData(dnidClient, pMsg->dwServerId, account, pMsg->byIndex, pMsg->dnidClient, callback);
    }

	void _p::DeletePlayer( DNID dnidClient, SQDeleteCharacterMsg *pMsg )
	{
        LPCSTR account = NULL;
		lite::Serialreader reader( pMsg->streamData );
		account = reader();
		if (account == NULL) 
			return;

		dpLayer.DeletePlayer(dnidClient, pMsg->dwServerId, account, pMsg->dwStaticID, pMsg->dnidClient);
	}

	void _p::SavePlayerData( DNID dnidClient, SQSaveCharacterMsg * pMsg)
	{
        // �°汾��֧�ִ�������ѹ������
        LPDWORD dataBuf = reinterpret_cast< LPDWORD >( &pMsg->PlayerData );
        LPVOID outBuf = NULL;
        size_t outSize = 0;
        if ( ( *dataBuf < sizeof( pMsg->PlayerData ) ) && TryDecoding_NilBuffer( &dataBuf[1], *dataBuf, outBuf, outSize ) )
        {
            // ����ѹ���汾�������ʺű���ĸ�ʽҲһ��仯�ˣ�
            size_t adSize = ( ( *dataBuf >> 2 ) + 1 ); // ��ȡ�������ʺ���ʼ��ַ��
            LPDWORD accountSize = &dataBuf[ 1 + adSize ];
            assert( outSize == sizeof( SFixData ) && *accountSize < 128 );
            if ( outSize == sizeof( SFixData ) && *accountSize < 128 )
            {
                LPCSTR account = ( LPCSTR )&accountSize[1];
                assert( account[ *accountSize - 1 ] == 0 );
			    dpLayer.SaveIntoCache( pMsg->dwServerId, *( SFixData* )outBuf, account );
            }
        }
        else
        {
            // δѹ���İ汾�����ݳʼ򵥽ṹ��������
		    try
		    {
			    lite::Serialreader ls( pMsg->streamData );
			    LPCSTR account = ls();

			    // ���ﲢ��ֱ�ӽ�����д�����ݿ⣬��ֻ�Ǽ�����Ψһ�Գ�ͻ���������ݸ����뻺�档����
			    dpLayer.SaveIntoCache( pMsg->dwServerId, pMsg->PlayerData, account );
		    }
		    catch( lite::Xcpt &e )
		    {
			    LogXcptMsg( e.GetErrInfo() );
		    }
        }
	}

	void _p::QueryDoubleSavePlayer( DNID dnidClient, SDoubleSavePlayerMsg *pMsg )
	{
        // �°汾���pMsg����ѹ��֧�֣�
        // �ṹ˵����ǰ4�ֽ�Ϊ����ѹ���󳤶ȣ����8�ֽ�Ϊ����ѹ����־��������Ϊѹ�����ݣ�
        LPDWORD dataBuf = reinterpret_cast< LPDWORD >( pMsg->data );
        LPVOID outBuf = NULL;
        size_t outSize = 0;
        if ( ( *dataBuf < sizeof( pMsg->data ) ) && TryDecoding_NilBuffer( &dataBuf[1], *dataBuf, outBuf, outSize ) )
        {
            assert( ( outSize == sizeof( SFixData ) ) || ( outSize == sizeof( SFixData ) * 2 ) );
            if ( ( outSize == sizeof( SFixData ) ) || ( outSize == sizeof( SFixData ) * 2 ) )
            {
                // ��Ϊѹ�����ݣ����ҽ���ɹ�������
                SFixData *data = ( SFixData* )outBuf;
		        if ( pMsg->gid[1] )
                {
                    data[0].m_byStoreFlag = 0;
                    data[1].m_byStoreFlag = 0;
                    assert( outSize == sizeof( SFixData ) * 2 );
			        dpLayer.DoubleSaveIntoCache( pMsg->serverId, data[0], data[1], pMsg->account[0], pMsg->account[1] );
                }
		        else
                {
                    data[0].m_byStoreFlag = 0;
                    assert( outSize == sizeof( SFixData ) );
			        dpLayer.SaveIntoCache( pMsg->serverId, data[0], pMsg->account[0] );
                }
            }
            else
            {
                char tempStr[256];
                sprintf( tempStr, "DBSERR_%u_%u.dump", ( DWORD )timeGetTime(), ( DWORD )time( NULL ) );
                TraceInfo_C( tempStr, "rawSiza = %d, outSize = %d", *dataBuf, outSize );

                std::ofstream stream( tempStr, std::ios::app | std::ios::binary );
                if ( stream.is_open() )
                {
                    if ( outSize > 0 && outSize < 0x100000 )
                        stream.write( ( LPCSTR )outBuf, ( std::streamsize )outSize );
                    stream.write( ( LPCSTR )&dataBuf[1], *dataBuf );
                }
            }
        }
        else
        {
            // ��ѹ�����ݣ���ԭ�����棡
		    if ( pMsg->gid[1] )
            {
                pMsg->data[0].m_byStoreFlag = 0;
                pMsg->data[1].m_byStoreFlag = 0;
			    dpLayer.DoubleSaveIntoCache( pMsg->serverId, pMsg->data[0], pMsg->data[1], pMsg->account[0], pMsg->account[1] );
            }
		    else
            {
                pMsg->data[0].m_byStoreFlag = 0;
			    dpLayer.SaveIntoCache( pMsg->serverId, pMsg->data[0], pMsg->account[0] );
            }
        }
	}

	void _p::QuerySavePlayerConfig( DNID dnidClient, SQLSavePlayerConfig *pMsg )
	{
// 		LPCSTR account = NULL;
// 		lite::Serialreader reader(pMsg->streamData);
// 		account = reader();
// 		if (account == NULL) 
// 			return;
// 		Callback4SavePlayerConfig callback = NULL;		
		/*dpLayer.GetPlayerData(dnidClient, pMsg->dwServerId, account, pMsg->byIndex, pMsg->dnidClient, callback);*/
		dpLayer.SavePlayerConfig(pMsg->streamData,pMsg->serverId,pMsg->playerconfig);
	}

	void _p::QueryGetAllPlayerRandList( DNID dnidClient, SQDBGETRankList *pMsg )
	{
		dpLayer.GetAllPlayerRankList(dnidClient, pMsg->serverID, pMsg->level);
	}

	void _p::SendMailSYS_SanGuo(S2D_SendMailSYS_MSG*pBaseMsg)
	{
		dpLayer.GetQuery().SendMail_sys(pBaseMsg);
	}
	void _p::SendMail_SanGuo(DNID dnidClient, S2D_SendMail_MSG*pBaseMsg)
	{
		dpLayer.GetQuery().SendMail(pBaseMsg);
	}
	void _p::GetMailInfoFromDB(DNID dnidClient , SQ_GetMailInfo_MSG* pMsg)
	{
		dpLayer.GetQuery().GetMailInfo_DB(dnidClient, pMsg);
	}
	void _p::GetMailAwards_DB_SANGUO(const DNID clientDnid, struct SQ_GetAwards_MSG* pMsg)
	{
		dpLayer.GetQuery().GetMailAwards_DB(clientDnid, pMsg);
	}
	void _p::SetMailState_DB_SANGUO(SQ_SetMailState_MSG* pMsg)
	{
		dpLayer.GetQuery().SetMailState_DB(pMsg);
	}
	void _p::test(string str)
	{
		dpLayer.GetQuery().test(str);
	}
}

static DataPersistentLayer::CacheUnit *unsafe_LocateCache( DWORD playerId )
{
	DataPersistentLayer::CacheMap::iterator it = _g::dpLayer.cacheMap.find( playerId );
	if ( it == _g::dpLayer.cacheMap.end() )
		return NULL;

    if ( it->second.data == NULL )
        return NULL;

	return &it->second;
}

/*
BOOL ItemUniqueChecker::HaveConflict( SFixData &data )
{
	DWORD playerId = data.m_dwStaticID;

	// ����װ�����еĵ���
	SEquipment *equipArray = data.m_Equip;
	for ( int i = 0; i < 8; i ++ )
	{
		SEquipment &item = equipArray[ i ];

		// ������������Ч������Ȼ����Ҫ��һ�����
		if ( item.wIndex == 0 )
			continue;
		
		// ������ߴ���Ѿ������ӵ�������ɫ����ô˵�����ߴ��ڳ�ͻ����ֹ��½
        BindStub *stub = uniqueMap.find( item.uniqueId() );

        // ���˼·�����������ߴ��ڣ����Ұ󶨽�ɫ�����Լ���������װ�������߾�ֻ������жϣ�
        if ( stub != NULL && stub->playerId != playerId )
			return true;
	}

	// ������������ֿ��еĵ��ߣ���Ϊ����64�����ӣ����Կ���ͳһ����
	SPackageItem *itemArray[4] = { data.m_pGoods, data.SFixStorage1::m_pStorageGoods,
		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };

	// ����������еĵ���
	for ( int a = 0; a < 4; a ++ )
	{
		for ( int i = 0; i < 64; i ++ )
		{
			SPackageItem &item = itemArray[ a ][ i ];

			// ������������Ч������Ȼ����Ҫ��һ�����
			if ( item.wIndex == 0 )
				continue;
			
		    // ������ߴ���Ѿ������ӵ�������ɫ����ô˵�����ߴ��ڳ�ͻ����ֹ��½
            BindStub *stub = uniqueMap.find( item.uniqueId() );

            // ���˼·�����������ߴ��ڣ����Ұ󶨽�ɫ�����Լ�������������������ǻ����еĵ��߻�ǲֿ����
            if ( stub != NULL )
                if ( stub->playerId != playerId && ( stub->where < 3 || stub->isCached ) )
			        return true;
        }
	}

	return false;
}
*/

struct _DWORD_ONCE
{
	_DWORD_ONCE() : value( 0 ) {}
	BOOL Init( DWORD v, BYTE _idx, BYTE _x, BYTE _y, BYTE _where ) 
    { 
        assert( v ); 
        if ( value != 0 ) 
            return false; 

        value = v; 
        idx = _idx;
        x = _x; 
        y = _y; 
        where = _where;
        return true;  
    }

	DWORD value;
	BYTE idx;
	BYTE x;
	BYTE y;
	BYTE where;
};

// ����������°汾��������itemSet�Ӹ������ݹ�����������һ��ʼ��֤������ͬһ������ϲ��ظ�
BOOL ItemUniqueChecker::SaveItemRelation( SFixData &data, SFixData *raw, BOOL fromGame )
{
	// XYD3 �޸ģ����߼����ƻ�û�����ף����Ծ�ֱ�ӱ�����
	if ( raw != NULL )
		*raw = data;

	return TRUE;

	// ������߹�����������һ������Ҫ��ѭ�Ĺ��򣬾�����Ҫ��ԭ���Ѿ������ù����ĵ��߽���󶨡�����
	// ��Ϊ���������ʱ��ֻ�������ϴ��ڵĵ��ߣ�����Щ�Ѿ����٣���ת�Ƶĵ��ߵĹ������ᱻ���������
	// Ȼ�󣬼����������õ���������ߣ���ô����Ϊ������ͻ�������쳣������

	// �Ӹ��������л�ȡ��ǰ�ĵ���ӳ���
// 	std::map< QWORD, _DWORD_ONCE > itemSet;
// 	DWORD playerId = data.m_dwStaticID;
// 
// 	// scorp
// 	{
// 		// �ȴӵ�ǰ���������л�ȡ��Ҫ��������е������ݣ����ұ�֤����߲��ظ�������
// 		SEquipment *equipArray = data.m_Equip;
// 		SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 			data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
//         LPCSTR info = ( raw == NULL ) ? "��ʼ��ʱ���������ظ����� [%d][%d][%d][0x%I64x]" : "����ʱ���������ظ����� [%d][%d][%d][0x%I64x]";
// 
// 		// ������ܻ������ʵ�ĸ���
// 		for ( int i = 0; i < 8; i ++ )
//         {
// 			if ( ( equipArray[ i ].wIndex ) && !itemSet[ equipArray[ i ].uniqueId() ].Init( playerId, i, 0, 0, 1 ) )
//             {
//                 // ������ֱ��ɾ�����Ƶ��ߣ�������ʧ�ܷ���
//                 TraceInfo_C( "CheckError.txt", info, data.m_dwStaticID, 1, equipArray[ i ].wIndex, equipArray[ i ].uniqueId() );
//                 memset( &equipArray[ i ], 0, sizeof( equipArray[ i ] ) );
// 				// return false;
//             }
//         }
// 
// 		for ( int a = 0; a < 4; a ++ )
//         {
// 			for ( int i = 0; i < 64; i ++ )
//             {
//                 if ( ( itemArray[ a ][ i ].wIndex ) && !itemSet[ itemArray[ a ][ i ].uniqueId() ].
//                     Init( playerId, i, itemArray[ a ][ i ].byCellX, itemArray[ a ][ i ].byCellY, 2 + a ) )
//                 {
//                     TraceInfo_C( "CheckError.txt", info, data.m_dwStaticID, 2 + a, itemArray[ a ][ i ].wIndex, itemArray[ a ][ i ].uniqueId() );
//                     memset( &itemArray[ a ][ i ], 0, sizeof( itemArray[ a ][ i ] ) );
// 					// return false;
//                 }
//             }
//         }
// 	}
// 
// 	// ������Ƕ�ȡ��ʼ���Ļ�������Ҫ�;����ݶԱȣ��ҳ���Ҫ��������Ĳ���
// 	if ( raw != NULL )
// 	{
// 		// ����װ�����еĵ���
// 		// ������������ֿ��еĵ��ߣ���Ϊ����64�����ӣ����Կ���ͳһ����
// 		SEquipment *equipArray = raw->m_Equip;
// 		SPackageItem *itemArray[4] = { raw->m_BaseGoods, raw->SFixStorage1::m_pStorageGoods,
// 			raw->SFixStorage2::m_pStorageGoods, raw->SFixStorage3::m_pStorageGoods };
// 
// 		for ( int i = 0; i < 8; i ++ )
// 		{
// 			SEquipment &item = equipArray[ i ];
// 
// 			// ������������Ч������Ȼ����Ҫ��һ�����
// 			if ( item.wIndex == 0 )
// 				continue;
// 
// 			// ��Ϊ�Ǵӻ����л�ȡ���ݣ����ҹ滮Ϊ�����ӳ�������һ�£��������¶���
// 			BindStub *bindStub = uniqueMap.find( item.uniqueId() );
//             assert( bindStub != NULL && bindStub->playerId == playerId );
//             if ( bindStub == NULL || bindStub->playerId != playerId )
//             {
//                 TraceInfo_C( "CheckError.txt", "�ڻ���������ķ����˳�ͻ����[%d][%d][%d][%d][0x%I64x]", 
//                     data.m_dwStaticID, bindStub->playerId, 1, equipArray[ i ].wIndex, equipArray[ i ].uniqueId() );
//                 continue;
//             }
// 
//             assert( bindStub->isCached );
//             assert( bindStub->idx == i ); 
//             assert( bindStub->x == 0 ); 
// 			assert( bindStub->y == 0 );
//             assert( bindStub->where == 1 );
// 
// 			BindStub *stub = NULL;
// 			std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.find( item.uniqueId() );
// 			if ( it == itemSet.end() )
// 			{
// 				// ���±���δ�ҵ�������ɾ�����֣���Ҫ�����
//                 uniqueMap.erase( item.uniqueId() );
// 			}
// 			else
// 			{
// 				// �ҵ���˵���õ��ߵ�����δ�仯����Ҫ��map���Ƴ����Ա�����ҳ������Ĳ��֡�����
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->x = it->second.x; 
// 			    bindStub->y = it->second.y;
//                 bindStub->where = it->second.where;
//                 itemSet.erase( it );
// 			}
// 		}
// 
// 		// ����������еĵ���
// 		for ( int a = 0; a < 4; a ++ )
// 		{
// 			for ( int i = 0; i < 64; i ++ )
// 			{
// 				SPackageItem &item = itemArray[ a ][ i ];
// 
// 				// ������������Ч������Ȼ����Ҫ��һ�����
// 				if ( item.wIndex == 0 )
// 					continue;
// 
// 				// ��Ϊ�Ǵӻ����л�ȡ���ݣ����ҹ滮Ϊ�����ӳ�������һ�£��������¶���
// 				BindStub *bindStub = uniqueMap.find( item.uniqueId() );
// 				assert( bindStub != NULL && bindStub->playerId == playerId );
//                 if ( bindStub == NULL || bindStub->playerId != playerId )
//                 {
//                     TraceInfo_C( "CheckError.txt", "�ڻ���������ķ����˳�ͻ����[%d][%d][%d][%d][0x%I64x]", 
//                         data.m_dwStaticID, bindStub->playerId, 2 + a, item.wIndex, item.uniqueId() );
//                     continue;
//                 }
// 
//                 assert( bindStub->isCached == true );
//                 assert( bindStub->idx == i ); 
//                 assert( bindStub->x == item.byCellX ); 
// 			    assert( bindStub->y == item.byCellY );
//                 assert( bindStub->where == 2 + a );
// 
// 				BindStub *stub = NULL;
// 				std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.find( item.uniqueId() );
// 				if ( it == itemSet.end() )
// 				{
// 				    // ���±���δ�ҵ�������ɾ�����֣���Ҫ�����
//                     uniqueMap.erase( item.uniqueId() );
// 				}
// 				else
// 				{
// 					// �ҵ���˵���õ��ߵ�����δ�仯����Ҫ��map���Ƴ����Ա�����ҳ������Ĳ��֡�����
//                     bindStub->isCached = true;
//                     bindStub->idx = it->second.idx; 
//                     bindStub->x = it->second.x; 
// 			        bindStub->y = it->second.y;
//                     bindStub->where = it->second.where;
// 					itemSet.erase( it );
// 				}
// 			}
// 		}
// 	}
// 
// 	// �������itemSet�еĵ��ߣ������ڸý�ɫ�»�õĵ��ߡ���Ҫ����ͻ
// 	if ( !itemSet.empty() )
//     {
//         LPCSTR info = ( raw == NULL ) ? "��ʼ��ʱ�����ظ����� [%d][%d][%d][%d][0x%I64x]" : "����ʱ�����ظ����� [%d][%d][%d][%d][0x%I64x]";
// 
// 	    for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); it ++ )
// 	    {
// 		    // �������Щ����Ӧ�����´����ģ������Ǵ�������ɫ�Ƕ���õġ�����
//     	    BindStub *bindStub = uniqueMap.find( it->first );
// 		    if ( bindStub == NULL )
//             {
//                 // ����������½����ߣ���Ҫ��ʼ������Ϣ
//                 bindStub = &uniqueMap[ it->first ];
// 		        bindStub->playerId = playerId;
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->where = it->second.where;
//                 bindStub->x = it->second.x;
//                 bindStub->y = it->second.y;
//             }
// 
//             // ������������������Ƴ������»ص���Ϸ����Ҫ��������Щ���߽������°󶨣�
//             // ������һ��ǰ���ǣ���Ҽ��絽������Ϸ����(һ�϶�������)��ɾ���˵��߻����˵��ߣ�
//             // ������İ����ݻ�����δͬ��������Ŀ�궪ʧ
//             else if ( raw == NULL && !bindStub->isCached )
//             {
//                 // ��������Ƿǻ�����ߣ��߼��Ͽ��Ա��µĶ�ȡ�������ǣ������ս����ֻ����һ�������ڻ����У�
//                 // �������ﲻ�ж� playerId ����ֱ�Ӹ��ǣ�
// 		        bindStub->playerId = playerId;
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->where = it->second.where;
//                 bindStub->x = it->second.x;
//                 bindStub->y = it->second.y;
//             }
// 		    else
//             {
//                 // ����������ģ���ֻ�����Ǹ����ˡ�����
//                 // ��2���������ȡ��ʼ�������ݱ��棬��ȡʱ��������ָ��ƣ���Ӧ������ҵ�½����Ҫ����������й��������ݣ���
//                 // ������ʱ����ֱ��ɾ����ͻ���ߣ�
//                 assert( bindStub->playerId != playerId );
//                 assert( bindStub->where >= 1 && bindStub->where <= 5 );
// 
//                 if ( raw == NULL && fromGame == false )
//                 {
//                     // ����ʧ�ܣ���������ҵ�½��
//                     AtRemovePlayerFromCache( data );
//                     return false;
//                 }
// 
//                 // ����ͻԴ����ʱ��ֱ��ɾ����ͻԴ���ߣ�����ɾ����ͻĿ����ߣ�
//                 SFixData *dst = &data;
//                 DWORD idx = it->second.idx;
//                 DWORD where = it->second.where;
//                 DataPersistentLayer::CacheUnit *cu = NULL;
//                 DWORD itemIndex = 0;
// 
//                 if ( bindStub->isCached )
//                 {
//                     cu = unsafe_LocateCache( bindStub->playerId );
// 
//                     // ӵ�� isCached ��־����Ȼ�����ɫ���ڻ�����
//                     assert( cu != NULL && cu->data != NULL );
// 
//                     // �ݴ�
//                     if ( cu != NULL )
//                     {
//                         assert( cu->data->m_dwStaticID == bindStub->playerId );
//                         dst = cu->data;
//                         idx = bindStub->idx;
//                         where = bindStub->where;
//                     }
//                 }
//                 else
//                 {
//                     // �ݴ���
//                     assert( unsafe_LocateCache( bindStub->playerId ) == NULL );
//                 }
// 
//                 BOOL deleteOk = FALSE;
//                 if ( where == 1 )
//                 {
//                     SEquipment &item = dst->m_Equip[ idx ];
// 
//                     assert( idx < 8 );
//                     assert( item.uniqueId() == bindStub->UniqueId() );
//                     if ( item.uniqueId() == bindStub->UniqueId() )
//                     {
//                         itemIndex = item.wIndex;
//                         memset( &item, 0, sizeof( item ) );
//                         deleteOk = TRUE;
//                     }
//                 }
//                 else if ( where >= 2 && where <= 5 )
//                 {
// 		            SPackageItem *itemArray[4] = { dst->m_BaseGoods, dst->SFixStorage1::m_pStorageGoods,
// 			            dst->SFixStorage2::m_pStorageGoods, dst->SFixStorage3::m_pStorageGoods };
// 
//                     SPackageItem &item = itemArray[ where - 2 ][ idx ];
// 
//                     assert( bindStub->idx < 64 );
//                     assert( item.uniqueId() == bindStub->UniqueId() );
//                     if ( item.uniqueId() == bindStub->UniqueId() )
//                     {
//                         itemIndex = item.wIndex;
//                         memset( &item, 0, sizeof( item ) );
//                         deleteOk = TRUE;
//                     }
//                 }
// 
//                 // ����Ӧ�ñ�Ȼɾ���ɹ���
//                 assert( deleteOk );
// 
//                 // ���ɾ�����ǶԷ������ݣ���ô��Ҫ���ϱ�ǣ������������̹رյ�ʱ��û������д�룬�����¸��Ƴ�ͻ��
//                 if ( cu && deleteOk )
//                     cu->modified = true;
// 
//                 TraceInfo_C( "CheckError.txt", "��������ʱ�����˳�ͻ����[%d][%d[%d,%d,%d]][%d[%d,%d,%d]][%d][0x%I64x]", 
//                     itemIndex, 
//                     data.m_dwStaticID, it->second.where, it->second.x, it->second.y,
//                     bindStub->playerId, bindStub->where, bindStub->x, bindStub->y, 
//                     dst->m_dwStaticID, it->first );
// 
//                 // ���ɾ���ɹ�������ɾ�����ǳ�ͻԴ������Ҫ�޸İ����ݵ��µĽ�ɫ���ϣ�
//                 if ( deleteOk && bindStub->playerId == dst->m_dwStaticID )
//                 {
// 		            bindStub->playerId = playerId;
//                     bindStub->isCached = true;
//                     bindStub->idx = it->second.idx; 
//                     bindStub->where = it->second.where;
//                     bindStub->x = it->second.x;
//                     bindStub->y = it->second.y;
//                 }
//             }
//         }
//     }
// 
// 	// ����ĩֱ�ӱ������ݵ�����Ŀ���У�
// 	if ( raw != NULL )
// 		*raw = data;
// 
// 	return true;
}

BOOL ItemUniqueChecker::AtRemovePlayerFromCache( SFixData &data )
{
	return TRUE;

    // �ú�����Ҫ�����ڽ�ɫ�ӻ������������Ҫͬʱ���ֿ�ĵ��߽��������
//     DWORD playerId = data.m_dwStaticID;
// 
// 	SEquipment *equipArray = data.m_Equip;
// 	SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
// 	// ������ܻ������ʵ�ĸ���
// 	for ( int i = 0; i < 8; i ++ )
//     {
//         if ( equipArray[ i ].wIndex == 0 )
//             continue;
// 
//         BindStub *bindStub = uniqueMap.find( equipArray[ i ].uniqueId() );
//         assert( bindStub != NULL && bindStub->playerId == playerId );
//         if ( bindStub == NULL || bindStub->playerId != playerId )
//             continue;
// 
//         bindStub->isCached = false;
//     }
// 
// 	for ( int a = 0; a < 4; a ++ )
//     {
// 		for ( int i = 0; i < 64; i ++ )
//         {
//             if ( itemArray[ a ][ i ].wIndex == 0 )
//                 continue;
// 
//             BindStub *bindStub = uniqueMap.find( itemArray[ a ][ i ].uniqueId() );
//             assert( bindStub != NULL && bindStub->playerId == playerId );
//             if ( bindStub == NULL || bindStub->playerId != playerId )
//                 continue;
// 
//             bindStub->isCached = false;
//         }
//     }
// 
// 	return true;
}

// ####################################################################################################
// ע�⣺����߼��У��������ݵĴ���Ӧ���ǣ����ͱ�������ֿ���
// ���һ���ֹ��������ɹ���һ������ʧ�ܵĻ�����ô�����е����ݺ�Ψһ�Թ�����Ͳ��ܱ�֤һ���ԣ�
// �������¸��Ӷ����ӣ����Ҳ�������
// ####################################################################################################
BOOL ItemUniqueChecker::AtLoadPlayerFromDatabase( SFixData &data, BOOL fromGame )
{
	// XYD3 ��ʱ
	BOOL rt = SaveItemRelation( data, NULL, fromGame );
	assert( rt );
	return rt;

	// ֱ�Ӽ�һ�Ѵ�����������������������
	// UGE::CriticalLock::Barrier4ReadWrite barrier( bigLocker );

	// ����ֻ�Ǽ�⣬������ڳ�ͻ����Ҫ����ͻ����
// 	DWORD playerId = data.m_dwStaticID;
// 
// 	// ����װ�����еĵ���
// 	SEquipment *equipArray = data.m_Equip;
// 	for ( int i = 0; i < 8; i ++ )
// 	{
// 		SEquipment &item = equipArray[ i ];
// 
// 		// ������������Ч������Ȼ����Ҫ��һ�����
// 		if ( item.wIndex == 0 )
// 			continue;
// 		
// 		// ������ߴ���Ѿ������ӵ�������ɫ����ô˵�����ߴ��ڳ�ͻ����ֹ��½
//         BindStub *stub = uniqueMap.find( item.uniqueId() );
// 
//         // ���˼·�����������ߴ��ڣ����Ұ󶨽�ɫ�����Լ���ͬʱ������߻������ڻ����У�
//         if ( stub != NULL && stub->playerId != playerId && stub->isCached )
//         {
//             if ( fromGame )
//                 TraceInfo_C( "CheckError.txt", "����Ϸ��ʼ������ʱ�����ظ����� %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//             else
//             {
//                 TraceInfo_C( "CheckError.txt", "��ȡʱ�����ظ����� %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                 memset( &item, 0, sizeof( SEquipment ) );
// 			    //return false;
//             }
//         }
// 	}
// 
// 	// ������������ֿ��еĵ��ߣ���Ϊ����64�����ӣ����Կ���ͳһ����
// 	SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
// 	// ����������еĵ���
// 	for ( int a = 0; a < 4; a ++ )
// 	{
// 		for ( int i = 0; i < 64; i ++ )
// 		{
// 			SPackageItem &item = itemArray[ a ][ i ];
// 
// 			// ������������Ч������Ȼ����Ҫ��һ�����
// 			if ( item.wIndex == 0 )
// 				continue;
// 			
// 		    // ������ߴ���Ѿ������ӵ�������ɫ����ô˵�����ߴ��ڳ�ͻ����ֹ��½
//             BindStub *stub = uniqueMap.find( item.uniqueId() );
// 
//             // ���˼·�����������ߴ��ڣ����Ұ󶨽�ɫ�����Լ���ͬʱ������߻������ڻ����У�
//             if ( stub != NULL && stub->playerId != playerId && stub->isCached )
//             {
//                 if ( fromGame )
//                     TraceInfo_C( "CheckError.txt", "����Ϸ��ʼ������ʱ�����ظ����� %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                 else
//                 {
//                     TraceInfo_C( "CheckError.txt", "��ȡʱ�����ظ����� %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                     memset( &item, 0, sizeof( SPackageItem ) );
// 			        //return false;
//                 }
//             }
//         }
// 	}
// 
// 	// ��Ȼ�����ڳ�ͻ����ô����Ӧ�ñض��ɹ���
//     BOOL result = SaveItemRelation( data, NULL, fromGame );
//     assert( result );
//     return result;
}

BOOL ItemUniqueChecker::AtSavePlayerIntoCache( SFixData &data, SFixData *dst )
{
	// ֱ�Ӽ�һ�Ѵ�����������������������
	// UGE::CriticalLock::Barrier4ReadWrite barrier( bigLocker );

    // ����Ҳ��������ˣ�ֱ��д�뻺���У�������߻��Զ�����ͻ������ɾ������
	//// ����ֻ�Ǽ�⣬������ڳ�ͻ����ô������󽫱����ý� delayMap
	//if ( HaveConflict( data ) )
	//{
	//	// ������ڳ�ͻ����ֱ�ӽ������ɫ�������ӳٱ���ӳ����У������Ƿ��Ѿ����ڣ�ֱ�Ӹ�ֵ
	//	return false;
	//}

	// ��Ȼ�����ڳ�ͻ����ô���߹�������Ӧ�ñض��ɹ�
	BOOL result = SaveItemRelation( data, dst, true );
	assert( result );
	return true;
}

BOOL ItemUniqueChecker::AtDoubleSavePlayerIntoCache( SFixData &data1, SFixData &data2, SFixData *dst1, SFixData *dst2 )
{
	return TRUE;

	// �˴�Ŀ�ģ����2�������ϵĵ��߱���������ظ�
	// �������ϵĵ��ߺ�uniqueMapҲ��������ظ���

	// �Ӹ��������л�ȡ��ǰ�ĵ���ӳ���
// 	std::map< QWORD, _DWORD_ONCE > itemSet;
// 
// 	SFixData *data[2] = { &data1, &data2 };
// 	DWORD pid[2] = { data1.m_dwStaticID, data2.m_dwStaticID };
// 	for ( int k = 0; k < 2; k ++ )
// 	{
// 		// �ȴӵ�ǰ���������л�ȡ��Ҫ��������е������ݣ����ұ�֤����߲��ظ�������
// 		SEquipment *equipArray = data[k]->m_Equip;
// 		SPackageItem *itemArray[4] = { data[k]->m_BaseGoods, data[k]->SFixStorage1::m_pStorageGoods,
// 			data[k]->SFixStorage2::m_pStorageGoods, data[k]->SFixStorage3::m_pStorageGoods };
// 
// 		// ������ܻ������ʵ�ĸ���
// 		for ( int i = 0; i < 8; i ++ )
// 			if ( ( equipArray[ i ].wIndex ) && !itemSet[ equipArray[ i ].uniqueId() ].Init( pid[k], i, 0, 0, 1 ) )
// 				return false;
// 
// 		for ( int a = 0; a < 4; a ++ )
// 			for ( int i = 0; i < 64; i ++ )
// 				if ( ( itemArray[ a ][ i ].wIndex ) && !itemSet[ itemArray[ a ][ i ].uniqueId() ].
//                     Init( pid[k], i, itemArray[ a ][ i ].byCellX, itemArray[ a ][ i ].byCellY, 2 + a ) )
// 					return false;
// 	}
// 
// 	// ���ж����е��߲����uniqueMap�ظ�������
// 	// ֮���Բ��� for ��д it ++������ Ҳ�����м���ܻ�ʹ�� erase ���±����жϡ�����
// 	for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); )
// 	{
// 		BindStub *bindStub = uniqueMap.find( it->first );
// 		if ( bindStub == NULL )
//         {
// 			// û�ҵ��Ļ�����Ȼ�����µ��ߡ�����ͨ��
// 			it = itemSet.erase( it );
// 			continue;
// 		}
// 
// 		// �ҵ��Ļ�������Ҫ�ж��ظ��ԡ�����
// 		// �����Լ�����δ�󶨵����ݣ��Ͳ���Ҫ��������ͨ��
//         if ( bindStub->playerId == it->second.value )
// 		{
// 			// ��Ҫ�Ƴ��ظ����֣����������Ҫ�޸ĵ�����
// 			it = itemSet.erase( it );
// 			continue;
// 		}
// 
// 		// ���������ڽ��������ˣ�Ҳ��������ͨ��������
//         if ( bindStub->playerId == pid[0] || bindStub->playerId == pid[1] )
// 		{
// 			it ++;
// 			continue;
// 		}
// 
// 		// �����ǷǷ������ˡ�����Ҳ�����ǶԷ�������δ���档������Ҫ�ȴ�����
// 		// return false;
//         // ���ڲ�����Ƿ����ߣ�erase��㲻���޸ĸõ��ߣ����� SaveItemRelation �ڲ������д���
// 		it = itemSet.erase( it );
// 	}
// 
// 	// ��ɼ�⣬�������ݶ���Ч�����Կ�ʼ�޸Ĺ������ݣ����ڵ���ת�ƵĲ��֣�
// 	for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); it ++ )
//     {
//         BindStub *stub = uniqueMap.find( it->first );
// 
//         // ��Ϊ�ϱ��Ѿ��ų������������ߣ����������Ȼ���ܹ��ػ�����ȡ���ĵ��ߣ�
//         assert( stub != NULL );
//         if ( stub == NULL )
//             continue;
// 
//         assert( stub->isCached );
//         //BindStub &stub = uniqueMap[ it->first ];
// 
//         // ���￪ʼת�Ƶ��ߣ�ע�⣬�������µ����߼���
//         // ��Ϊֱ���޸�stub�ᵼ���߼���ͻ��������˵����
//         // ���������޸�Ϊ��ɾ������Ͱ󶨱��е�ת�Ƶ��ߣ�����ֻɾ����ɫ2�����ݾͿ����ˣ���Ϊ��ɫ1�ȱ��棩
//         // SaveItemRelation �У��ȱ���Ľ�ɫ1���ͷŵ��Լ�ת�Ƴ�ȥ�ĵ��ߣ����ͽ�ɫ2�ṩ�ĵ��߽����󶨹�ϵ
//         if ( stub->playerId == dst2->m_dwStaticID )
//         {
//             assert( stub->where >= 1 && stub->where <= 5 );
//             BOOL deleteOk = FALSE;
//             if ( stub->where == 1 )
//             {
//                 assert( stub->idx < 8 );
//                 SEquipment &item = dst2->m_Equip[ stub->idx ];
//                 assert( item.uniqueId() == stub->UniqueId() );
//                 if ( item.uniqueId() == stub->UniqueId() )
//                 {
//                     memset( &item, 0, sizeof( item ) );
//                     uniqueMap.erase( stub->UniqueId() );
//                     deleteOk = TRUE;
//                 }
//             }
//             else if ( stub->where >= 2 && stub->where <= 5 )
//             {
// 		        SPackageItem *itemArray[4] = { dst2->m_BaseGoods, dst2->SFixStorage1::m_pStorageGoods,
// 			        dst2->SFixStorage2::m_pStorageGoods, dst2->SFixStorage3::m_pStorageGoods };
//                 assert( stub->idx < 64 );
//                 SPackageItem &item = itemArray[ stub->where - 2 ][ stub->idx ];
//                 assert( stub->x == item.byCellX );
//                 assert( stub->y == item.byCellY );
//                 assert( item.uniqueId() == stub->UniqueId() );
//                 if ( item.uniqueId() == stub->UniqueId() )
//                 {
//                     memset( &item, 0, sizeof( item ) );
//                     uniqueMap.erase( stub->UniqueId() );
//                     deleteOk = TRUE;
//                 }
//             }
//             assert( deleteOk );
//         }
// 
//         ////// ע�⣺���ﵼ���˱���ʱ��assert!
//         ////// ��Ϊ֮ǰ���߼������������еĵ����ǺͰ󶨱��еĵ���һ�µ�
//         ////// ��������������ǰ�޸��˰󶨱���δ����޸Ļ�������
//         ////// �������SaveItemRelationʱ������assert
//         ////stub.playerId = it->second.value;
//         ////stub.idx = it->second.idx;
//         ////stub.where = it->second.where;
//         ////stub.x = it->second.x;
//         ////stub.y = it->second.y;
//     }
// 
//     // �����Ҫͨ�� SaveItemRelation �������ӡ�ɾ���ĵ��߸���
// 	// ��Ȼ�����ڳ�ͻ����ô���߹�������Ӧ�ñض��ɹ���
//     BOOL ck1 = SaveItemRelation( data1, dst1, true );
//     BOOL ck2 = SaveItemRelation( data2, dst2, true );
//     assert( ck1 && ck2 );
// 	return ck1 && ck2;
}

static const LPCSTR _WHERE_STRING[16] = { "0","װ����","��Ʒ��","�ֿ�1","�ֿ�2","�ֿ�3","6","7","8" };

void _TypeItemInfo( QWORD id )
{
    {
        UGE::CriticalLock::Barrier4ReadWrite barrier( _g::dpLayer.lock );
        ItemUniqueChecker::BindStub *stub = _g::dpLayer.iuChecker.uniqueMap.find( id );
        if ( stub == NULL )
            rfalse( 2, 1, "û���ҵ�ָ��IDΪ0x%I64x�ĵ��߰���Ϣ\r\n", id );
        else
        {
            assert( stub->UniqueId() == id );
            rfalse( 2, 1, "IDΪ[0x%I64x]�ĵ��ߵİ���Ϣ��\r\n"
                "��ɫID[%d], ����״̬[%s]\r\n"
                "λ��[%s], x/y = [%d/%d], idx = [%d]\r\n", 
                id, stub->playerId, stub->isCached ? "����" : "����",
                _WHERE_STRING[ stub->where ], stub->x, stub->y, stub->idx );
        }
    }
}

void _CheckItemInfo()
{
//     DWORD a[5] = { 0, 0, 0, 0, 0 };
//     DWORD b[2][5] = { { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 } };
//     {
//         UGE::CriticalLock::Barrier4ReadWrite barrier( _g::dpLayer.lock );
// 
//         for ( DataPersistentLayer::CacheMap::iterator it = _g::dpLayer.cacheMap.begin();
//             it != _g::dpLayer.cacheMap.end(); it ++ )
//         {
//             SFixProperty *data = it->second.data;
//             for ( int i = 0; i < 8; i ++ )
//                 if ( data->m_Equip[i].wIndex ) 
//                     a[0] ++;
// 
// 		    SPackageItem *itemArray[4] = { data->m_BaseGoods, data->SFixStorage1::m_pStorageGoods,
// 			    data->SFixStorage2::m_pStorageGoods, data->SFixStorage3::m_pStorageGoods };
// 
//             for ( int c = 0; c < 4; c ++ )
//                 for ( int i = 0; i < 64; i ++ )
//                     if ( itemArray[c][i].wIndex ) 
//                         a[1+c] ++;
//         }
// 
//         ItemUniqueChecker::BindStubHashTable &uniqueMap = _g::dpLayer.iuChecker.uniqueMap;
//         size_t total = uniqueMap.allocArray.size() * ItemUniqueChecker::BindStubHashTable::ALLOCCAPACITY;
//         for ( size_t i=0; i<total; i ++ )
//         {
//             ItemUniqueChecker::BindStub &stub = uniqueMap.select( ( DWORD )i );
//             if ( stub.UniqueId() == 0 )
//                 continue;
// 
//             assert( stub.where >= 1 && stub.where <= 5 );
//             if ( stub.where >= 1 && stub.where <= 5 )
//                 b[!stub.isCached][ stub.where - 1 ]++;
//         }
//             
//         rfalse( 2, 1, "��ɫ�������ܵ���Ч��������Ϊ\r\n"
//             "[���][%d][%d,%d,%d,%d,%d]\r\n"
//             "[����][%d][%d,%d,%d,%d,%d]\r\n"
//             "[����][%d][%d,%d,%d,%d,%d]\r\n",
//             a[0]+a[1]+a[2]+a[3]+a[4], a[0], a[1], a[2], a[3], a[4],
//             b[0][0]+b[0][1]+b[0][2]+b[0][3]+b[0][4], b[0][0], b[0][1], b[0][2], b[0][3], b[0][4],
//             b[1][0]+b[1][1]+b[1][2]+b[1][3]+b[1][4], b[1][0], b[1][1], b[1][2], b[1][3], b[1][4] );
//     }
}

void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber ) 
{
    size_t cacheNumber = _g::dpLayer.cacheMap.size();
    size_t accountNumber = _g::dpLayer.accountMap.size();
    size_t itemBindNumber = _g::dpLayer.iuChecker.uniqueMap.size();
    size_t conflictNumber = 0;
    size_t eventQNumber = _g::dpLayer.queryQueue.size();
    size_t saveQNumber = _g::dpLayer.updateQueue.size();

    LARGE_INTEGER freq;
    ::QueryPerformanceFrequency( &freq );
    size_t commitTimes = _g::dpLayer.commitTimes;
    size_t saveTimes = _g::dpLayer.totalSaveTimes;
    float maxSaveDuration = ( float )( _g::dpLayer.maxSaveDuration / ( double )freq.QuadPart );
    float minSaveDuration = ( float )( _g::dpLayer.minSaveDuration / ( double )freq.QuadPart );
    float avgSaveDuration = _g::dpLayer.totalSaveTimes ? 
        ( ( float )( ( _g::dpLayer.totalSaveTicks / ( double )_g::dpLayer.totalSaveTimes ) / ( double )freq.QuadPart ) ) : 0;

    rfalse( 2, 1, "�����ɫ���� %6d\tƥ���˺����� %6d\r\n"
        "�󶨵������� %6d\t��ͻ��ɫ���� %6d\r\n"
        "��Ϣ�������� %6d\tRPC �������� %6d\r\n"
        "�¼��������� %6d\t����������� %6d\r\n"
        "�ύ������� %6d\tʵ�ʱ������ %6d\r\n"
        "������ߺ�ʱ %.5f ��ͺ�ʱ %.5f ƽ����ʱ %.5f\r\n"
        "��ͻ���ͷֲ� %d\t%d\r\n"
        "�ײ�������Ϣ���� %d",
        cacheNumber, accountNumber,
        itemBindNumber, conflictNumber,
        msgQNumber, rpcQNumber,
        eventQNumber, saveQNumber,
        commitTimes, saveTimes,
        maxSaveDuration, minSaveDuration, avgSaveDuration,
        _g::dpLayer.conf1, _g::dpLayer.conf2, npkQNumber );
}

int _SaveAll( HANDLE quitEvent )
{
    UGE::CriticalLock::Barrier4ReadWrite barrier( _g::dpLayer.lock );

    int nt = 0;

    for ( DataPersistentLayer::CacheMap::iterator it = _g::dpLayer.cacheMap.begin();
        it != _g::dpLayer.cacheMap.end(); it ++ )
    {
        if ( it->second.modified && it->second.data )
        {
            // ֻ����,���������ٶ���,��Ϊ���ٵ�ͬʱ����ķ���������״̬Ҳ�������...
            // it->second.data->m_byStoreFlag = SFixProperty::ST_LOGOUT;
            // it->second.removeMe = true;

            DataPersistentLayer::UpdateEvent ue = { it->second.data->m_dwStaticID, it->second.serverId };
	        _g::dpLayer.updateQueue.push( ue );
            nt ++;
        }
    }

    // ���˳��¼����
    _g::dpLayer.quitEvent = quitEvent;

    return nt;
}
SectionMessageManager::SectionMessageManager()
{
	m_id = DBSERVER;
}
void SectionMessageManager::sendSectionMessage(DNID dnidClient,SSectionMsg* msg)
{
	SendMessage(dnidClient,msg,sizeof(SSectionMsg));
}