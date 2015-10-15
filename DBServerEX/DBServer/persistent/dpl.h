#pragma once

#include "pub/thread.h"
#include "networkmodule/logtypedef.h"
#include "networkmodule/playertypedef.h"
#include "../NetDbModuleEx.h"

struct ThreadHandle
{
    ThreadHandle() : threadHandle( NULL ), threadID( 0 ), breakFlag( 0 ), param( NULL ) {}

    void Join() { 
        if ( threadHandle && !breakFlag ) { 
            breakFlag = 1; 
            WaitForSingleObject( threadHandle, INFINITE ); 
            threadHandle = NULL;
            threadID = 0; 
            breakFlag = 0;
            param = NULL;
        } 
    }

    HANDLE  threadHandle;
    UINT    threadID;
    int     breakFlag;  // 为正时，表示外部要求退，为负时，为内部自行退出。。。
    void    *param;

    typedef unsigned ( __stdcall * FuncType ) ( void * );
};

struct SQGetCharacterListMsg;
struct SQCreatePlayerMsg;
struct SQGetCharacterMsg;
struct SDoubleSavePlayerMsg;

namespace _g        // globals
{
    DWORD GetServerID();
	
    namespace _p    // persistent
    {
        typedef void ( __stdcall * Callback4QueryPlayerList )( DNID dnidClient, LPCSTR account, SCharListData chList[3], DWORD retCode );
		typedef void ( __stdcall * CallBack4CreatePlayer) ( DNID dnidClient, SACreatePlayerMsg *pmsg, DWORD retCode );
        typedef void ( __stdcall * Callback4QuerySelectPlayer )( DNID dnidClient, LPCSTR account, DWORD uid, SFixData &data, DWORD retCode );
		typedef void ( __stdcall * Callback4SavePlayerConfig )(DNID dnidClient, LPCSTR account);

		// 玩家数据解析写出
		void _WriteRole();
        // 启动和结束持久化服务!
        BOOL Initialize( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName );

        // 如果设置了退出事件，则该事件将在完全退出后被设置信号
        int Shutdown( HANDLE quitEvent = NULL );

        // 启动备份操作，将所有角色保存，并启动备份标志
        int Backup( BOOL *atBackup, LPCSTR title, LPCSTR bakPath );

        // 获取角色列表，这是第一步，并且在处理后，相应帐号会进行加锁！
        void QueryPlayerList( DNID dnidClient, SQGetCharacterListMsg *pMsg );

        // 创建角色和获取角色列表使用相同的回调函数，因为处理逻辑可以达到一致
        void QueryCreatePlayer( DNID dnidClient, SQCreatePlayerMsg *pMsg );

        // 获取角色数据，准备进入游戏场景了
        void QuerySelectPlayer( DNID dnidClient, SQGetCharacterMsg *pMsg );

		//删除角色数据
		void DeletePlayer( DNID dnidClient, SQDeleteCharacterMsg *pMsg);

		// 保存玩家数据
		void SavePlayerData(DNID dnidClient, SQSaveCharacterMsg *pMsg);

		// 当出现角色道具交换或数据强制更新时，区域服务器发过来的请求
		void QueryDoubleSavePlayer( DNID dnidClient, SDoubleSavePlayerMsg *pMsg );

		// 设置玩家的基本属性
		void SetPlayerInitData(SFixData &fixData);

		// 保存玩家电脑配置数据
		void QuerySavePlayerConfig( DNID dnidClient, SQLSavePlayerConfig *pMsg );
		// 获取全部玩家排行旁数据
		void QueryGetAllPlayerRandList( DNID dnidClient, SQDBGETRankList *pMsg );

		void SendMailSYS_SanGuo(S2D_SendMailSYS_MSG*pBaseMsg);//系统发全服
		void SendMail_SanGuo(DNID dnidClient, S2D_SendMail_MSG*pBaseMsg);//系统发玩家
		void GetMailInfoFromDB(DNID dnidClient, SQ_GetMailInfo_MSG* pMsg);//取玩家邮件信息列表
		void GetMailAwards_DB_SANGUO(const DNID clientDnid, struct SQ_GetAwards_MSG* pMsg);//玩家取奖励
		void SetMailState_DB_SANGUO(SQ_SetMailState_MSG* pMsg);//玩家设置状态
		void test(string str);
    }
}

// 返璞归真：
// 现在的道具唯一性检测是显得比较复杂了，能不能就像当初那么简单呢？？？？
// 旧版本的方案：
// 玩家的道具被独立单条存放，一旦玩家的道具出现重复，则后边的道具将旧的数据覆盖（也就是出现复制品丢失）
// 新版本。。。save/doublesave/resolveConflict。。。虽然只有这个3个函数，但怎么想怎么复杂。。。麻烦。。。
// 重新考虑后的思路。。。回归数据覆盖逻辑。。。
// 虽然已经写入数据库的部分是没办法改回来了，但是可以考虑在无法覆盖的情况下删除新复制品。。。
// 还是使用一个查找表，仍然保证缓存中的道具唯一。
// 当出现冲突时，直接销毁冲突源的道具，保存冲突目标的道具，如果冲突源已经下线，则销毁冲突目标的道具。。。
// 这样做的保证逻辑是，如果玩家还在线（并且唯一持有），那么就算缓存数据被删除后，他仍然能够覆盖回来
// 最终，只有save/doublesave两个保存逻辑，并且取消了delayMap
// 然后，再在登陆服务器/或区域服务器上作一次唯一保证，那么就很安全了
// 同上逻辑，由于缓存中的数据被销毁后，可以被再次覆盖
// 那么，doublesave也可以被取消，而使用2个save来代替
// 但是这个情况下，必然会出现道具冲突，为了方便检测，还是同时使用save/doublesave吧！

// 关于仓库道具绑定：
// 假如道具始终绑定在角色id上，那么，当同一个账号的玩家共用仓库道具时，id必然冲突。。。
// 如果按照现在的模式，当角色下线后，清除仓库道具绑定关系，那么如果出现复制呢？道具不是就重复拉？
// 如果下线时，这些道具绑定被置为一个特殊值，而这些值也只能被读取覆盖，那么就可以做到防止重复保存！
// 思路还是很简单的，可以轻易验证
class ItemUniqueChecker
{
public:
	ItemUniqueChecker() {}

public:
	// 当角色从数据库读取到缓存时
	// 需要做的事情：检测冲突，并注册道具关联（放置于uniqueMap）
	// 外部调用必须判断 返回值为 TRUE 时，才能保存入缓存，否则属于：道具存在重复，角色被拒绝加载
	BOOL AtLoadPlayerFromDatabase(SFixData &data, BOOL fromGame);

    // 当角色下线时，使用该函数将所有道具的 isCached 修改为 false，主要用来解决仓库绑定id转换的问题
	BOOL AtRemovePlayerFromCache(SFixData &data);

	// 当角色从游戏世界提交数据，保存到缓存时
	// 需要做的事情：检测冲突（放置于delayMap），更新/注册道具关联
	// 外部调用必须判断 返回值为 TRUE 时，才能保存入缓存，否则属于：道具存在重复，保存被挂起
	BOOL AtSavePlayerIntoCache(SFixData &data, SFixData *dst);

	// 这个函数是用于处理特殊消息，DoubleSave，该消息主要出现于道具在玩家之间进行转移的情况下
	// 目前会出现的地方有，玩家交换道具、玩家购买地摊上的道具
	BOOL AtDoubleSavePlayerIntoCache(SFixData &data1, SFixData &data2, SFixData *dst1, SFixData *dst2);

private:
	// 检测是否存在冲突，非线程安全
	BOOL HaveConflict(SFixData &data);

	// 保存道具关联，非线程安全
	BOOL SaveItemRelation(SFixData &data, SFixData *raw, BOOL fromGame);

	// 通过道具的唯一认识符关联的，唯一性冲突检测映射表
    struct BindStubHashTable;

    struct BindStub
    {
        QWORD UniqueId() { return uniqueId; }

    private:
		QWORD uniqueId;			// 道具唯一ID
		DWORD nextIdx;			// hash拉链的下一个节点的分配偏移量

    public:
        DWORD playerId;         // 绑定角色ID
        BYTE  idx;              // 道具在数据列表中的位置
        BYTE  x         : 4;    // 
        BYTE  y         : 4;    // 
        BYTE  where     : 3;    // 数据的所处位置（装备栏、物品栏、仓库1、2、3）
        BYTE  isCached  : 1;    // 数据是否在于缓存中

        // 还有12个bit未被使用，可考虑用作将来的功能扩展

        friend struct ItemUniqueChecker::BindStubHashTable;
    };
 
	// 现在正式开始使用hash来管理道具冲突
    struct BindStubHashTable
    {
    private:
		static const int HASHKEYCAPACITY	= 0x100000;		// 100w个桶，占地4MB
        static const int ALLOCCAPACITY		= 0x10000;		// 每次扩展6.5w个桶
        static const int INVALIDKEY			= 0xffffff;		// 每次扩展6.5w个桶

        inline DWORD Hash(QWORD uniqueId)
        {
            // 基于道具唯一ID原理，构造散列值
            SItemBase *item = reinterpret_cast< SItemBase* >( reinterpret_cast< LPBYTE >( &uniqueId ) - 3 );
            return ( DWORD )( ( item->details.uniqueTimestamp & 0x3fff ) | 
                ( item->details.uniqueServerId << 14 ) |
                ( ( item->details.uniqueCreateIndex & 0xf ) << 16 ) );
        }

        BindStub& select( DWORD idx )
        {
            assert( idx < allocArray.size() * ALLOCCAPACITY );
            return allocArray[ idx >> 16 ][ idx & 0xffff ];
        }

        BindStub& alloc()
        {
            // 在可用节点还没有用尽的时候，不需要继续分配，直接返回有效节点
            if ( unuseHeader == INVALIDKEY )
            {
                // 如果用尽，那么这个地方就需要新建节点了
                BindStub *block = new BindStub[ ALLOCCAPACITY ];
                memset( block, 0, sizeof( BindStub ) * ALLOCCAPACITY );
                size_t curSlot = allocArray.size();
                allocArray.push_back( block );
				int i = 1;
                for ( ; i < ALLOCCAPACITY; i ++ )
                    block[ i - 1 ].nextIdx = ( DWORD )( curSlot * ALLOCCAPACITY + i );

                block[ i - 1 ].nextIdx = unuseHeader;
                unuseHeader = ( DWORD )( curSlot * ALLOCCAPACITY );
            }

            // 返回新分配的第一个节点
            BindStub &stub = select( unuseHeader );
            DWORD temp = stub.nextIdx;
            stub.nextIdx = unuseHeader;
            unuseHeader = temp;
            usedSize ++;
            return stub;
        }

    public:
        BindStub& operator [] ( QWORD uniqueId )
        {
            DWORD key = Hash( uniqueId );
            DWORD idx = hashTable[ key ] & INVALIDKEY;
            while ( idx != INVALIDKEY )
            {
                BindStub &stub = select( idx );
                if ( stub.uniqueId == uniqueId )
                    return stub;
                idx = stub.nextIdx;
            }

            // 没找到指定道具，需要增加新节点
            BindStub &stub = alloc();
            DWORD temp = stub.nextIdx;
            stub.nextIdx = hashTable[ key ] & INVALIDKEY;
            hashTable[ key ] = temp;
            stub.uniqueId = uniqueId;
            return stub;
        }

        BindStub* find( QWORD uniqueId )
        {
            DWORD key = Hash( uniqueId );
            DWORD idx = hashTable[ key ] & INVALIDKEY;
            while ( idx != INVALIDKEY )
            {
                BindStub &stub = select( idx );
                if ( stub.uniqueId == uniqueId )
                    return &stub;
                idx = stub.nextIdx;
            }

            return NULL;
        }

        BOOL erase( QWORD uniqueId )
        {
            DWORD key = Hash( uniqueId );
            DWORD idx = hashTable[ key ] & INVALIDKEY;
            if ( idx == INVALIDKEY )
                return false;

            BindStub *iter = &select( idx );
            if ( iter->uniqueId == uniqueId )
            {
                hashTable[ key ] = iter->nextIdx;
                iter->nextIdx = unuseHeader;
                unuseHeader = idx;
                usedSize --;
                iter->uniqueId = 0;
                return true;
            }

            while ( iter->nextIdx != INVALIDKEY )
            {
                BindStub &stub = select( iter->nextIdx );
                if ( stub.uniqueId == uniqueId )
                {
                    DWORD temp = iter->nextIdx;
                    iter->nextIdx = stub.nextIdx;
                    stub.nextIdx = unuseHeader;
                    unuseHeader = temp;
                    usedSize --;
                    stub.uniqueId = 0;
                    return true;
                }

                iter = &stub;
            }

            return false;
        }

        size_t size() { return usedSize; }

        BindStubHashTable() : usedSize( 0 ), unuseHeader( INVALIDKEY ) { memset( hashTable, -1, sizeof( hashTable ) ); }

        ~BindStubHashTable() { for ( DWORD i=0; i<allocArray.size(); i++ ) delete allocArray[i]; }

    private:
        std::vector< BindStub* > allocArray;
        DWORD usedSize;
        DWORD unuseHeader;
        DWORD hashTable[ HASHKEYCAPACITY ];

        friend void _CheckItemInfo();
    }; 

    // ########################################################################################################
    // ########################################################################################################
    // 注意注意：这个对象不管是以前的std::map还是现在的hashTable，都是非线程安全的，使用时注意！
    BindStubHashTable uniqueMap; 
	// std::map< QWORD, BindStub > uniqueMap;
    // ########################################################################################################

	// 一把大锁，直接管理了所有涉及到资源访问的地方
	// 以后再慢慢考虑优化问题。。。
	// UGE::CriticalLock bigLocker;
	// 现在连大锁都不用了，直接使用角色数据缓存部分的锁！

    friend struct Query4QueryPlayerList;
    friend void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    friend void _TypeItemInfo( QWORD id );
    friend void _CheckItemInfo();
};

// 一个将结构化数据转为二进制数据的工具接口
// 数据库角色数据表中需要单独一栏“快照”数据，用于角色选择时使用！
interface ISerializableData
{
    // 用于适应多线程环境
    virtual void Lock() = 0; 
    virtual void Unlock() = 0; 

    // 序列化对象的唯一ID，有助于开发扩展
    virtual unsigned int UniqueId() = 0; 
    virtual unsigned int UniqueId( unsigned int uid ) = 0; 

    // 用于和持久层打交道的通信中间段	
    struct SectorInfo { unsigned int sector; void *data; unsigned int size; };

    // 确定当前序列化器会将数据分为多少个部分，当前状态下的 SectorInfo::data 是字段名称
    // 既然是返回的常量指针引用，也就说明了该数据是在序列化器内部静态存在的！
    virtual size_t GetSectorList( SectorInfo const *& sectorList ) = 0;

    // 将当前数据序列化为n个部分，sector指明需要序列化的数据段，data是外边传进来要填充数据的空间，size指明空间大小
    // 如果data为NULL，表示该数据被查明未更新，不需要写入数据库，而data非NULL时则可能表示数据被清空。。。
    virtual BOOL EncodeData( SectorInfo *array, size_t length ) = 0; 

    // 将传入的数据反序列化为结构化数据
    virtual BOOL DecodeData( const SectorInfo *array, size_t length ) = 0; 
};

class DataPersistentLayer
{
public:
    BOOL InitDpl();  // 创建保存线程，用于和数据库（或磁盘等其他保存逻辑数据交换）
    void Release( HANDLE quitEvent );
    virtual int LoopService( ThreadHandle * ); // 负责做数据持久化的线程。。。

	BOOL LoadIntoCache( DWORD serverId, LPCSTR account, SFixData &data, BOOL fromGame );
	BOOL SaveIntoCache( DWORD serverId, SFixData &data, LPCSTR account );
	BOOL DoubleSaveIntoCache( DWORD serverId, SFixData &d1, SFixData &d2, LPCSTR acc1, LPCSTR acc2 );

    DWORD CheckAccountExist( std::string &account );

public:
	// 这里的 timeStamp 为最后一次保存时间。。。
	// immediate 为是否需要立即保存进数据库。。。
	// removeMe 为是否需要从缓存中清除该角色。。。
	struct CacheUnit { DWORD timeStamp, serverId; BOOL modified, removeMe; SFixData *data; std::string account;
        CacheUnit() : timeStamp( 0 ), serverId( 0 ), modified( 0 ), removeMe( 0 ), data( 0 ) {} };
    struct UpdateEvent { DWORD uniqueId, serverId; };

    typedef std::map< DWORD, CacheUnit > CacheMap;
    typedef std::map< std::string, DWORD > AccountMap;
    typedef std::queue< UpdateEvent > UpdateQueue;

private:
    BOOL SaveToDB( CacheUnit &unit );

private:
    ThreadHandle threadHandle;
    static int PersistentThread( ThreadHandle * ); // 负责做数据持久化的线程。。。

public:
    // 现在的共享内存缓存机制对于线程冲突的处理...
    // 由于linux下,线程ID可能超大...并且会和windows下一样出现ID重用...所以...思路需要重新考虑
    // 参见SMCacheAdv

    // 准备1万个角色数据作为缓存！
    // 如果将这个数据块开辟到共享缓存空间的话，即可实现程序崩溃后自动恢复。。。
    //SPlayerProperty player[10000];

    // 这里只是提供了一个快速定位和持久管理的方法
    // 注意：这里的缓存中使用到了ISerializableData的指针，但是这里却没有接管指针所有权（因为使用的裸指针）
    // 不管这里的指针是出自于静态数组也好，还是动态分配出来的多个离散地址也好，管理的权力都不在这里！！！
    // 如果这个数据是进程内的，那么就是一个简单的数据缓存，如果是基于共享内存的，那么该持久层就可以支持崩溃恢复
    CacheMap cacheMap;          // 用于快速定位的映射表
    AccountMap accountMap;
    UGE::CriticalLock lock;
    
    // 由于底层在处理时是属于轮询机制，就算是数据更新得再频繁，也不会导致数据库压力
    // 同样，针对那些非活动的对象，可以将数据的保存时间压后。。。
    // 就算1w个角色，轮询最大间隔为10分钟，那么也能够保证每秒处理16个对象。。。
    // 一般来说，5k个角色，间隔20分钟，那么，每秒就只需要处理4个对象。。。
    // 然后再针对不同更新频率等级的数据进行对比，就可以只更新少量的数据到数据库（磁盘）。。。

    // 这里使用的是简单的queue，也就说明了内部不会破坏轮询数据顺序。。。
    // 使用DWORD是为了和cache的关键字进行配合，在这里去找到需要更新的实际数据
    // 后一个DWORD是用来控制保存时间或其他操作扩展用
    // 如果在cache中找不到，则说明数据已被销毁了（可能是使用了强制保存等逻辑）
    // 强制保存，如果作为实时处理，则在同时保存需求增加时，负载会陡然增加。。。
    // 如果只是向保存队列updateQueue附加一个ID，则主逻辑线不会受到阻塞，而负载也可以通过逻辑调整来均衡
    // 优势：如果是单一数据，则数据锁的管理范围变大，（cacheMap被主逻辑访问几率很大）而导致资源冲突的机会增加降低性能
    // 如果是这样将数据分开，则锁粒度会降低很多，updateQueue的锁定不会导致cacheMap的访问冲突。。。
    UpdateQueue updateQueue;

	ItemUniqueChecker iuChecker;

	friend DataPersistentLayer::CacheUnit *unsafe_LocateCache( DWORD playerId );
	friend struct Query4DeletePlayer;
    friend void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    friend int _SaveAll( HANDLE quitEvent );
    friend void _g::_p::QueryPlayerList( DNID dnidClient, SQGetCharacterListMsg *pMsg );

    __int64 totalSaveTicks;
    __int64 maxSaveDuration;
    __int64 minSaveDuration;
    int totalSaveTimes;
    int totalSaveFailTimes;
    int commitTimes;
    int conf1, conf2;

    HANDLE quitEvent;
    BOOL *atBackup;
    std::string path, title;
};

class DataPersistentLayer4XYD1 : public DataPersistentLayer
{
public:
	bool Init( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName );
    // 获取帐号对应的角色列表
    void GetChList( DNID dnidClient, DWORD serverId, LPCSTR account, DNID RetDnid,  _g::_p::Callback4QueryPlayerList callback );
    void NewPlayer( DNID dnidClient, DWORD serverId, LPCSTR account, DWORD index, SFixData &data, _g::_p::CallBack4CreatePlayer callback );       // 创建新角色
    void GetPlayerData( DNID dnidClient, DWORD serverId, LPCSTR account, BYTE byIndex, DNID RetDnid, _g::_p::Callback4QuerySelectPlayer callback );   // 获取角色数据
    void DeletePlayer( DNID dnidClient, DWORD serverId, LPCSTR account, DWORD playerId, DNID retDnid );    // 删除角色
	void SavePlayerConfig(LPCSTR account, DWORD serverId, LPCSTR PlayerConfig);
	void GetAllPlayerRankList(DNID dnidClient,DNID serverId, DWORD level);
	// 这个函数现在直接变为缓存处理逻辑了，所以就没有使用独立的处理线程。。。
    // void UpdateData( DWORD serverId, LPCSTR account );      // 更新角色数据

    void RPCStream(); // 特殊处理，用于数据扩展

    int LoopService( ThreadHandle * );

    struct QueryBase
    {
	    virtual const std::string &GetKey() const = 0;
        virtual void Execute() = 0;
        virtual void Release() = 0;
    };

    struct Element
    {
        Element() : query( 0 ) {}
        Element( QueryBase *other ) : query( other ) {}
	    typedef std::string _Tkey;
        const std::string &GetKey() const { return query->GetKey(); }
        QueryBase *query;
    };

	inline	CNetDBModuleEx& GetQuery() { return m_Query;};

protected:
    // 专用于角色数据操作的对象
	CNetDBModuleEx m_Query;

private:
	// 此处使用了对象唯一的队列，然而，这里的操作类型可能存在
	// 获取角色列表、创建、删除、获取、保存角色数据
	// 以上5种消息需要必定保证不会在列表中存在相同类型的重复！
	// [A] 获取角色列表可能同时存在多次请求，所以，如果使用覆盖唯一，则该列表不会出现问题
	// [B] 创建、删除、获取可能会由于请求的时序而导致逻辑错误，但是如果使用覆盖唯一，则这几个单元都会是单独成立的
	// [C] 保存，可能会存在重复提交多个请求，这个也很正常。。。所以覆盖唯一后，就只有最后一个成立
	// 以上3类消息在正常情况下是不会重复存在的，除非出现了很严重的问题。。。
    UGE::mtUniqueQueue< Element > queryQueue;

    friend void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    friend int _SaveAll( HANDLE quitEvent );
};

extern BOOL LoadPlayerInitProperty();