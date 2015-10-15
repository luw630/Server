////////////////////////////////////////////////////////////
//	FileName		:	NetDBModule.cpp                   //
//	FileAuthor		:	Luou                              //          
//	FileCreateDate	:	2003-9-26                         //
//	FileDescription	:	NetDBModule上访问数据库的类       //
//                      由Query继承而来                   //
//  Reviewed on 26th 2003                                 //
////////////////////////////////////////////////////////////
#include "NetDBModule.h"
#include "pub/dwt.h"
#include <time.h>
#include "pub\ConstValue.h"
#include "LITESERIALIZER/lite.h"

extern BOOL SendMessage( DNID dnidClient, LPCVOID lpMsg, size_t wSize );
extern void LogMsg( LPCSTR filename, LPCSTR szLogMessage, int ex = -1 );
#define LogErrorMsg( str ) LogMsg( "debugError.txt", str )

#define   SafeArrayInit() \
	VARIANT   VarItemBuffer;	\
	SAFEARRAY tempSA;	\
	tempSA.cbElements = 1;	\
	tempSA.cDims = 1; \
	tempSA.cLocks = 0; \
	tempSA.fFeatures = FADF_HAVEVARTYPE; \
	tempSA.rgsabound[0].lLbound = 0; \
	VarItemBuffer.vt = VT_ARRAY | VT_UI1; 

struct DecimalConvert : public DECIMAL
{
    DecimalConvert( const SItemBase &item )
    { 
        Hi32 = 0;
        Lo64 = item.uniqueId(); // ( ( ( __int64& )item ) >> 24 ) & 0x000000ffffffffff;
        signscale = 0;

        // Lo64 = ( Lo64 & 0x00000000000fffff ) | ( Lo64 & 0xfffffffffff00000 ) 
    }

	DecimalConvert( const QWORD value)
	{
		Hi32 = 0;
		scale = 0;
		sign = ( value & 0x8000000000000000 ) != 0;
        Lo64 = value & 0x7fffffffffffffff;
	}
};


int CNetDBModule::GetPlayerCharList( LPCSTR pAccount, SCharListData *pCharListData)
{
    char sqlcmd[2048];
    sprintf( sqlcmd, "select StaticID, [Index], bySex, byBRON, bySchool, byLevel, Name, "
        "TongName, Title, SysTitle_I, SysTitle_II from PlayerProperty where Account = '%s'", 
        pAccount );

    CString strTemp;
    int iCount = 0;
    try
    {
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        if(FALSE == Query::Open())
            return SAGetCharacterListMsg::RET_ERROR_INFO;

        while(Query::iRecordCount != 0 && !Query::Eof)
        {
            pCharListData[iCount].m_dwStaticID   = Query::Fields->GetItem(_variant_t(("StaticID")))->Value;
            pCharListData[iCount].m_byIndex      = Query::Fields->GetItem(_variant_t(("Index")))->Value;
            pCharListData[iCount].m_byBRON       = Query::Fields->GetItem(_variant_t(("byBRON")))->Value;
            pCharListData[iCount].m_bySchool     = Query::Fields->GetItem(_variant_t(("bySchool")))->Value;   
            pCharListData[iCount].m_wLevel       = Query::Fields->GetItem(_variant_t(("byLevel")))->Value;
            pCharListData[iCount].m_bySex        = Query::Fields->GetItem(_variant_t(("bySex")))->Value;

            strTemp= (char*)(_bstr_t)Query::Fields->GetItem(_variant_t(("Title")))->Value;
            strTemp.TrimRight();
			dwt::strcpy( pCharListData[iCount].m_szTitle, strTemp.GetBuffer(0), sizeof(pCharListData[iCount].m_szTitle) );
            strTemp.ReleaseBuffer();

            strTemp = (char*)(_bstr_t)Query::Fields->GetItem(_variant_t(("Name")))->Value;
            strTemp.TrimRight();
            dwt::strcpy( pCharListData[iCount].m_szName, strTemp.GetBuffer(0), sizeof(pCharListData[iCount].m_szName) );			
            strTemp.ReleaseBuffer();

            strTemp = (char*)(_bstr_t)Query::Fields->GetItem(_variant_t(("TongName")))->Value;
            strTemp.TrimRight();
            dwt::strcpy( pCharListData[iCount].m_szTongName, strTemp.GetBuffer(), sizeof( pCharListData[iCount].m_szTongName) );
            strTemp.ReleaseBuffer();

            pCharListData[iCount].m_dwSysTitle[0] = Query::Fields->GetItem(_variant_t(("SysTitle_I")))->Value;
            pCharListData[iCount].m_dwSysTitle[1] = Query::Fields->GetItem(_variant_t(("SysTitle_II")))->Value;
            if(FALSE == Query::Next())
                return SAGetCharacterListMsg::RET_ERROR_INFO;

            iCount ++;
            if(iCount>=3)
                break;
        }
    }
    catch (_com_error &e) 
    {
        LogErrorMsg("CNetDBModule::GetPlayerCharList"+e.Description());
        return SAGetCharacterListMsg::RET_ERROR_INFO;
    }
    return SAGetCharacterListMsg::RET_SUCCESS;
}

BOOL CNetDBModule::DeletePlayerData(DWORD dwStaticID, LPSTR szName11)
{
    szName11[0] = 0;
    _ParameterPtr retName;

    try
    {
        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "PlayerDB.DeletePlayer";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        m_pCommand->Parameters->GetItem( "@PlayerStaticID" )->Value = dwStaticID;
        retName = m_pCommand->Parameters->GetItem( "@PlayerName" );

        m_pCommand->Execute(NULL,NULL,m_pCommand->CommandType);
    }
    catch (_com_error &e) 
    {
        LogErrorMsg("CNetDBModule::DeletePlayerData" + e.Description());
        return FALSE;
    }

    try
    {
        dwt::strcpy( szName11, _bstr_t( retName->Value ), 11 );
    }
    catch ( ... )
    {
        LogErrorMsg("CNetDBModule::DeletePlayerData ATL_Xcpt...");
        return FALSE;
    }

    return TRUE;
}

int CNetDBModule::GetPlayerTask(DWORD dwSID, SPlayerTasks &PlayerTasks)
{
    char sqlcmd[2048];
    try
    {
        sprintf(sqlcmd, "select * from  PlayerTask where StaticID = %u", dwSID);
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        if(FALSE == Query::Open())
            return 0;

        if (Query::iRecordCount > MAX_TASK_NUMBER)
            return 0;

        int iLooper;
        for(iLooper = 0;iLooper<Query::iRecordCount;iLooper++)
        {
            PlayerTasks.PlayerTask[iLooper].PlayerTaskID = Query::Fields->GetItem(_variant_t("TaskID"))->Value;
            PlayerTasks.PlayerTask[iLooper].PlayerTaskStatus = Query::Fields->GetItem(_variant_t("TaskStatus"))->Value;
            if(FALSE == Query::Next())
                return 0;

            if ( iLooper >= sizeof( SPlayerTasks ) / sizeof( SPlayerTask ) )
                return 1;
        }
    }
    catch(_com_error &e)
    {
        LogErrorMsg("CNetDBModule::GetPlayerTask"+e.Description());
        return 0;
    }
    return 1;
}

int CNetDBModule::CreatePlayerData(LPCSTR pAccount, WORD wIndex, SCreateFixProperty &PlayerData)
{
    DWORD dwProcRet =0;
    DWORD dwResult = 0;

    if ( IsBadStringPtr(pAccount, MAX_ACCOUNT) )
        return FALSE;

    try
    {

        size_t strsize = strlen(pAccount);
        if ( strsize >= MAX_ACCOUNT )
        {
            LogError("溢出！！！（account）", pAccount);
            return FALSE;
        }


        if (strlen(PlayerData.m_szName) > 10)
        {
            PlayerData.m_szName[10] = 0;
            LogError("溢出！！！（name）", PlayerData.m_szName);
        }

        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "PlayerDB.CreateNewPlayer";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        _ParameterPtr ProRet = m_pCommand->Parameters->GetItem( "@RETURN_VALUE" );

        m_pCommand->Parameters->GetItem( "@PlayerVersion" )->Value = PlayerData.m_wVersion;
        m_pCommand->Parameters->GetItem( "@PlayerAccount" )->Value = pAccount;
        m_pCommand->Parameters->GetItem( "@PlayerName" )->Value = PlayerData.m_szName;
        m_pCommand->Parameters->GetItem( "@PlayerIndex" )->Value = wIndex;
        m_pCommand->Parameters->GetItem( "@PlayerSex" )->Value = PlayerData.m_bySex;
        m_pCommand->Parameters->GetItem( "@PlayerBron" )->Value = PlayerData.m_byBRON;
        m_pCommand->Parameters->GetItem( "@PlayerSchool" )->Value = PlayerData.m_bySchool;
        m_pCommand->Parameters->GetItem( "@PlayerRegionID" )->Value = PlayerData.m_wCurRegionID;
        m_pCommand->Parameters->GetItem( "@PlayerSegX" )->Value = PlayerData.m_wSegX;
        m_pCommand->Parameters->GetItem( "@PlayerSegY" )->Value = PlayerData.m_wSegY;
        m_pCommand->Parameters->GetItem( "@MaxStaticID" )->Value = 0;

        m_pCommand->Execute(NULL,NULL,m_pCommand->CommandType);
        if ( ProRet->Value.intVal > 0 )
        {
            PlayerData.m_dwStaticID = ProRet->Value.intVal;
            return 1;
        }
        else if(ProRet->Value.intVal == -1)
        {
            return 2;
        }
        else if(ProRet->Value.intVal == -2)
        {
            return 3;
        }
        else 
        {
            return 4;
        }
    }
    catch (_com_error &e) 
    {
        GUID iGuid = e.GUID();
        LogErrorMsg("CNetDBModule::CreatePlayerData"+e.Description());
        return FALSE;
    }
    return TRUE;
}

BOOL CNetDBModule::SavePlayerRelations(DWORD dwSID, SPlayerRelation &Relation)
{
    char sqlcmd[2048];
    try
    {
        if(m_pRecordset->State)
            m_pRecordset->Close();

        sprintf(sqlcmd, "delete from PlayerRelation where StaticID = %u", dwSID);
        m_pConnection->Execute(sqlcmd, NULL, adCmdText);

        int iIndexA = 0;
        int iIndexB = 0;
        SPlayerRelates *lpRelates = NULL;

        while (iIndexA < sizeof(Relation.m_PlayerFriends) / sizeof(SPlayerRelates))
        {
            lpRelates = &Relation.m_PlayerFriends[iIndexA ++];

            if (lpRelates->szName[0] != 0)
            {
                sprintf(sqlcmd,"insert into PlayerRelation "
                    "(StaticID, TheName, TheRelation, TheStaticID, DearValue) "
                    "values (%u, '%s', %d, %u, %d)",
                    dwSID, lpRelates->szName, lpRelates->byRelation, 
                    lpRelates->dwStaticID, lpRelates->wDearValue);

                m_pConnection->Execute(sqlcmd, NULL, adCmdText);
            }
        }

        while (iIndexB < sizeof(Relation.m_PlayerBlocks) / sizeof(SPlayerRelates))
        {
            lpRelates = &Relation.m_PlayerBlocks[iIndexB ++];

            if (lpRelates->szName[0] != 0)
            {
                sprintf(sqlcmd,"insert into PlayerRelation "
                    "(StaticID, TheName, TheRelation, TheStaticID, DearValue) "
                    "values (%u, '%s', %d, %u, %d)",
                    dwSID, lpRelates->szName, lpRelates->byRelation, 
                    lpRelates->dwStaticID, lpRelates->wDearValue);

                m_pConnection->Execute(sqlcmd, NULL, adCmdText);
            }
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::SavePlayerRelations "+e.Description());
        return FALSE;
    }

    return TRUE;
}

BOOL CNetDBModule::GetPlayerRelations(DWORD dwSID, SPlayerRelation &Relation)
{
    char sqlcmd[2048];

    memset(&Relation, 0, sizeof(SPlayerRelation));

    try
    {
        sprintf(sqlcmd,"select rtrim(TheName) as TheName, TheStaticID, TheRelation, DearValue "
            "from PlayerRelation where StaticID = %u",dwSID);

        if(m_pRecordset->State)
            m_pRecordset->Close();

        m_pRecordset->Open(sqlcmd, m_pConnection.GetInterfacePtr(), 
            adOpenForwardOnly, adLockReadOnly, adCmdText);

        int iIndexA = 0;
        int iIndexB = 0;
        SPlayerRelates *lpRelates = NULL;

        while (!m_pRecordset->adoEOF)
        {
            FieldsPtr fields = m_pRecordset->GetFields();

            int iTheRelation = fields->GetItem("TheRelation")->Value;

            if (iTheRelation == 0)
            {
                if (iIndexA < sizeof(Relation.m_PlayerFriends) / sizeof(SPlayerRelates))
                {
                    lpRelates = &Relation.m_PlayerFriends[iIndexA];
                    iIndexA ++;
                }
            }
            else if (iTheRelation == 1 || iTheRelation == 2)// 2 新加的仇人名单zgc
            {
                if (iIndexB < sizeof(Relation.m_PlayerBlocks) / sizeof(SPlayerRelates))
                {
                    lpRelates = &Relation.m_PlayerBlocks[iIndexB];
                    iIndexB ++;
                }
            }

            if (lpRelates != NULL)
            {
                lpRelates->byRelation = (BYTE)iTheRelation;
                lpRelates->dwStaticID = fields->GetItem("TheStaticID")->Value;
                lpRelates->wDearValue = fields->GetItem("DearValue")->Value;

                dwt::strcpy(lpRelates->szName, 
                    (_bstr_t)fields->GetItem("TheName")->Value, 
                    sizeof(lpRelates->szName));

                lpRelates = NULL;
            }

            m_pRecordset->MoveNext();
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::GetPlayerRelations "+e.Description());
        return FALSE;
    }

    return TRUE;
}

BOOL CNetDBModule::GetPlayerData(LPCSTR pAccount,BYTE byIndex,DWORD &dwSID,SFixBaseData  &PlayerData,SFixPackage &PlayerPackage, DWORD serverId )
{
    char sqlcmd[2048];

    try
    {
        DWORD dwStaticID;
        sprintf(sqlcmd,"select * from PlayerProperty where Account = '%s' and [Index] = %d",pAccount,byIndex );
        Query::m_SQL->Clear();

        Query::m_SQL->Add(sqlcmd);
        if(FALSE == Query::Open())
            return FALSE;

        if(Query::iRecordCount!=0)
        {
            dwStaticID              = Query::Fields->GetItem(_variant_t(("StaticID")))->Value;
            dwSID                   = dwStaticID;
            PlayerData.m_dwConsumePoint   = Query::Fields->GetItem(_variant_t(("Version")))->Value;
            PlayerData.m_bySex      = Query::Fields->GetItem(_variant_t(("bySex")))->Value;
            PlayerData.m_byBRON     = Query::Fields->GetItem(_variant_t(("byBRON")))->Value;
            PlayerData.m_bySchool   = Query::Fields->GetItem(_variant_t(("bySchool")))->Value;

            PlayerData.m_byLevel    = Query::Fields->GetItem(_variant_t(("byLevel")))->Value;

            DECIMAL expdecimal      = Query::Fields->GetItem(_variant_t(("Exp")))->Value;
            PlayerData.m_iExp       = expdecimal.Lo64;

            PlayerData.m_dwMoney    = Query::Fields->GetItem(_variant_t(("Money")))->Value;
            PlayerData.m_dwStoreMoney = Query::Fields->GetItem(_variant_t(("StoreMoney")))->Value;

            PlayerData.m_dwBaseHP   = Query::Fields->GetItem(_variant_t(("BaseHP")))->Value;
            PlayerData.m_wBaseMP    = Query::Fields->GetItem(_variant_t(("BaseMP")))->Value;
            PlayerData.m_wBaseSP    = Query::Fields->GetItem(_variant_t(("BaseSP")))->Value;

            PlayerData.m_dwCurHP    = Query::Fields->GetItem(_variant_t(("CurHP")))->Value;
            PlayerData.m_wCurMP     = Query::Fields->GetItem(_variant_t(("CurMP")))->Value;
            PlayerData.m_wCurSP     = Query::Fields->GetItem(_variant_t(("CurSP")))->Value;

            PlayerData.m_wEN        = Query::Fields->GetItem(_variant_t(("EN")))->Value;
            PlayerData.m_wST        = Query::Fields->GetItem(_variant_t(("ST")))->Value;
            PlayerData.m_wIN        = Query::Fields->GetItem(_variant_t(("IN")))->Value;
            PlayerData.m_wAG        = Query::Fields->GetItem(_variant_t(("AG")))->Value;
            PlayerData.m_wLU        = Query::Fields->GetItem(_variant_t(("LU")))->Value;

            PlayerData.m_byENState  = Query::Fields->GetItem(_variant_t(("ENState")))->Value;
            PlayerData.m_bySTState  = Query::Fields->GetItem(_variant_t(("STState")))->Value;
            PlayerData.m_byINState  = Query::Fields->GetItem(_variant_t(("INState")))->Value;
            PlayerData.m_byAGState  = Query::Fields->GetItem(_variant_t(("AGState")))->Value;

            PlayerData.m_byPoint    = Query::Fields->GetItem(_variant_t(("byPoint")))->Value;
            PlayerData.m_byAmuck    = Query::Fields->GetItem(_variant_t(("byAmuck")))->Value;
            PlayerData.m_sXValue    = Query::Fields->GetItem(_variant_t(("byBadness")))->Value;

            PlayerData.m_wCurRegionID     = Query::Fields->GetItem(_variant_t(("CurRegionID")))->Value;
            PlayerData.m_wSegX      = Query::Fields->GetItem(_variant_t(("SegX")))->Value;
            PlayerData.m_wSegY      = Query::Fields->GetItem(_variant_t(("SegY")))->Value;

            CString szTemp;
            szTemp = (char *)(_bstr_t)Query::Fields->GetItem(_variant_t("Name"))->Value;
            szTemp.TrimRight();			
			//容错处理只要是名字长度越界就不能放入游戏世界
			if ( szTemp.GetLength() >  sizeof(PlayerData.m_szName)-1 )
			 return FALSE;

			dwt::strcpy( PlayerData.m_szName, szTemp.GetBuffer(), sizeof(PlayerData.m_szName));
            szTemp.ReleaseBuffer();


            szTemp = (char *)(_bstr_t)Query::Fields->GetItem(_variant_t("TongName"))->Value;
            szTemp.TrimRight();
			if ( szTemp.GetLength() >  sizeof(PlayerData.m_szTongName)-1 )
			 return FALSE;
            dwt::strcpy( PlayerData.m_szTongName, szTemp.GetBuffer(), sizeof ( PlayerData.m_szTongName ) );
            szTemp.ReleaseBuffer();

            szTemp = (char *)(_bstr_t)Query::Fields->GetItem(_variant_t("Title"))->Value;
            szTemp.TrimRight();
			if ( szTemp.GetLength() >  sizeof(PlayerData.m_szTitle)-1 )
			 return FALSE;
            dwt::strcpy(PlayerData.m_szTitle, szTemp.GetBuffer(), sizeof(PlayerData.m_szTitle));
            szTemp.ReleaseBuffer();


            PlayerData.m_dwSysTitle[0]     = Query::Fields->GetItem(_variant_t(("SysTitle_I")))->Value;
            PlayerData.m_dwSysTitle[1]     = Query::Fields->GetItem(_variant_t(("SysTitle_II")))->Value;
            PlayerData.m_CurTelergy = Query::Fields->GetItem(_variant_t(("CurTelergyLevel")))->Value;
            PlayerData.m_byTelergyStep = Query::Fields->GetItem(_variant_t(("TelergyStep")))->Value;
            PlayerData.m_wMaxHPOther = Query::Fields->GetItem(_variant_t(("MaxHPOther")))->Value;
            PlayerData.m_wMaxMPOther = Query::Fields->GetItem(_variant_t(("MaxMPOther")))->Value;
            PlayerData.m_wMaxSPOther = Query::Fields->GetItem(_variant_t(("MaxSPOther")))->Value;
            PlayerData.m_byPKValue   = Query::Fields->GetItem(_variant_t(("PKValue")))->Value;
            PlayerData.m_byVenapointCount  = Query::Fields->GetItem(_variant_t(("VenapointCount")))->Value;
            PlayerData.m_dwVenapointState[0] = Query::Fields->GetItem(_variant_t(("VenapointState1")))->Value;
            PlayerData.m_dwVenapointState[1] = Query::Fields->GetItem(_variant_t(("VenapointState2")))->Value;
            PlayerData.m_dwVenapointState[2] = Query::Fields->GetItem(_variant_t(("VenapointState3")))->Value;
            PlayerData.m_dwVenapointState[3] = Query::Fields->GetItem(_variant_t(("VenapointState4")))->Value;
            PlayerData.m_dwVenapointState[4] = Query::Fields->GetItem(_variant_t(("VenapointState5")))->Value;
            PlayerData.m_dwVenapointState[5] = Query::Fields->GetItem(_variant_t(("VenapointState6")))->Value;
            PlayerData.m_dwVenapointState[6] = Query::Fields->GetItem(_variant_t(("VenapointState7")))->Value;
            PlayerData.m_dwVenapointState[7] = Query::Fields->GetItem(_variant_t(("VenapointState8")))->Value;
            PlayerData.m_dwVenapointState[8] = Query::Fields->GetItem(_variant_t(("VenapointState9")))->Value;
            PlayerData.m_wAddHPSpeed         = Query::Fields->GetItem(_variant_t(("AddHPSpeed")))->Value;
            PlayerData.m_wAddMPSpeed         = Query::Fields->GetItem(_variant_t(("AddMPSpeed")))->Value;
            PlayerData.m_wAddSPSpeed         = Query::Fields->GetItem(_variant_t(("AddSPSpeed")))->Value;
            PlayerData.m_byNTime             = Query::Fields->GetItem(_variant_t(("NTime")))->Value;
            PlayerData.m_byLeaveTime         = Query::Fields->GetItem(_variant_t(("LeaveTime")))->Value;
            PlayerData.m_nStartTime[0]       = Query::Fields->GetItem(_variant_t(("StartTimeI")))->Value;
            PlayerData.m_nStartTime[1]       = Query::Fields->GetItem(_variant_t(("StartTimeII")))->Value;
            PlayerData.m_wScriptID           = Query::Fields->GetItem(_variant_t(("ScriptID")))->Value;
            PlayerData.m_wScriptIcon         = Query::Fields->GetItem(_variant_t(("ScriptIcon")))->Value;
            PlayerData.m_bWarehouseLocked    = Query::Fields->GetItem(_variant_t(("WarehouseLocked")))->Value;
            DECIMAL decimal                  = Query::Fields->GetItem(_variant_t(("MarryDate")))->Value;
            PlayerData.m_qwMarryDate         = decimal.Lo64;

            szTemp = (char *)(_bstr_t)Query::Fields->GetItem(_variant_t("MateName"))->Value;
            szTemp.TrimRight();
            dwt::strcpy(PlayerData.m_szMateName, szTemp.GetBuffer(), 11);
            szTemp.ReleaseBuffer();

            sprintf(sqlcmd,"select * from PlayerProperty where Account = '%s' and WarehouseLocked <> 0", pAccount);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;

            PlayerData.m_bWarehouseLocked = (Query::iRecordCount != 0);

            sprintf(sqlcmd,"select * from PlayerTelergy where StaticID = %u",dwStaticID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;

            int iIndex = 0;

            while(!Query::Eof && Query::iRecordCount != 0)
            {
                PlayerData.m_Telergy[iIndex].wTelergyID = Query::Fields->GetItem(_variant_t(("TelergyID")))->Value;
                PlayerData.m_Telergy[iIndex].byTelergyLevel  = Query::Fields->GetItem(_variant_t(("TelergyLevel")))->Value;
                PlayerData.m_Telergy[iIndex].dwTelergyVal = Query::Fields->GetItem(_variant_t(("TelergyVal")))->Value;
                if(iIndex >= MAX_EQUIPTELERGY-1)
                    break;
                iIndex ++;
                if(FALSE == Query::Next())
                    return FALSE;
            }

			//------------------读取新的扩展心法---------------------------------------
			sprintf(sqlcmd,"select * from ExtraTelergy where StaticID = %u",dwStaticID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;           

            if (!Query::Eof && Query::iRecordCount != 0)
            {
                SExtraTelergy &et = PlayerData.m_ExtraTelergy;
                et.telergy[0].byTelergyLevel = Query::Fields->GetItem(_variant_t(("TelergyLevel1")))->Value;
                et.telergy[0].dwTelergyVal   = Query::Fields->GetItem(_variant_t(("TelergyVal1")))->Value;
				et.telergy[0].wTelergyID     = Query::Fields->GetItem(_variant_t(("TelergyID1")))->Value;
                et.telergy[1].byTelergyLevel = Query::Fields->GetItem(_variant_t(("TelergyLevel2")))->Value;
                et.telergy[1].dwTelergyVal   = Query::Fields->GetItem(_variant_t(("TelergyVal2")))->Value;
				et.telergy[1].wTelergyID     = Query::Fields->GetItem(_variant_t(("TelergyID2")))->Value;

                et.state[0] = Query::Fields->GetItem(_variant_t(("state1")))->Value;
                et.state[1] = Query::Fields->GetItem(_variant_t(("state2")))->Value;
                et.segment = Query::Fields->GetItem(_variant_t(("segment")))->Value;
                et.charge = Query::Fields->GetItem(_variant_t(("charge")))->Value;
            }
			//----------------------------------------------------------------------------

            iIndex = 0;
            ////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////
            //取玩家衣服的颜色
            sprintf(sqlcmd,"select * from PlayerEquipColor where StaticID = %u",dwStaticID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;

            if(Query::iRecordCount != 0)
            {
                PlayerData.m_wEquipCol[0] = Query::Fields->GetItem(_variant_t(("EquipCol1")))->Value;
                PlayerData.m_wEquipCol[1] = Query::Fields->GetItem(_variant_t(("EquipCol2")))->Value;
                PlayerData.m_wEquipCol[2] = Query::Fields->GetItem(_variant_t(("EquipCol3")))->Value;
                PlayerData.m_wEquipCol[3] = Query::Fields->GetItem(_variant_t(("EquipCol4")))->Value;

                PlayerData.m_byEquipColCount[0] = Query::Fields->GetItem(_variant_t(("EquipColCount1")))->Value;
                PlayerData.m_byEquipColCount[1] = Query::Fields->GetItem(_variant_t(("EquipColCount2")))->Value;
                PlayerData.m_byEquipColCount[2] = Query::Fields->GetItem(_variant_t(("EquipColCount3")))->Value;
                PlayerData.m_byEquipColCount[3] = Query::Fields->GetItem(_variant_t(("EquipColCount4")))->Value;
            }

            /*
            ////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////
            //取玩家的该服务器上的帮派名
			Query::PrepareProcedure( "p_GetMemberFactionName" );
			Query::m_pCommand->Prepared;
			Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= serverId;
			Query::m_pCommand->Parameters->GetItem( "@MemberName" )->Value		= PlayerData.m_szName;
			_RecordsetPtr prset = Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
			if( prset != NULL && prset->State && !prset->adoEOF )
			{
				FieldsPtr fields = prset->GetFields();
				TrimRight( PlayerData.m_szTongName, (char *)(_bstr_t)fields->GetItem("FactionName")->Value, sizeof( PlayerData.m_szTongName ) );
			}
            */

            ////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////
            //取玩家的技能
            sprintf(sqlcmd,"select * from PlayerSkill where StaticID = %u",dwStaticID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;

            iIndex = 0;

            while(!Query::Eof && Query::iRecordCount != 0)
            {
                iIndex = Query::Fields->GetItem(_variant_t(("Number")))->Value;
                iIndex --;
                if(iIndex<0)
                {                
                    if(FALSE == Query::Next())
                        return FALSE;
                    continue;
                }
                PlayerData.m_pSkills[iIndex].byTypeID = Query::Fields->GetItem(_variant_t(("byTypeID")))->Value;
                PlayerData.m_pSkills[iIndex].byLevel  = Query::Fields->GetItem(_variant_t(("byLevel")))->Value;
                PlayerData.m_pSkills[iIndex].dwProficiency = Query::Fields->GetItem(_variant_t(("Proficiency")))->Value;
                if(FALSE == Query::Next())
                    return FALSE;
            }

            sprintf(sqlcmd,"select * from PlayerTroopsSkill where StaticID = %u",dwStaticID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            if(FALSE == Query::Open())
                return FALSE;

            while(!Query::Eof && Query::iRecordCount != 0)
            {
                iIndex = Query::Fields->GetItem(_variant_t(("Number")))->Value;
                iIndex --;
                if(iIndex< 0)
                {                
                    if(FALSE == Query::Next())
                        return FALSE;
                    continue;
                }
                PlayerData.m_pTroopsSkills[iIndex].byTypeID = Query::Fields->GetItem(_variant_t(("byTypeID")))->Value;
                PlayerData.m_pTroopsSkills[iIndex].byLevel  = 0;
                PlayerData.m_pTroopsSkills[iIndex].dwProficiency = 0;
                if(FALSE == Query::Next())
                    return FALSE;
            }    
			//Get Equipment data
			if( !this->GetPlayerEquipment(  dwStaticID, PlayerData.m_Equip ) )
				return FALSE;

			//get package data
			if( !this->GetPlayerPackageItem( dwStaticID, PlayerPackage.m_pGoods ) )
				return FALSE;
            sprintf(sqlcmd,"update PlayerProperty set LoadTime = GetDate(),savetime = GetDate() where StaticID = %u",dwSID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            Query::ExecSQL();
        }
        else
        {
            return FALSE;
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::GetPlayerData"+e.Description());
        return FALSE;
    }
    return TRUE;
}
BOOL CNetDBModule::GetPlayerEquipment( const DWORD dwSID,   SEquipment* m_Equip )
{
	char sqlcmd[2048];
	sprintf(sqlcmd,"select UniqueIF,Number,ItemBuffer from NEW_PlayerEquipment where StaticID = %u",dwSID);
    Query::m_SQL->Clear();
    Query::m_SQL->Add(sqlcmd);
    if(FALSE == Query::Open())
        return FALSE;
	INT iIndex = 0;
    while(!Query::Eof && Query::iRecordCount != 0 )
    {
        iIndex = Query::Fields->GetItem(_variant_t(("Number")))->Value;
        iIndex--;
        if(iIndex<0)
        {                
            if(FALSE == Query::Next())
                return FALSE;
            continue;
        }
		else if ( iIndex >= 8 )
			return FALSE;

		if( !Query::GetBolb( "ItemBuffer", &m_Equip[iIndex], sizeof(SEquipment) ) )
			return FALSE;

        _variant_t uid = Query::Fields->GetItem( _variant_t("UniqueIF") )->Value;
        if ( ( uid.vt != VT_DECIMAL ) || ( ( uid.decVal.Lo64 & 0xffffff0000000000 ) != 0 ) )
            return FALSE;

        m_Equip[iIndex].uniqueId( uid.decVal.Lo64 );

        if(FALSE == Query::Next())
            return FALSE;
    }
	return TRUE;
}
BOOL CNetDBModule::GetPlayerPackageItem( const DWORD dwSID, SPackageItem* m_pGoods )
{
	char sqlcmd[2048];
	sprintf(sqlcmd,"select UniqueIF, ByCellX, ByCellY, ItemBuffer from NEW_PlayerPackageItem where StaticID = %u", dwSID);
    Query::m_SQL->Clear();
    Query::m_SQL->Add(sqlcmd);
    if(FALSE == Query::Open())
        return FALSE;
	int iIndex = 0;
    while (!Query::Eof && Query::iRecordCount != 0) 
    {
		SRawItemBuffer* buffer = &static_cast<SRawItemBuffer&>(m_pGoods[iIndex]);
        m_pGoods[iIndex].byCellX  = ( BYTE )Query::Fields->GetItem(_variant_t(("ByCellX")))->Value;
        m_pGoods[iIndex].byCellY  = ( BYTE )Query::Fields->GetItem(_variant_t(("ByCellY")))->Value;
		if( !Query::GetBolb( "ItemBuffer", (LPVOID&)buffer, sizeof(SRawItemBuffer) ) )
			return FALSE;

        _variant_t uid = Query::Fields->GetItem( _variant_t("UniqueIF") )->Value;
        if ( ( uid.vt != VT_DECIMAL ) || ( ( uid.decVal.Lo64 & 0xffffff0000000000 ) != 0 ) )
            return FALSE;

        m_pGoods[iIndex].uniqueId( uid.decVal.Lo64 );

        SEquipment*     equip = reinterpret_cast<SEquipment*>(buffer);
        if(FALSE == Query::Next())
            return FALSE;
			iIndex++;
    }
	return TRUE;
}
DWORD CNetDBModule::GetPlayerStaticID(char *pAccount,BYTE byIndex)
{
    char sqlcmd[2048];

    DWORD dwTheStaticID = 0;
    sprintf(sqlcmd,"select StaticID from PlayerProperty where Account ='%s' and [Index] = %d",pAccount,byIndex);
    try
    {
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        if(FALSE == Query::Open())
            return 0;
        if(!Query::Eof && Query::iRecordCount != 0)
        {
            dwTheStaticID = Query::Fields->GetItem(_variant_t(("StaticID")))->Value;
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::GetPlayerStaticID" + e.Description());
        return 0;
    }
    return dwTheStaticID;
};

BOOL CNetDBModule::GetPlayerWareHouse(LPCSTR pAccount,SPackageItem * PlayerStorage,BYTE bySequence)
{
	char sqlcmd[2048];
    if ( IsBadStringPtr(pAccount, MAX_ACCOUNT) )
        return FALSE;
    try
    {
        switch(bySequence)
        {
        case 1:
            sprintf(sqlcmd,"select UniqueIF,ItemBuffer,ByCellX,ByCellY from NEW_APlayerWarehouseI  where Account = '%s'",pAccount);
            break;
        case 2:
            sprintf(sqlcmd,"select UniqueIF,ItemBuffer,ByCellX,ByCellY from NEW_APlayerWarehouseII where Account = '%s'",pAccount);
            break;
        case 3:
            sprintf(sqlcmd,"select UniqueIF,ItemBuffer,ByCellX,ByCellY from NEW_APlayerWarehouseIII where Account = '%s'",pAccount);
            break;
        }
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        if(FALSE == Query::Open())
            return FALSE;
        int iIndex = 0;
        while (!Query::Eof && Query::iRecordCount != 0) 
        {
			SRawItemBuffer* buffer = &static_cast<SRawItemBuffer&>(PlayerStorage[iIndex]);
			PlayerStorage[iIndex].byCellX  = ( BYTE )Query::Fields->GetItem(_variant_t(("ByCellX")))->Value;
			PlayerStorage[iIndex].byCellY  = ( BYTE )Query::Fields->GetItem(_variant_t(("ByCellY")))->Value;
			if( !Query::GetBolb( "ItemBuffer", buffer, sizeof(SRawItemBuffer) ) )
				return FALSE;

            _variant_t uid = Query::Fields->GetItem( _variant_t("UniqueIF") )->Value;
            if ( ( uid.vt != VT_DECIMAL ) || ( ( uid.decVal.Lo64 & 0xffffff0000000000 ) != 0 ) )
                return FALSE;

            PlayerStorage[iIndex].uniqueId( uid.decVal.Lo64 );

            if(FALSE == Query::Next())
                return FALSE;
			iIndex++;
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModoule::GetPlayerWareHouse" + e.Description());
        return FALSE;
    }
    return TRUE;
  }

BOOL CNetDBModule::SavePlayerTask(DWORD dwSID ,SPlayerTasks &PlayerTasks)
{
    char sqlcmd[2048];

    int iLooper ;

    sprintf(sqlcmd,"delete PlayerTask where StaticID = %u",dwSID);

    Query::m_SQL->Clear();
    Query::m_SQL->Add(sqlcmd);
    Query::BeginTransAction();
    Query::ExecSQL();

    try
    {
        for(iLooper = 0;iLooper<MAX_TASK_NUMBER;iLooper++)
        {
            if ( PlayerTasks.PlayerTask[iLooper].PlayerTaskID != 0 )
            {
                sprintf(sqlcmd,"insert into PlayerTask values(%d,%d,%d,0,0)",dwSID,PlayerTasks.PlayerTask[iLooper].PlayerTaskID,PlayerTasks.PlayerTask[iLooper].PlayerTaskStatus);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                Query::ExecSQL();
            }
        }
    }
    catch(_com_error e)
    {
        Query::RollbackTransAction();
        LogErrorMsg("CNetDBModule::SavePlayerTask" + e.Description());
        return FALSE;
    }
    Query::EndTransAction();
    return TRUE;
}

BOOL CNetDBModule::SavePlayerWareHouse(LPCSTR pAccount,SPackageItem * PlayerStorage,BYTE bySequence)
{
	
	char DataBaseName[128] = {0};
    if ( IsBadStringPtr(pAccount, MAX_ACCOUNT) )
        return FALSE;

	int ntime = static_cast<int>( timeGetTime() );

    switch(bySequence)
    {
    case 1:
        _snprintf( DataBaseName, sizeof ( DataBaseName )-1,"NEW_APlayerWareHouseI");
        break;
    case 2:
       _snprintf( DataBaseName, sizeof ( DataBaseName )-1,"NEW_APlayerWareHouseII"); 
        break;
    case 3:
       _snprintf( DataBaseName, sizeof ( DataBaseName )-1,"NEW_APlayerWareHouseIII"); 
        break;
    }

	 // try
  //  {
		//
		//char sqlcmd[2048];
  //      Query::BeginTransAction();
  //      sprintf(sqlcmd,"DELETE %s WHERE Account = '%s'",DataBaseName,pAccount);
  //      Query::m_SQL->Clear();
  //      Query::m_SQL->Add(sqlcmd);
  //      Query::ExecSQL();

		//sprintf( sqlcmd, "SELECT * FROM %s WHERE Account='%s'", DataBaseName, pAccount );
		//Query::Close();
		//Query::m_SQL->Clear();
		//Query::m_SQL->Add( sqlcmd );
		//if( !Query::Open() )
		//	return FALSE;
		//BOOL SavePlayerWareHouse=FALSE;
  //      for (int iLooper = 0;iLooper < MAX_ITEM_NUMBER ;iLooper++)
  //      {
  //          if( iLooper < wItemNumber )
  //          {
  //              if( PlayerStorage[iLooper].wIndex != 0)
  //              {
		//			SRawItemBuffer& buffer = static_cast<SRawItemBuffer&>(PlayerStorage[iLooper]);
		//			const SEquipment* equip = reinterpret_cast<const SEquipment*>(&buffer);
		//			Query::m_pRecordset->AddNew();
		//			Query::m_pRecordset->PutCollect("Account",_variant_t( pAccount ));	
		//			Query::m_pRecordset->PutCollect("ItemIndex",_variant_t(equip->wIndex));
		//			Query::m_pRecordset->PutCollect("UniqueIF",_variant_t( *(__int64*)equip ));
		//			Query::m_pRecordset->PutCollect("ByCellX",_variant_t(  PlayerStorage[iLooper].byCellX ));
		//			Query::m_pRecordset->PutCollect("ByCellY",_variant_t(  PlayerStorage[iLooper].byCellY ));
		//			Query::AddBlob( "ItemBuffer",  &buffer, sizeof(buffer) );
		//			SavePlayerWareHouse = TRUE;
  //              }
  //         }
  //      }
		//if( SavePlayerWareHouse )
		//	Query::m_pRecordset->Update();
  //      Query::m_SQL->Clear();
  //  }
		try
		{
			Query::BeginTransAction();
			Query::PrepareProcedure( "p_SavePlayerWareHouse" );
			Query::m_pCommand->Prepared;

			SafeArrayInit();	
			for ( int iLooper = 0; iLooper < MAX_ITEM_NUMBER ; ++iLooper )
			{
				if( PlayerStorage[iLooper].wIndex != 0)
				{
					Query::m_pCommand->Parameters->GetItem( "@Account" )->Value = pAccount ;
					Query::m_pCommand->Parameters->GetItem( "@ix" )->Value = PlayerStorage[iLooper].byCellX ;
					Query::m_pCommand->Parameters->GetItem( "@iY" )->Value = PlayerStorage[iLooper].byCellY;

					Query::m_pCommand->Parameters->GetItem( "@itemindex" )->Value = ( WORD )PlayerStorage[iLooper].wIndex;
					Query::m_pCommand->Parameters->GetItem( "@UniqueID" )->Value = DecimalConvert( PlayerStorage[iLooper] );

					tempSA.pvData = static_cast< SItemBase* >(  &PlayerStorage[iLooper] );;
					VarItemBuffer.parray = &tempSA;
			        tempSA.rgsabound[0].cElements = PlayerStorage[iLooper].size;		
					Query::m_pCommand->Parameters->GetItem( "@buf")->Value = VarItemBuffer;
					Query::m_pCommand->Parameters->GetItem( "@LastTime" )->Value = ntime;
					Query::m_pCommand->Parameters->GetItem("@WareHouseIndex")->Value = bySequence;
					Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);	
					assert( Query::ReturnOK() );		
				}

			}
			//将本次操作没有更新的数据全部删掉
			
			CString tmpSQL;
			tmpSQL.Format("delete %s where account = '%s' and UpdateTime <> %d",DataBaseName,pAccount,ntime);
			BOOL result = Query::ExecCommandSQL( tmpSQL.GetBuffer() );
            assert( result );
			Query::EndTransAction();
			return  TRUE; 

		}		
    catch(_com_error e)
    {
        Query::RollbackTransAction();
        LogErrorMsg("CNetDBModule::SavePlayerWareHouse"+e.Description());
        return FALSE;
    }
    Query::EndTransAction();
	return TRUE;
   
}    

BOOL CNetDBModule::SavePlayerData( DWORD dwServerID, DWORD dwSID , SFixBaseData &PlayerData , SFixPackage &PlayerPackage)
{
	char sqlcmd[2048];

	try
	{  
        /* 
        ####################################################################################
        现在不处理帮派成员的保存，因为成员和帮派的对应关系已经直接设置到登录服务器的缓存中了
        ####################################################################################
		// 如果需要的话就保存玩家帮派数据,这里面的数据是不可靠的，目的只是为了暂时保存下
		// 玩家的帮派数据，正确的数据以SQSaveFactionMsg消息为准
		// 在储存过程中如果帮派里有数据，就更新，没有就直接删除掉
		Query::PrepareProcedure( "p_SaveFactionMemberList" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= dwServerID;
		Query::m_pCommand->Parameters->GetItem( "@Name" )->Value			= PlayerData.m_szName;
		Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= PlayerData.m_szTongName;
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
		assert( Query::ReturnOK() );
        */

		sprintf(sqlcmd,"update PlayerProperty set "
			"Version   = %d,"
			//"bySex     = %d,"
			//"byBRON    = %d,"
			//"bySchool  = %d,"
			"byLevel   = %d,"
            "Exp       = %I64d,"
            "Money     = %u,"
            "StoreMoney= %u,"
            "BaseHP    = %d,"
            "BaseMP    = %d,"
            "BaseSP    = %d,"
            "CurHP     = %d,"
            "CurMP     = %d,"
            "CurSP     = %d,"
            "EN        = %d,"
            "ST        = %d,"
            "[IN]      = %d,"
            "AG        = %d,"
            "LU        = %d,"
            "ENState   = %d,"
            "STState   = %d,"
            "INState   = %d,"
            "AGState   = %d,"
            "byPoint   =%d,"
            "byAmuck   = %d,"
            "byBadness = %d,"
            "CurRegionID = %d,"
            "SegX      =%d,"
            "SegY      = %d,"
           // "Name      = '%s',"
            "TongName  ='%s',"
            "Title     ='%s',"
            "SysTitle_I =%d,"
            "SysTitle_II =%d,"
            "CurTelergyLevel =%d,"
            "TelergyStep = %d,"
            "MaxHPOther = %d,"
            "MaxMPOther = %d,"
            "MaxSPOther = %d,"
            "PKValue =%d,"
            "VenapointCount =%d,"
            "VenapointState1 = %d,"
            "VenapointState2 = %d,"
            "VenapointState3 = %d,"
            "VenapointState4 = %d,"
            "VenapointState5 = %d,"
            "VenapointState6 = %d,"
            "VenapointState7 = %d,"
            "VenapointState8 = %d,"
            "VenapointState9 = %d,"
            "AddHPSpeed = %d,"
            "AddMPSpeed = %d,"
            "AddSPSpeed = %d,"
            "NTime = %d,"
            "LeaveTime = %d,"
            "StartTimeI = %d,"
            "StartTimeII = %d,"
            "SaveTime  = GETDATE(),"
           // "LoadTime  = GETDATE(), "
            "OnlineTime = OnlineTime + DateDiff(Minute,SaveTime,GetDate()), "
            "ScriptID = %d, "
            "ScriptIcon = %d, "
            "WareHouseLocked = %d, "
            "MateName = '%s', "
            "MarryDate = %I64u "
            "where "
            "StaticID = %u",
            PlayerData.m_dwConsumePoint,
            //PlayerData.m_bySex,
            //PlayerData.m_byBRON,
            //PlayerData.m_bySchool,
            PlayerData.m_byLevel,
            PlayerData.m_iExp,
            PlayerData.m_dwMoney,
            PlayerData.m_dwStoreMoney,
            PlayerData.m_dwBaseHP,
            PlayerData.m_wBaseMP,
            PlayerData.m_wBaseSP,
            PlayerData.m_dwCurHP,
            PlayerData.m_wCurMP,
            PlayerData.m_wCurSP,
            PlayerData.m_wEN,
            PlayerData.m_wST,
            PlayerData.m_wIN,
            PlayerData.m_wAG,
            PlayerData.m_wLU,
            PlayerData.m_byENState,
            PlayerData.m_bySTState,
            PlayerData.m_byINState,
            PlayerData.m_byAGState,
            PlayerData.m_byPoint,
            PlayerData.m_byAmuck,
            PlayerData.m_sXValue,
            PlayerData.m_wCurRegionID,
            PlayerData.m_wSegX,
            PlayerData.m_wSegY,
           // PlayerData.m_szName,
            PlayerData.m_szTongName,
            PlayerData.m_szTitle,
            PlayerData.m_dwSysTitle[0],
            PlayerData.m_dwSysTitle[1],
            PlayerData.m_CurTelergy,
            PlayerData.m_byTelergyStep,
            PlayerData.m_wMaxHPOther,
            PlayerData.m_wMaxMPOther,
            PlayerData.m_wMaxSPOther,
            PlayerData.m_byPKValue,
            PlayerData.m_byVenapointCount,
            PlayerData.m_dwVenapointState[0],
            PlayerData.m_dwVenapointState[1],
            PlayerData.m_dwVenapointState[2],
            PlayerData.m_dwVenapointState[3],
            PlayerData.m_dwVenapointState[4],
            PlayerData.m_dwVenapointState[5],
            PlayerData.m_dwVenapointState[6],
            PlayerData.m_dwVenapointState[7],
            PlayerData.m_dwVenapointState[8],
            PlayerData.m_wAddHPSpeed,
            PlayerData.m_wAddMPSpeed,
            PlayerData.m_wAddSPSpeed,
            PlayerData.m_byNTime,
            PlayerData.m_byLeaveTime,
            PlayerData.m_nStartTime[0],
            PlayerData.m_nStartTime[1],
            PlayerData.m_wScriptID,
            PlayerData.m_wScriptIcon,
            PlayerData.m_bWarehouseLocked,
            PlayerData.m_szMateName,
            PlayerData.m_qwMarryDate,
            dwSID
            );

        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        if (!Query::ExecSQL())
        {
            char buffer[1024];

            SYSTEMTIME s;
            GetLocalTime(&s);

            sprintf(buffer, "[%d-%d-%d %d=%d=%d].mem", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);
            LogErrorMsg(sqlcmd);
            void TraceMemory(LPVOID pDumpEntry, int iOffset, unsigned int uiSize, LPCSTR szFilename);
            TraceMemory(&PlayerData, 0, sizeof(PlayerData), buffer);

        }

        //////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////
        //保存玩家的颜色
       
        sprintf(sqlcmd, "update PlayerEquipColor set "
            "EquipCol1 = %d,"
            "EquipCol2 = %d,"
            "EquipCol3 = %d,"
            "EquipCol4 = %d,"
            "EquipColCount1 = %d,"
            "EquipColCount2 = %d,"
            "EquipColCount3 = %d,"
            "EquipColCount4 = %d "
            "where StaticID = %u ",
            PlayerData.m_wEquipCol[0],
            PlayerData.m_wEquipCol[1],
            PlayerData.m_wEquipCol[2],
            PlayerData.m_wEquipCol[3],
            PlayerData.m_byEquipColCount[0],
            PlayerData.m_byEquipColCount[1],
            PlayerData.m_byEquipColCount[2],
            PlayerData.m_byEquipColCount[3],
            dwSID );

        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        Query::ExecSQL();

        //update PlayerTelergy
        sprintf(sqlcmd,"delete PlayerTelergy where StaticID = %u",dwSID);
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        Query::ExecSQL();

        for (int iLooper = 0; iLooper<MAX_EQUIPTELERGY; iLooper++)
        {
            if(PlayerData.m_Telergy[iLooper].byTelergyLevel != 0)
            {
                sprintf(sqlcmd,"insert into PlayerTelergy values (%u,%d,%d,%u)",dwSID,PlayerData.m_Telergy[iLooper].wTelergyID,PlayerData.m_Telergy[iLooper].dwTelergyVal,PlayerData.m_Telergy[iLooper].byTelergyLevel);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                Query::ExecSQL();
            }
        }

		//---------------------------------------------------------------------------------------
		 //保存新的扩展心法
        sprintf(sqlcmd,"delete ExtraTelergy where StaticID = %u",dwSID);
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        Query::ExecSQL();

        SExtraTelergy &et = PlayerData.m_ExtraTelergy;
        sprintf(sqlcmd,"insert into ExtraTelergy values (%u,%d,%d,%u,%d,%d,%u,%d,%d,%d,%d)",
			dwSID,
            et.telergy[0].wTelergyID, et.telergy[0].dwTelergyVal, et.telergy[0].byTelergyLevel,
            et.telergy[1].wTelergyID, et.telergy[1].dwTelergyVal, et.telergy[1].byTelergyLevel,
            et.state[0], et.state[1], ( int )et.segment, ( int )et.charge );

        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        Query::ExecSQL();

        // debug for telergy lost
        //void DEBUG_FOR_TelergyLost_UpdateTelergyData(int state, char name[11], STelergy *p6data);
        //DEBUG_FOR_TelergyLost_UpdateTelergyData(2, PlayerData.m_szName, PlayerData.m_Telergy);

        //update PlayerEquip
		if( !this->SavePlayerEquipment( dwSID, PlayerData.m_Equip ) )
			return false;
        /*
        for (int iLooper = 0;iLooper < 8 ;iLooper ++)
        {
            if(PlayerData.m_Equip[iLooper].byType != 0)
            {
                sprintf(sqlcmd,"select StaticID from PlayerEquipment where StaticID = %u and Number =%d",dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                if(FALSE == Query::Open())
                    return FALSE;

                if(Query::iRecordCount != 0)
                {

                    sprintf(sqlcmd,"update PlayerEquipment set "
                                   "bySize = %d, "
                                   "byType = %d, "
                                   "byIndex = %d, "
                                   "bySocketNumber=%d, "
                                   "bySocket1 = %d, "
                                   "bySocket2 = %d, "
                                   "bySocket3 = %d, "
                                   "bySocket4 = %d, "
                                   "bySocket5 = %d, "
                                   "Seed = %d "
                                   "where StaticID = %u and "
                                   "Number = %d",
                                   PlayerData.m_Equip[iLooper].bySize,
                                   PlayerData.m_Equip[iLooper].byType,
                                   PlayerData.m_Equip[iLooper].wIndex,
                                   PlayerData.m_Equip[iLooper].bySocketNumber,
                                   PlayerData.m_Equip[iLooper].bySocket[0],
                                   PlayerData.m_Equip[iLooper].bySocket[1],
                                   PlayerData.m_Equip[iLooper].bySocket[2],
                                   PlayerData.m_Equip[iLooper].bySocket[3],
                                   PlayerData.m_Equip[iLooper].bySocket[4],
                                   PlayerData.m_Equip[iLooper].dwSeed,
                                   dwSID,
                                   iLooper+1
                                   );

                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
                else
                {
                    sprintf(sqlcmd,"insert into  PlayerEquipment (StaticID,Number,bySize,byType,byIndex,"
                        "bySocketNumber,bySocket1,bySocket2,bySocket3,bySocket4,bySocket5,Seed)"
                        "values (%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d) ",
                        dwSID,
                        iLooper+1,
                        PlayerData.m_Equip[iLooper].bySize,
                        PlayerData.m_Equip[iLooper].byType,
                        PlayerData.m_Equip[iLooper].wIndex,
                        PlayerData.m_Equip[iLooper].bySocketNumber,
                        PlayerData.m_Equip[iLooper].bySocket[0],
                        PlayerData.m_Equip[iLooper].bySocket[1],
                        PlayerData.m_Equip[iLooper].bySocket[2],
                        PlayerData.m_Equip[iLooper].bySocket[3],
                        PlayerData.m_Equip[iLooper].bySocket[4],
                        PlayerData.m_Equip[iLooper].dwSeed
                        );       
                    try
                    {
                        Query::m_SQL->Clear();
                        Query::m_SQL->Add(sqlcmd);
                        Query::ExecSQL();
                    }
                    catch (_com_error e) 
                    {
                        LogErrorMsg("CNetDBModule::SavePlayerData PlayerEquipment"+e.Description());
                        return FALSE;
                    }
                }
            }
            else
            {
                sprintf(sqlcmd,"delete PlayerEquipment where StaticID = %u and Number = %d",dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                Query::ExecSQL();
            }
        }*/

        //update PlayerPackageItem
		if( !this->SavePlayerPackageItem( dwSID, PlayerPackage.m_pGoods ) )
			return false;
        /*try
        {
            sprintf(sqlcmd,"delete PlayerPackageItem where StaticID = %u",dwSID);
            Query::m_SQL->Clear();
            Query::m_SQL->Add(sqlcmd);
            Query::BeginTransAction();
            Query::ExecSQL();

            for (int iLooper = 0;iLooper < MAX_ITEM_NUMBER ;iLooper++)
            {
                if(PlayerPackage.m_pGoods[iLooper].byType != 0)
                {
                    sprintf(sqlcmd,"insert into PlayerPackageItem values (%d,%d,%d,%d,"
                        "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",
                        dwSID,
                        iLooper+1,
                        PlayerPackage.m_pGoods[iLooper].bySize,
                        PlayerPackage.m_pGoods[iLooper].byType,
                        PlayerPackage.m_pGoods[iLooper].wIndex,
                        PlayerPackage.m_pGoods[iLooper].byCellX,
                        PlayerPackage.m_pGoods[iLooper].byCellY,
                        PlayerPackage.m_pGoods[iLooper].byDummy[0],
                        PlayerPackage.m_pGoods[iLooper].byDummy[1],
                        PlayerPackage.m_pGoods[iLooper].byDummy[2],
                        PlayerPackage.m_pGoods[iLooper].byDummy[3],
                        PlayerPackage.m_pGoods[iLooper].byDummy[4],
                        PlayerPackage.m_pGoods[iLooper].byDummy[5],
                        PlayerPackage.m_pGoods[iLooper].byDummy[6],
                        PlayerPackage.m_pGoods[iLooper].byDummy[7],
                        PlayerPackage.m_pGoods[iLooper].byDummy[8],
                        PlayerPackage.m_pGoods[iLooper].byDummy[9],
                        PlayerPackage.m_pGoods[iLooper].byDummy[10],
                        PlayerPackage.m_pGoods[iLooper].byDummy[11],
                        PlayerPackage.m_pGoods[iLooper].byDummy[12],
                        PlayerPackage.m_pGoods[iLooper].byDummy[13],
                        PlayerPackage.m_pGoods[iLooper].byDummy[14],
                        PlayerPackage.m_pGoods[iLooper].byDummy[15]);

                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
                else
                {
                    sprintf(sqlcmd,"delete PlayerPackageItem where StaticID = %u and Number >= %d",dwSID,iLooper+1);
                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                    break;
                }
            }
        }
        catch(_com_error &e)
        {
            Query::RollbackTransAction();
            LogErrorMsg("CNetDBModule::SavePlayerData"+e.Description());
            return FALSE;
        }
        Query::EndTransAction();
        */
        //update PlayerSkill
        for(int iLooper = 0 ;iLooper < 12;iLooper ++)
        {
            if(PlayerData.m_pSkills[iLooper].byTypeID < 54 && PlayerData.m_pSkills[iLooper].byLevel != 0)
            {
                sprintf(sqlcmd,"select StaticID from PlayerSkill where StaticID = %u and Number = %d",dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                if(FALSE == Query::Open())
                    return FALSE;

                if(Query::IsEmpty() == FALSE)
                {
                    sprintf(sqlcmd,"update PlayerSkill set "
                        "byTypeID = %d, "
                        "byLevel  = %d, "
                        "Proficiency = %d "
                        "where StaticID = %u and Number = %d",
                        PlayerData.m_pSkills[iLooper].byTypeID,
                        PlayerData.m_pSkills[iLooper].byLevel,
                        PlayerData.m_pSkills[iLooper].dwProficiency,
                        dwSID,
                        iLooper+1
                        );
                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
                else
                {
                    sprintf(sqlcmd,"insert into PlayerSkill values (%d,%d,%d,%d,%d) ",
                        dwSID,
                        iLooper+1,
                        PlayerData.m_pSkills[iLooper].byTypeID,
                        PlayerData.m_pSkills[iLooper].byLevel,
                        PlayerData.m_pSkills[iLooper].dwProficiency
                        );
                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
            }
            else
            {
                sprintf(sqlcmd,"delete PlayerSkill where StaticID = %u and Number = %d",dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                Query::ExecSQL();
            }
        }

        //update PlayerTroopsSkill
        for(int iLooper = 0 ;iLooper < 4;iLooper ++)
        {
            if(PlayerData.m_pTroopsSkills[iLooper].byTypeID != 0)
            {
                sprintf(sqlcmd,"select StaticID from PlayerTroopsSkill where StaticID = %u and Number = %d" ,dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                if(FALSE == Query::Open())
                    return FALSE;

                if(Query::IsEmpty() == FALSE)
                {
                    sprintf(sqlcmd,"update PlayerTroopsSkill set "
                        "byTypeID = %d "
                        "where StaticID = %u and Number = %d",
                        PlayerData.m_pTroopsSkills[iLooper].byTypeID,
                        dwSID,
                        iLooper+1
                        );
                        //",byLevel  = %d "
                        // PlayerData.m_pTroopsSkills[iLooper].byLevel,

                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
                else
                {
                    sprintf(sqlcmd,"insert into PlayerTroopsSkill values(%d,%d,%d,0) ",
                        dwSID,
                        iLooper+1,
                        PlayerData.m_pTroopsSkills[iLooper].byTypeID
                        );
                        //,PlayerData.m_pTroopsSkills[iLooper].byLevel
                    Query::m_SQL->Clear();
                    Query::m_SQL->Add(sqlcmd);
                    Query::ExecSQL();
                }
            }
            else
            {
                sprintf(sqlcmd,"delete PlayerTroopsSkill where StaticID = %u and Number = %d",dwSID,iLooper+1);
                Query::m_SQL->Clear();
                Query::m_SQL->Add(sqlcmd);
                Query::ExecSQL();
            }
        }
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::SavePlayerData"+e.Description());
        return FALSE;
    }
    return TRUE;
};

BOOL CNetDBModule::SaveWareHouseLock( LPCSTR szAccount, BOOL bWarehouseLocked)
{
    char sqlcmd[2048];
    try
    {
        sprintf(sqlcmd,"update PlayerProperty set WarehouseLocked = %d where Account = '%s'", bWarehouseLocked, szAccount);
        Query::m_SQL->Clear();
        Query::m_SQL->Add(sqlcmd);
        Query::ExecSQL();
    }
    catch(_com_error e)
    {
        LogErrorMsg("CNetDBModule::SaveWareHouseLock"+e.Description());
        return FALSE;
    }
    return TRUE;
}

BOOL CNetDBModule::GetGMOperation(char * szAccount,SAGetGMLogMsg &AGetGMLogMsg)
{
    char sqlcmd[2048];

    sprintf(sqlcmd,"select * from GMLog where Account = '%s'",szAccount);
    Query::m_SQL->Clear();
    Query::m_SQL->Add(sqlcmd);
    if(FALSE == Query::Open())
        return FALSE;

    return TRUE;
}

BOOL CNetDBModule::SaveGMOperation(char *szAccount , SQSaveGMLogMsg * pQSaveGMLogMsg)
{
    // ###########################################################
    // 莫名其妙……谁写的代码？？？
    // ###########################################################
    return 0;
}

//---------------------------------------------------------------------------------------------------------

BOOL CNetDBModule::SendMail(SQMailSendMsg *pMsg,int &AccepterID)
{
    int iRet = 0;

    try
    {
        _ParameterPtr adoRet;
        _ParameterPtr adoID;

        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "dbo.sp_WriteNoteMsg";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        adoRet = m_pCommand->Parameters->GetItem("@RETURN_VALUE");
        adoID = m_pCommand->Parameters->GetItem("@AccepterID");

        m_pCommand->Parameters->GetItem("@SenderID")->Value = pMsg->m_iStaticID;
        m_pCommand->Parameters->GetItem("@AccepterName")->Value = pMsg->m_mail.m_szRecver;
        m_pCommand->Parameters->GetItem("@NoteMsg")->Value = pMsg->m_mail.m_szText;

        m_pCommand->Execute(NULL, NULL, m_pCommand->CommandType);

        iRet = adoRet->Value.intVal;

        if (adoID->Value.vt != NULL)
        {
            AccepterID = adoID->Value;
        }
    }

    catch(_com_error &e)
    {
        std::string strError = e.ErrorMessage()+e.Description();

        LogErrorMsg("CNetAccountDBModule::SendMail"+e.Description());

        return FALSE;
    }

    if (iRet != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNetDBModule::RecvMail(SAMailRecvMsg *pMsg,int iStaticID)
{
    int iRet = 0;

    try
    {
        _ParameterPtr adoRet;

        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "dbo.sp_ReadNoteMsg";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        adoRet = m_pCommand->Parameters->GetItem("@RETURN_VALUE");

        m_pCommand->Parameters->GetItem("@AccepterID")->Value = iStaticID;
        m_pCommand->Parameters->GetItem("@nPageIndex")->Value = pMsg->m_iIndex;

        _RecordsetPtr prset;

        prset = m_pCommand->Execute(NULL, NULL, m_pCommand->CommandType);

        if ((prset == NULL) || (!prset->State))
        {
            return FALSE;
        }

        iRet = adoRet->Value.intVal;

        int i = 0;

        while (!prset->adoEOF)
        {
            FieldsPtr fields = prset->GetFields();

            pMsg->m_mail[i].m_bIsRead = fields->GetItem("isRead")->Value;
            pMsg->m_mail[i].m_iNumber = fields->GetItem("nIndex")->Value;

            strcpy(pMsg->m_mail[i].m_szSender,(_bstr_t)fields->GetItem("SenderName")->Value);
            strcpy(pMsg->m_mail[i].m_szTime,(_bstr_t)fields->GetItem("SendTime")->Value);
            strcpy(pMsg->m_mail[i].m_szText,(_bstr_t)fields->GetItem("NoteMsg")->Value);

            prset->MoveNext();

            i++;
        }

    }

    catch(_com_error &e)
    {
        std::string strError = e.ErrorMessage()+e.Description();

        LogErrorMsg("CNetAccountDBModule::DeleteMail"+e.Description());

        return FALSE;
    }

    if (iRet != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNetDBModule::DeleteMail(SQMailDeleteMsg *pMsg)
{
    int iRet = 0;

    try
    {
        _ParameterPtr adoRet;

        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "dbo.sp_DeleteNoteMsg";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        adoRet = m_pCommand->Parameters->GetItem("@RETURN_VALUE");

        m_pCommand->Parameters->GetItem("@AccepterID")->Value = pMsg->m_iStaticID;
        m_pCommand->Parameters->GetItem("@nIndex")->Value = pMsg->m_iNumber;

        m_pCommand->Execute(NULL, NULL, m_pCommand->CommandType);

        iRet = adoRet->Value.intVal;
    }

    catch(_com_error &e)
    {
        std::string strError = e.ErrorMessage()+e.Description();

        LogErrorMsg("CNetAccountDBModule::DeleteMail"+e.Description());

        return FALSE;
    }

    if (iRet != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNetDBModule::NewMail(SQNewMailMsg *pMsg)
{
    int iRet = 0;
    int iOut = 0;

    try
    {
        _ParameterPtr adoRet;
        _ParameterPtr ParOut;

        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "dbo.sp_CheckNewMsg";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        adoRet = m_pCommand->Parameters->GetItem("@RETURN_VALUE");
        ParOut = m_pCommand->Parameters->GetItem("@Result");

        m_pCommand->Parameters->GetItem("@AccepterID")->Value = pMsg->m_iStaticID;

        m_pCommand->Execute(NULL, NULL, m_pCommand->CommandType);

        iRet = adoRet->Value.intVal;

        if (ParOut->Value.vt != VT_NULL)
        {
            iOut = ParOut->Value;
        }
    }

    catch(_com_error &e)
    {
        std::string strError = e.ErrorMessage()+e.Description();

        LogErrorMsg("CNetAccountDBModule::DeleteMail"+e.Description());

        return FALSE;
    }

    if (iRet != 0)
    {
        return FALSE;
    }

    if (iOut == 1)
    {
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------------------------------------

BOOL CNetDBModule::DBGetAccoutByName(const char * szName,char * szAccount)
{
    int iRet = 0;
    try
    {
        _ParameterPtr adoRet;
        _ParameterPtr ParOut;		
        m_pCommand->ActiveConnection = m_pConnection;
        m_pCommand->CommandText = "dbo.sp_GetAccountByName";
        m_pCommand->CommandType = adCmdStoredProc;
        m_pCommand->Parameters->Refresh();

        adoRet = m_pCommand->Parameters->GetItem("@RETURN_VALUE");
        ParOut = m_pCommand->Parameters->GetItem("@szAccount");
        m_pCommand->Parameters->GetItem("@szName")->Value = szName;
        m_pCommand->Execute(NULL, NULL, m_pCommand->CommandType);
        iRet = adoRet->Value.intVal;
        if ( 0 == iRet)
        {
            CString szTmpStr;
            szTmpStr = ParOut->Value;
            dwt::strcpy(szAccount,szTmpStr.GetBuffer(),MAX_ACCOUNT);
            szTmpStr.ReleaseBuffer();
            return TRUE;
        } else
            return FALSE;
    }
    catch(_com_error &e)
    {
        std::string strError = e.ErrorMessage()+e.Description();

        LogErrorMsg("CNetAccountDBModule::sp_GetAccountByName"+e.Description());

        return FALSE;
    }
}

BOOL CNetDBModule::GetCharacterData( LPCSTR account, int index, SFixBaseData &base, SFixPackage &package, SPlayerTasks &tasks, SPlayerRelation &relation, DWORD serverId )
{
    DWORD dwSID = 0;

    if ( !GetPlayerData( account, index, dwSID, base, package, serverId ) )
        return false;

    if ( !GetPlayerTask( dwSID, tasks ) )
        return false;

    if ( !GetPlayerRelations( dwSID, relation ) )
        return false;

    return true;
}

BOOL CNetDBModule::GetWareHouseData( LPCSTR account, SPackageItem depot[3][MAX_ITEM_NUMBER] )
{
    if ( !GetPlayerWareHouse( account, depot[0], 1 ) )
        return false;

    if ( !GetPlayerWareHouse( account, depot[1], 2 ) )
        return false;

    if ( !GetPlayerWareHouse( account, depot[2], 3 ) )
        return false;

    return true;
}
BOOL CNetDBModule::SavePlayerEquipment( const DWORD dwSID,  SEquipment* m_Equip )
{
	
	int ntime = static_cast<int>( timeGetTime() );
	try
	{
		Query::BeginTransAction();
		Query::PrepareProcedure( "p_SavePlayerEquipment" );
		Query::m_pCommand->Prepared;

		SafeArrayInit();
	
		for ( int iLooper = 0; iLooper <  SEquipment::EQUIPMENT_MAXNUMBER  ; ++iLooper )
		{
			
			if( m_Equip[iLooper].wIndex != 0 )
			{
				Query::m_pCommand->Parameters->GetItem( "@sid" )->Value = dwSID;
				Query::m_pCommand->Parameters->GetItem( "@number" )->Value = iLooper+1;
				Query::m_pCommand->Parameters->GetItem( "@itemindex" )->Value = ( WORD )m_Equip[iLooper].wIndex;
				Query::m_pCommand->Parameters->GetItem( "@UniqueID" )->Value = DecimalConvert( m_Equip[iLooper] );
			
				tempSA.rgsabound[0].cElements = m_Equip[iLooper].size;
				tempSA.pvData =  &m_Equip[iLooper];
	
				VarItemBuffer.vt = VT_ARRAY | VT_UI1; 
				VarItemBuffer.parray = &tempSA;
				Query::m_pCommand->Parameters->GetItem( "@buf")->Value = VarItemBuffer;
				Query::m_pCommand->Parameters->GetItem( "@LastTime" )->Value = ntime;
				Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);				
				assert( Query::ReturnOK() );
			}
		
		}
		//将本次操作没有更新的数据全部删掉
		Query::PrepareProcedure("p_DeleteTable");
		Query::m_pCommand->Parameters->GetItem( "@TableName")->Value = "New_PlayerEquipment";
		Query::m_pCommand->Parameters->GetItem( "@SID")->Value = dwSID; 
		Query::m_pCommand->Parameters->GetItem( "@updateTime")->Value = ntime;
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);	
		assert( Query::ReturnOK() );		  
		Query::EndTransAction();
		return  TRUE; 
	}
    catch(_com_error &e)
    {
        Query::RollbackTransAction();
        LogErrorMsg("CNetDBModule::p_SavePlayerEquipment"+e.Description());
        return FALSE;
    }


}

BOOL CNetDBModule::SavePlayerPackageItem( const DWORD dwSID, const SPackageItem* m_pGoods )
{
	try
	{	int ntime = static_cast<int>( timeGetTime() );
		Query::BeginTransAction();
		Query::PrepareProcedure( "p_SavePlayerPackage" );
		Query::m_pCommand->Prepared;

		SafeArrayInit();	
		for ( int iLooper = 0; iLooper < MAX_ITEM_NUMBER ; ++iLooper )
		{
			if( m_pGoods[iLooper].wIndex != 0)
			{
				Query::m_pCommand->Parameters->GetItem( "@sid" )->Value = dwSID;
				Query::m_pCommand->Parameters->GetItem( "@iX" )->Value = m_pGoods[iLooper].byCellX ;
				Query::m_pCommand->Parameters->GetItem( "@iY" )->Value = m_pGoods[iLooper].byCellY;

				Query::m_pCommand->Parameters->GetItem( "@itemindex" )->Value = ( WORD )m_pGoods[iLooper].wIndex;
				Query::m_pCommand->Parameters->GetItem( "@UniqueID" )->Value = DecimalConvert( m_pGoods[iLooper] );

				tempSA.pvData = const_cast< SItemBase* >( &static_cast< const SItemBase& >( m_pGoods[iLooper] ) );
				tempSA.rgsabound[0].cElements = m_pGoods[iLooper].size;		
				VarItemBuffer.parray = &tempSA;
				Query::m_pCommand->Parameters->GetItem( "@buf")->Value = VarItemBuffer;
				Query::m_pCommand->Parameters->GetItem( "@LastTime" )->Value = ntime;
				Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
				assert( Query::ReturnOK() );		
			}
		}

		//将本次操作没有更新的数据全部删掉
		Query::PrepareProcedure("p_DeleteTable");
		Query::m_pCommand->Parameters->GetItem( "@TableName")->Value = "New_PlayerPackageItem";
		Query::m_pCommand->Parameters->GetItem( "@SID")->Value = dwSID; 
		Query::m_pCommand->Parameters->GetItem( "@updateTime")->Value = ntime;
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);	
		assert( Query::ReturnOK() );		  
		Query::EndTransAction();
		return  TRUE; 
	}
    catch(_com_error &e)
    {
        Query::RollbackTransAction();
        LogErrorMsg("CNetDBModule::SavePlayerPackageItem"+e.Description());
        return FALSE;
    }

}

__declspec( thread ) char tempFactionBuffer[ sizeof( SaveFactionData[ MAX_FACTION_NUMBER ] ) ];
__declspec( thread ) char tempMsgBuffer[ sizeof( SAGetFactionMsg ) ];

void CNetDBModule::SaveFactionDataToDB( SQSaveFactionMsg *pMsg )
{
	if ( pMsg == NULL || pMsg->nFactions >= MAX_FACTION_NUMBER )
		return;

	// 首先将数据全部提取出来
	SaveFactionData *stFaction = ( SaveFactionData* )tempFactionBuffer;
	LPBYTE streamIter = (LPBYTE)pMsg->streamFaction;
	const DWORD dwFactionMax = pMsg->nFactions;
	for( DWORD n = 0; n < dwFactionMax; n++ )
	{
		// 读取帮派头数据
		stFaction[n].stFaction = *( SaveFactionData::SaveFactionInfo* )streamIter;
		streamIter += sizeof( SaveFactionData::SaveFactionInfo );

		// 成员数据
		size_t memberSize = sizeof( SaveFactionData::SaveMemberInfo ) * stFaction[n].stFaction.byMemberNum;
		memcpy( stFaction[n].stMember, streamIter, memberSize );
		streamIter += memberSize;
	}

	try
	{
		// 更新帮派列表
		{
			Query::BeginTransAction();
			Query::PrepareProcedure( "p_SaveFactionList" );
			Query::m_pCommand->Prepared;
			for( DWORD n = 0; n < dwFactionMax; n++ )
			{
				//Query::m_pCommand->Parameters->GetItem( "@FactionLastTime" )->Value = stFaction[n].stFaction.dwLastTime;
				Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
				Query::m_pCommand->Parameters->GetItem( "@UpdateTime" )->Value		= pMsg->nTime;

                Query::m_pCommand->Parameters->GetItem( "@FactionMaster" )->Value	= stFaction[n].stMember->szName;
				Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= stFaction[n].stFaction.szFactionName;
				Query::m_pCommand->Parameters->GetItem( "@FactionCreator" )->Value	= stFaction[n].stFaction.szCreatorName;
				Query::m_pCommand->Parameters->GetItem( "@FactionMemo" )->Value		= stFaction[n].stFaction.szFactionMemo;
				Query::m_pCommand->Parameters->GetItem( "@FactionLevel" )->Value	= stFaction[n].stFaction.byFactionLevel;
				Query::m_pCommand->Parameters->GetItem( "@FactionActive" )->Value	= stFaction[n].stFaction.dwFactionActive;
				Query::m_pCommand->Parameters->GetItem( "@FactionMoney" )->Value	= stFaction[n].stFaction.iMoney;
				Query::m_pCommand->Parameters->GetItem( "@FactionWood" )->Value		= stFaction[n].stFaction.dwWood;
				Query::m_pCommand->Parameters->GetItem( "@FactionStone" )->Value	= stFaction[n].stFaction.dwStone;
				Query::m_pCommand->Parameters->GetItem( "@FactionMine" )->Value		= stFaction[n].stFaction.dwMine;
				Query::m_pCommand->Parameters->GetItem( "@FactionPaper" )->Value	= stFaction[n].stFaction.dwPaper;
				Query::m_pCommand->Parameters->GetItem( "@FactionJade" )->Value		= stFaction[n].stFaction.dwJade;
				Query::m_pCommand->Parameters->GetItem( "@FactionId" )->Value	= stFaction[n].stFaction.factionId;

				Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
				assert( Query::ReturnOK() );	
			}

			// 将本次操作没有更新的数据全部删掉
			if( pMsg->bSaveMode )
			{
				Query::PrepareProcedure( "p_DeleteFactionData" );
				Query::m_pCommand->Parameters->GetItem( "@TableName" )->Value		= "FactionList";
				Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
				Query::m_pCommand->Parameters->GetItem( "@updateTime" )->Value		= pMsg->nTime;
				Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
				assert( Query::ReturnOK() );
			}

			Query::EndTransAction();
		}

		// 更新帮派成员列表
		{
			Query::BeginTransAction();
			Query::PrepareProcedure( "p_SaveFactionMemberList" );
			Query::m_pCommand->Prepared;
			for( DWORD n = 0; n < dwFactionMax; n++ )
			{
				for( int i = 0; i < stFaction[n].stFaction.byMemberNum; i++ )
				{
					Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
					Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= stFaction[n].stFaction.szFactionName;
					Query::m_pCommand->Parameters->GetItem( "@Name" )->Value			= stFaction[n].stMember[i].szName;
					Query::m_pCommand->Parameters->GetItem( "@CustomTitle" )->Value		= stFaction[n].stMember[i].szCustomTitle;
					Query::m_pCommand->Parameters->GetItem( "@UpdateTime" )->Value		= pMsg->nTime;
					Query::m_pCommand->Parameters->GetItem( "@Donation" )->Value		= stFaction[n].stMember[i].iDonation;
					Query::m_pCommand->Parameters->GetItem( "@Accomplishment" )->Value  = stFaction[n].stMember[i].dwAccomplishment;
					Query::m_pCommand->Parameters->GetItem( "@WeeklySalary" )->Value	= stFaction[n].stMember[i].dwWeeklySalary;
					Query::m_pCommand->Parameters->GetItem( "@JoinTime" )->Value		= stFaction[n].stMember[i].dwJoinTime;
					DWORD dwFactionRight = 0;
					memcpy( &dwFactionRight, &stFaction[n].stMember[i], sizeof( dwFactionRight ) );
					Query::m_pCommand->Parameters->GetItem( "@FactionRight" )->Value	= dwFactionRight;
					Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
					assert( Query::ReturnOK() );	
				}
			}

			// 将本次操作没有更新的数据全部删掉
			if( pMsg->bSaveMode )
			{
				Query::PrepareProcedure( "p_DeleteFactionData" );
				Query::m_pCommand->Parameters->GetItem( "@TableName" )->Value		= "FactionMemberList";
				Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
				Query::m_pCommand->Parameters->GetItem( "@updateTime" )->Value		= pMsg->nTime;
				Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
				assert( Query::ReturnOK() );
			}

			Query::EndTransAction();
		}
	}
	catch(_com_error &e)
	{
		Query::RollbackTransAction();
		LogErrorMsg( "CNetDBModule::SaveFactionData"+e.Description() );
		return;
	}
}

void CNetDBModule::LoadFactionToLogin( const DNID dwSID, const __int32 nServerID )
{
	if( nServerID <= 0 )
		return;

	SaveFactionData *stFaction = ( SaveFactionData* )tempFactionBuffer;
	SAGetFactionMsg &factionMsg = * new ( tempMsgBuffer ) SAGetFactionMsg();
	ZeroMemory( stFaction,sizeof(stFaction) );

	_RecordsetPtr prset;
	int nFactionMax = 0;

	try
	{ 
		// 取帮派列表数据
		Query::PrepareProcedure( "p_GetFactionList" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value = nServerID;
		prset = Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
		if ((prset == NULL) || (!prset->State))
        {
			MessageBox( 0,"获取帮派列表时 p_GetFactionList 执行出现错误",0,0 );
            exit( -1 );
        }

		while( !prset->adoEOF )
		{
			FieldsPtr fields = prset->GetFields();

            memset( &stFaction[nFactionMax], 0, sizeof( stFaction[0] ) );

            TrimRight( stFaction[nFactionMax].stMember->szName,        (char *)(_bstr_t)fields->GetItem( _variant_t( "FactionMaster" ) )->Value, sizeof( stFaction[nFactionMax].stMember->szName ) );
			TrimRight( stFaction[nFactionMax].stFaction.szFactionName, (char *)(_bstr_t)fields->GetItem( _variant_t( "FactionName" ) )->Value, sizeof( stFaction[nFactionMax].stFaction.szFactionName ) );
			TrimRight( stFaction[nFactionMax].stFaction.szFactionMemo, (char *)(_bstr_t)fields->GetItem( _variant_t( "FactionMemo" ) )->Value, sizeof( stFaction[nFactionMax].stFaction.szFactionMemo ) );
			TrimRight( stFaction[nFactionMax].stFaction.szCreatorName, (char *)(_bstr_t)fields->GetItem( _variant_t( "FactionCreator" ) )->Value, sizeof( stFaction[nFactionMax].stFaction.szCreatorName ) );
			//stFaction[nFactionMax].stFaction.dwLastTime		= ( DWORD )fields->GetItem( "FactionLastTime" )->Value;
			stFaction[nFactionMax].stFaction.byFactionLevel = fields->GetItem( "FactionLevel" )->Value;
			stFaction[nFactionMax].stFaction.dwFactionActive= ( DWORD )fields->GetItem( "FactionActive" )->Value;
			stFaction[nFactionMax].stFaction.iMoney			= ( DWORD )fields->GetItem( "FactionMoney" )->Value;	
			stFaction[nFactionMax].stFaction.dwWood			= ( DWORD )fields->GetItem( "FactionWood" )->Value;	
			stFaction[nFactionMax].stFaction.dwStone		= ( DWORD )fields->GetItem( "FactionStone" )->Value;	
			stFaction[nFactionMax].stFaction.dwMine			= ( DWORD )fields->GetItem( "FactionMine" )->Value;	
			stFaction[nFactionMax].stFaction.dwPaper		= ( DWORD )fields->GetItem( "FactionPaper" )->Value;	
			stFaction[nFactionMax].stFaction.dwJade			= ( DWORD )fields->GetItem( "FactionJade" )->Value;	
			
			stFaction[nFactionMax].stFaction.factionId	 = fields->GetItem( "FactionId" )->Value;
			
			nFactionMax++;
			prset->MoveNext();
		}

		// 取所有玩家的帮派数据
		int nFactionIndex = 0;
		while( nFactionIndex < nFactionMax )
		{
			// 取帮派里面所有人的数据
			Query::PrepareProcedure( "p_GetFactionForMemberList" );
			Query::m_pCommand->Prepared;
			Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= nServerID;
			Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= stFaction[nFactionIndex].stFaction.szFactionName;
			prset = Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
			if( prset == NULL || !prset->State )
			{
				nFactionIndex++;
				continue;
			}

            stFaction[nFactionIndex].stMember->dwJoinTime = -1;
            SaveFactionData::SaveMemberInfo *iter = &stFaction[nFactionIndex].stMember[1];
            
            int byMemberIndex = 0;
			while( !prset->adoEOF && byMemberIndex < MAX_MEMBER_NUMBER ) 
			{
                byMemberIndex ++;
				FieldsPtr fields = prset->GetFields();
				
				// 成员数据
				iter->dwJoinTime    =	( DWORD )fields->GetItem(_variant_t(("JoinTime")))->Value;
				iter->iDonation	    =	( DWORD )fields->GetItem(_variant_t(("MemberDonation")))->Value;
				iter->dwAccomplishment  =	( DWORD )fields->GetItem(_variant_t(("MemberAccomplishment")))->Value;
				iter->dwWeeklySalary    =	( DWORD )fields->GetItem(_variant_t(("WeeklySalary")))->Value;
				reinterpret_cast< DWORD& >( *iter ) = ( DWORD )fields->GetItem(_variant_t(("FactionRight")))->Value;
				TrimRight( iter->szName, (char*)(_bstr_t)fields->GetItem( _variant_t( ( "MemberName" ) ) )->Value, sizeof( iter->szName ) );
				TrimRight( iter->szCustomTitle, (char*)(_bstr_t)fields->GetItem( _variant_t( ( "MemberCustomTitle" ) ) )->Value, sizeof( iter->szCustomTitle ) );

                if ( dwt::strcmp( iter->szName, stFaction[nFactionIndex].stMember->szName, sizeof( iter->szName ) ) == 0 )
                {
                    stFaction[nFactionIndex].stMember[0] = *iter;
                    reinterpret_cast< DWORD& >( *iter ) |= ~0x7ffff;
                    iter->Title = 8;
                }
                else
                {
                    if ( iter->Title == 8 )
                    {   // 特殊处理，不可能有两个帮主
                        reinterpret_cast< DWORD& >( *iter ) &= 0x7ffff;
                        iter->Title = 0;
                    }

				    iter ++;
                }

				// 下一个数据
				prset->MoveNext();
			}

            if ( stFaction[nFactionIndex].stMember->dwJoinTime == -1 )
            {
                extern BOOL TraceInfo( LPCSTR szFileName, LPCSTR szFormat, ... );
                TraceInfo( "factionErr.log", "帮派[%s]没找到帮主[%s]！", 
                    stFaction[nFactionIndex].stFaction.szFactionName, stFaction[nFactionIndex].stMember->szName );
                byMemberIndex = 0;
            }

			stFaction[nFactionIndex].stFaction.byMemberNum = byMemberIndex;
			nFactionIndex++;
        }
	}
	catch(_com_error &e)
	{
		Query::RollbackTransAction();
		LogErrorMsg( "CNetDBModule::LoadFactionToLogin"+e.Description() );
		return;
	}

	SaveFactionData *iter = stFaction;
    factionMsg.isLast = FALSE;
	LPBYTE streamIter = (LPBYTE)factionMsg.streamFaction;
	DWORD nFactions = 0;

	for ( int n = 0; n < nFactionMax; n ++, iter ++ )
	{
        if ( iter->stFaction.byMemberNum == 0 )
            continue;

		DWORD size = sizeof( SaveFactionData::SaveFactionInfo ) + iter->stFaction.byMemberNum * sizeof( SaveFactionData::SaveMemberInfo );
		factionMsg.dwSendSize += size;

		// 写入帮派头数据
		*( SaveFactionData::SaveFactionInfo* )streamIter = iter->stFaction;
		streamIter += sizeof( SaveFactionData::SaveFactionInfo );
		
		// 成员数据
		memcpy( streamIter, iter->stMember, sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum );
		streamIter += sizeof( SaveFactionData::SaveMemberInfo ) * iter->stFaction.byMemberNum;

		nFactions ++;

		if ( factionMsg.dwSendSize + size >= sizeof( factionMsg.streamFaction ) )
		{
			factionMsg.nFactions = nFactions;
			SendMessage( dwSID, &factionMsg, factionMsg.dwSendSize + 
				( sizeof( factionMsg ) - sizeof( factionMsg.streamFaction ) ) );

			streamIter = (LPBYTE)factionMsg.streamFaction;
			nFactions = 0;
			factionMsg.dwSendSize = 0;
		}
	}

    // 最后这个消息用于通知登陆服务器读取正常完成！所以就算没有读到任何帮派，也需要发送该消息
    factionMsg.isLast = TRUE;
	factionMsg.nFactions = nFactions;
	SendMessage( dwSID, &factionMsg, factionMsg.dwSendSize + 
		( sizeof( factionMsg ) - sizeof( factionMsg.streamFaction ) ) );
}

void CNetDBModule::TrimRight( char* szBuffer, char* str, size_t bufSize )
{
	CString strTemp;
	strTemp = str;
	strTemp.TrimRight();
	dwt::strcpy( szBuffer, strTemp.GetBuffer( 0 ),static_cast<int> (bufSize) );
}

BOOL CNetDBModule::SaveBuildingData( SQBuildingSaveMsg *pmsg)
{
	int ntime = static_cast<int>( timeGetTime() );
	Query::BeginTransAction();
	Query::PrepareProcedure( "p_SaveBuildingData" );
	Query::m_pCommand->Prepared;
	try
	{
        // 这个是特殊处理，==4 时属于没有数据！
        if ( ( ( LPWORD )pmsg->Buffers )[ 0 ] > 4 )
        {
            lite::Serialreader slr( pmsg->Buffers );
            while ( slr.curSize() < slr.maxSize() )
            {
                lite::Variant lvt = slr();
                if ( lvt.dataType != lite::Variant::VT_POINTER ) 
                    continue;

                lite::Serialreader slr2( ( void* )lvt._pointer );
                lite::Variant lvt2 = slr2();
                if ( ( lvt2.dataType != lite::Variant::VT_POINTER ) || ( lvt2.dataSize != sizeof( SaveBlock_fixed ) ) )
                    continue;

                const SaveBlock_fixed *block = ( const SaveBlock_fixed* )lvt2._pointer;

			    Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value = pmsg->dwServerID;
			    Query::m_pCommand->Parameters->GetItem( "@ParentID" )->Value = block->parentId;
			    Query::m_pCommand->Parameters->GetItem( "@ItemIndex" )->Value = block->index;
			    Query::m_pCommand->Parameters->GetItem( "@UniqueId" )->Value = DecimalConvert( block->UniqueId() );

			    SafeArrayInit();
			    tempSA.rgsabound[0].cElements = lvt.dataSize;
			    tempSA.pvData = ( void* )lvt._pointer;

			    VarItemBuffer.vt = VT_ARRAY | VT_UI1; 
			    VarItemBuffer.parray = &tempSA;
			    Query::m_pCommand->Parameters->GetItem( "@Buffer" )->Value = VarItemBuffer;
			    Query::m_pCommand->Parameters->GetItem( "@LastTime" )->Value = ntime;
			    Query::m_pCommand->Execute(NULL, NULL, Query::m_pCommand->CommandType );				
			    assert( Query::ReturnOK() );
		    }
        }

		//将本次操作没有更新的数据全部删掉
		Query::PrepareProcedure( "p_DelBuildingData" );
		Query::m_pCommand->Parameters->GetItem( "@ParentID")->Value = pmsg->wParentID;
		Query::m_pCommand->Parameters->GetItem( "@ServerID")->Value = pmsg->dwServerID; 
		Query::m_pCommand->Parameters->GetItem( "@updateTime")->Value = ntime;
		Query::m_pCommand->Execute( NULL, NULL, Query::m_pCommand->CommandType );	
		assert( Query::ReturnOK() );	
		Query::EndTransAction();
		return  TRUE; 
	}
    catch( lite::Xcpt &e )
	{
		Query::RollbackTransAction();
        LogErrorMsg( _bstr_t( "CNetDBModule::SaveBuildingData lite::" ) + e.GetErrInfo() );
		return FALSE;
	}
	catch(_com_error &e)
	{
		Query::RollbackTransAction();
		LogErrorMsg("CNetDBModule::SaveBuildingData "+e.Description());
		return FALSE;
    }
}

//返回值是当前数据包的总大小，如果为0代表数据有错误
__declspec( thread ) BYTE tmpMsgBuffer[ 0xffff ];
__declspec( thread ) BYTE commBuffer[0xfff];

BOOL CNetDBModule::GetBuildingData( DNID clientDnid, SQBuildingGetMsg *pMsg )
{
	TCHAR sqlcmd[1024];
	_snprintf(sqlcmd, sizeof sqlcmd, 
        "SELECT Buffer FROM BuildingList WHERE ParentID = %d AND ServerID = %d", 
        pMsg->wParentID, pMsg->dwServerID );
    Query::m_SQL->Clear();
    Query::m_SQL->Add(sqlcmd);
    if(FALSE == Query::Open())
        return FALSE;

	WORD wIndex = 0;	
	DWORD dwTotalSize = 0;
	size_t size = 0;	//本次从数据库中读取的缓冲区的大小	

    SABuildingGetMsg &msg = * new ( tmpMsgBuffer ) SABuildingGetMsg;
    msg.dwZoneID = pMsg->dwZoneID;
    msg.wParentID = pMsg->wParentID;
    msg.retMsg = SABuildingGetMsg::RET_SUCCESS;
	lite::Serializer slm( msg.Buffers, sizeof( tmpMsgBuffer ) - sizeof( msg ) );
	
    while ( !Query::Eof && Query::iRecordCount != 0 ) 
    {	
		ZeroMemory( commBuffer, sizeof( commBuffer ) );
		size =  Query::GetBolb( "Buffer", commBuffer, sizeof( commBuffer ) );
		if ( size == 0 )	
		    return FALSE;

        try { slm( commBuffer, size ); }
        catch ( lite::FullBuffer & )
        {
            // 缓存已经满了，先把前面部分数据发送了来
            msg.WBufSize = sizeof( tmpMsgBuffer ) - slm.EndEdition();
            SendMessage( clientDnid, &msg, msg.WBufSize );
            slm.Rewind();
        }

		dwTotalSize += size;
		size = 0;
        if ( !Query::Next() )
            return FALSE;
		wIndex++;
    }

    msg.WBufSize = sizeof( tmpMsgBuffer ) - slm.EndEdition();
    SendMessage( clientDnid, &msg, msg.WBufSize );

	//Msg.wNumbers = wIndex;
	//Msg.WBufSize =  sizeof BYTE * SENDER_MAXBUFFER - slm.EndEdition();

	//考虑到数据对齐问题,前面一部份wActualSize 不太固定，所以用总大小 - 动态BUF大小
	//dwTotalSize += wIndex * ( sizeof msg.Buffers[0] - sizeof msg.Buffers[0].buffer );
	return TRUE;
}

void CNetDBModule::GetFacBBSText( const DNID dwSID, SQGetFacBBSTextMsg *pMsg )
{
	if( pMsg == NULL )
		return;

	if( pMsg->szName[0] == 0 )
		return;

	try
	{
		Query::PrepareProcedure( "p_GetFactionBBS_Text" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@IndexID" )->Value		= pMsg->wIndexID;
		_RecordsetPtr prset = Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
		if( prset != NULL && prset->State && !prset->adoEOF )
		{
			FieldsPtr fields = prset->GetFields();

			SAGetFacBBSTextMsg msg;
			lite::Serializer lsm( msg.streamData, sizeof( msg.streamData ) );
			lsm( pMsg->szName );
			lsm( (char *)(_bstr_t)fields->GetItem("BBSText")->Value );
			msg.nFreeSize = lsm.EndEdition();
			msg.wIndexID = pMsg->wIndexID;
			SendMessage( dwSID, &msg, sizeof( msg ) - msg.nFreeSize );
		}
	}
	catch ( lite::Xcpt &e )
	{
		LogErrorMsg( _bstr_t( "CNetDBModule::GetFacBBSText lite::" ) + e.GetErrInfo() );
		return;
	}
	catch( _com_error &e )
	{
		LogErrorMsg( "CNetDBModule::GetFacBBSText"+e.Description() );
		return;
	}
}

void CNetDBModule::GetFacBBS( const DNID dwSID, SQGetFacBBSMsg *pMsg )
{
	if( pMsg == NULL )
		return;

	if( pMsg->szFaction[0] == 0 || pMsg->nServerID <= 0 || pMsg->szName[0] == 0 )
		return;

	SAGetFacBBSMsg msg;
	msg.byBBSNum = 0;
	dwt::strcpy( msg.szName, pMsg->szName, sizeof( msg.szName ) );

	try
	{
		Query::PrepareProcedure( "p_GetFactionBBS_Page" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
		Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= pMsg->szFaction;
		Query::m_pCommand->Parameters->GetItem( "@PageIndex" )->Value		= pMsg->byPage + 1;
		Query::m_pCommand->Parameters->GetItem( "@UpTime" )->Value			= pMsg->dwUpTime;
		Query::m_pCommand->Parameters->GetItem( "@DownTime" )->Value		= pMsg->dwDownTime;
		_RecordsetPtr prset = Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
		while( prset != NULL && prset->State && !prset->adoEOF && msg.byBBSNum < 7 )
		{
			FieldsPtr fields = prset->GetFields();
			TrimRight( msg.stFactionBBS[ msg.byBBSNum ].szName,  (char *)(_bstr_t)fields->GetItem("MemberName")->Value, sizeof( msg.stFactionBBS[ msg.byBBSNum ].szName  ) );
			TrimRight( msg.stFactionBBS[ msg.byBBSNum ].szTitle, (char *)(_bstr_t)fields->GetItem("BBSTitle")->Value,	sizeof( msg.stFactionBBS[ msg.byBBSNum ].szTitle ) );
			msg.stFactionBBS[ msg.byBBSNum ].dwUpdataTime	=	( DWORD )fields->GetItem("UpdataTime")->Value;
			msg.stFactionBBS[ msg.byBBSNum ].wIndexID		=	( DWORD )fields->GetItem("IndexID")->Value;
			msg.stFactionBBS[ msg.byBBSNum ].byType			=	( DWORD )fields->GetItem("Type")->Value;
			prset->MoveNext();
			msg.byBBSNum++;
		}
	}
	catch(_com_error &e)
	{
		LogErrorMsg( "CNetDBModule::GetFacBBS"+e.Description() );
		return;
	}

	msg.byPage = pMsg->byPage;
	SendMessage( dwSID, &msg, sizeof( msg ) - sizeof( msg.stFactionBBS[0] ) * ( MAX_BBSLIST - msg.byBBSNum ) );
}

void CNetDBModule::SaveFacBBS( const DNID dwSID, SQSaveFacBBSMsg *pMsg )
{
	if( pMsg == NULL )
		return;

	if( pMsg->nServerID <= 0 )
		return;

	try
	{
		lite::Serialreader ls( pMsg->streamData );
		LPCSTR szTitle = ls();
		LPCSTR szText = ls();

		Query::PrepareProcedure( "p_SaveFactionBBS" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@IndexID" )->Value			= pMsg->wIndexID;
		Query::m_pCommand->Parameters->GetItem( "@ServerID" )->Value		= pMsg->nServerID;
		Query::m_pCommand->Parameters->GetItem( "@FactionName" )->Value		= pMsg->szFaction;
		Query::m_pCommand->Parameters->GetItem( "@MemberName" )->Value		= pMsg->szName;
		Query::m_pCommand->Parameters->GetItem( "@BBSTitle" )->Value		= szTitle;
		Query::m_pCommand->Parameters->GetItem( "@BBSText" )->Value			= szText;
		Query::m_pCommand->Parameters->GetItem( "@UpdataTime" )->Value		= (DWORD)time(NULL);
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
		assert( Query::ReturnOK() );
	}
	catch ( lite::Xcpt &e )
	{
		LogErrorMsg( _bstr_t( "CNetDBModule::SaveFacBBS lite::" ) + e.GetErrInfo() );
		return ;
	}
	catch(_com_error &e)
	{
		LogErrorMsg( "CNetDBModule::SaveFacBBS"+e.Description() );
		return;
	}
}

void CNetDBModule::DelFacBBS( const DNID dwSID, SQDelFacBBSMsg *pMsg )
{
	try
	{
		Query::PrepareProcedure( "p_DelFactionBBS" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@IndexID" )->Value		= pMsg->wIndex;
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
	}
	catch(_com_error &e)
	{
		LogErrorMsg( "CNetDBModule::DelFacBBS"+e.Description() );
		return;
	}
}

void CNetDBModule::SetFacBBSTop( const DNID dwSID, SQSetFacBBSTopMsg *pMsg )
{
	try
	{
		Query::PrepareProcedure( "p_SetFactionBBSTop" );
		Query::m_pCommand->Prepared;
		Query::m_pCommand->Parameters->GetItem( "@IndexID" )->Value		= pMsg->wIndex;
		Query::m_pCommand->Parameters->GetItem( "@type" )->Value			= pMsg->byTop;
		Query::m_pCommand->Execute(NULL,NULL,Query::m_pCommand->CommandType);
	}
	catch(_com_error &e)
	{
		LogErrorMsg( "CNetDBModule::SetFacBBSTop"+e.Description() );
		return;
	}
}
