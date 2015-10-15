#pragma once

// 玩家数据管理器
// 内存中的数据持久层！用作玩家数据的快速存取！

// 新版本中，服务器锁定状态已经迁移到数据库内部了，并且在请求角色列表时，仅仅是一个请求传递的简单操作！

#include "networkmodule/playertypedef.h"
#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "netdbmodule/netdbmodule.h"
#include "pub/thread.h"
#include "pub/objectmanager.h"

// 每一个对象的缓存数据，现在一个账号缓存中就只保留了一个角色数据
struct AccountData :
	public UGE::CriticalLock
{
private:
	using UGE::CriticalLock::Lock;
	using UGE::CriticalLock::Unlock;

public:
    // 角色服务器上的状态
	enum { ST_NORMAL, ST_LOGIN, ST_LOGOUT, ST_HANGUP };

    // 删除的状态 
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

    // 账号，唯一标识符
    std::string account;

	// 用于存储的SeverID，这个数据现在已经直接放于数据库中了，并且每一个DBServer实际上也仅仅是一个数据缓存的中间层而已
	// DWORD dwServerId;

    // 用于保存的二进制数据包
    struct BinaryPackage
    {
        DWORD   version;            // 二进制数据包的版本号
        LPVOID  rawData;            // 未压缩时的数据指针
        LPVOID  pakData;            // 压缩后的数据指针（默认情况下数据处于开放状态，在读取和保存时处于压缩状态）
        WORD    pakSize;            // 二进制数据包的压缩后大小
        WORD    rawSize;            // 二进制数据包的原始大小
    } package;
    
    SFixProperty fullData;          // 解码后的二进制数据包！现在用SFixProperty来设定为

    BOOL warehousechanged;          // 仓库数据是否被改变？用于数据保存时优化！
	DWORD lastSaveTime;             // 最后一次的保存时间
};

struct UniqueElement
{
	typedef std::string _Tkey;
    const AccountData *_data;   // 因为只要数据还在保存中，那么这个数据就不该被销毁，那么也就该一直存在于objMap中
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
    // 用于响应网络消息的函数
    void GetCharacterList( Query &query, DNID dnid, struct SQGetCharacterListMsg *pMsg  );
    void DeleteCharacter ( Query &query, DNID dnid, struct SQDeleteCharacterMsg *pMsg   );
    void CreateCharacter ( Query &query, DNID dnid, struct SQCreatePlayerMsg *pMsg      );
    void GetCharacter    ( Query &query, DNID dnid, struct SQGetCharacterMsg *pMsg      );
    void SaveCharacter   ( Query &query, DNID dnid, struct SQSaveCharacterMsg *pMsg     );

	//留言相关
	void SendMail        ( Query &query, DNID dnid, struct SQMailSendMsg *pMsg          );
	void RecvMail        ( Query &query, DNID dnid, struct SQMailRecvMsg *pMsg          );
	void DeleteMail      ( Query &query, DNID dnid, struct SQMailDeleteMsg *pMsg        );
	void NewMail         ( Query &query, DNID dnid, struct SQNewMailMsg *pMsg           );

private:
    // 操作cache的函数
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
