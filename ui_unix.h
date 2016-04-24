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
};
#define uiUnixControl(this) ((uiUnixControl *) (this))

// TODO document
#define uiUnixDefineControlWithOnDestroy(type, onDestroy) \
	static void _ ## type ## CommitDestroy(uiControl *c) \
	{ \
		type *this = type(c); \
		onDestroy; \
		g_object_unref(this->widget); \
	} \
	static uintptr_t _ ## type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->widget); \
	} \
	static void _ ## type ## ContainerUpdateState(uiControl *c) \
	{ \
		/* do nothing */ \
	}

#define uiUnixDefineControl(type) \
	uiUnixDefineControlWithOnDestroy(type, (void) this;)

#define uiUnixFinishNewControl(variable, type) \
	uiControl(variable)->CommitDestroy = _ ## type ## CommitDestroy; \
	uiControl(variable)->Handle = _ ## type ## Handle; \
	uiControl(variable)->ContainerUpdateState = _ ## type ## ContainerUpdateState; \
	uiUnixFinishControl(uiControl(variable));

// This is a function used to set up a control.
// Don't call it directly; use uiUnixFinishNewControl() instead.
_UI_EXTERN void uiUnixFinishControl(uiControl *c);

// uiUnixStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
_UI_EXTERN char *uiUnixStrdupText(const char *);

#ifdef __cplusplus
}
#endif

#endif
