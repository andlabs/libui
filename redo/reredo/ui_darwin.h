// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __LIBUI_UI_DARWIN_H__
#define __LIBUI_UI_DARWIN_H__

typedef struct uiDarwinControl uiDarwinControl;
struct uiDarwinControl {
	uiControl c;
	void (*Relayout)(uiDarwinControl *);
};
_UI_EXTERN uintmax_t uiDarwinControlType(void);
#define uiDarwinControl(this) ((uiDarwinControl *) uiIsA((this), uiDarwinControlType(), 1))
// TODO document
_UI_EXTERN void uiDarwinControlRelayoutParent(uiDarwinControl *);

// TODO document
#define uiDarwinDefineControlWithOnDestroy(type, typefn, handlefield, onDestroy) \
	static uintmax_t _ ## type ## Type = 0; \
	uintmax_t typefn(void) \
	{ \
		if (_ ## type ## Type == 0) \
			_ ## type ## Type = uiRegisterType(#type, uiDarwinControlType(), sizeof (type)); \
		return _ ## type ## Type; \
	} \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *this = type(c); \
		onDestroy; \
		[this->handlefield release]; \
	} \
	static uintptr_t _ ## type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->handlefield); \
	} \
	static void _ ## type ## ContainerUpdateState(uiControl *c) \
	{ \
		/* do nothing */ \
	} \
	static void _ ## type ## Relayout(uiDarwinControl *c) \
	{ \
		uiDarwinControlRelayoutParent(uiDarwinControl(c)); \
	}

#define uiDarwinDefineControl(type, typefn, handlefield) \
	uiDarwinDefineControlWithOnDestroy(type, typefn, handlefield, (void) this;)

#define uiDarwinFinishNewControl(variable, type) \
	uiControl(variable)->CommitDestroy = _ ## type ## CommitDestroy; \
	uiControl(variable)->Handle = _ ## type ## Handle; \
	uiControl(variable)->ContainerUpdateState = _ ## type ## ContainerUpdateState; \
	uiDarwinControl(variable)->Relayout = _ ## type ## Relayout; \
	uiDarwinFinishControl(uiControl(variable));

// This is a function used to set up a control.
// Don't call it directly; use uiDarwinFinishNewControl() instead.
_UI_EXTERN void uiDarwinFinishControl(uiControl *c);

// Use this function as a shorthand for setting control fonts.
_UI_EXTERN void uiDarwinSetControlFont(NSControl *c, NSControlSize size);

// You can use this function from within your control implementations to return text strings that can be freed with uiFreeText().
_UI_EXTERN char *uiDarwinNSStringToText(NSString *);

#endif
