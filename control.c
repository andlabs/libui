// 7 april 2015
#include "uipriv.h"

uintptr_t uiControlHandle(uiControl *c)
{
	return (*(c->handle))(c);
}

// TODO do this for the others
