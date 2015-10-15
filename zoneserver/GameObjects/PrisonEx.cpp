#include "StdAfx.h"
#include ".\prisonex.h"
#include "player.h"


extern LPIObject GetPlayerBySID(DWORD dwStaticID);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);

CPrisonEx::CPrisonEx(void)
{
    runCheckTiem = 0;
}

CPrisonEx::~CPrisonEx(void)
{
}


void CPrisonEx::OnRun(void)
{
    CRegion::OnRun();

    // 每分钟进来一次
    if ( abs( ( int )timeGetTime()  - ( int )runCheckTiem ) > 60000 )
    {
        runCheckTiem = timeGetTime();
        for ( std::map< DWORD, WORD >::iterator iter = prisonerMap.begin(); iter != prisonerMap.end(); iter++ )
        {      
_run_next:
            if ( iter == prisonerMap.end() )
                break;

            if ( CPlayer *player = ( CPlayer* )( GetPlayerBySID( iter->first ) )->DynamicCast( IID_PLAYER ) )
            {
                if ( player->m_ParentRegion->DynamicCast( IID_PRISONEX ) )
                {
                    if ( iter->second == 0 )
                    {
                        PutPlayerIntoDestRegion( player, 1, 100, 100 );
                        iter = prisonerMap.erase( iter );
                        goto _run_next;
                    }
                    else
                        iter->second --;
                }
            }   
        }
    }
}

int CPrisonEx::AddObject(LPIObject pChild)
{
    return CRegion::AddObject(pChild);
}

int CPrisonEx::DelObject(LPIObject pChild)
{
    return CRegion::DelObject(pChild);
}

void CPrisonEx::AddPrisoner( DWORD sid, WORD freeTiem )
{
    prisonerMap[ sid ] = freeTiem;
}

void CPrisonEx::DelPrisoner( DWORD sid )
{
    prisonerMap.erase( sid );
}