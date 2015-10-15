#include "stdafx.h"
#include "dpl.h"
#include "LITESERIALIZER/Lite.h"
#include "networkmodule/logtypedef.h"
#include "networkmodule/playertypedef.h"
#include "networkmodule/datamsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "time.h"
#include "../区域服务器/GameObjects/CPlayerDefine.h"
#include "../区域服务器/GameObjects/CPlayerService.h"

extern BOOL TryDecoding_NilBuffer( const void * src_data, size_t src_size, void *&dst_data, size_t &dst_size );

const static char *CONST_TITLES[] = { "少林弟子", "峨嵋弟子", "武当弟子", "圣火弟子","星宿弟子" };

struct PlayerInitProperty
{
	int HP;
	int MP;
	int SP;
	int HPBottleIdx;		// 回血药的索引
	int MPBottleIdx;		// 回蓝药的索引
};

static PlayerInitProperty _s_PlayerInitProperty;

BOOL LoadPlayerInitProperty()
{
	IniFiles IniFile;
    if (!IniFile.open("player_init_property.ini"))
        return rfalse(0, 0, "无法打开配置文件player_init_property.ini");

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
    // 保证该逻辑的前提：
    // updateQueue中的对象删除只会发生在 PersistentThread 中，对象添加则无限制
    // updateQueue可以存在冗余事件，但map中的对象必定是唯一存在的
    // 更新事件可以超过对象数量，但每个对象必定存在一个或以上的更新事件
    
    // 关于校验标记：其实也就是时间戳
    // 在提交更新时，CacheMap中的时间戳被更新为提交时间
    // 在更新线程中，更新事件中时间戳为最后一次更新时间
    // 在强制更新状态下，该CacheMap被修改的同时，会额外附加一个更新事件到队列最前面，
    // 更新线程直接取到最新的事件，判断：更新时间在提交时间之后
    DataPersistentLayer *_this = ( DataPersistentLayer* )handle->param;

	// 由于这个设定，必须要保证这个函数不会被重入！！！
	static SFixData	basedata;
	static SWareHouses			warehouse;

    while ( ( handle->breakFlag == 0 && _this->quitEvent == NULL ) || _this->updateQueue.size() )
    {
        // 循环事件只会出现在正常渠道中
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
            // 从更新队列中取出数据对象
            if ( !_this->updateQueue.empty() ) 
            {
                // 先获取更新事件，并弹出队列
                ue = _this->updateQueue.front();
                _this->updateQueue.pop();

                // 是否是备份消息！
                if ( _this->atBackup && ue.uniqueId == 0 && ue.serverId == 0 )
                {
                    // 启动备份！！！
                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().Backup( _this->path.c_str(), _this->title.c_str() );
                    *_this->atBackup = 0;
                    _this->atBackup = NULL;
                    goto __retry;
                }

                // 通过关联的uid到cacheMap中查找缓存单元
                // 这里有可能找不到目标，说明目标已经被提前销毁
                // （该情况会出现在立即保存并销毁对象的情况下）
                // 则丢弃该事件并继续获取有效数据

                CacheMap::iterator it = _this->cacheMap.find( ue.uniqueId );
                if ( it == _this->cacheMap.end() ) 
                    goto __retry;

                // 如果对象有效，则获取的同时加锁，避免被其他线程抢占甚至提前销毁
                cu = it->second;

				// 判断以下：
				// 如果该角色不是需要立即保存下线（或服务器关闭，或备份状态），并且和上一次保存时间比较相差很短（小于30分钟），则不考虑进一步保存！
				int saveMargin = abs( ( int )( timeGetTime() - it->second.timeStamp ) );
                if ( !cu.removeMe && _this->quitEvent == NULL && _this->atBackup == 0 )
                    if ( !cu.modified || ( cu.modified && ( saveMargin < ( 1000 * 60 * 30 ) ) ) )
                        goto __retry;

				// 如果当前事件属于立即更新时附加的临时事件，则不作任何处理，否则添加回更新队列首部
                // 如果在提交更新时对象已经处于立即更新状态，则说明在近期已经存在了附加更新事件
                // 所以这时不再额外附加事件，而仅仅修改数据即可（重复附加会导致存在2个额外的事件，其中一个将无法被销毁）
                //if ( cu.immediate ) 
                //    it->second.immediate = false;
                //else
                //{
                //    ue.timeStamp = timeGetTime();
                //    _this->updateQueue.push( ue );
                //}

                // 如果是更新后销毁，则不在这里处理，因为会导致过长的阻塞，所以在更新完毕后才销毁对象
                // 如果提交更新时，对象存在并且已经是处于销毁状态，则说明该对象还未被彻底删除，所以只需取消removeMe即可
				
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

                    // 已经获取数据用于保存,修改状态
                    it->second.modified = false;
				}

                haveData = true;
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////

        // 如果没有数据需要更新，则休眠后再次尝试
        if ( !haveData ) { Sleep( 10 ); continue; }

        BOOL result = true;

        LARGE_INTEGER lib, lie;
        lib.QuadPart = 0;
        lie.QuadPart = 0;

        // 保持连接
        if ( !reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().KeepAlive( 0, "持久层" ) )
            result = false; // 如果保持连接失败，则只能放弃保存
        else
        {
        // 有数据的情况下，需要更新保存
        if ( cu.data != NULL && cu.modified )
        {
            ::QueryPerformanceCounter( &lib );

		    result = reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().
			    SavePlayerData( cu.account.c_str(), ue.serverId, basedata, &warehouse );
            if ( !result )
            {
                // 如果保存失败，则说明某些地方出问题了，那么就应该继续呆在缓存里，不予释放
            }
            else
            {
                ::QueryPerformanceCounter( &lie );
            }
        }

        // 由于写数据只会有当前这么一个线程，所以不用加锁！
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
        // 如果该对象需要被销毁，即需要再次处理cacheMap
        if ( result && cu.removeMe )
        {
            UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

            CacheMap::iterator it = _this->cacheMap.find( ue.uniqueId );
            // 需要再次确定数据未被修改,并且确实需要销毁
            if ( it != _this->cacheMap.end() && it->second.removeMe && !it->second.modified ) 
			{
		        // 在玩家退出游戏时，做完保存之后要将玩家解除锁定
                if ( it->second.data != NULL )
                {
		            if ( it->second.data->m_byStoreFlag != SFixData::ST_LOGOUT )
                        continue;

                    reinterpret_cast< DataPersistentLayer4XYD1* >( _this )->GetQuery().
                        UpdateAccountLock( cu.account.c_str(), ue.serverId, FALSE );

                    // 并且需要修改道具的绑定状态（ isCahced = false ）
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
                // 保存失败，需要将修改标记改回来
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
            // 这个地方存在一个逻辑漏洞
            // 如果玩家正在游戏当中，但是通过异常手段发送了一个解除绑定的消息过来
            // 玩家的数据就被清除了，然后他在重新获取角色数据到其他服，那么这个旧的数据就会覆盖游戏中新的数据

            // 由于操作到了缓存，直接加锁处理
	        UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock );

            DataPersistentLayer::AccountMap::iterator it1 = _this->accountMap.find( account );
            if ( it1 != _this->accountMap.end() )
            {
                DataPersistentLayer::CacheMap::iterator it2 = _this->cacheMap.find( it1->second );
                // 这里是一个特殊处理，也就是认为：包含了角色数据的就一定是存在游戏环境中的数据，所以不能轻易解除绑定
                // 2008-6-16 添加serverId的判断，避免不是源服务器，但给出了注销请求导致角色数据异常的错误
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

        // 在获取角色列表数据时，如果该帐号已经被绑定在缓存中，则不能继续往下执行
        {
            // 由于操作到了缓存，不使用CheckAccountExist，而直接加锁处理
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
                    return ; // 该角色还有效存在时请求获取角色列表……不允许这种状况出现
                }
                OutputDebugString( "帐号列表和缓存列表不匹配！！！" );
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

//初始化
bool DataPersistentLayer4XYD1::Init( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName)
{
	return m_Query.Init( UserName, PassWord, DBIP, DBName );
}

//循环处理数据
int DataPersistentLayer4XYD1::LoopService( ThreadHandle *tHandle )
{
    if ( !queryQueue.empty() )
    {
        Element element;
        // 这里保证优先处理所有的外部提交请求
        while ( queryQueue.mtsafe_get_and_pop( element ) )
        {
            // 这里可能存在的外部提交请求就只有4个，获取列表、新建角色、获取角色、删除角色
            // 假如数据库连接失败，则直接返回好了。
            if ( !m_Query.KeepAlive( 0, "持久层扩展" ) )
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
        // 在创建角色数据时，如果该帐号已经被绑定在缓存中，则不能继续往下执行
        if ( _this->CheckAccountExist(account))
            return;

		SACreatePlayerMsg Acreatemsg;					// 如果某个位的角色不存在，那么它相应位上的数据就为空
		
		Acreatemsg.qwSerialNo = 0;	
		//Acreatemsg.dnidClient = data.m_Exp;				// 获取GID
		//data.m_Exp = 0;									// 回复经验
        data.m_version = SFixData::GetVersion();	// 设置版本

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
// 		// 在获取角色数据时，如果该帐号已经被绑定在缓存中，则不能继续往下执行
// 		if (_this->CheckAccountExist(account))
// 		{
// 			// 这里可能还不能添加容错（如果角色已经在游戏当中，玩家重复请求后，会导致数据被覆盖）
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

        // 在获取角色数据时，如果该帐号已经被绑定在缓存中，则不能继续往下执行
        if (_this->CheckAccountExist(account))
        {
            // 这里可能还不能添加容错（如果角色已经在游戏当中，玩家重复请求后，会导致数据被覆盖）
            return;
        }

		// 从数据库获取数据，并更新到缓存
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
		// 先要保证这个角色不存在于缓存中
		int deleteResult = 0;

        // 在删除角色数据时，如果该帐号已经被绑定在缓存中，则不能继续往下执行
        if (!_this->CheckAccountExist(account))
        {
			UGE::CriticalLock::Barrier4ReadWrite barrier( _this->lock);
			DataPersistentLayer4XYD1::CacheMap::iterator it = _this->cacheMap.find(playerId);
			if (it != _this->cacheMap.end())
				return;

			// 由于需要保证在删除过程中，不会有其他逻辑加载角色，所以就将数据库操作也放进临界区了
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
			
// 			// 发消息,发送到登陆服务器，然后区域服务器直接从LogServer拉取数据
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
// 创建新角色
void DataPersistentLayer4XYD1::NewPlayer(DNID dnidClient, DWORD serverId, LPCSTR account, DWORD index, SFixData &data, _g::_p::CallBack4CreatePlayer callback)
{
    queryQueue.mtsafe_push(new Query4NewPlayer(this, dnidClient, serverId, account, index, data, callback));
}

// 获取帐号对应的角色列表
void DataPersistentLayer4XYD1::GetChList(DNID dnidClient, DWORD serverId, LPCSTR account, DNID RetDnid, _g::_p::Callback4QueryPlayerList callback)
{
    queryQueue.mtsafe_push(new Query4QueryPlayerList(this, dnidClient, serverId, account, RetDnid, callback));
}

// 获取角色数据
void DataPersistentLayer4XYD1::GetPlayerData( DNID dnidClient, DWORD serverId, LPCSTR account,  BYTE byIndex, DNID RetDnid,  _g::_p::Callback4QuerySelectPlayer callback )
{
    queryQueue.mtsafe_push( new Query4SelectPlayer(this, dnidClient, serverId, account, byIndex, RetDnid, callback));
}

// 删除角色
void DataPersistentLayer4XYD1::DeletePlayer(DNID dnidClient, DWORD serverId, LPCSTR account, DWORD playerId, DNID retDnid)
{
    queryQueue.mtsafe_push(new Query4DeletePlayer(this, dnidClient, serverId, account, playerId, retDnid));
}

/*
// 更新角色数据
void DataPersistentLayer4XYD1::UpdateData( DWORD serverId, LPCSTR account )
{
    queryQueue.mtsafe_push( new Query4UpdatePlayer( this, dnidClient, serverId, account, byIndex, RetDnid, callback ) );
}
*/

// 特殊处理，用于数据扩展
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
	// 由于操作到了缓存，直接加锁处理
	UGE::CriticalLock::Barrier4ReadWrite barrier(lock);

	// 需要预先检测该角色是否已经存在于缓存中了，如果是的话，说明某个地方出现了异常
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
	cu.modified		= fromGame;		// 如果是从游戏中来的数据，就应该保存！
	cu.removeMe		= 0;
	cu.timeStamp	= timeGetTime();

	return true;
}

BOOL DataPersistentLayer::SaveIntoCache(DWORD serverId, SFixData &data, LPCSTR account)
{
    // 游客的数据是不需要处理的（因为这里的acc来自于一个有效的buffer，所以直接使用其前4个字节进行比较）
    if (*(LPDWORD)account == *(LPDWORD)"游客")
        return false;

	UGE::CriticalLock::Barrier4ReadWrite barrier(lock);

    commitTimes ++;

	// 先要确定这是一个已经被缓存的数据
    DataPersistentLayer::CacheMap::iterator it = cacheMap.find(data.m_dwStaticID);
    if (it == cacheMap.end())
    {
        // 这里需要特殊处理，添加容错逻辑（数据库服务器可能会被单独意外关闭）
        // 注意，LoadIntoCache 里有一个重复锁
        if (!LoadIntoCache( serverId, account, data, true))
		    return false;

        it = cacheMap.find( data.m_dwStaticID );
        if ( it == cacheMap.end() )
            return false;
    }

    // 排除未加载数据就出现保存的逻辑！
    if ( it->second.data == NULL )
        return false;

    CacheUnit &cu = it->second;
	if ( !iuChecker.AtSavePlayerIntoCache( data, cu.data ) )
    {
        conf1 ++;
		return false;
    }

	cu.modified = true;												    // 标识数据已经被修改!
	cu.removeMe = ( data.m_byStoreFlag == SFixData::ST_LOGOUT );    // 是否需要尽快保存进数据库并解除锁定！！！

	UpdateEvent ue = { data.m_dwStaticID, serverId };
	updateQueue.push( ue );

	return true;
}

BOOL DataPersistentLayer::DoubleSaveIntoCache( DWORD serverId, SFixData &d1, SFixData &d2, LPCSTR acc1, LPCSTR acc2 )
{
    // 游客的数据是不需要处理的！（因为这里的acc来自于一个有效的buffer，所以直接使用其前4个字节进行比较）
    if ( *( LPDWORD )acc1 == *( LPDWORD )"游客" )
        return SaveIntoCache( serverId, d2, acc2 );
    else if ( *( LPDWORD )acc2 == *( LPDWORD )"游客" )
        return SaveIntoCache( serverId, d1, acc1 );

	UGE::CriticalLock::Barrier4ReadWrite barrier( lock );

    commitTimes += 2;

    DataPersistentLayer::CacheMap::iterator it1 = cacheMap.find( d1.m_dwStaticID );
    DataPersistentLayer::CacheMap::iterator it2 = cacheMap.find( d2.m_dwStaticID );
    if ( it1 == cacheMap.end() || it2 == cacheMap.end() )
    {
        // 容错处理
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

    // 排除未加载数据就出现保存的逻辑！
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
	cu2.removeMe = d2.m_byStoreFlag == SFixData::ST_LOGOUT;	// 是否需要尽快保存进数据库并解除锁定！！！

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
        // 打备份事件标志
        if ( atBackup ) 
            *atBackup = 1;

        _g::dpLayer.title = title;
        _g::dpLayer.path = bakPath;
        _g::dpLayer.atBackup = atBackup;

        int _SaveAll( HANDLE quitEvent );
        int ck = _SaveAll( NULL );

        // 向底层通知备份事件！
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

        // 将请求提交到线程里执行，这样可以解决操作时间过长导致主逻辑阻塞的问题
		dpLayer.GetChList(dnidClient, pMsg->dwServerId, account, pMsg->dnidClient, callback);
    }

	void _p::SetPlayerInitData(SFixData &fixproperty)
	{
		fixproperty.m_bNewPlayer = true;
		fixproperty.m_byStoreFlag = SFixData::ST_LOGIN;
		fixproperty.m_EnduranceData.m_dwEndurance = 90;
		fixproperty.m_dwLevel = 1;


		//新建武将装备初始化
		SHeroData tempHeroData;
		memset(&tempHeroData, 0, sizeof(SHeroData));
		SHeroEquipment tempEquip;
		memset(&tempEquip, 0, sizeof(SHeroEquipment));
		/*for (int i = 0; i < EQUIP_MAX; i++)
			memcpy(&tempHeroData.m_Equipments[i], &tempEquip, sizeof(SEquipment));*/
			
		//新建武将属性初始化
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
        // 新版本可支持传过来的压缩数据
        LPDWORD dataBuf = reinterpret_cast< LPDWORD >( &pMsg->PlayerData );
        LPVOID outBuf = NULL;
        size_t outSize = 0;
        if ( ( *dataBuf < sizeof( pMsg->PlayerData ) ) && TryDecoding_NilBuffer( &dataBuf[1], *dataBuf, outBuf, outSize ) )
        {
            // 数据压缩版本，包括帐号保存的格式也一起变化了！
            size_t adSize = ( ( *dataBuf >> 2 ) + 1 ); // 获取对齐后的帐号起始地址！
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
            // 未压缩的版本，数据呈简单结构化。。。
		    try
		    {
			    lite::Serialreader ls( pMsg->streamData );
			    LPCSTR account = ls();

			    // 这里并不直接将数据写入数据库，而只是检测道具唯一性冲突，并将数据更新入缓存。。。
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
        // 新版本添加pMsg数据压缩支持！
        // 结构说明：前4字节为数据压缩后长度，其后8字节为数据压缩标志，再往后为压缩数据！
        LPDWORD dataBuf = reinterpret_cast< LPDWORD >( pMsg->data );
        LPVOID outBuf = NULL;
        size_t outSize = 0;
        if ( ( *dataBuf < sizeof( pMsg->data ) ) && TryDecoding_NilBuffer( &dataBuf[1], *dataBuf, outBuf, outSize ) )
        {
            assert( ( outSize == sizeof( SFixData ) ) || ( outSize == sizeof( SFixData ) * 2 ) );
            if ( ( outSize == sizeof( SFixData ) ) || ( outSize == sizeof( SFixData ) * 2 ) )
            {
                // 此为压缩数据，并且解码成功。。。
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
            // 非压缩数据，按原样保存！
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

	// 处理装备栏中的道具
	SEquipment *equipArray = data.m_Equip;
	for ( int i = 0; i < 8; i ++ )
	{
		SEquipment &item = equipArray[ i ];

		// 如果该项道具无效，则自然不需要进一步检测
		if ( item.wIndex == 0 )
			continue;
		
		// 如果道具存根已经被链接到其他角色，那么说明道具存在冲突，禁止登陆
        BindStub *stub = uniqueMap.find( item.uniqueId() );

        // 检测思路，如果这个道具存在，并且绑定角色不是自己。。。（装备栏道具就只有这个判断）
        if ( stub != NULL && stub->playerId != playerId )
			return true;
	}

	// 处理道具栏，仓库中的道具，因为都是64个格子，所以可以统一处理
	SPackageItem *itemArray[4] = { data.m_pGoods, data.SFixStorage1::m_pStorageGoods,
		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };

	// 处理道具栏中的道具
	for ( int a = 0; a < 4; a ++ )
	{
		for ( int i = 0; i < 64; i ++ )
		{
			SPackageItem &item = itemArray[ a ][ i ];

			// 如果该项道具无效，则自然不需要进一步检测
			if ( item.wIndex == 0 )
				continue;
			
		    // 如果道具存根已经被链接到其他角色，那么说明道具存在冲突，禁止登陆
            BindStub *stub = uniqueMap.find( item.uniqueId() );

            // 检测思路，如果这个道具存在，并且绑定角色不是自己，并且这个道具正好是缓存中的道具或非仓库道具
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

// 这个函数的新版本反过来做itemSet从更新数据构建，可以在一开始保证道具在同一玩家身上不重复
BOOL ItemUniqueChecker::SaveItemRelation( SFixData &data, SFixData *raw, BOOL fromGame )
{
	// XYD3 修改，道具检测机制还没看明白，所以就直接保存了
	if ( raw != NULL )
		*raw = data;

	return TRUE;

	// 保存道具关联，这里有一个必须要遵循的规则，就是需要将原本已经建立好关联的道具解除绑定。。。
	// 因为，如果保存时，只处理身上存在的道具，则那些已经销毁，或转移的道具的关联不会被解除。。。
	// 然后，假如其他人拿到了这个道具，那么会因为关联冲突而导致异常。。。

	// 从更新数据中获取当前的道具映射表
// 	std::map< QWORD, _DWORD_ONCE > itemSet;
// 	DWORD playerId = data.m_dwStaticID;
// 
// 	// scorp
// 	{
// 		// 先从当前保存数据中获取需要处理的所有道具数据，并且保证其道具不重复。。。
// 		SEquipment *equipArray = data.m_Equip;
// 		SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 			data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
//         LPCSTR info = ( raw == NULL ) ? "初始化时发现自身重复道具 [%d][%d][%d][0x%I64x]" : "保存时发现自身重复道具 [%d][%d][%d][0x%I64x]";
// 
// 		// 这里可能会检测出真实的复制
// 		for ( int i = 0; i < 8; i ++ )
//         {
// 			if ( ( equipArray[ i ].wIndex ) && !itemSet[ equipArray[ i ].uniqueId() ].Init( playerId, i, 0, 0, 1 ) )
//             {
//                 // 现在是直接删除复制道具，而不是失败返回
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
// 	// 如果不是读取初始化的话，就需要和旧数据对比，找出需要解除关联的部分
// 	if ( raw != NULL )
// 	{
// 		// 处理装备栏中的道具
// 		// 处理道具栏，仓库中的道具，因为都是64个格子，所以可以统一处理
// 		SEquipment *equipArray = raw->m_Equip;
// 		SPackageItem *itemArray[4] = { raw->m_BaseGoods, raw->SFixStorage1::m_pStorageGoods,
// 			raw->SFixStorage2::m_pStorageGoods, raw->SFixStorage3::m_pStorageGoods };
// 
// 		for ( int i = 0; i < 8; i ++ )
// 		{
// 			SEquipment &item = equipArray[ i ];
// 
// 			// 如果该项道具无效，则自然不需要进一步检测
// 			if ( item.wIndex == 0 )
// 				continue;
// 
// 			// 因为是从缓存中获取数据，并且规划为缓存和映射表数据一致，则作以下断言
// 			BindStub *bindStub = uniqueMap.find( item.uniqueId() );
//             assert( bindStub != NULL && bindStub->playerId == playerId );
//             if ( bindStub == NULL || bindStub->playerId != playerId )
//             {
//                 TraceInfo_C( "CheckError.txt", "在缓存中意外的发现了冲突道具[%d][%d][%d][%d][0x%I64x]", 
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
// 				// 在新表中未找到，属于删除部分，需要解除绑定
//                 uniqueMap.erase( item.uniqueId() );
// 			}
// 			else
// 			{
// 				// 找到，说明该道具的所属未变化，需要从map中移除，以便最后找出新增的部分。。。
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->x = it->second.x; 
// 			    bindStub->y = it->second.y;
//                 bindStub->where = it->second.where;
//                 itemSet.erase( it );
// 			}
// 		}
// 
// 		// 处理道具栏中的道具
// 		for ( int a = 0; a < 4; a ++ )
// 		{
// 			for ( int i = 0; i < 64; i ++ )
// 			{
// 				SPackageItem &item = itemArray[ a ][ i ];
// 
// 				// 如果该项道具无效，则自然不需要进一步检测
// 				if ( item.wIndex == 0 )
// 					continue;
// 
// 				// 因为是从缓存中获取数据，并且规划为缓存和映射表数据一致，则作以下断言
// 				BindStub *bindStub = uniqueMap.find( item.uniqueId() );
// 				assert( bindStub != NULL && bindStub->playerId == playerId );
//                 if ( bindStub == NULL || bindStub->playerId != playerId )
//                 {
//                     TraceInfo_C( "CheckError.txt", "在缓存中意外的发现了冲突道具[%d][%d][%d][%d][0x%I64x]", 
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
// 				    // 在新表中未找到，属于删除部分，需要解除绑定
//                     uniqueMap.erase( item.uniqueId() );
// 				}
// 				else
// 				{
// 					// 找到，说明该道具的所属未变化，需要从map中移除，以便最后找出新增的部分。。。
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
// 	// 最后还留在itemSet中的道具，即属于该角色新获得的道具。需要检测冲突
// 	if ( !itemSet.empty() )
//     {
//         LPCSTR info = ( raw == NULL ) ? "初始化时发现重复道具 [%d][%d][%d][%d][0x%I64x]" : "保存时发现重复道具 [%d][%d][%d][%d][0x%I64x]";
// 
// 	    for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); it ++ )
// 	    {
// 		    // 这里的这些道具应该是新创建的，或者是从其他角色那儿获得的。。。
//     	    BindStub *bindStub = uniqueMap.find( it->first );
// 		    if ( bindStub == NULL )
//             {
//                 // 这里的属于新建道具，需要初始化绑定信息
//                 bindStub = &uniqueMap[ it->first ];
// 		        bindStub->playerId = playerId;
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->where = it->second.where;
//                 bindStub->x = it->second.x;
//                 bindStub->y = it->second.y;
//             }
// 
//             // 这里是由于玩家下线推出后，重新回到游戏，需要将他的那些道具进行重新绑定！
//             // 这里有一个前提是：玩家假如到其他游戏区域(一拖多的情况下)，删除了道具或交易了道具，
//             // 那这里的绑定数据会由于未同步而导致目标丢失
//             else if ( raw == NULL && !bindStub->isCached )
//             {
//                 // 这个道具是非缓存道具，逻辑上可以被新的读取操作覆盖！但最终结果，只保持一份数据在缓存中！
//                 // 所以这里不判断 playerId 而是直接覆盖！
// 		        bindStub->playerId = playerId;
//                 bindStub->isCached = true;
//                 bindStub->idx = it->second.idx; 
//                 bindStub->where = it->second.where;
//                 bindStub->x = it->second.x;
//                 bindStub->y = it->second.y;
//             }
// 		    else
//             {
//                 // 出现在这里的，就只可能是复制了。。。
//                 // 有2个情况：读取初始化和数据保存，读取时，如果出现复制，则不应该让玩家登陆（需要反向清除所有关联绑定数据）！
//                 // 而保存时，则直接删除冲突道具！
//                 assert( bindStub->playerId != playerId );
//                 assert( bindStub->where >= 1 && bindStub->where <= 5 );
// 
//                 if ( raw == NULL && fromGame == false )
//                 {
//                     // 返回失败，不允许玩家登陆！
//                     AtRemovePlayerFromCache( data );
//                     return false;
//                 }
// 
//                 // （冲突源还在时，直接删除冲突源道具，否则删除冲突目标道具）
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
//                     // 拥有 isCached 标志，必然代表角色还在缓存中
//                     assert( cu != NULL && cu->data != NULL );
// 
//                     // 容错！
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
//                     // 容错检测
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
//                 // 这里应该必然删除成功的
//                 assert( deleteOk );
// 
//                 // 如果删除的是对方的数据，那么需要打上标记，避免在最后进程关闭的时候，没有重新写入，而导致复制冲突！
//                 if ( cu && deleteOk )
//                     cu->modified = true;
// 
//                 TraceInfo_C( "CheckError.txt", "保存数据时发现了冲突道具[%d][%d[%d,%d,%d]][%d[%d,%d,%d]][%d][0x%I64x]", 
//                     itemIndex, 
//                     data.m_dwStaticID, it->second.where, it->second.x, it->second.y,
//                     bindStub->playerId, bindStub->where, bindStub->x, bindStub->y, 
//                     dst->m_dwStaticID, it->first );
// 
//                 // 如果删除成功，并且删除的是冲突源，则需要修改绑定数据到新的角色身上！
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
// 	// 在最末直接保存数据到缓存目标中！
// 	if ( raw != NULL )
// 		*raw = data;
// 
// 	return true;
}

BOOL ItemUniqueChecker::AtRemovePlayerFromCache( SFixData &data )
{
	return TRUE;

    // 该函数主要用于在角色从缓存中清除后，需要同时将仓库的道具解除关联！
//     DWORD playerId = data.m_dwStaticID;
// 
// 	SEquipment *equipArray = data.m_Equip;
// 	SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
// 	// 这里可能会检测出真实的复制
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
// 注意：这个逻辑中，关联数据的处理应该是，检测和保存独立分开，
// 如果一部分关联建立成功，一部分又失败的话，那么缓存中的数据和唯一性关联表就不能保证一致性，
// 反而导致复杂度增加，结果也会出错。。
// ####################################################################################################
BOOL ItemUniqueChecker::AtLoadPlayerFromDatabase( SFixData &data, BOOL fromGame )
{
	// XYD3 暂时
	BOOL rt = SaveItemRelation( data, NULL, fromGame );
	assert( rt );
	return rt;

	// 直接加一把大锁。。。不考虑性能问题
	// UGE::CriticalLock::Barrier4ReadWrite barrier( bigLocker );

	// 这里只是检测，如果存在冲突，需要做冲突处理
// 	DWORD playerId = data.m_dwStaticID;
// 
// 	// 处理装备栏中的道具
// 	SEquipment *equipArray = data.m_Equip;
// 	for ( int i = 0; i < 8; i ++ )
// 	{
// 		SEquipment &item = equipArray[ i ];
// 
// 		// 如果该项道具无效，则自然不需要进一步检测
// 		if ( item.wIndex == 0 )
// 			continue;
// 		
// 		// 如果道具存根已经被链接到其他角色，那么说明道具存在冲突，禁止登陆
//         BindStub *stub = uniqueMap.find( item.uniqueId() );
// 
//         // 检测思路，如果这个道具存在，并且绑定角色不是自己，同时这个道具还存在于缓存中！
//         if ( stub != NULL && stub->playerId != playerId && stub->isCached )
//         {
//             if ( fromGame )
//                 TraceInfo_C( "CheckError.txt", "从游戏初始化缓存时发现重复道具 %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//             else
//             {
//                 TraceInfo_C( "CheckError.txt", "读取时发现重复道具 %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                 memset( &item, 0, sizeof( SEquipment ) );
// 			    //return false;
//             }
//         }
// 	}
// 
// 	// 处理道具栏，仓库中的道具，因为都是64个格子，所以可以统一处理
// 	SPackageItem *itemArray[4] = { data.m_BaseGoods, data.SFixStorage1::m_pStorageGoods,
// 		data.SFixStorage2::m_pStorageGoods, data.SFixStorage3::m_pStorageGoods };
// 
// 	// 处理道具栏中的道具
// 	for ( int a = 0; a < 4; a ++ )
// 	{
// 		for ( int i = 0; i < 64; i ++ )
// 		{
// 			SPackageItem &item = itemArray[ a ][ i ];
// 
// 			// 如果该项道具无效，则自然不需要进一步检测
// 			if ( item.wIndex == 0 )
// 				continue;
// 			
// 		    // 如果道具存根已经被链接到其他角色，那么说明道具存在冲突，禁止登陆
//             BindStub *stub = uniqueMap.find( item.uniqueId() );
// 
//             // 检测思路，如果这个道具存在，并且绑定角色不是自己，同时这个道具还存在于缓存中！
//             if ( stub != NULL && stub->playerId != playerId && stub->isCached )
//             {
//                 if ( fromGame )
//                     TraceInfo_C( "CheckError.txt", "从游戏初始化缓存时发现重复道具 %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                 else
//                 {
//                     TraceInfo_C( "CheckError.txt", "读取时发现重复道具 %d[0x%I64x][%d]", data.m_dwStaticID, item.uniqueId(), item.wIndex );
//                     memset( &item, 0, sizeof( SPackageItem ) );
// 			        //return false;
//                 }
//             }
//         }
// 	}
// 
// 	// 既然不存在冲突，那么保存应该必定成功！
//     BOOL result = SaveItemRelation( data, NULL, fromGame );
//     assert( result );
//     return result;
}

BOOL ItemUniqueChecker::AtSavePlayerIntoCache( SFixData &data, SFixData *dst )
{
	// 直接加一把大锁。。。不考虑性能问题
	// UGE::CriticalLock::Barrier4ReadWrite barrier( bigLocker );

    // 现在也不作检测了，直接写入缓存中，但是里边会自动将冲突道具作删除处理
	//// 这里只是检测，如果存在冲突，那么这个对象将被放置进 delayMap
	//if ( HaveConflict( data ) )
	//{
	//	// 如果存在冲突，则直接将这个角色放置于延迟保存映射表中，不管是否已经存在，直接赋值
	//	return false;
	//}

	// 既然不存在冲突，那么道具关联保存应该必定成功
	BOOL result = SaveItemRelation( data, dst, true );
	assert( result );
	return true;
}

BOOL ItemUniqueChecker::AtDoubleSavePlayerIntoCache( SFixData &data1, SFixData &data2, SFixData *dst1, SFixData *dst2 )
{
	return TRUE;

	// 此处目的，检测2个人身上的道具本身不会出现重复
	// 并且身上的道具和uniqueMap也不会出现重复！

	// 从更新数据中获取当前的道具映射表
// 	std::map< QWORD, _DWORD_ONCE > itemSet;
// 
// 	SFixData *data[2] = { &data1, &data2 };
// 	DWORD pid[2] = { data1.m_dwStaticID, data2.m_dwStaticID };
// 	for ( int k = 0; k < 2; k ++ )
// 	{
// 		// 先从当前保存数据中获取需要处理的所有道具数据，并且保证其道具不重复。。。
// 		SEquipment *equipArray = data[k]->m_Equip;
// 		SPackageItem *itemArray[4] = { data[k]->m_BaseGoods, data[k]->SFixStorage1::m_pStorageGoods,
// 			data[k]->SFixStorage2::m_pStorageGoods, data[k]->SFixStorage3::m_pStorageGoods };
// 
// 		// 这里可能会检测出真实的复制
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
// 	// 再判断所有道具不会和uniqueMap重复。。。
// 	// 之所以不在 for 上写 it ++。。。 也就是中间可能会使用 erase 导致遍历中断。。。
// 	for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); )
// 	{
// 		BindStub *bindStub = uniqueMap.find( it->first );
// 		if ( bindStub == NULL )
//         {
// 			// 没找到的话，当然属于新道具。。。通过
// 			it = itemSet.erase( it );
// 			continue;
// 		}
// 
// 		// 找到的话，就需要判断重复性。。。
// 		// 属于自己或是未绑定的数据，就不需要处理。。。通过
//         if ( bindStub->playerId == it->second.value )
// 		{
// 			// 需要移除重复部分，最后留下需要修改的数据
// 			it = itemSet.erase( it );
// 			continue;
// 		}
// 
// 		// 这里是属于交换道具了，也可以允许通过。。。
//         if ( bindStub->playerId == pid[0] || bindStub->playerId == pid[1] )
// 		{
// 			it ++;
// 			continue;
// 		}
// 
// 		// 最后就是非法道具了。。。也可能是对方的数据未保存。。。需要等待。。
// 		// return false;
//         // 现在不处理非法道具（erase后便不会修改该道具），由 SaveItemRelation 内部来进行处理
// 		it = itemSet.erase( it );
// 	}
// 
// 	// 完成检测，所有数据都有效，所以开始修改关联数据（属于道具转移的部分）
// 	for ( std::map< QWORD, _DWORD_ONCE >::iterator it = itemSet.begin(); it != itemSet.end(); it ++ )
//     {
//         BindStub *stub = uniqueMap.find( it->first );
// 
//         // 因为上边已经排除掉了新增道具，所以这里必然是能够重缓存中取出的道具！
//         assert( stub != NULL );
//         if ( stub == NULL )
//             continue;
// 
//         assert( stub->isCached );
//         //BindStub &stub = uniqueMap[ it->first ];
// 
//         // 这里开始转移道具，注意，现在重新调整逻辑！
//         // 因为直接修改stub会导致逻辑冲突（见后面说明）
//         // 所以现在修改为：删除缓存和绑定表中的转移道具（这里只删除角色2的数据就可以了，因为角色1先保存）
//         // SaveItemRelation 中，先保存的角色1会释放掉自己转移出去的道具，并和角色2提供的道具建立绑定关系
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
//         ////// 注意：这里导致了保存时的assert!
//         ////// 因为之前的逻辑是期望缓存中的道具是和绑定表中的道具一致的
//         ////// 但是由于这里提前修改了绑定表，而未配合修改缓存数据
//         ////// 结果导致SaveItemRelation时出现了assert
//         ////stub.playerId = it->second.value;
//         ////stub.idx = it->second.idx;
//         ////stub.where = it->second.where;
//         ////stub.x = it->second.x;
//         ////stub.y = it->second.y;
//     }
// 
//     // 最后还需要通过 SaveItemRelation 将新增加、删除的道具更新
// 	// 既然不存在冲突，那么道具关联保存应该必定成功！
//     BOOL ck1 = SaveItemRelation( data1, dst1, true );
//     BOOL ck2 = SaveItemRelation( data2, dst2, true );
//     assert( ck1 && ck2 );
// 	return ck1 && ck2;
}

static const LPCSTR _WHERE_STRING[16] = { "0","装备栏","物品栏","仓库1","仓库2","仓库3","6","7","8" };

void _TypeItemInfo( QWORD id )
{
    {
        UGE::CriticalLock::Barrier4ReadWrite barrier( _g::dpLayer.lock );
        ItemUniqueChecker::BindStub *stub = _g::dpLayer.iuChecker.uniqueMap.find( id );
        if ( stub == NULL )
            rfalse( 2, 1, "没有找到指定ID为0x%I64x的道具绑定信息\r\n", id );
        else
        {
            assert( stub->UniqueId() == id );
            rfalse( 2, 1, "ID为[0x%I64x]的道具的绑定信息：\r\n"
                "角色ID[%d], 控制状态[%s]\r\n"
                "位于[%s], x/y = [%d/%d], idx = [%d]\r\n", 
                id, stub->playerId, stub->isCached ? "缓存" : "磁盘",
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
//         rfalse( 2, 1, "角色缓存中总的有效道具数量为\r\n"
//             "[玩家][%d][%d,%d,%d,%d,%d]\r\n"
//             "[缓存][%d][%d,%d,%d,%d,%d]\r\n"
//             "[磁盘][%d][%d,%d,%d,%d,%d]\r\n",
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

    rfalse( 2, 1, "缓存角色数量 %6d\t匹配账号数量 %6d\r\n"
        "绑定道具数量 %6d\t冲突角色数量 %6d\r\n"
        "消息队列数量 %6d\tRPC 队列数量 %6d\r\n"
        "事件队列数量 %6d\t保存队列数量 %6d\r\n"
        "提交保存次数 %6d\t实际保存次数 %6d\r\n"
        "保存最高耗时 %.5f 最低耗时 %.5f 平均耗时 %.5f\r\n"
        "冲突类型分布 %d\t%d\r\n"
        "底层网络消息数量 %d",
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
            // 只保存,不考虑销毁对象,因为销毁的同时对象的服务器锁定状态也被清除了...
            // it->second.data->m_byStoreFlag = SFixProperty::ST_LOGOUT;
            // it->second.removeMe = true;

            DataPersistentLayer::UpdateEvent ue = { it->second.data->m_dwStaticID, it->second.serverId };
	        _g::dpLayer.updateQueue.push( ue );
            nt ++;
        }
    }

    // 打退出事件标记
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