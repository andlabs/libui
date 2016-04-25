// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __LIBUI_UI_UNIX_H__
#define __LIBUI_UI_UNIX_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiUnixControl uiUnixControl;
struct uiUnixControl {
	uiControl c;
	void (*SetContainer)(uiUnixControl *, GtkContainer *, gboolean);
};
#define uiUnixControl(this) ((uiUnixControl *) (this))
// TODO document
_UI_EXTERN void uiUnixControlSetContainer(uiUnixControl *, GtkContainer *, gboolean);

#define uiUnixControlDefaultDestroy(type, handlefield) \
	static void type ## Destroy(uiControl *c) \
	{ \
		uiControlVerifyDestroy(c); \
		[type(c)->handlefield release]; \
		uiFreeControl(c); \
	}
#define uiUnixControlDefaultHandle(type, handlefield) \
	static uintptr_t type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->handlefield); \
	}
#define uiUnixControlDefaultParent(type, handlefield) \
	static uiControl *type ## Parent(uiControl *c) \
	{ \
		return uiUnixControl(c)->parent; \
	}
#define uiUnixControlDefaultSetParent(type, handlefield) \
	static void type ## SetParent(uiControl *c, uiControl *parent) \
	{ \
		uiControlVerifySetParent(c, parent); \
		uiUnixControl(c)->parent = parent; \
	}
#define uiUnixControlDefaultToplevel(type, handlefield) \
	static int type ## Toplevel(uiControl *c) \
	{ \
		return 0; \
	}
#define uiUnixControlDefaultVisible(type, handlefield) \
	static int type ## Visible(uiControl *c) \
	{ \
		return uiUnixControl(c)->visible; \
	}
#define uiUnixControlDefaultShow(type, handlefield) \
	static void type ## Show(uiControl *c) \
	{ \
		uiUnixControl(c)->visible = YES; \
		[type(c)->handlefield setHidden:NO]; \
	}
#define uiUnixControlDefaultHide(type, handlefield) \
	static void type ## Hide(uiControl *c) \
	{ \
		uiUnixControl(c)->visible = NO; \
		[type(c)->handlefield setHidden:YES]; \
	}
#define uiUnixControlDefaultEnabled(type, handlefield) \
	static int type ## Enabled(uiControl *c) \
	{ \
		return uiUnixControl(c)->enabled; \
	}
#define uiUnixControlDefaultEnable(type, handlefield) \
	static void type ## Enable(uiControl *c) \
	{ \
		uiUnixControl(c)->enabled = YES; \
		uiControlSyncEnableState(c, uiControlEnabledToUser(c)); \
	}
#define uiUnixControlDefaultDisable(type, handlefield) \
	static void type ## Disable(uiControl *c) \
	{ \
		uiUnixControl(c)->enabled = NO; \
		uiControlSyncEnableState(c, uiControlEnabledToUser(c)); \
	}
#define uiUnixControlDefaultSyncEnableState(type, handlefield) \
	static void type ## SyncEnableState(uiControl *c, int enabled) \
	{ \
		if ([type(c)->handlefield respondsToSelector:@selector(setEnabled:)]) \
			[((id) type(c)->handlefield) setEnabled:enabled]; /* id cast to make compiler happy; thanks mikeash in irc.freenode.net/#macdev */ \
	}
#define uiUnixControlDefaultSetSuperview(type, handlefield) \
	static void type ## SetSuperview(uiUnixControl *c, NSView *superview) \
	{ \
		[type(c)->handlefield setTranslatesAutoresizingMaskIntoConstraints:NO]; \
		if (superview == nil) \
			[type(c)->handlefield removeFromSuperview]; \
		else \
			[superview addSubview:type(c)->handlefield]; \
	}

#define uiUnixControlAllDefaultsExceptDestroy(type, handlefield) \
	uiUnixControlDefaultHandle(type, handlefield) \
	uiUnixControlDefaultParent(type, handlefield) \
	uiUnixControlDefaultSetParent(type, handlefield) \
	uiUnixControlDefaultToplevel(type, handlefield) \
	uiUnixControlDefaultVisible(type, handlefield) \
	uiUnixControlDefaultShow(type, handlefield) \
	uiUnixControlDefaultHide(type, handlefield) \
	uiUnixControlDefaultEnabled(type, handlefield) \
	uiUnixControlDefaultEnable(type, handlefield) \
	uiUnixControlDefaultDisable(type, handlefield) \
	uiUnixControlDefaultSyncEnableState(type, handlefield) \
	uiUnixControlDefaultSetSuperview(type, handlefield)

#define uiUnixControlAllDefaults(type, handlefield) \
	uiUnixControlDefaultDestroy(type, handlefield) \
	uiUnixControlAllDefaultsExceptDestroy(type, handlefield)

// TODO document
#define uiUnixNewControl(type, var) \
	var = type(uiUnixAllocControl(sizeof (type), type ## Signature, #type)); \
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
	uiControl(var)->SyncEnableState = type ## SyncEnableState; \
	uiUnixControl(var)->SetSuperview = type ## SetSuperview; \
	uiUnixControl(var)->visible = YES; \
	uiUnixControl(var)->enabled = YES;
// TODO document
_UI_EXTERN uiUnixControl *uiUnixAllocControl(size_t n, uint32_t typesig, const char *typenamestr);

// uiUnixStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
_UI_EXTERN char *uiUnixStrdupText(const char *);

#ifdef __cplusplus
}
#endif

#endif
