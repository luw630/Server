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

    // Ϊ���ñ���������������������ݿ�����������������
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

    // ����
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

                    // ������������ʱ����ȷ��ѡ����һ���˳�ʱ��
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

    // ���Բ�ɾ����
    {
        Barrier barrier( *_accdata );

        if ( _accdata->delState == AccountData::DT_WAITDELETE )
        {
            GetApp().m_PlayerCache.RemoveObject( _accdata.GetKey() );
            _accdata->delState = AccountData::DT_DELETED;

            // ��ǲ���¼�������,�������ٽ�����������ݸ���
            if ( selecter < 3 )
            {
                reallyDelete        = TRUE;
                temp.onlineTime     = _accdata->players[selecter].base.m_pTroopsSkills[0].dwProficiency;
                temp.offlineTime    = _accdata->players[selecter].base.m_pTroopsSkills[1].dwProficiency;
                temp.offlineSegment = _accdata->players[selecter].base.m_pTroopsSkills[2].dwProficiency;
            }
        }
    }

    // ���¼�ʱ���ݵ������Լ�ʱ��
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

    // ���accdata��״̬���������Ҫɾ������Ҳ����Ҫ���棬��ֱ�ӷ���
    if ( !bDelete && !accdata->warehousechanged )
        return;

    if ( bDelete )
    {
        // �������ķ�����IDȡ����ʹ�������ܴ��ڵĵ�½�����ܹ�����ִ�У�
        accdata->server_id = 0;
    }

    accdata->dwLastSaveTime = timeGetTime();
    accdata->delState = bDelete ? AccountData::DT_WAITDELETE : AccountData::DT_NULL;
    GetApp().saveQueue.mtsafe_push( accdata );
}

void CDAppMain::SaveAll( BOOL bDelete )
{ 
    // �������л����еĶ��󣬲�������� PlayerDataCache::PushIntoSaveQueue( accdata, false )
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
	rfalse( 2, 1, "��ʼ����SaveTimeʱ���¼����" );

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

        fSaveTime << "��������" << timeLog.times.size() << std::endl
            << "ƽ��ִ��ʱ�䣺" << fEnd / timeLog.times.size() << std::endl;	

	    timeLog.times.clear();
    }

	rfalse( 2, 1, "���ݶ�ȡ��ϣ�" );
}

// ����ͳ����Ч�˺������ķº���
struct CalculateEntity
{
    typedef void result_type;

    CalculateEntity() : count( 0 ) {}

    void operator () ( PlayerDataCache::_element &element )
    {
        // ��Ϊֻ��Ҫ��ȡģ������������ʾ�Ϳ����ˣ���������û�н� element.second ����
        if ( element.second->warehouseloaded ) 
            count ++;
    }

    int count;
};

void CDAppMain::DisplayInfo()
{
    // �������л����еĶ��󣬲�������� PlayerDataCache::PushIntoSaveQueue( accdata, false )
    CalculateEntity calculator;
    m_PlayerCache.Traverse( boost::bind( boost::ref( calculator ), _1 ) );

 rfalse( 2, 1, "����״̬��%s\r\nmsgQueue.size = %d\r\nSaveQueue.size = %d\r\naccountNumber = %d\r\nentityNumber = %d", 
        m_bQuit ? "�˳�������" : "��������", msgQueue.size(), saveQueue.size(), m_PlayerCache.Size(), calculator.count );
}
