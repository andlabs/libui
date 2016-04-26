// 21 april 2016

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __LIBUI_UI_WINDOWS_H__
#define __LIBUI_UI_WINDOWS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiWindowsSizing uiWindowsSizing;

typedef struct uiWindowsControl uiWindowsControl;
struct uiWindowsControl {
	uiControl c;
	uiControl *parent;
	BOOL enabled;
	BOOL visible;
	void (*SetParentHWND)(uiWindowsControl *, HWND);
	void (*MinimumSize)(uiWindowsControl *, intmax_t *, intmax_t *);
	void (*AssignControlIDZOrder)(uiWindowsControl *, LONG_PTR *, HWND *);
};
#define uiWindowsControl(this) ((uiWindowsControl *) (this))
// TODO document
_UI_EXTERN void uiWindowsControlSetParentHWND(uiWindowsControl *, HWND);
_UI_EXTERN void uiWindowsControlMinimumSize(uiWindowsControl *, intmax_t *, intmax_t *);
_UI_EXTERN void uiWindowsControlAssignControlIDZOrder(uiWindowsControl *, LONG_PTR *, HWND *);

// TODO document
xxxx

// TODO document
_UI_EXTERN HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);

// TODO document
_UI_EXTERN void uiWindowsEnsureDestroyWindow(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsEnsureSetParent(HWND hwnd, HWND parent);

// TODO document
_UI_EXTERN void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR *controlID, HWND *insertAfter);

// TODO document
_UI_EXTERN char *uiWindowsWindowText(HWND hwnd);
_UI_EXTERN void uiWindowsSetWindowText(HWND hwnd, const char *text);

// TODO document
_UI_EXTERN intmax_t uiWindowsWindowTextWidth(HWND hwnd);

// TODO document
// TODO point out this should only be used in a resize cycle
_UI_EXTERN void uiWindowsEnsureMoveWindowDuringResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_COMMANDHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_COMMANDHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_NOTIFYHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_NOTIFYHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_HSCROLLHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_HSCROLLHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterReceiveWM_WININICHANGE(HWND hwnd);
_UI_EXTERN void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND hwnd);

// TODO document
typedef struct uiWindowsSizing uiWindowsSizing;
struct uiWindowsSizing {
	int BaseX;
	int BaseY;
	LONG InternalLeading;
};
_UI_EXTERN void uiWindowsGetSizing(HWND hwnd, uiWindowsSizing *sizing);
_UI_EXTERN void uiWindowsSizingDlgUnitsToPixels(HWND hwnd, uiWindowsSIzing *sizing, int *x, int *y);
// TODO make private
#define uiWindowsDlgUnitsToX(dlg, baseX) MulDiv((dlg), baseX, 4)
#define uiWindowsDlgUnitsToY(dlg, baseY) MulDiv((dlg), baseY, 8)

// TODO document
_UI_EXTERN HWND uiWindowsMakeContainer(void (*onResize)(void *data), void *data);

#ifdef __cplusplus
}
#endif

#endif
