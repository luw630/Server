#include "StdAfx.h"
#include "GBK_VisibleTest.h"
// GBK规则：
// Leadbyte     (81 -> FE)
// Followbyte   (40 -> FE) xpt (7F)

static bool GBK_VisibleTable[0xfe - 0x81 + 1][0xfe - 0x40 + 1];

int GBK_InitVisibleTable()
{
    HDC hdc = CreateCompatibleDC( GetWindowDC( GetDesktopWindow() ) );
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, 12, 12);
    RECT rect = {0, 0, 12, 12};

	BITMAPINFO bi;

	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = 12;
	bi.bmiHeader.biHeight = 12;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 16;
	bi.bmiHeader.biCompression = BI_RGB;

    LOGFONT lf;

	lf.lfHeight = 0xfffffff4;
	lf.lfWidth = 0x00000000;
	lf.lfEscapement = 0x00000000;
	lf.lfOrientation = 0x00000000;
	lf.lfWeight = 0x00000190;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = 0;
	lf.lfOutPrecision = 0x03;
	lf.lfClipPrecision = 0x02;
	lf.lfQuality = 0x01;
	lf.lfPitchAndFamily = 0x02;
	strcpy(lf.lfFaceName, "宋体");

    HFONT hFont = CreateFontIndirect(&lf);
    if(hFont == 0)
    {
        MessageBox(NULL, "字体创建失败！", "Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }
    
    SelectObject(hdc, hFont);
	SelectObject(hdc, hBmp);

	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));

    unsigned short TestBuf[12][12];

    for (unsigned short lead=0x81; lead<0xff; lead++)
    for (unsigned short follow=0x40; follow<0xff; follow++)
    {
        bool test = false;

        if (follow != 0x7f)
        {
            unsigned short ucode = MAKEWORD(lead, follow);
		    DrawText(hdc, (char *)&ucode, 2, &rect, 0);
		    GetDIBits(hdc, hBmp, 0, 12, *TestBuf, &bi, 0);

		    for (int nY=0; nY<12; nY++)
		    {
			    for(int nX = 0; nX<12; nX++)
			    {
                    test |= (TestBuf[nY][nX] != 0);
			    }
		    }
        }

        GBK_VisibleTable[lead - 0x81][follow - 0x40] = test;
    }

    DeleteObject(hFont);
    DeleteObject(hBmp);
    DeleteDC(hdc);

    return 1;
}

int init = GBK_InitVisibleTable();

bool isVisibleCharacterA(char ascii)
{
    return isgraph(ascii) != 0;
}

bool isVisibleCharacterW(wchar_t ucode)
{
    DWORD lead = LOBYTE(ucode) - 0x81;
    DWORD follow = HIBYTE(ucode) - 0x40;

    if (lead > 0xfe - 0x81)
        return false;

    if (follow > 0xfe - 0x40)
        return false;

    return GBK_VisibleTable[lead][follow];
}
