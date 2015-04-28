// 13 august 2014
#include "uipriv_unix.h"


static void uipOSContainer_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	intmax_t x, y, width, height;

	x = allocation->x + c->marginLeft;
	y = allocation->y + c->marginTop;
	width = allocation->width - (c->marginLeft + c->marginRight);
	height = allocation->height - (c->marginTop + c->marginBottom);
}



// TODO convert other methods of other backends to pp arg p instance variable

static void parentDestroy(uiOSContainer *cc)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	// first, destroy the main control
	if (c->mainControl != NULL) {
		// we have to do this before we can destroy controls
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, NULL);
		uiControlDestroy(c->mainControl);
		c->mainControl = NULL;
	}
	// now we can mark the parent as ready to be destroyed
	c->canDestroy = TRUE;
	// finally, destroy the parent
	g_object_unref(G_OBJECT(c));
	// and free ourselves
	uiFree(cc);
}

static void parentSetMainControl(uiOSContainer *cc, uiControl *mainControl)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, NULL);
	}
	c->mainControl = mainControl;
	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 1);
		uiControlSetOSContainer(c->mainControl, cc);
	}
}

static void parentSetMargins(uiOSContainer *cc, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	c->marginLeft = left;
	c->marginTop = top;
	c->marginRight = right;
	c->marginBottom = bottom;
}
