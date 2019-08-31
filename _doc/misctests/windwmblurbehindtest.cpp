// 31 august 2019
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600	/* from Microsoft's winnls.h */
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600	/* from Microsoft's pdh.h */
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <stdio.h>
#include <stdlib.h>

// cl windwmblurbehindtest.cpp -MT -link user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib dwmapi.lib windows.res

void diele(const char *func)
{
	DWORD le;

	le = GetLastError();
	fprintf(stderr, "%s: %I32u\n", func, le);
	exit(EXIT_FAILURE);
}

void diehr(const char *func, HRESULT hr)
{
	fprintf(stderr, "%s: 0x%08I32X\n", func, hr);
	exit(EXIT_FAILURE);
}

// TODO if we merge this into libui proper, this will need to be deduplicated
static inline HRESULT lastErrorToHRESULT(DWORD lastError)
{
	if (lastError == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(lastError);
}

HINSTANCE hInstance;
HRGN rgn;
BOOL created = FALSE;
HBRUSH opaquebrush;

#define OURWIDTH 320
#define OURHEIGHT 240

void onWM_CREATE(HWND hwnd)
{
	DWM_BLURBEHIND dbb;
	HRESULT hr = S_OK;

	rgn = CreateRectRgn(0, 0, OURWIDTH / 2, OURHEIGHT);
	if (rgn == NULL)
		diele("CreateRectRgn()");
	ZeroMemory(&dbb, sizeof (DWM_BLURBEHIND));
	dbb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	dbb.fEnable = TRUE;
	dbb.hRgnBlur = NULL;//rgn;
	hr = DwmEnableBlurBehindWindow(hwnd, &dbb);
	if (hr != S_OK)
		diehr("DwmEnableBlurBehindWindow()", hr);
CreateWindowExW(0,
L"BUTTON",L"Hello",
WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
3*OURWIDTH/4,150,
OURWIDTH/6,25,
hwnd,NULL,hInstance,NULL);
SetWindowTheme(CreateWindowExW(0,
L"BUTTON",L"Hello",
WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
3*OURWIDTH/4,200,
OURWIDTH/6,25,
hwnd,NULL,hInstance,NULL),L"",L"");
}

void onWM_PAINT(HWND hwnd)
{
	HDC dc;
	PAINTSTRUCT ps;
	RECT r;

	ZeroMemory(&ps, sizeof (PAINTSTRUCT));
	dc = BeginPaint(hwnd, &ps);
	if (dc == NULL)
		diele("BeginPaint()");
	// First, fill with BLACK_BRUSH to satisfy the DWM
	r.left = 0;
	r.top = 0;
	r.right = OURWIDTH;
	r.bottom = OURHEIGHT;
	// TODO check error
	FillRect(dc, &r, (HBRUSH) GetStockObject(BLACK_BRUSH));
static DWORD c=GetSysColor(COLOR_BTNFACE);
BYTE x[4];
x[3]=0xFF;
x[2]=GetRValue(c);
x[1]=GetGValue(c);
x[0]=GetBValue(c);
static HBITMAP b=CreateBitmap(1,1,1,32,x);
if(b==NULL)diele("CreateBitmap()");
HDC dc2=CreateCompatibleDC(dc);
if(dc==NULL)diele("CreateCompatibleDC()");
HGDIOBJ prev=SelectObject(dc2,b);
StretchBlt(dc,OURWIDTH/2,0,OURWIDTH/2,OURHEIGHT,
dc2,0,0,1,1,SRCCOPY);
SelectObject(dc2,prev);
DeleteDC(dc2);
DeleteObject(b);
r.left=3*OURWIDTH/4;
r.top=100;
r.right=7*OURWIDTH/8;
r.bottom=25;
auto m=SetBkMode(dc,TRANSPARENT);
TextOutW(dc,r.left,r.top,L"Hello",5);
SetBkMode(dc, m);
	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		onWM_CREATE(hwnd);
		break;
	case WM_PAINT:
		onWM_PAINT(hwnd);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

int main(int argc, char *argv[])
{
	STARTUPINFOW si;
	int nCmdShow;
	INITCOMMONCONTROLSEX icc;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	DWORD dwStyle, dwExStyle;
	RECT r;
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;
	HRESULT hr;

	hInstance = (HINSTANCE) (&__ImageBase);
	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES;
	if (InitCommonControlsEx(&icc) == 0)
		diele("InitCommonControlsEx()");

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		diele("LoadIconW(IDI_APPLICATION)");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		diele("LoadCursorW(IDC_ARROW)");

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		diele("RegisterClassW()");

	dwStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
	dwExStyle = 0;
	r.left = 0;
	r.top = 0;
	r.right = OURWIDTH;
	r.bottom = OURHEIGHT;
	if (AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle) == 0)
		diele("AdjustWindowRectEx()");
	mainwin = CreateWindowExW(dwExStyle,
		L"mainwin", L"Main Window",
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, hInstance, NULL);
	if (mainwin == NULL)
		diele("CreateWindowExW(L\"mainwin\")");

	ShowWindow(mainwin, nCmdShow);
	if (UpdateWindow(mainwin) == 0)
		diele("UpdateWindow()");

	for (;;) {
		int res;

		res = GetMessageW(&msg, NULL, 0, 0);
		if (res < 0)
			diele("GetMessageW()");
		if (res == 0)
			break;
		if (IsDialogMessageW(mainwin, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return 0;
}
