/****************************************************************
//  FileName:   XYDDebug.h
//  Author:		Wonly
//  Create:		2003.5.26
//  Desc:       ≥ˆ¥Ì≤‚ ‘£¨–ß¬ ≤‚ ‘£¨ƒ⁄¥Ê≤‚ ‘
****************************************************************/

#ifndef     _XYDDEBUG_H_
#define     _XYDDEBUG_H_

#include "stdio.h"


__int64     StartGetTime(int id);
int         EndGetTime(int id);

extern  int     g_iComTestTime;
extern  int     g_iTime1;
extern  int     g_iTime2;

inline unsigned __int64 GetCycleCount()
{
 __asm _emit 0x0F
 __asm _emit 0x31
}


#define PUTFILENAME     "Debug.txt"
class   OutPutDebug
{
    FILE    *m_pFt;
    int     m_Frame;
public:
    OutPutDebug(void);
    ~OutPutDebug(void);

    BOOL    Open(char *filename);
    void    Close();

    void    PutString(char *str, ...);
    void    Update();
};

#endif      //_XYDDEBUG_H_