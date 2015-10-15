#ifndef     _SpecialPlayerRcd_H_
#define     _SpecialPlayerRcd_H_

#include "DBaseIntList.h"
#include <list>
using namespace std;

#define     FILENAME_SPECIAL_PLAYER        "Data\\SpecialPlayer.txt"

struct  SChar
{
    char    szName[32];
};

class   CSpecialPlayerRcd
{
    list<SChar>      lsSpcPlayer;

public:
    CSpecialPlayerRcd(void);
    ~CSpecialPlayerRcd(void);

    BOOL        LoadData(void);

    void        OnPlayerOps( char const * const szName, char *fmt, ... );

    void        AddRcdPlayer( char *szName );
    BOOL        DelRcdPlayer( char *szName );
};

extern  CSpecialPlayerRcd    *g_pSpecialPlayerRcd;

#endif  _SpecialPlayerRcd_H_