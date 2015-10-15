#pragma once

// ������ݹ�����
// �ڴ��е����ݳ־ò㣡����������ݵĿ��ٴ�ȡ��

// �°汾�У�����������״̬�Ѿ�Ǩ�Ƶ����ݿ��ڲ��ˣ������������ɫ�б�ʱ��������һ�����󴫵ݵļ򵥲�����

#include "networkmodule/playertypedef.h"
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "netdbmodule/netdbmodule.h"
#include "pub/thread.h"
#include "pub/objectmanager.h"

// ÿһ������Ļ������ݣ�����һ���˺Ż����о�ֻ������һ����ɫ����
struct AccountData :
	public UGE::CriticalLock
{
private:
	using UGE::CriticalLock::Lock;
	using UGE::CriticalLock::Unlock;

public:
    // ��ɫ�������ϵ�״̬
	enum { ST_NORMAL, ST_LOGIN, ST_LOGOUT, ST_HANGUP };

    // ɾ����״̬ 
    enum { DT_NULL, DT_WAITDELETE, DT_DELETED } delState; 

public:
    AccountData() : server_id( 0 ) 
    { 
        memset( depot,   0, sizeof( depot   ) );
        memset( players, 0, sizeof( players ) );
        warehousechanged = false;
        warehouseloaded = false;
		memset( byOperaterType, 0, sizeof( byOperaterType ) );
		delState = DT_NULL;

		dwLastSaveTime = timeGetTime();
    }

    ~AccountData() { }

    // �˺ţ�Ψһ��ʶ��
    std::string account;

	// ���ڴ洢��SeverID��������������Ѿ�ֱ�ӷ������ݿ����ˣ�����ÿһ��DBServerʵ����Ҳ������һ�����ݻ�����м�����
	// DWORD dwServerId;

    // ���ڱ���Ķ��������ݰ�
    struct BinaryPackage
    {
        DWORD   version;            // ���������ݰ��İ汾��
        LPVOID  rawData;            // δѹ��ʱ������ָ��
        LPVOID  pakData;            // ѹ���������ָ�루Ĭ����������ݴ��ڿ���״̬���ڶ�ȡ�ͱ���ʱ����ѹ��״̬��
        WORD    pakSize;            // ���������ݰ���ѹ�����С
        WORD    rawSize;            // ���������ݰ���ԭʼ��С
    } package;
    
    SFixProperty fullData;          // �����Ķ��������ݰ���������SFixProperty���趨Ϊ

    BOOL warehousechanged;          // �ֿ������Ƿ񱻸ı䣿�������ݱ���ʱ�Ż���
	DWORD lastSaveTime;             // ���һ�εı���ʱ��
};

struct UniqueElement
{
	typedef std::string _Tkey;
    const AccountData *_data;   // ��ΪֻҪ���ݻ��ڱ����У���ô������ݾͲ��ñ����٣���ôҲ�͸�һֱ������objMap��
	std::string &GetKey() const { return _data->account; }
	UniqueElement( const AccountData *other ) : _data( other ) {}
	UniqueElement() {}
};

class Query;

class PlayerDataCache : CObjectManager< std::string, AccountData, UGE::CriticalLock >
{
public:
	void PushIntoSaveQueue( AccountData *data, BOOL bDelete = TRUE );

public:
    // ������Ӧ������Ϣ�ĺ���
    void GetCharacterList( Query &query, DNID dnid, struct SQGetCharacterListMsg *pMsg  );
    void DeleteCharacter ( Query &query, DNID dnid, struct SQDeleteCharacterMsg *pMsg   );
    void CreateCharacter ( Query &query, DNID dnid, struct SQCreatePlayerMsg *pMsg      );
    void GetCharacter    ( Query &query, DNID dnid, struct SQGetCharacterMsg *pMsg      );
    void SaveCharacter   ( Query &query, DNID dnid, struct SQSaveCharacterMsg *pMsg     );

	//�������
	void SendMail        ( Query &query, DNID dnid, struct SQMailSendMsg *pMsg          );
	void RecvMail        ( Query &query, DNID dnid, struct SQMailRecvMsg *pMsg          );
	void DeleteMail      ( Query &query, DNID dnid, struct SQMailDeleteMsg *pMsg        );
	void NewMail         ( Query &query, DNID dnid, struct SQNewMailMsg *pMsg           );

private:
    // ����cache�ĺ���
    int GetCharacterList ( Query &query, DWORD serverId, LPCSTR account, struct SCharListData data[3]               );
    int DeleteCharacter  ( Query &query, DWORD serverId, LPCSTR account, DWORD staticId, char name[11]              );
    int CreateCharacter  ( Query &query, DWORD serverId, LPCSTR account, int index, struct SCreateFixProperty &data );
    int GetCharacter     ( Query &query, DWORD serverId, LPCSTR account, int index, struct SFixProperty &data       );
    int SaveCharacter    ( Query &query, DWORD serverId, LPCSTR account, const struct SFixProperty &data            );
    void UnlockServerBind( Query &query, DWORD serverId, LPCSTR account                                             );

private:
	BOOL LocateAndLock( LPCSTR account, boost::shared_ptr< AccountData > &ptr, 
        UGE::CriticalLock::Barrier4ReadWrite &barrier, bool bNeedCreate = false );
};
