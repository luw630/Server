#pragma once

#ifdef DLL_MAKER
    #define DLL __declspec ( dllexport )
#else
    #define DLL __declspec ( dllimport )
#endif

struct iUpdateCfg
{
    // 获取平台接口对象
    static DLL iUpdateCfg* GetInstance();

    virtual BOOL ParseCommandLine( LPCSTR commandLine ) = 0;
    virtual LPCSTR GetParameter( LPCSTR name ) = 0;
    virtual int GetIntParameter( LPCSTR name ) = 0;
    virtual BOOL GameStart( LPCSTR filename ) = 0;
    virtual VOID GameExit() = 0;
};
