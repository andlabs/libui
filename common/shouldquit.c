// 9 may 2015
#include "../ui.h"
#include "uipriv.h"

static int defaultOnShouldQuit(void *data)
{
	return 0;
}

static int (*onShouldQuit)(void *) = defaultOnShouldQuit;
static void *onShouldQuitData = NULL;

void uiOnShouldQuit(int (*f)(void *), void *data)
{
	onShouldQuit = f;
	onShouldQuitData = data;
}

int uiprivShouldQuit(void)
{
	return (*onShouldQuit)(onShouldQuitData);
}
