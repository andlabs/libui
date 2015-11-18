// 17 november 2015

/*
This file assumes that you have included the various Haiku header files and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Haiku.
*/

#ifndef __LIBUI_UI_HAIKU_HPP__
#define __LIBUI_UI_WAIKU_HPP__

#ifndef __cplusplus
#error Sorry; ui_haiku.hpp can currently only be used from C++ code.
#endif

extern "C" {

typedef struct uiHaikuControl uiHaikuControl;
struct uiHaikuControl {
	uiControl c;
};
_UI_EXTERN uintmax_t uiHaikuControlType(void);
#define uiHaikuControl(this) ((uiHaikuControl *) uiIsA((this), uiHaikuControlType(), 1))

// TODO document
#define uiHaikuDefineControlWithOnDestroy(type, typefn, handlefield, onDestroy) \
	static uintmax_t _ ## type ## Type = 0; \
	uintmax_t typefn(void) \
	{ \
		if (_ ## type ## Type == 0) \
			_ ## type ## Type = uiRegisterType(#type, uiHaikuControlType(), sizeof (type)); \
		return _ ## type ## Type; \
	} \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *hthis = type(c); \
		onDestroy; \
		delete hthis->handlefield; \
	} \
	static uintptr_t _ ## type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->handlefield); \
	} \
	static void _ ## type ## ContainerUpdateState(uiControl *c) \
	{ \
		/* do nothing */ \
	}

#define uiHaikuDefineControl(type, typefn, handlefield) \
	uiHaikuDefineControlWithOnDestroy(type, typefn, handlefield, (void) hthis;)

#define uiHaikuFinishNewControl(variable, type) \
	uiControl(variable)->CommitDestroy = _ ## type ## CommitDestroy; \
	uiControl(variable)->Handle = _ ## type ## Handle; \
	uiControl(variable)->ContainerUpdateState = _ ## type ## ContainerUpdateState; \
	uiHaikuFinishControl(uiControl(variable));

// This is a function used to set up a control.
// Don't call it directly; use uiHaikuFinishNewControl() instead.
_UI_EXTERN void uiHaikuFinishControl(uiControl *c);

// uiHaikuStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
_UI_EXTERN char *uiHaikuStrdupText(const char *);

}

#endif
