#include "StdAfx.h"

#include "PlayerDataCache.h"
#include "DMainApp.h"
#include "NetDBModule/NetDBModule.h"
#include "pub/Thread.h"
#include "pub/StopWatch.h"

static struct { std::vector< double > times; UGE::CriticalLock lock; } timeLog;

typedef UGE::CriticalLock::Barrier4ReadWrite Barrier;

void SaveProcedure( Query &query, UniqueElement &_accdata )
{
    std::string account;
    SPackageItem depot[3][MAX_ITEM_NUMBER];
    AccountData::PodData players[3];

	DWORD dwServerID = 0;
    bool warehouseloaded;
    bool warehousechanged;
    BYTE byOperaterType[3];

    // 为了让保存操作不被阻塞！将数据拷贝出来，单独处理！
    {
        Barrier barrier( *_accdata );

        account = _accdata->account;
        warehouseloaded = _accdata->warehouseloaded;
        warehousechanged = _accdata->warehousechanged;

        memcpy( depot, _accdata->depot, sizeof( depot ) );
        memcpy( players, _accdata->players, sizeof( players ) );
        memcpy( byOperaterType, _accdata->byOperaterType, sizeof( byOperaterType ) );

        _accdata->warehousechanged = false;
        _accdata->byOperaterType[0] = 
        _accdata->byOperaterType[1] = 
        _accdata->byOperaterType[2] = 0;
		dwServerID = _accdata->dwServerId;
    }

    CStopWatch timer;
    timer.tStart();

    int selecter = 3;
    DWORD maxoffline = 0;

    // 保存
    {
        if ( !account.empty() && warehouseloaded )
        {
            #define DBMODULE reinterpret_cast< CNetDBModule* >( &query )

            if ( warehousechanged )
            {
                for ( int i=0; i<3; i++ )
                    DBMODULE->SavePlayerWareHouse( account.c_str(), depot[i], i+1 );

                // warehousechanged = false;
            }

            for ( int i=0; i<3; i++ )
            {  
                if ( 0 != byOperaterType[i] )
                {
                    #define PLAYERDATA players[i]

					DBMODULE->SavePlayerData        ( dwServerID, PLAYERDATA.static_id, PLAYERDATA.base, PLAYERDATA.package );
                    DBMODULE->SavePlayerRelations   ( PLAYERDATA.static_id, PLAYERDATA.relation                 );
                    DBMODULE->SavePlayerTask        ( PLAYERDATA.static_id, PLAYERDATA.tasks                    );
                    DBMODULE->SaveWareHouseLock     ( account.c_str(),      PLAYERDATA.base.m_bWarehouseLocked  );

                    // 根据最后的下线时间来确定选择哪一个退出时刻
                    if ( maxoffline < PLAYERDATA.base.m_pTroopsSkills[2].dwProficiency )
                    {
                        maxoffline = PLAYERDATA.base.m_pTroopsSkills[2].dwProficiency;
                        selecter = i;
                    }

                    #undef PLAYERDATA

                    // byOperaterType[i] = 0;
                }
            }

            #undef DBMODULE
        }
    }

    AccountTimeSet temp;
    BOOL reallyDelete = FALSE;

    // 测试并删除！
    {
        Barrier barrier( *_accdata );

        if ( _accdata->delState == AccountData::DT_WAITDELETE )
        {
            GetApp().m_PlayerCache.RemoveObject( _accdata.GetKey() );
            _accdata->delState = AccountData::DT_DELETED;

            // 标记并记录相关数据,用于在临界区外进行数据更新
            if ( selecter < 3 )
            {
                reallyDelete        = TRUE;
                temp.onlineTime     = _accdata->players[selecter].base.m_pTroopsSkills[0].dwProficiency;
                temp.offlineTime    = _accdata->players[selecter].base.m_pTroopsSkills[1].dwProficiency;
                temp.offlineSegment = _accdata->players[selecter].base.m_pTroopsSkills[2].dwProficiency;
            }
        }
    }

    // 更新计时数据到防沉迷计时器
    if ( reallyDelete )
    {
        BOOL result = GetApp().m_PlayerCache.limitManager.InsertObject( temp, account, true );
        assert( result );
    }

    if ( timeLog.times.size() < 50000 )
    {
        UGE::CriticalLock::Barrier4ReadWrite  barriers( timeLog.lock );
        timeLog.times.push_back( timer.tEnd() );
    }
}

void PlayerDataCache::PushIntoSaveQueue( boost::shared_ptr< AccountData > &accdata, BOOL bDelete )
{
    Barrier barrier( *accdata );

    // 检测accdata的状态，如果不需要删除并且也不需要保存，则直接返回
    if ( !bDelete && !accdata->warehousechanged )
        return;

    if ( bDelete )
    {
        // 将锁定的服务器ID取消，使后续可能存在的登陆操作能够继续执行！
        accdata->server_id = 0;
    }

    accdata->dwLastSaveTime = timeGetTime();
    accdata->delState = bDelete ? AccountData::DT_WAITDELETE : AccountData::DT_NULL;
    GetApp().saveQueue.mtsafe_push( accdata );
}

void CDAppMain::SaveAll( BOOL bDelete )
{ 
    // 遍历所有缓存中的对象，并对其调用 PlayerDataCache::PushIntoSaveQueue( accdata, false )
    m_PlayerCache.Traverse( 
        boost::bind( 
            &PlayerDataCache::PushIntoSaveQueue, 
            &m_PlayerCache, 
            boost::bind( &PlayerDataCache::_element::second, _1 ),
            bDelete
        )
    );
}

void CDAppMain::PrintTimeLog()
{
	rfalse( 2, 1, "开始保存SaveTime时间记录……" );

    // critical scope
    {
	    UGE::CriticalLock::Barrier4ReadWrite barrier( timeLog.lock );

	    std::ofstream fSaveTime( "SaveTime.txt" );
	    fSaveTime.precision( 10 );
	    fSaveTime.setf( std::ios::fixed, std::ios::fixed ); 
	    double fEnd = 0;

        for ( size_t j = 0; j < timeLog.times.size(); j ++ )
	    {
            fSaveTime << timeLog.times[j] << std::endl;
		    fEnd += timeLog.times[j];
	    }

        fSaveTime << "总人数：" << timeLog.times.size() << std::endl
            << "平均执行时间：" << fEnd / timeLog.times.size() << std::endl;	

	    timeLog.times.clear();
    }

	rfalse( 2, 1, "数据读取完毕！" );
}

// 用于统计有效账号数量的仿函数
struct CalculateEntity
{
    typedef void result_type;

    CalculateEntity() : count( 0 ) {}

    void operator () ( PlayerDataCache::_element &element )
    {
        // 因为只需要获取模糊数据用于显示就可以了，所以这里没有将 element.second 加锁
        if ( element.second->warehouseloaded ) 
            count ++;
    }

    int count;
};

void CDAppMain::DisplayInfo()
{
    // 遍历所有缓存中的对象，并对其调用 PlayerDataCache::PushIntoSaveQueue( accdata, false )
    CalculateEntity calculator;
    m_PlayerCache.Traverse( boost::bind( boost::ref( calculator ), _1 ) );

 rfalse( 2, 1, "进程状态：%s\r\nmsgQueue.size = %d\r\nSaveQueue.size = %d\r\naccountNumber = %d\r\nentityNumber = %d", 
        m_bQuit ? "退出保存中" : "正常运行", msgQueue.size(), saveQueue.size(), m_PlayerCache.Size(), calculator.count );
}
