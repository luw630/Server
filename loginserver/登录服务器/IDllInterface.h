#pragma once
#ifdef DLL_MAKER
#define DLL __declspec ( dllexport )
#else
#define DLL __declspec ( dllimport )
#endif
interface IDllInterface
{

public:
	// 初始化&释放
	virtual bool Initialize() =0;
	virtual void UnInitialize()=0;
	// DLL版本信息字符串
	virtual LPCSTR GetVersionString() =0;
	virtual bool ExecuteCmd( HWND src, WPARAM wparam, LPVOID data, size_t size  )=0;
	virtual bool SetObject( void * pOject )=0;
};

typedef IDllInterface& ( LOADSELF )( );
