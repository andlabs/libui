// 25 may 2020
#include "uipriv.h"

uint32_t uiWindowType(void)
{
	if (!uiprivCheckInitializedAndThread())
		return 0;
	return uiprivSysWindowType();
}

uiWindow *uiNewWindow(void)
{
	if (!uiprivCheckInitializedAndThread())
		return NULL;
	return uiprivSysNewWindow();
}

const char *uiWindowTitle(uiWindow *w)
{
	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (w == NULL) {
		uiprivProgrammerErrorNullPointer("uiWindow", uiprivFunc);
		return NULL;
	}
	return uiprivSysWindowTitle(w);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (w == NULL) {
		uiprivProgrammerErrorNullPointer("uiWindow", uiprivFunc);
		return;
	}
	if (title == NULL) {
		uiprivProgrammerErrorNullPointer("title", uiprivFunc);
		return;
	}
	uiprivSysWindowSetTitle(w, title);
}

uiControl *uiWindowChild(uiWindow *w)
{
	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (w == NULL) {
		uiprivProgrammerErrorNullPointer("uiWindow", uiprivFunc);
		return NULL;
	}
	return uiprivSysWindowChild(w);
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (w == NULL) {
		uiprivProgrammerErrorNullPointer("uiWindow", uiprivFunc);
		return;
	}
	uiprivSysWindowSetChild(w, child);
}
