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
	void (*SyncEnableState)(uiWindowsControl *, int);
	void (*SetParentHWND)(uiWindowsControl *, HWND);
	void (*MinimumSize)(uiWindowsControl *, intmax_t *, intmax_t *);
	void (*ChildMinimumSizeChanged)(uiWIndowsControl *);
	void (*AssignControlIDZOrder)(uiWindowsControl *, LONG_PTR *, HWND *);
};
#define uiWindowsControl(this) ((uiWindowsControl *) (this))
// TODO document
_UI_EXTERN void uiWindowsControlSyncEnableState(uiWindowsControl *, int);
_UI_EXTERN void uiWindowsControlSetParentHWND(uiWindowsControl *, HWND);
_UI_EXTERN void uiWindowsControlMinimumSize(uiWindowsControl *, intmax_t *, intmax_t *);
_UI_EXTERN void uiWindowsControlChildMinimumSizeChanged(uiWindowsControl *);
_UI_EXTERN void uiWindowsControlAssignControlIDZOrder(uiWindowsControl *, LONG_PTR *, HWND *);

// TODO document
#define uiWindowsControlDefaultDestroy(type) \
	static void type ## Destroy(uiControl *c) \
	{ \
		uiControlVerifyDestroy(c); \
		uiWindowsEnsureDestroyWindow(type(c)->hwnd); \
		uiFreeControl(c); \
	}
#define uiWindowsControlDefaultHandle(type) \
	static uintptr_t type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->hwnd); \
	}
#define uiWindowsControlDefaultParent(type) \
	static uiControl *type ## Parent(uiControl *c) \
	{ \
		return uiWindowsControl(c)->parent; \
	}
#define uiWindowsControlDefaultSetParent(type) \
	static void type ## SetParent(uiControl *c, uiControl *parent) \
	{ \
		uiControlVerifySetParent(c, parent); \
		uiWindowsControl(c)->parent = parent; \
	}
#define uiWindowsControlDefaultToplevel(type) \
	static int type ## Toplevel(uiControl *c) \
	{ \
		return 0; \
	}
#define uiWindowsControlDefaultVisible(type) \
	static int type ## Visible(uiControl *c) \
	{ \
		return uiWindowsControl(c)->visible; \
	}
#define uiWindowsControlDefaultShow(type) \
	static void type ## Show(uiControl *c) \
	{ \
		uiWindowsControl(c)->visible = YES; \
		ShowWindow(type(c)->hwnd, SW_SHOW); \
	}
#define uiWindowsControlDefaultHide(type) \
	static void type ## Hide(uiControl *c) \
	{ \
		uiWindowsControl(c)->visible = NO; \
		ShowWindow(type(c)->hwnd, SW_HIDE); \
	}
#define uiWindowsControlDefaultEnabled(type) \
	static int type ## Enabled(uiControl *c) \
	{ \
		return uiWindowsControl(c)->enabled; \
	}
#define uiWindowsControlDefaultEnable(type) \
	static void type ## Enable(uiControl *c) \
	{ \
		uiWindowsControl(c)->enabled = YES; \
		uiWindowsControlSyncEnableState(uiWindowsControl(c), uiControlEnabledToUser(c)); \
	}
#define uiWindowsControlDefaultDisable(type) \
	static void type ## Disable(uiControl *c) \
	{ \
		uiWindowsControl(c)->enabled = NO; \
		uiWindowsControlSyncEnableState(uiWindowsControl(c), uiControlEnabledToUser(c)); \
	}
#define uiWindowsControlDefaultSyncEnableState(type) \
	static void type ## SyncEnableState(uiWindowsControl *c, int enabled) \
	{ \
		if (uiWindowsShouldStopSyncEnableState(c, enabled)) \
			return; \
		EnableWindow(type(c)->hwnd, enabled); \
	}
#define uiWindowsControlDefaultSetParentHWND(type) \
	static void type ## SetParentHWND(uiWindowsControl *c, HWND parent) \
	{ \
		uiWindowsEnsureSetParent(type(c)->hwnd, parent); \
	}
// note that there is no uiWindowsControlDefaultMinimumSize(); you MUST define this yourself!
#define uiWindowsDefaultChildMinimumSizeChanged(type) \
	static void type ## ChildMinimumSizeChanged)(uiWIndowsControl *c) \
	{ \
		/* do nothing; default has no children */ \
	}
#define uiWindowsDefaultAssignControlIDZorder(type) \
	static void type ## AssignControlIDZOrder)(uiWindowsControl *c, LONG_PTR *cID, HWND *zorder) \
	{ \
		uiWindowsEnsureAssignControlIDZOrder(c, cID, zorder); \
	}

#define uiWindowsControlAllDefaultsExceptDestroy(type) \
	uiWindowsControlDefaultHandle(type) \
	uiWindowsControlDefaultParent(type) \
	uiWindowsControlDefaultSetParent(type) \
	uiWindowsControlDefaultToplevel(type) \
	uiWindowsControlDefaultVisible(type) \
	uiWindowsControlDefaultShow(type) \
	uiWindowsControlDefaultHide(type) \
	uiWindowsControlDefaultEnabled(type) \
	uiWindowsControlDefaultEnable(type) \
	uiWindowsControlDefaultDisable(type) \
	uiWindowsControlDefaultSyncEnableState(type) \
	uiWindowsControlDefaultSetParentHWND(type) \
	uiWindowsDefaultChildMinimumSizeChanged(type) \
	uiWindowsDefaultAssignControlIDZorder(type)

#define uiWindowsControlAllDefaults(type) \
	uiWindowsControlDefaultDestroy(type) \
	uiWindowsControlAllDefaultsExceptDestroy(type)

// TODO document
#define uiWindowsNewControl(type, var) \
	var = type(uiWindowsAllocControl(sizeof (type), type ## Signature, #type)); \
	uiControl(var)->Destroy = type ## Destroy; \
	uiControl(var)->Handle = type ## Handle; \
	uiControl(var)->Parent = type ## Parent; \
	uiControl(var)->SetParent = type ## SetParent; \
	uiControl(var)->Toplevel = type ## Toplevel; \
	uiControl(var)->Visible = type ## Visible; \
	uiControl(var)->Show = type ## Show; \
	uiControl(var)->Hide = type ## Hide; \
	uiControl(var)->Enabled = type ## Enabled; \
	uiControl(var)->Enable = type ## Enable; \
	uiControl(var)->Disable = type ## Disable; \
	uiWindowsControl(var)->SyncEnableState = type ## SyncEnableState; \
	uiWindowsControl(var)->SetParentHWND = type ## SetParentHWND; \
	uiWindowsControl(var)->MinimumSize = type ## MinimumSize; \
	uiWindowsControl(var)->ChildMinimumSizeChanged = type ## ChildMinimumSizeChanged; \
	uiWindowsControl(var)->AssignControlIDZOrder = type ## AssignControlIDZorder; \
	uiWindowsControl(var)->visible = YES; \
	uiWindowsControl(var)->enabled = YES;
// TODO document
_UI_EXTERN uiWindowsControl *uiWindowsAllocControl(size_t n, uint32_t typesig, const char *typenamestr);

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
_UI_EXTERN void uiWindowsSizingDlgUnitsToPixels(uiWindowsSIzing *sizing, int *x, int *y);
_UI_EXTERN void uiWindowsSizingStandardPadding(uiWindowsSizing *sizing, int *x, int *y);

// TODO document
_UI_EXTERN HWND uiWindowsMakeContainer(void (*onResize)(void *data), void *data);

// TODO document
_UI_EXTERN void uiWindowsControlNotifyMinimumSizeChanged(uiWindowsControl *);

#ifdef __cplusplus
}
#endif

#endif
