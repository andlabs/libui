// 7 april 2015

/*
This file assumes that you have imported <Cocoa/Cocoa.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __LIBUI_UI_DARWIN_H__
#define __LIBUI_UI_DARWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiDarwinControl uiDarwinControl;
struct uiDarwinControl {
	uiControl c;
	uiControl *parent;
	BOOL enabled;
	BOOL visible;
	void (*AddSubview)(uiDarwinControl *, NSView *);
	void (*Relayout)(uiDarwinControl *);
};
#define uiDarwinControl(this) ((uiDarwinControl *) (this))
// TODO document
_UI_EXTERN void uiDarwinControlAddSubview(uiDarwinControl *, NSView *);
_UI_EXTERN void uiDarwinControlTriggerRelayout(uiDarwinControl *);

#define uiDarwinControlDefaultDestroy(type, handlefield) \
	static void type ## Destroy(uiControl *c) \
	{ \
		uiControlVerifyDestroy(c); \
		[type(c)->handlefield release]; \
	}
#define uiDarwinControlDefaultHandle(type, handlefield) \
	static uintptr_t type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->handlefield); \
	}
#define uiDarwinControlDefaultParent(type) \
	static uiControl *type ## Parent(uiControl *c) \
	{ \
		return uiDarwinControl(c)->parent; \
	}
#define uiDarwinControlDefaultSetParent(type, handlefield) \
	static void type ## SetParent(uiControl *c, uiControl *parent) \
	{ \
		uiControlVerifySetParent(c, parent); \
		uiDarwinControl(c)->parent = parent; \
		if (uiDarwinControl(c)->parent == NULL) \
			[type(c)->handlefield removeFromSuperview]; \
		else \
			uiDarwinControlAddSubview(uiDarwinControl(uiDarwinControl(c)->parent), type(c)->handlefield); \
	}
#define uiDarwinControlDefaultToplevel(type) \
	static int type ## Toplevel(uiControl *c) \
	{ \
		return 0; \
	}
#define uiDarwinControlDefaultVisible(type) \
	static int type ## Visible(uiDarwinControl *c) \
	{ \
		/* TODO */ \
		return uiDarwinControl(c)->visible; \
	}
// TODO others here
#define uiDarwinControlDefaultAddSubview(type) \
	static void type ## AddSubview(uiDarwinControl *c, NSView *subview) \
	{ \
		/* TODO do nothing or log? one of the two */ \
	}
#define uiDarwinControlDefaultRelayout(type) \
	static void type ## Relayout(uiDarwinControl *c) \
	{ \
		/* do nothing */ \
	}

#define uiDarwinControlAllDefaults(type, handlefield) \
	uiDarwinControlDefaultDestroy(type, handlefield) \
	uiDarwinControlDefaultHandle(type, handlefield) \
	uiDarwinControlDefaultParent(type) \
	uiDarwinControlDefaultSetParent(type, handlefield) \
	uiDarwinControlDefaultToplevel(type) \
	xxxxx \
	uiDarwinControlDefaultAddSubview(type) \
	uiDarwinControlDefaultRelayout(type)

// TODO document
#define uiDarwinNewControl(var, type) \
	var = type(uiDarwinNewControl(sizeof (type), type ## Signature, #type)) \
	TODO
_UI_EXTERN uiDarwinControl *uiDarwinAllocControl(size_t n, uint32_t typesig, const char *typenamestr);

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

#ifdef __cplusplus
}
#endif

#endif
