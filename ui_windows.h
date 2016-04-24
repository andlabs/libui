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
	void (*CommitSetParent)(uiWindowsControl *, HWND);
	void (*MinimumSize)(uiWindowsControl *, uiWindowsSizing *, intmax_t *, intmax_t *);
	void (*Relayout)(uiWindowsControl *, intmax_t, intmax_t, intmax_t, intmax_t);
	void (*AssignControlIDZOrder)(uiWindowsControl *, LONG_PTR *, HWND *);
	void (*ArrangeChildrenControlIDsZOrder)(uiWindowsControl *);
};
#define uiWindowsControl(this) ((uiWindowsControl *) (this))
// TODO document
_UI_EXTERN void uiWindowsControlQueueRelayout(uiWindowsControl *);

// TODO document
#define uiWindowsDefineControlWithOnDestroy(type, onDestroy) \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *me = type(c); \
		onDestroy; \
		uiWindowsEnsureDestroyWindow(me->hwnd); \
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
		uiWindowsEnsureMoveWindowDuringResize(type(c)->hwnd, x, y, width, height); \
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

#define uiWindowsDefineControl(type) \
	uiWindowsDefineControlWithOnDestroy(type, (void) me;)

// TODO document
// TODO rename the macro?
#define uiNewControl(type) uiWindowsNewControl(sizeof (type), type ## Signature, #type)
_UI_EXTERN uiWindowsControl *uiWindowsNewControl(size_t n, uint32_t typesig, const char *typenamestr);

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

// TODO document
_UI_EXTERN HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);

// TODO document
_UI_EXTERN void uiWindowsEnsureDestroyWindow(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsEnsureSetParent(HWND hwnd, HWND parent);

// TODO document
_UI_EXTERN void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR controlID, HWND insertAfter);

// TODO document
_UI_EXTERN char *uiWindowsWindowText(HWND hwnd);
_UI_EXTERN void uiWindowsSetWindowText(HWND hwnd, const char *text);

// TODO document
_UI_EXTERN intmax_t uiWindowsWindowTextWidth(HWND hwnd);

// TODO document
// TODO keep uiWindowsControl?
_UI_EXTERN void uiWindowsControlQueueRelayout(uiWindowsControl *c);

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




// everything below here is TODO

_UI_EXTERN void uiWindowsFinishControl(uiControl *c);
_UI_EXTERN void uiWindowsRearrangeControlIDsZOrder(uiControl *c);

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

#ifdef __cplusplus
}
#endif

#endif
