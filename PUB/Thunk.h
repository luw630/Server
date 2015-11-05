
//窗口过程(WNDPROC) 的非static类成员函数封装~~~~~


#include <windows.h>

class CThunk;

#pragma pack(push,1)	//该结构必须以字节对齐
struct Thunk			//一段可执行代码
{
	BYTE		Call;
	int			Offset;
	WNDPROC		Proc;
	BYTE		Code[5];
	CThunk*		This;
	BYTE		Jmp;
	BYTE		ECX; 
};
#pragma pack(pop)


//使用方法：继承CThunk，重载OwnProc函数为窗口过程
//CThunk类负责把windows调用proc的操作转到OwnProc
//注意： CreateWindow(Ex)的时候 使用CThunk的Create(Ex)函数~~~~~！！！

class CThunk			
{
public:
	CThunk()
	{
		m_hWnd = NULL;
        m_thunk = NULL;
		// CreateThunk();
	}

	virtual ~CThunk()
	{
		Thunk *p = (Thunk*)m_thunk;

		delete p;
	}

	virtual LRESULT WINAPI OwnProc(UINT message,WPARAM wParam,LPARAM lParam) = 0;

	HWND Create(LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,
				int nHeight,HWND hWndParent,HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		return m_hWnd = CreateWindow(lpClassName,
							lpWindowName,
							dwStyle,
							x,
							y,
							nWidth,
							nHeight,
							hWndParent,
							hMenu,
							hInstance,
							LPVOID(this)
							);
	}

	HWND CreateEx(DWORD dwExStyle,LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,
				  int nHeight,HWND hWndParent,HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		return CreateWindowEx(dwExStyle,
							lpClassName,
							lpWindowName,
							dwStyle,
							x,
							y,
							nWidth,
							nHeight,
							hWndParent,
							hMenu,
							hInstance,
							LPVOID(this)
							);
	}

	static LRESULT WINAPI InitProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
		if ( message == WM_NCCREATE )
		{
            /*
			CThunk *pt = NULL;
			pt = (CThunk*)((LPCREATESTRUCT)lParam)->lpCreateParams;

			if(pt)
			{
				//记录hWnd
				pt->m_hWnd = hWnd;

				//改变窗口过程为m_thunk
				SetWindowLong(hWnd,GWL_WNDPROC,(LONG)pt->GetThunk());
				return (*(WNDPROC)(pt->GetThunk()))(hWnd,message,wParam,lParam);   
			}
            */
            SetWindowLong( hWnd, GWL_USERDATA, ( LONG )(ptrdiff_t)( ( LPCREATESTRUCT )lParam )->lpCreateParams ); 
		} 

        if ( CThunk *pt = ( CThunk* )(ptrdiff_t)GetWindowLong( hWnd, GWL_USERDATA ) )
            if ( pt->m_hWnd == hWnd )
		        return pt->OwnProc( message, wParam, lParam );

		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	static LRESULT WINAPI stdProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
			return DefWindowProc(hWnd,message,wParam,lParam);
		//CThunk* pthis = (CThunk*)hWnd;

		//return pthis->OwnProc(message,wParam,lParam);
	}

	void CreateThunk()
	{
		Thunk*  pt	= new Thunk;
		//call Offset
		//调用code[0]，call执行时会把下一条指令压栈，即把Proc压栈
		pt->Call	= 0xE8;															// call [rel]32
		pt->Offset	= (size_t)&(((Thunk*)0)->Code)-(size_t)&(((Thunk*)0)->Proc);	// 偏移量，跳过Proc到Code[0]
		pt->Proc	= CThunk::stdProc;												//静态窗口过程
		pt->Code[0] = 0x59;															//pop ecx

		//mov dword ptr [esp+0x4],this
		//Proc已弹出，栈顶是返回地址，紧接着就是HWND了。
		//[esp+0x4]就是HWND
		pt->Code[1] = 0xC7;															// mov
		pt->Code[2] = 0x44;															// dword ptr
		pt->Code[3] = 0x24;															// disp8[esp]
		pt->Code[4] = 0x04;															// +4
		pt->This	= this;

		//偷梁换柱成功！跳转到Proc
		//jmp [ecx]
		pt->Jmp		= 0xFF;															// jmp [r/m]32
		pt->ECX		= 0x21;															// [ecx]

		m_thunk		= (WNDPROC)pt;

		return;
	}

	WNDPROC GetThunk()
	{
		return m_thunk;
	}

	WNDPROC		m_thunk;
	HWND		m_hWnd;
};