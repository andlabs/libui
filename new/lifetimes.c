// 25 april 2015
#include "ui.h"
#include "uipriv.h"

// called by uiParentDestroy() and any container control in its uiControlDestroy()
void properlyDestroyControl(uiControl *c)
{
	uiControlSetOSContainer(c, NULL);
	uiControlSetHasParent(c, 0);
	uiControlDestroy(c);
}
