////////////////////////////////////////////////////////////
//	FileName		:	NetDBModule.h                     //
//	FileAuthor		:	Luou                              //          
//	FileCreateDate	:	2003-9-26                         //
//	FileDescription	:	NetDBModule上访问数据库的类,      //
//                      由Query继承而来                   //  
//  Reviewed on 26th 2003                                 //
////////////////////////////////////////////////////////////
#pragma    once
#include <list>
#include "querylib\QueryClass.h"
#include "NetWorkModule\DataMsgs.h"
#include "NetWorkModule\MailMsg.h"
#include "NetWorkModule\FactionBBSMsgs.h"
#include "NetWorkModule\logtypedef.h"
#include "NetWorkModule\BuildingTypedef.h"

// 在这一层只封装了操作，所以可以通过 reinterpret_cast 将 Query* 直接转为 CNetDBModule*
class CNetDBModule : public Query
{
public:
	void GetFacBBSText( const DNID dwSID, SQGetFacBBSTextMsg *pMsg );
	void GetFacBBS( const DNID dwSID, SQGetFacBBSMsg *pMsg );
	void SaveFacBBS( const DNID dwSID, SQSaveFacBBSMsg *pMsg );
	void DelFacBBS( const DNID dwSID, SQDelFacBBSMsg *pMsg );
	void SetFacBBSTop( const DNID dwSID, SQSetFacBBSTopMsg *pMsg );
public:
	void SaveFactionDataToDB( SQSaveFactionMsg *pMsg );
	void LoadFactionToLogin( const DNID dwSID, const __int32 nServerID );
public:
	int GetPlayerCharList(LPCSTR pAccount, SCharListData * pCharListData);
    BOOL CreatePlayerData(LPCSTR pAccount,WORD wIndex, SCreateFixProperty &PlayerData);
    BOOL DeletePlayerData(DWORD dwStaticID, LPSTR szName11);

    BOOL GetPlayerWareHouse(LPCSTR pAccount,SPackageItem * PlayerStorage,BYTE bySequence);
    BOOL SavePlayerWareHouse(LPCSTR pAccount,SPackageItem * PlayerStorage,BYTE bySequence);

    BOOL GetPlayerData(LPCSTR pAccount, BYTE byIndex, DWORD &dwSID, SFixBaseData &PlayerData, SFixPackage &PlayerPackage,DWORD serverId);
    BOOL SavePlayerData(DWORD dwServerID, DWORD dwSID, SFixBaseData &PlayerData, SFixPackage &PlayerPackage);
private:
	BOOL SavePlayerEquipment( const DWORD dwSID,  SEquipment* m_Equip);
    BOOL SavePlayerPackageItem( const DWORD dwSID, const SPackageItem* m_pGoods );
public:
    BOOL GetPlayerTask(DWORD dwSID ,SPlayerTasks &PlayerTasks);
    BOOL SavePlayerTask(DWORD dwSID,SPlayerTasks &PlayerTasks);

    BOOL SavePlayerRelations(DWORD dwSID,SPlayerRelation &Relation);
    BOOL GetPlayerRelations(DWORD dwSID,SPlayerRelation &Relation);

    DWORD GetPlayerStaticID(char *pAccount,BYTE byIndex);
    BOOL SaveWareHouseLock( LPCSTR szAccount, BOOL bWarehouseLocked);

    BOOL SaveGMOperation(char *szAccount , SQSaveGMLogMsg * pQSaveGMLogMsg);
    BOOL GetGMOperation(char *szAccount , SAGetGMLogMsg &QSaveGMLogMsg);

//--------------------------------------------------------------------------------------------
	BOOL SendMail(SQMailSendMsg *pMsg,int &AccepterID);
	BOOL RecvMail(SAMailRecvMsg *pMsg,int iStaticID);
	BOOL DeleteMail(SQMailDeleteMsg *pMsg);
	BOOL NewMail(SQNewMailMsg *pMsg);

//--------------------------------------------------------------------------------------------
    BOOL MakeTimeToString(LPSYSTEMTIME SystemTime,char * TimeString);
    BOOL UpdatePlayerData(DWORD dwSID,SFixBaseData &PlayerData);
    BOOL GetPlayerData(char *szName, char *szAccount, DWORD &dwStaticID, SFixBaseData &PlayerData, SFixPackage &PlayerPackage);
private:
	BOOL GetPlayerEquipment( const DWORD dwSID,   SEquipment* m_Equip );
    BOOL GetPlayerPackageItem( const DWORD dwSID, SPackageItem* m_pGoods );
public:
	BOOL DBGetAccoutByName(const char * szName,char * szAccount );

    BOOL GetCharacterData( LPCSTR account, int index, SFixBaseData &base, SFixPackage &package, SPlayerTasks &tasks, SPlayerRelation &relation, DWORD serverId );
    BOOL GetWareHouseData( LPCSTR account, SPackageItem depot[3][MAX_ITEM_NUMBER] );
	BOOL SaveBuildingData( SQBuildingSaveMsg *pmsg );
	BOOL GetBuildingData ( DNID clientDnid, SQBuildingGetMsg *pMsg );

private:
	void TrimRight( char* szBuffer, char* str, size_t bufSize );
};
