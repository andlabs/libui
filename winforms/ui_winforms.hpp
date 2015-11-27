// 7 april 2015

/*
This file assumes that you have included <vccrt.h> and "ui.h" beforehand, as well as #using <System.dll> and the three WPF DLLs (TODO) (but not necessarily beforehand). It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __LIBUI_UI_WINDOWS_H__
#define __LIBUI_UI_WINDOWS_H__

#ifndef __cplusplus_cli
#error Sorry; ui_winforms.hpp can currently only be used from C++/CLI code.
#endif

extern "C" {

typedef struct uiWindowsSizing uiWindowsSizing;

typedef struct uiWindowsControl uiWindowsControl;
struct uiWindowsControl {
	uiControl c;
	// TODO make truly private
	gcroot<System::Windows::UIElement ^> *genericHandle;
};
_UI_EXTERN uintmax_t uiWindowsControlType(void);
#define uiWindowsControl(this) ((uiWindowsControl *) uiIsA((this), uiWindowsControlType(), 1))

// TODO document
#define uiWindowsDefineControlWithOnDestroy(type, typefn, handle, onDestroy) \
	static uintmax_t _ ## type ## Type = 0; \
	uintmax_t typefn(void) \
	{ \
		if (_ ## type ## Type == 0) \
			_ ## type ## Type = uiRegisterType(#type, uiWindowsControlType(), sizeof (type)); \
		return _ ## type ## Type; \
	} \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *hthis = type(c); \
		onDestroy; \
		delete uiWindowsControl(c)->genericHandle; \
		delete hthis->handle; \
	} \
	static uintptr_t _ ## type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (uiWindowsControl(c)->genericHandle); \
	} \
	static void _ ## type ## ContainerUpdateState(uiControl *c) \
	{ \
		/* do nothing */ \
	}

#define uiWindowsDefineControl(type, typefn, handle) \
	uiWindowsDefineControlWithOnDestroy(type, typefn, handle, (void) hthis;)

#define uiWindowsFinishNewControl(variable, type, handle) \
	uiControl(variable)->CommitDestroy = _ ## type ## CommitDestroy; \
	uiControl(variable)->Handle = _ ## type ## Handle; \
	uiControl(variable)->ContainerUpdateState = _ ## type ## ContainerUpdateState; \
	uiWindowsControl(variable)->genericHandle = new gcroot<System::Windows::UIElement ^>(); \
	*(uiWindowsControl(variable)->genericHandle) = *(variable->handle); \
	uiWindowsFinishControl(uiControl(variable));

// This is a function used to set up a control.
// Don't call it directly; use uiWindowsFinishNewControl() instead.
_UI_EXTERN void uiWindowsFinishControl(uiControl *c);

// TODO document
_UI_EXTERN char *uiWindowsCLRStringToText(gcroot<System::String ^> str);

}

#endif
