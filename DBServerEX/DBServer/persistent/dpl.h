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
    int     breakFlag;  // Ϊ��ʱ����ʾ�ⲿҪ���ˣ�Ϊ��ʱ��Ϊ�ڲ������˳�������
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

		// ������ݽ���д��
		void _WriteRole();
        // �����ͽ����־û�����!
        BOOL Initialize( LPCTSTR UserName, LPCTSTR PassWord, LPCTSTR DBIP, LPCTSTR DBName );

        // ����������˳��¼�������¼�������ȫ�˳��������ź�
        int Shutdown( HANDLE quitEvent = NULL );

        // �������ݲ����������н�ɫ���棬���������ݱ�־
        int Backup( BOOL *atBackup, LPCSTR title, LPCSTR bakPath );

        // ��ȡ��ɫ�б����ǵ�һ���������ڴ������Ӧ�ʺŻ���м�����
        void QueryPlayerList( DNID dnidClient, SQGetCharacterListMsg *pMsg );

        // ������ɫ�ͻ�ȡ��ɫ�б�ʹ����ͬ�Ļص���������Ϊ�����߼����Դﵽһ��
        void QueryCreatePlayer( DNID dnidClient, SQCreatePlayerMsg *pMsg );

        // ��ȡ��ɫ���ݣ�׼��������Ϸ������
        void QuerySelectPlayer( DNID dnidClient, SQGetCharacterMsg *pMsg );

		//ɾ����ɫ����
		void DeletePlayer( DNID dnidClient, SQDeleteCharacterMsg *pMsg);

		// �����������
		void SavePlayerData(DNID dnidClient, SQSaveCharacterMsg *pMsg);

		// �����ֽ�ɫ���߽���������ǿ�Ƹ���ʱ�����������������������
		void QueryDoubleSavePlayer( DNID dnidClient, SDoubleSavePlayerMsg *pMsg );

		// ������ҵĻ�������
		void SetPlayerInitData(SFixData &fixData);

		// ������ҵ�����������
		void QuerySavePlayerConfig( DNID dnidClient, SQLSavePlayerConfig *pMsg );
		// ��ȡȫ���������������
		void QueryGetAllPlayerRandList( DNID dnidClient, SQDBGETRankList *pMsg );

		void SendMailSYS_SanGuo(S2D_SendMailSYS_MSG*pBaseMsg);//ϵͳ��ȫ��
		void SendMail_SanGuo(DNID dnidClient, S2D_SendMail_MSG*pBaseMsg);//ϵͳ�����
		void GetMailInfoFromDB(DNID dnidClient, SQ_GetMailInfo_MSG* pMsg);//ȡ����ʼ���Ϣ�б�
		void GetMailAwards_DB_SANGUO(const DNID clientDnid, struct SQ_GetAwards_MSG* pMsg);//���ȡ����
		void SetMailState_DB_SANGUO(SQ_SetMailState_MSG* pMsg);//�������״̬
		void test(string str);
    }
}

// ��豹��棺
// ���ڵĵ���Ψһ�Լ�����ԵñȽϸ����ˣ��ܲ��ܾ��񵱳���ô���أ�������
// �ɰ汾�ķ�����
// ��ҵĵ��߱�����������ţ�һ����ҵĵ��߳����ظ������ߵĵ��߽��ɵ����ݸ��ǣ�Ҳ���ǳ��ָ���Ʒ��ʧ��
// �°汾������save/doublesave/resolveConflict��������Ȼֻ�����3������������ô����ô���ӡ������鷳������
// ���¿��Ǻ��˼·�������ع����ݸ����߼�������
// ��Ȼ�Ѿ�д�����ݿ�Ĳ�����û�취�Ļ����ˣ����ǿ��Կ������޷����ǵ������ɾ���¸���Ʒ������
// ����ʹ��һ�����ұ���Ȼ��֤�����еĵ���Ψһ��
// �����ֳ�ͻʱ��ֱ�����ٳ�ͻԴ�ĵ��ߣ������ͻĿ��ĵ��ߣ������ͻԴ�Ѿ����ߣ������ٳ�ͻĿ��ĵ��ߡ�����
// �������ı�֤�߼��ǣ������һ����ߣ�����Ψһ���У�����ô���㻺�����ݱ�ɾ��������Ȼ�ܹ����ǻ���
// ���գ�ֻ��save/doublesave���������߼�������ȡ����delayMap
// Ȼ�����ڵ�½������/���������������һ��Ψһ��֤����ô�ͺܰ�ȫ��
// ͬ���߼������ڻ����е����ݱ����ٺ󣬿��Ա��ٴθ���
// ��ô��doublesaveҲ���Ա�ȡ������ʹ��2��save������
// �����������£���Ȼ����ֵ��߳�ͻ��Ϊ�˷����⣬����ͬʱʹ��save/doublesave�ɣ�

// ���ڲֿ���߰󶨣�
// �������ʼ�հ��ڽ�ɫid�ϣ���ô����ͬһ���˺ŵ���ҹ��òֿ����ʱ��id��Ȼ��ͻ������
// ����������ڵ�ģʽ������ɫ���ߺ�����ֿ���߰󶨹�ϵ����ô������ָ����أ����߲��Ǿ��ظ�����
// �������ʱ����Щ���߰󶨱���Ϊһ������ֵ������ЩֵҲֻ�ܱ���ȡ���ǣ���ô�Ϳ���������ֹ�ظ����棡
// ˼·���Ǻܼ򵥵ģ�����������֤
class ItemUniqueChecker
{
public:
	ItemUniqueChecker() {}

public:
	// ����ɫ�����ݿ��ȡ������ʱ
	// ��Ҫ�������飺����ͻ����ע����߹�����������uniqueMap��
	// �ⲿ���ñ����ж� ����ֵΪ TRUE ʱ�����ܱ����뻺�棬�������ڣ����ߴ����ظ�����ɫ���ܾ�����
	BOOL AtLoadPlayerFromDatabase(SFixData &data, BOOL fromGame);

    // ����ɫ����ʱ��ʹ�øú��������е��ߵ� isCached �޸�Ϊ false����Ҫ��������ֿ��idת��������
	BOOL AtRemovePlayerFromCache(SFixData &data);

	// ����ɫ����Ϸ�����ύ���ݣ����浽����ʱ
	// ��Ҫ�������飺����ͻ��������delayMap��������/ע����߹���
	// �ⲿ���ñ����ж� ����ֵΪ TRUE ʱ�����ܱ����뻺�棬�������ڣ����ߴ����ظ������汻����
	BOOL AtSavePlayerIntoCache(SFixData &data, SFixData *dst);

	// ������������ڴ���������Ϣ��DoubleSave������Ϣ��Ҫ�����ڵ��������֮�����ת�Ƶ������
	// Ŀǰ����ֵĵط��У���ҽ������ߡ���ҹ����̯�ϵĵ���
	BOOL AtDoubleSavePlayerIntoCache(SFixData &data1, SFixData &data2, SFixData *dst1, SFixData *dst2);

private:
	// ����Ƿ���ڳ�ͻ�����̰߳�ȫ
	BOOL HaveConflict(SFixData &data);

	// ������߹��������̰߳�ȫ
	BOOL SaveItemRelation(SFixData &data, SFixData *raw, BOOL fromGame);

	// ͨ�����ߵ�Ψһ��ʶ�������ģ�Ψһ�Գ�ͻ���ӳ���
    struct BindStubHashTable;

    struct BindStub
    {
        QWORD UniqueId() { return uniqueId; }

    private:
		QWORD uniqueId;			// ����ΨһID
		DWORD nextIdx;			// hash��������һ���ڵ�ķ���ƫ����

    public:
        DWORD playerId;         // �󶨽�ɫID
        BYTE  idx;              // �����������б��е�λ��
        BYTE  x         : 4;    // 
        BYTE  y         : 4;    // 
        BYTE  where     : 3;    // ���ݵ�����λ�ã�װ��������Ʒ�����ֿ�1��2��3��
        BYTE  isCached  : 1;    // �����Ƿ����ڻ�����

        // ����12��bitδ��ʹ�ã��ɿ������������Ĺ�����չ

        friend struct ItemUniqueChecker::BindStubHashTable;
    };
 
	// ������ʽ��ʼʹ��hash��������߳�ͻ
    struct BindStubHashTable
    {
    private:
		static const int HASHKEYCAPACITY	= 0x100000;		// 100w��Ͱ��ռ��4MB
        static const int ALLOCCAPACITY		= 0x10000;		// ÿ����չ6.5w��Ͱ
        static const int INVALIDKEY			= 0xffffff;		// ÿ����չ6.5w��Ͱ

        inline DWORD Hash(QWORD uniqueId)
        {
            // ���ڵ���ΨһIDԭ������ɢ��ֵ
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
            // �ڿ��ýڵ㻹û���þ���ʱ�򣬲���Ҫ�������䣬ֱ�ӷ�����Ч�ڵ�
            if ( unuseHeader == INVALIDKEY )
            {
                // ����þ�����ô����ط�����Ҫ�½��ڵ���
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

            // �����·���ĵ�һ���ڵ�
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

            // û�ҵ�ָ�����ߣ���Ҫ�����½ڵ�
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
    // ע��ע�⣺������󲻹�����ǰ��std::map�������ڵ�hashTable�����Ƿ��̰߳�ȫ�ģ�ʹ��ʱע�⣡
    BindStubHashTable uniqueMap; 
	// std::map< QWORD, BindStub > uniqueMap;
    // ########################################################################################################

	// һ�Ѵ�����ֱ�ӹ����������漰����Դ���ʵĵط�
	// �Ժ������������Ż����⡣����
	// UGE::CriticalLock bigLocker;
	// �����������������ˣ�ֱ��ʹ�ý�ɫ���ݻ��沿�ֵ�����

    friend struct Query4QueryPlayerList;
    friend void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    friend void _TypeItemInfo( QWORD id );
    friend void _CheckItemInfo();
};

// һ�����ṹ������תΪ���������ݵĹ��߽ӿ�
// ���ݿ��ɫ���ݱ�����Ҫ����һ�������ա����ݣ����ڽ�ɫѡ��ʱʹ�ã�
interface ISerializableData
{
    // ������Ӧ���̻߳���
    virtual void Lock() = 0; 
    virtual void Unlock() = 0; 

    // ���л������ΨһID�������ڿ�����չ
    virtual unsigned int UniqueId() = 0; 
    virtual unsigned int UniqueId( unsigned int uid ) = 0; 

    // ���ںͳ־ò�򽻵���ͨ���м��	
    struct SectorInfo { unsigned int sector; void *data; unsigned int size; };

    // ȷ����ǰ���л����Ὣ���ݷ�Ϊ���ٸ����֣���ǰ״̬�µ� SectorInfo::data ���ֶ�����
    // ��Ȼ�Ƿ��صĳ���ָ�����ã�Ҳ��˵���˸������������л����ڲ���̬���ڵģ�
    virtual size_t GetSectorList( SectorInfo const *& sectorList ) = 0;

    // ����ǰ�������л�Ϊn�����֣�sectorָ����Ҫ���л������ݶΣ�data����ߴ�����Ҫ������ݵĿռ䣬sizeָ���ռ��С
    // ���dataΪNULL����ʾ�����ݱ�����δ���£�����Ҫд�����ݿ⣬��data��NULLʱ����ܱ�ʾ���ݱ���ա�����
    virtual BOOL EncodeData( SectorInfo *array, size_t length ) = 0; 

    // ����������ݷ����л�Ϊ�ṹ������
    virtual BOOL DecodeData( const SectorInfo *array, size_t length ) = 0; 
};

class DataPersistentLayer
{
public:
    BOOL InitDpl();  // ���������̣߳����ں����ݿ⣨����̵����������߼����ݽ�����
    void Release( HANDLE quitEvent );
    virtual int LoopService( ThreadHandle * ); // ���������ݳ־û����̡߳�����

	BOOL LoadIntoCache( DWORD serverId, LPCSTR account, SFixData &data, BOOL fromGame );
	BOOL SaveIntoCache( DWORD serverId, SFixData &data, LPCSTR account );
	BOOL DoubleSaveIntoCache( DWORD serverId, SFixData &d1, SFixData &d2, LPCSTR acc1, LPCSTR acc2 );

    DWORD CheckAccountExist( std::string &account );

public:
	// ����� timeStamp Ϊ���һ�α���ʱ�䡣����
	// immediate Ϊ�Ƿ���Ҫ������������ݿ⡣����
	// removeMe Ϊ�Ƿ���Ҫ�ӻ���������ý�ɫ������
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
    static int PersistentThread( ThreadHandle * ); // ���������ݳ־û����̡߳�����

public:
    // ���ڵĹ����ڴ滺����ƶ����̳߳�ͻ�Ĵ���...
    // ����linux��,�߳�ID���ܳ���...���һ��windows��һ������ID����...����...˼·��Ҫ���¿���
    // �μ�SMCacheAdv

    // ׼��1�����ɫ������Ϊ���棡
    // �����������ݿ鿪�ٵ�������ռ�Ļ�������ʵ�ֳ���������Զ��ָ�������
    //SPlayerProperty player[10000];

    // ����ֻ���ṩ��һ�����ٶ�λ�ͳ־ù���ķ���
    // ע�⣺����Ļ�����ʹ�õ���ISerializableData��ָ�룬��������ȴû�нӹ�ָ������Ȩ����Ϊʹ�õ���ָ�룩
    // ���������ָ���ǳ����ھ�̬����Ҳ�ã����Ƕ�̬��������Ķ����ɢ��ַҲ�ã������Ȩ���������������
    // �����������ǽ����ڵģ���ô����һ���򵥵����ݻ��棬����ǻ��ڹ����ڴ�ģ���ô�ó־ò�Ϳ���֧�ֱ����ָ�
    CacheMap cacheMap;          // ���ڿ��ٶ�λ��ӳ���
    AccountMap accountMap;
    UGE::CriticalLock lock;
    
    // ���ڵײ��ڴ���ʱ��������ѯ���ƣ����������ݸ��µ���Ƶ����Ҳ���ᵼ�����ݿ�ѹ��
    // ͬ���������Щ�ǻ�Ķ��󣬿��Խ����ݵı���ʱ��ѹ�󡣡���
    // ����1w����ɫ����ѯ�����Ϊ10���ӣ���ôҲ�ܹ���֤ÿ�봦��16�����󡣡���
    // һ����˵��5k����ɫ�����20���ӣ���ô��ÿ���ֻ��Ҫ����4�����󡣡���
    // Ȼ������Բ�ͬ����Ƶ�ʵȼ������ݽ��жԱȣ��Ϳ���ֻ�������������ݵ����ݿ⣨���̣�������

    // ����ʹ�õ��Ǽ򵥵�queue��Ҳ��˵�����ڲ������ƻ���ѯ����˳�򡣡���
    // ʹ��DWORD��Ϊ�˺�cache�Ĺؼ��ֽ�����ϣ�������ȥ�ҵ���Ҫ���µ�ʵ������
    // ��һ��DWORD���������Ʊ���ʱ�������������չ��
    // �����cache���Ҳ�������˵�������ѱ������ˣ�������ʹ����ǿ�Ʊ�����߼���
    // ǿ�Ʊ��棬�����Ϊʵʱ��������ͬʱ������������ʱ�����ػᶸȻ���ӡ�����
    // ���ֻ���򱣴����updateQueue����һ��ID�������߼��߲����ܵ�������������Ҳ����ͨ���߼�����������
    // ���ƣ�����ǵ�һ���ݣ����������Ĺ���Χ��󣬣�cacheMap�����߼����ʼ��ʺܴ󣩶�������Դ��ͻ�Ļ������ӽ�������
    // ��������������ݷֿ����������Ȼή�ͺܶ࣬updateQueue���������ᵼ��cacheMap�ķ��ʳ�ͻ������
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
    // ��ȡ�ʺŶ�Ӧ�Ľ�ɫ�б�
    void GetChList( DNID dnidClient, DWORD serverId, LPCSTR account, DNID RetDnid,  _g::_p::Callback4QueryPlayerList callback );
    void NewPlayer( DNID dnidClient, DWORD serverId, LPCSTR account, DWORD index, SFixData &data, _g::_p::CallBack4CreatePlayer callback );       // �����½�ɫ
    void GetPlayerData( DNID dnidClient, DWORD serverId, LPCSTR account, BYTE byIndex, DNID RetDnid, _g::_p::Callback4QuerySelectPlayer callback );   // ��ȡ��ɫ����
    void DeletePlayer( DNID dnidClient, DWORD serverId, LPCSTR account, DWORD playerId, DNID retDnid );    // ɾ����ɫ
	void SavePlayerConfig(LPCSTR account, DWORD serverId, LPCSTR PlayerConfig);
	void GetAllPlayerRankList(DNID dnidClient,DNID serverId, DWORD level);
	// �����������ֱ�ӱ�Ϊ���洦���߼��ˣ����Ծ�û��ʹ�ö����Ĵ����̡߳�����
    // void UpdateData( DWORD serverId, LPCSTR account );      // ���½�ɫ����

    void RPCStream(); // ���⴦������������չ

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
    // ר���ڽ�ɫ���ݲ����Ķ���
	CNetDBModuleEx m_Query;

private:
	// �˴�ʹ���˶���Ψһ�Ķ��У�Ȼ��������Ĳ������Ϳ��ܴ���
	// ��ȡ��ɫ�б�������ɾ������ȡ�������ɫ����
	// ����5����Ϣ��Ҫ�ض���֤�������б��д�����ͬ���͵��ظ���
	// [A] ��ȡ��ɫ�б����ͬʱ���ڶ���������ԣ����ʹ�ø���Ψһ������б����������
	// [B] ������ɾ������ȡ���ܻ����������ʱ��������߼����󣬵������ʹ�ø���Ψһ�����⼸����Ԫ�����ǵ���������
	// [C] ���棬���ܻ�����ظ��ύ����������Ҳ���������������Ը���Ψһ�󣬾�ֻ�����һ������
	// ����3����Ϣ������������ǲ����ظ����ڵģ����ǳ����˺����ص����⡣����
    UGE::mtUniqueQueue< Element > queryQueue;

    friend void _DisplayInfo( size_t msgQNumber, size_t rpcQNumber, size_t npkQNumber );
    friend int _SaveAll( HANDLE quitEvent );
};

extern BOOL LoadPlayerInitProperty();