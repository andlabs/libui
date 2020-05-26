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
	// TODO check for w == NULL?
	return uiprivSysWindowTitle(w);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	// TODO check for w == NULL?
	// TODO other errors
	uiprivSysWindowSetTitle(w, title);
}
