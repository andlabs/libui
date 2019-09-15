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
#include "_res/detours.h"
#include <stdio.h>
#include <stdlib.h>

// cl windwmblurbehindtest.cpp -MT -link user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib dwmapi.lib _res\detours.lib _res\windows.res

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

static void paintIntoBuffer(HWND hwnd, UINT uMsg, HDC dc, RECT *r)
{
	HPAINTBUFFER bbuf;
	HDC bdc;
	HRESULT hr;

	// TODO begin check errors
	bbuf = BeginBufferedPaint(dc, r, BPBF_TOPDOWNDIB, NULL, &bdc);
	if (uMsg == WM_PAINT)
		SendMessageW(hwnd, WM_PRINTCLIENT, (WPARAM) bdc, PRF_CLIENT | PRF_ERASEBKGND);
	else
		SendMessageW(hwnd, WM_PRINT, (WPARAM) bdc, PRF_ERASEBKGND | PRF_NONCLIENT);
	hr = BufferedPaintSetAlpha(bbuf, NULL, 255);
	hr = EndBufferedPaint(bbuf, TRUE);
	// TODO end check errors
}

static HWND bpHWND = NULL;
static HDC bpDC = NULL;
static HDC bpPSDC = NULL;
static HDC bpBufDC = NULL;
static HPAINTBUFFER bpBuf = NULL;

static HDC (WINAPI *origBeginPaint)(HWND hwnd, LPPAINTSTRUCT ps) = BeginPaint;
static HDC WINAPI ourBeginPaint(HWND hwnd, LPPAINTSTRUCT ps)
{
	HDC dc, bdc;
	HPAINTBUFFER bbuf;

	if (bpHWND == NULL) {
		dc = (*origBeginPaint)(hwnd, ps);
		if (dc == NULL)
			return NULL;
		bbuf = BeginBufferedPaint(dc, &(ps->rcPaint), BPBF_TOPDOWNDIB, NULL, &bdc);
		if (bbuf == NULL)		// just draw normally, not much else we can do but deal with the graphical glitches
			return dc;
		bpHWND = hwnd;
		bpDC = dc;
		bpPSDC = ps->hdc;
		ps->hdc = bdc;
		bpBufDC = bdc;
		bpBuf = bbuf;
		return bdc;
	}
	return (*origBeginPaint)(hwnd, ps);
}

static HANIMATIONBUFFER bpHAB = NULL;
static HDC bpFromDC = NULL;
static HDC bpFromBufDC = NULL;
static HPAINTBUFFER bpFromBuf = NULL;
static HDC bpToDC = NULL;
static HDC bpToBufDC = NULL;
static HPAINTBUFFER bpToBuf = NULL;

static HANIMATIONBUFFER (STDAPICALLTYPE *origBeginBufferedAnimation)(HWND hwnd, HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, BP_ANIMATIONPARAMS *pAnimationParams, HDC *phdcFrom, HDC *phdcTo) = BeginBufferedAnimation;
static HANIMATIONBUFFER STDAPICALLTYPE ourBeginBufferedAnimation(HWND hwnd, HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, BP_ANIMATIONPARAMS *pAnimationParams, HDC *phdcFrom, HDC *phdcTo)
{
	HANIMATIONBUFFER hab;
	HDC bdc;
	HPAINTBUFFER bbuf;
	bool doit;

	doit = false;
	if (bpHWND != NULL && bpHWND == hwnd && bpBufDC != NULL && bpBufDC == hdcTarget) {
		doit = true;
		hdcTarget = bpDC;
	}
	hab = (*origBeginBufferedAnimation)(hwnd, hdcTarget, prcTarget, dwFormat, pPaintParams, pAnimationParams, phdcFrom, phdcTo);
	if (!doit)
		return hab;
	if (hab == NULL)
		return NULL;
	bpHAB = hab;
	if (phdcFrom != NULL && *phdcFrom != NULL) {
		bbuf = BeginBufferedPaint(*phdcFrom, prcTarget, BPBF_TOPDOWNDIB, NULL, &bdc);
		if (bbuf != NULL) {		// otherwise, just draw normally, not much else we can do but deal with the graphical glitches
			bpFromDC = *phdcFrom;
			*phdcFrom = bdc;
			bpFromBufDC = bdc;
			bpFromBuf = bbuf;
		}
	}
	if (phdcTo != NULL && *phdcTo != NULL) {
		bbuf = BeginBufferedPaint(*phdcTo, prcTarget, BPBF_TOPDOWNDIB, NULL, &bdc);
		if (bbuf != NULL) {
			bpToDC = *phdcTo;
			*phdcTo = bdc;
			bpToBufDC = bdc;
			bpToBuf = bbuf;
		}
	}
	return hab;
}

static BOOL (STDAPICALLTYPE *origBufferedPaintRenderAnimation)(HWND hwnd, HDC hdcTarget) = BufferedPaintRenderAnimation;
static BOOL STDAPICALLTYPE ourBufferedPaintRenderAnimation(HWND hwnd, HDC hdcTarget)
{
	if (bpHWND != NULL && bpHWND == hwnd && bpBufDC != NULL && bpBufDC == hdcTarget)
		hdcTarget = bpDC;
	return (*origBufferedPaintRenderAnimation)(hwnd, hdcTarget);
}

static HRESULT (STDAPICALLTYPE *origEndBufferedAnimation)(HANIMATIONBUFFER hbpAnimation, BOOL fUpdateTarget) = EndBufferedAnimation;
static HRESULT STDAPICALLTYPE ourEndBufferedAnimation(HANIMATIONBUFFER hbpAnimation, BOOL fUpdateTarget)
{
	HRESULT hrFrom1 = S_OK;
	HRESULT hrFrom2 = S_OK;
	HRESULT hrTo1 = S_OK;
	HRESULT hrTo2 = S_OK;
	HRESULT hr = S_OK;

	if (bpHAB != NULL && bpHAB == hbpAnimation) {
		if (bpFromDC != NULL) {
			hrFrom1 = BufferedPaintSetAlpha(bpFromBuf, NULL, 255);
			hrFrom2 = EndBufferedPaint(bpFromBuf, TRUE);
			bpFromDC = NULL;
			bpFromBufDC = NULL;
			bpFromBuf = NULL;
		}
		if (bpToDC != NULL) {
			hrTo1 = BufferedPaintSetAlpha(bpToBuf, NULL, 255);
			hrTo2 = EndBufferedPaint(bpToBuf, TRUE);
			bpToDC = NULL;
			bpToBufDC = NULL;
			bpToBuf = NULL;
		}
		bpHAB = NULL;
	}
	hr = (*origEndBufferedAnimation)(hbpAnimation, fUpdateTarget);
	if (hrFrom1 != S_OK)
		return hrFrom1;
	if (hrFrom2 != S_OK)
		return hrFrom2;
	if (hrTo1 != S_OK)
		return hrTo1;
	if (hrTo2 != S_OK)
		return hrTo2;
	return hr;
}

static BOOL (WINAPI *origEndPaint)(HWND hwnd, CONST PAINTSTRUCT *ps) = EndPaint;
static BOOL WINAPI ourEndPaint(HWND hwnd, CONST PAINTSTRUCT *ps)
{
	HRESULT hr1, hr2;
	BOOL ret;
	DWORD lasterr;

	if (bpHWND == NULL || bpHWND != hwnd)
		return (*origEndPaint)(hwnd, ps);
	// TODO don't const this
	((PAINTSTRUCT *) ps)->hdc = bpPSDC;
	hr1 = BufferedPaintSetAlpha(bpBuf, NULL, 255);
	hr2 = EndBufferedPaint(bpBuf, TRUE);
	ret = (*origEndPaint)(hwnd, ps);
	lasterr = GetLastError();
	bpHWND = NULL;
	bpDC = NULL;
	bpPSDC = NULL;
	bpBufDC = NULL;
	bpBuf = NULL;
	if (hr1 != S_OK) {
		SetLastError(ERROR_GEN_FAILURE);
		if (HRESULT_FACILITY(hr1) == FACILITY_WIN32)
			SetLastError(HRESULT_CODE(hr1));
		return 0;
	}
	if (hr2 != S_OK) {
		SetLastError(ERROR_GEN_FAILURE);
		if (HRESULT_FACILITY(hr2) == FACILITY_WIN32)
			SetLastError(HRESULT_CODE(hr2));
		return 0;
	}
	SetLastError(lasterr);
	return ret;
}

static LRESULT CALLBACK buttonSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	PAINTSTRUCT ps;
	HDC dc;
	RECT r;

	switch (uMsg) {
	case WM_PAINT:
		// TODO begin check errors
#if 0
		dc = BeginPaint(hwnd, &ps);
//		paintIntoBuffer(hwnd, uMsg, dc, &(ps.rcPaint));
		DefSubclassProc(hwnd, uMsg, (WPARAM) dc, lParam);
		EndPaint(hwnd, &ps);
#elif 1
		if (wParam != 0)
			break;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&((PVOID &) origBeginPaint), ourBeginPaint);
		DetourAttach(&((PVOID &) origBeginBufferedAnimation), ourBeginBufferedAnimation);
//TODO		DetourAttach(&((PVOID &) origBufferedPaintRenderAnimation), ourBufferedPaintRenderAnimation);
		DetourAttach(&((PVOID &) origEndBufferedAnimation), ourEndBufferedAnimation);
		DetourAttach(&((PVOID &) origEndPaint), ourEndPaint);
		DetourTransactionCommit();

		DefSubclassProc(hwnd, uMsg, wParam, lParam);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&((PVOID &) origBeginPaint), ourBeginPaint);
		DetourDetach(&((PVOID &) origBeginBufferedAnimation), ourBeginBufferedAnimation);
//TODO		DetourDetach(&((PVOID &) origBufferedPaintRenderAnimation), ourBufferedPaintRenderAnimation);
		DetourDetach(&((PVOID &) origEndBufferedAnimation), ourEndBufferedAnimation);
		DetourDetach(&((PVOID &) origEndPaint), ourEndPaint);
		DetourTransactionCommit();
#else
		DefSubclassProc(hwnd, uMsg, wParam, lParam);
		if (0) {
			RECT r;

			GetClientRect(hwnd, &r);
			MapWindowRect(hwnd, GetParent(hwnd), &r);
			InvalidateRect(GetParent(hwnd), &r, TRUE);
		}
#endif
		// TODO end check errors
		return 0;
//TODO	case WM_NCPAINT:
		// TODO begin check errors
		dc = GetDCEx(hwnd, (HRGN) wParam, DCX_WINDOW | DCX_INTERSECTRGN);
		GetRgnBox((HRGN) wParam, &r);
		paintIntoBuffer(hwnd, uMsg, dc, &r);
		ReleaseDC(hwnd, dc);
		// TODO end check errors
		return 0;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, buttonSubProc, uIdSubclass) == FALSE)
			diele("RemoveWindowSubclass()");
                break;
        }
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
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
HWND w1=CreateWindowExW(0,
L"BUTTON",L"Hello",
WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
3*OURWIDTH/4,150,
OURWIDTH/6,25,
hwnd,NULL,hInstance,NULL);
SetWindowSubclass(w1, buttonSubProc, 0, 0);
HWND w2=CreateWindowExW(0,
L"BUTTON",L"Hello",
WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
3*OURWIDTH/4,200,
OURWIDTH/6,25,
hwnd,NULL,hInstance,NULL);
SetWindowTheme(w2,L"",L"");
SetWindowSubclass(w2, buttonSubProc, 0, 0);
}

void doPaint(HWND hwnd, HDC dc, RECT *rcPaint)
{
	RECT r, r2;
	HPAINTBUFFER bbuf;
	HDC bdc;
	HRESULT hr;

	// First, fill with BLACK_BRUSH to satisfy the DWM
	r.left = 0;
	r.top = 0;
	r.right = OURWIDTH;
	r.bottom = OURHEIGHT;
	// TODO check error
	FillRect(dc, &r, (HBRUSH) GetStockObject(BLACK_BRUSH));

	r.left = OURWIDTH / 2;
	// TODO check error
	IntersectRect(&r2, &r, rcPaint);
	bbuf = BeginBufferedPaint(dc, &r, BPBF_TOPDOWNDIB, NULL, &bdc);
	if (bbuf == NULL)
		diele("BeginBufferedPaint()");
	// TODO start check errors
	FillRect(bdc, &r, GetSysColorBrush(COLOR_BTNFACE));
	r.left = 3 * OURWIDTH / 4;
	r.top = 100;
	r.right = 7 * OURWIDTH / 8;
	r.bottom = 25;
	auto m = SetBkMode(bdc,TRANSPARENT);
	TextOutW(bdc, r.left, r.top, L"Hello", 5);
	SetBkMode(bdc, m);
	// TODO end check errors
	hr = BufferedPaintSetAlpha(bbuf, NULL, 255);
	if (hr != S_OK)
		diehr("BufferedPaintSetAlpha()", hr);
	hr = EndBufferedPaint(bbuf, TRUE);
	if (hr != S_OK)
		diehr("EndBufferedPaint()", hr);
}

void onWM_PAINT(HWND hwnd, WPARAM wParam)
{
	HDC dc;
	PAINTSTRUCT ps;
	RECT rcPaint;

	if (wParam != 0) {
		// TODO check errors
		GetClientRect(hwnd, &rcPaint);
		doPaint(hwnd, (HDC) wParam, &rcPaint);
		return;
	}

	ZeroMemory(&ps, sizeof (PAINTSTRUCT));
	dc = BeginPaint(hwnd, &ps);
	if (dc == NULL)
		diele("BeginPaint()");
	doPaint(hwnd, dc, &(ps.rcPaint));
	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		onWM_CREATE(hwnd);
		break;
	case WM_PAINT:
//	case WM_PRINTCLIENT:
//	case WM_ERASEBKGND:
		onWM_PAINT(hwnd, wParam);
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

	hr = BufferedPaintInit();
	if (hr != S_OK)
		diehr("BufferedPaintInit()", hr);

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
	BufferedPaintUnInit();
	return 0;
}
