// 11 june 2015
#include "uipriv_unix.h"

struct window {
	uiWindow w;
	GtkWidget *widget;
	uiControl *child;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
};

uiDefineControlType(uiWindow, uiTypeWindow, struct window)

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->widget);
}

static void windowContainerUpdateState(uiControl *c)
{
	struct window *w = (struct window *) c;

	if (w->child != NULL)
		uiControlUpdateState(w->child);
}

static char *windowTitle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return PUT_CODE_HERE;
}

static void windowSetTitle(uiWindow *ww, const char *title)
{
	struct window *w = (struct window *) ww;

	PUT_CODE_HERE;
	// don't queue resize; the caption isn't part of what affects layout and sizing of the client area (it'll be ellipsized if too long)
}

static void windowOnClosing(uiWindow *ww, int (*f)(uiWindow *, void *), void *data)
{
	struct window *w = (struct window *) ww;

	w->onClosing = f;
	w->onClosingData = data;
}

static void windowSetChild(uiWindow *ww, uiControl *child)
{
	struct window *w = (struct window *) ww;

	if (w->child != NULL)
		uiControlSetParent(w->child, NULL);
	w->child = child;
	if (w->child != NULL) {
		uiControlSetParent(w->child, uiControl(w));
		uiControlQueueResize(w->child);
	}
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return w->margined;
}

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	w->margined = margined;
	uiControlQueueResize(uiControl(w));
}

static void windowResizeChild(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	if (w->child == NULL)
		return;
	PUT_CODE_HERE;
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;

	w = (struct window *) MAKE_CONTROL_INSTANCE(uiTypeWindow());

	PUT_CODE_HERE;

	w->onClosing = defaultOnClosing;

	uiControl(w)->Handle = windowHandle;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;
	uiWindow(w)->ResizeChild = windowResizeChild;

	return uiWindow(w);
}
