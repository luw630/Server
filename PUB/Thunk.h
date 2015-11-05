
//���ڹ���(WNDPROC) �ķ�static���Ա������װ~~~~~


#include <windows.h>

class CThunk;

#pragma pack(push,1)	//�ýṹ�������ֽڶ���
struct Thunk			//һ�ο�ִ�д���
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


//ʹ�÷������̳�CThunk������OwnProc����Ϊ���ڹ���
//CThunk�ฺ���windows����proc�Ĳ���ת��OwnProc
//ע�⣺ CreateWindow(Ex)��ʱ�� ʹ��CThunk��Create(Ex)����~~~~~������

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
				//��¼hWnd
				pt->m_hWnd = hWnd;

				//�ı䴰�ڹ���Ϊm_thunk
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
		//����code[0]��callִ��ʱ�����һ��ָ��ѹջ������Procѹջ
		pt->Call	= 0xE8;															// call [rel]32
		pt->Offset	= (size_t)&(((Thunk*)0)->Code)-(size_t)&(((Thunk*)0)->Proc);	// ƫ����������Proc��Code[0]
		pt->Proc	= CThunk::stdProc;												//��̬���ڹ���
		pt->Code[0] = 0x59;															//pop ecx

		//mov dword ptr [esp+0x4],this
		//Proc�ѵ�����ջ���Ƿ��ص�ַ�������ž���HWND�ˡ�
		//[esp+0x4]����HWND
		pt->Code[1] = 0xC7;															// mov
		pt->Code[2] = 0x44;															// dword ptr
		pt->Code[3] = 0x24;															// disp8[esp]
		pt->Code[4] = 0x04;															// +4
		pt->This	= this;

		//͵�������ɹ�����ת��Proc
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