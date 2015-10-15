#include "StdAfx.h"
#include "factionregion.h"
#include "factionmanager.h"
#include "player.h"
#include "trigger.h"
#include "region.h"

extern int SendToLoginServer( SMessage *data, long size );
//extern BOOL GetFactionHeaderInfo( LPCSTR szFactionName, SFactionData::SFaction &factionHeader );
extern BOOL UpdateFaction( SFactionData::SFaction &faction );

CFactionRegion::CFactionRegion(void) :
    CRegion()
{
}

CFactionRegion::~CFactionRegion(void)
{
}

int CFactionRegion::OnCreate(_W64 long pParameter)
{
    CreateParameter &param = * reinterpret_cast< CreateParameter* >( pParameter );

    int result = CRegion::OnCreate( param.mapTemplateId );
    if ( !result )
        return result;

    // ��������������Լ��Ļ�������Ҫ����������Ϊ�Լ����³���ID��
    if ( m_wReLiveRegionID == m_wRegionID )
        m_wReLiveRegionID = param.factionRegionId;

    // �޸ĳ������
    m_wRegionID = param.factionRegionId;

    // Ĭ�Ͻ���һ�����͵��޸�Ϊ��������������� Y + 2
    if ( ( m_byMaxTrap >= 1 ) && ( m_pTrap->m_dwType == CTrigger::TT_CHANGE_REGION ) )
    {
		m_pTrap->SChangeRegion.dwNewRegionGID = 0;
        m_pTrap->SChangeRegion.wNewRegionID = param.parentRegionId;
        m_pTrap->SChangeRegion.wStartX = param.xEntry;
        m_pTrap->SChangeRegion.wStartY = param.yEntry + 2;
    }

    property = param;

    // ���½������ȥ���󳡾��ϵĽ�����!
//     SQBuildingGetMsg msg;
//     msg.wParentID = m_wRegionID;
//     SendToLoginServer( &msg, sizeof( msg ) );

    return TRUE;
}

CNpc * CFactionRegion::CreateNpc( PVOID pParameter, DWORD controlId )
{
	CNpc *npc = CRegion::CreateNpc( pParameter, controlId );
	if ( npc == NULL )
		return NULL;

    int num = GetNpcCount();
	SFactionData::SFaction faction;
// 	if ( !GetFactionHeaderInfo( property.name.c_str(), faction ) )
// 			return npc;

    faction.byNpcNum = ( BYTE )num;
	UpdateFaction( faction );

	return npc;
}

CBuilding *CFactionRegion::CreateBuilding( PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate, DWORD controlId )
{
	CBuilding *region = CRegion::CreateBuilding( pParameter, pPlayer, isGenerate, controlId );
	if ( region == NULL )
		return NULL;

	SFactionData::SFaction faction;
// 	if ( !GetFactionHeaderInfo( property.name.c_str(), faction ) )
// 		return region;

	//faction.byBuildingNum ++;
	UpdateFaction( faction );

	return region;
}

int CFactionRegion::DelNpc( DWORD dwGID )
{
	int ret = CRegion::DelNpc( dwGID );
	if ( ret == 0 )
		return 0;

    int num = GetNpcCount();
// 	SFactionData::SFaction faction;
// 	if ( !GetFactionHeaderInfo( property.name.c_str(), faction ) )
// 		return ret;

//	faction.byNpcNum = (BYTE)num;
	//UpdateFaction( faction );

	return ret;
}

int	CFactionRegion::DelBuilding( DWORD dwGID )
{
	int ret = CRegion::DelBuilding( dwGID );
	if ( ret == 0 )
		return 0;

// 	SFactionData::SFaction faction;
// 	if ( !GetFactionHeaderInfo( property.name.c_str(), faction ) )
// 		return ret;

	//faction.byBuildingNum --;
//	UpdateFaction( faction );

	return ret;
}

int CFactionRegion::AddObject(LPIObject pChild)
{
    int result = CRegion::AddObject(pChild);
    if ( !result )
        return result;

    if ( CPlayer *player = ( CPlayer* )pChild->DynamicCast( IID_PLAYER ) )
    {
        // �������ҽ�����ɳ����Ļ���Ӧ������ҷ���һ���������Ϣ��֪ͨ�ͻ��ˣ�
        SEntryRegionNotify msg;
        msg.type = 0;
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
        slm( property.name.c_str() );
        player->SendMsg( &msg, sizeof( msg ) - slm.EndEdition() );
    }

    return true;
}

int CFactionRegion::DelObject(LPIObject pChild)
{
    return CRegion::DelObject(pChild);
}

