// 7 april 2015

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __LIBUI_UI_WINDOWS_H__
#define __LIBUI_UI_WINDOWS_H__

typedef struct uiWindowsSizing uiWindowsSizing;

typedef struct uiWindowsControl uiWindowsControl;
struct uiWindowsControl {
	uiControl c;
	void (*CommitSetParent)(uiWindowsControl *, HWND);
	void (*MinimumSize)(uiWindowsControl *, uiWindowsSizing *, intmax_t *, intmax_t *);
	void (*Relayout)(uiWindowsControl *, intmax_t, intmax_t, intmax_t, intmax_t);
	void (*AssignControlIDZOrder)(uiWindowsControl *, LONG_PTR *, HWND *);
	void (*ArrangeChildrenControlIDsZOrder)(uiWindowsControl *);
};
_UI_EXTERN uintmax_t uiWindowsControlType(void);
#define uiWindowsControl(this) ((uiWindowsControl *) uiIsA((this), uiWindowsControlType(), 1))
// TODO document
_UI_EXTERN void uiWindowsControlQueueRelayout(uiWindowsControl *);

// TODO document
#define uiWindowsDefineControlWithOnDestroy(type, typefn, onDestroy) \
	static uintmax_t _ ## type ## Type = 0; \
	uintmax_t typefn(void) \
	{ \
		if (_ ## type ## Type == 0) \
			_ ## type ## Type = uiRegisterType(#type, uiWindowsControlType(), sizeof (type)); \
		return _ ## type ## Type; \
	} \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *this = type(c); \
		onDestroy; \
		uiWindowsEnsureDestroyWindow(this->hwnd); \
	} \
	static uintptr_t _ ## type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->hwnd); \
	} \
	static void _ ## type ## ContainerUpdateState(uiControl *c) \
	{ \
		/* do nothing */ \
	} \
	static void _ ## type ## CommitSetParent(uiWindowsControl *c, HWND parent) \
	{ \
		uiWindowsEnsureSetParent(type(c)->hwnd, parent); \
	} \
	static void _ ## type ## Relayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height) \
	{ \
		uiWindowsEnsureMoveWindow(type(c)->hwnd, x, y, width, height); \
	} \
	static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height); \
	static void _ ## type ## AssignControlIDZOrder(uiWindowsControl *c, LONG_PTR *controlID, HWND *insertAfter) \
	{ \
		uiWindowsEnsureAssignControlIDZOrder(type(c)->hwnd, *controlID, *insertAfter); \
		(*controlID)++; \
		*insertAfter = type(c)->hwnd; \
	} \
	static void _ ## type ## ArrangeChildrenControlIDsZOrder(uiWindowsControl *c) \
	{ \
		/* do nothing */ \
	}

#define uiWindowsDefineControl(type, typefn) \
	uiWindowsDefineControlWithOnDestroy(type, typefn, (void) this;)

#define uiWindowsFinishNewControl(variable, type) \
	uiControl(variable)->CommitDestroy = _ ## type ## CommitDestroy; \
	uiControl(variable)->Handle = _ ## type ## Handle; \
	uiControl(variable)->ContainerUpdateState = _ ## type ## ContainerUpdateState; \
	uiWindowsControl(variable)->CommitSetParent = _ ## type ## CommitSetParent; \
	uiWindowsControl(variable)->MinimumSize = minimumSize; \
	uiWindowsControl(variable)->Relayout = _ ## type ## Relayout; \
	uiWindowsControl(variable)->AssignControlIDZOrder = _ ## type ## AssignControlIDZOrder; \
	uiWindowsControl(variable)->ArrangeChildrenControlIDsZOrder = _ ## type ## ArrangeChildrenControlIDsZOrder; \
	uiWindowsFinishControl(uiControl(variable));

// This is a function used to set up a control.
// Don't call it directly; use uiWindowsFinishNewControl() instead.
_UI_EXTERN void uiWindowsFinishControl(uiControl *c);

// This creates a HWND compatible with libui.
// It has no failure state; libui handles errors for you.
_UI_EXTERN HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);

// This is a wrapper for certain Windows API functions; use them to have libui handle errors for you.
_UI_EXTERN void uiWindowsEnsureDestroyWindow(HWND hwnd);
_UI_EXTERN void uiWindowsEnsureSetParent(HWND hwnd, HWND parent);

// Use this in your Relayout() implementation to move and resize HWNDs. libui handles errors for you.
_UI_EXTERN void uiWindowsEnsureMoveWindow(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height);

// Use this in implementations of AssignControlIDZOrder().
// libui handles errors for you.
_UI_EXTERN void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR controlID, HWND insertAfter);

// Use this to tell a control's parent that the control needs to rearrange its Z-order.
_UI_EXTERN void uiWindowsRearrangeControlIDsZOrder(uiControl *);

////////////////////////////////////////////
/////////////////// TODO ///////////////////
////////////////////////////////////////////

// These provide single-HWND implementations of uiControl methods you can use in yours.
_UI_EXTERN void uiWindowsUtilDestroy(HWND hwnd);
_UI_EXTERN void uiWindowsUtilSetParent(HWND hwnd, uiControl *parent);
_UI_EXTERN void uiWindowsUtilShow(HWND hwnd);
_UI_EXTERN void uiWindowsUtilHide(HWND hwnd);
_UI_EXTERN void uiWindowsUtilEnable(HWND hwnd);
_UI_EXTERN void uiWindowsUtilDisable(HWND hwnd);
_UI_EXTERN uintptr_t uiWindowsUtilStartZOrder(HWND hwnd);
_UI_EXTERN uintptr_t uiWindowsUtilSetZOrder(HWND hwnd, uintptr_t insertAfter);
_UI_EXTERN int uiWindowsUtilHasTabStops(HWND hwnd);

// This contains the Windows-specific parts of the uiSizing structure.
// BaseX and BaseY are the dialog base units.
// InternalLeading is the standard control font's internal leading; labels in uiForms use this for correct Y positioning.
// CoordFrom and CoordTo are the window handles to convert coordinates passed to uiControlResize() from and to (viaa MapWindowRect()) before passing to one of the Windows API resizing functions.
struct uiWindowsSizing {
	intmax_t XPadding;
	intmax_t YPadding;
	int BaseX;
	int BaseY;
	LONG InternalLeading;
	HWND CoordFrom;
	HWND CoordTo;
};
// Use these to create and destroy uiWindowsSizings.
_UI_EXTERN uiWindowsSizing *uiWindowsNewSizing(HWND hwnd);
_UI_EXTERN void uiWindowsFreeSizing(uiWindowsSizing *d);
// Use these in your preferredSize() implementation with baseX and baseY.
#define uiWindowsDlgUnitsToX(dlg, baseX) MulDiv((dlg), baseX, 4)
#define uiWindowsDlgUnitsToY(dlg, baseY) MulDiv((dlg), baseY, 8)

// and use this if you need the text of the window width
_UI_EXTERN intmax_t uiWindowsWindowTextWidth(HWND hwnd);

// these functions get and set the window text for a single HWND
// the value returned should be freed with uiFreeText()
_UI_EXTERN char *uiWindowsUtilText(HWND);
_UI_EXTERN void uiWindowsUtilSetText(HWND, const char *);

// These provide event handling.
// For WM_COMMAND, the WORD parameter is the notification code.
// For WM_HSCROLL, the WORD parameter is the scroll operation.
_UI_EXTERN void uiWindowsRegisterWM_COMMANDHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
_UI_EXTERN void uiWindowsRegisterWM_NOTIFYHandler(HWND, BOOL (*)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *);
_UI_EXTERN void uiWindowsRegisterWM_HSCROLLHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
_UI_EXTERN void uiWindowsUnregisterWM_COMMANDHandler(HWND);
_UI_EXTERN void uiWindowsUnregisterWM_NOTIFYHandler(HWND);
_UI_EXTERN void uiWindowsUnregisterWM_HSCROLLHandler(HWND);
_UI_EXTERN void uiWindowsRegisterReceiveWM_WININICHANGE(HWND);
_UI_EXTERN void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND);

#endif
