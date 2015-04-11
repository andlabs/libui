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

void uiControlSetParent(uiControl *c, uintptr_t parent)
{
	(*(c->setParent))(c, parent);
}

void uiControlRemoveParent(uiControl *c)
{
	(*(c->removeParent))(c);
}

void uiControlPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	(*(c->preferredSize))(c, d, width, height);
}

void uiControlResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	(*(c->resize))(c, x, y, width, height, d);
}

int uiControlVisible(uiControl *c)
{
	return (*(c->visible))(c);
}

void uiControlShow(uiControl *c)
{
	(*(c->show))(c);
}

void uiControlHide(uiControl *c)
{
	(*(c->hide))(c);
}

void uiControlContainerShow(uiControl *c)
{
	(*(c->containerShow))(c);
}

void uiControlContainerHide(uiControl *c)
{
	(*(c->containerHide))(c);
}

void uiControlEnable(uiControl *c)
{
	(*(c->enable))(c);
}

void uiControlDisable(uiControl *c)
{
	(*(c->disable))(c);
}

void uiControlContainerEnable(uiControl *c)
{
	(*(c->containerEnable))(c);
}

void uiControlContainerDisable(uiControl *c)
{
	(*(c->containerDisable))(c);
}
