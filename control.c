// 7 april 2015
#include "uipriv.h"

void uiControlDestroy(uiControl *c)
{
	(*(c->destroy))(c);
}

uintptr_t uiControlHandle(uiControl *c)
{
	return (*(c->handle))(c);
}

// TODO do this for the others
