#include "stdafx.h"
#include "DSpecialPlayerRcd.h"
#include "pub\ConstValue.h"

CSpecialPlayerRcd    *g_pSpecialPlayerRcd = NULL;

CSpecialPlayerRcd::CSpecialPlayerRcd()
{
}
CSpecialPlayerRcd::~CSpecialPlayerRcd()
{
}
///////////////////////////////////////////////
BOOL        CSpecialPlayerRcd::LoadData(void)
{
    bool    bRet = false;
    //char    szName[32];
    dwt::ifstream    FileStream;
    SChar   SName;

    FileStream.open( FILENAME_SPECIAL_PLAYER );
    if( !FileStream.is_open() )
        return rfalse( "无法打开[%s] Data File ！", FILENAME_SPECIAL_PLAYER );
    
    while( !FileStream.eof() )
    {
        FileStream  >> SName.szName;
        lsSpcPlayer.push_back( SName );
    }

    FileStream.close();

    return  bRet;
}

void        CSpecialPlayerRcd::AddRcdPlayer( char *szName )
{
    SChar   SName;
	dwt::strcpy(SName.szName, szName, CONST_USERNAME);
    lsSpcPlayer.push_back( SName );
}

BOOL        CSpecialPlayerRcd::DelRcdPlayer( char *szName )
{
    bool        bRet = false;
    SChar       *pName;
    list<SChar>::iterator    it;

    it = lsSpcPlayer.begin();
    while( it!=lsSpcPlayer.end() )
    {
        pName = &(*it);
		if (dwt::strcmp(szName, pName->szName, CONST_USERNAME) == 0)
        {
            lsSpcPlayer.erase( it );
            bRet = true;
            break;
        }
        it ++;
    }

    return  bRet;
}

void        CSpecialPlayerRcd::OnPlayerOps( char const * const szName, char *fmt, ... )
{
    FILE    *pf;
    //char    *pName;
    SChar   *pName;
    static  char    szShow[256];
    static  char    szFileName[256];
    list<SChar>::iterator    it;

    it = lsSpcPlayer.begin();
    while( it!=lsSpcPlayer.end() )
    {
        pName = &(*it);
		if (dwt::strcmp(szName, pName->szName, CONST_USERNAME) == 0)
        {
			dwt::strcpy(szFileName, pName->szName, CONST_USERNAME);
			dwt::_strncat( szFileName, ".txt", sizeof(".txt") );
            pf = fopen( szFileName, "a+" );
            if( pf==NULL )  return;
            
            va_list header;
            va_start( header, fmt );
            _vsnprintf( szShow, 256, fmt, header );
            va_end( header );

            fprintf( pf, szShow );

            fclose( pf );
            break;
        }
        it ++;
    }
}
