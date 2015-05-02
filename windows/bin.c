// 27 april 2015
#include "uipriv_windows.h"

struct bin {
	uiContainer c;
	void (*baseDestroy)(uiControl *);
	uiControl *mainControl;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
};

void binDestroy(uiControl *c)
{
	struct bin *b = (struct bin *) c;
	HWND hwnd;

	// ensure clean removal by making sure the bin has no OS parent
	hwnd = (HWND) uiControlHandle(uiControl(b));
	if (GetAncestor(hwnd, GA_PARENT) != NULL)
		complain("attempt to destroy bin %p while it has an OS parent", b);
	// don't chain up to base here; we need to destroy children ourselves first
	if (b->mainControl != NULL) {
		uiControlSetParent(b->mainControl, NULL);
		uiControlDestroy(b->mainControl);
	}
	// NOW we can chain up to base
	(*(b->baseDestroy))(uiControl(b));
	uiFree(b);
}

void binPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct bin *b = (struct bin *) c;
	intmax_t marginX, marginY;

	if (b->mainControl == NULL) {
		*width = 0;
		*height = 0;
		return;
	}
	uiControlPreferredSize(b->mainControl, d, width, height);
	marginX = uiDlgUnitsToX(b->marginLeft, d->sys->baseX) + uiDlgUnitsToX(b->marginRight, d->sys->baseX);
	marginY = uiDlgUnitsToY(b->marginTop, d->sys->baseY) + uiDlgUnitsToY(b->marginBottom, d->sys->baseY);
	*width += marginX;
	*height += marginY;
}

void binResizeChildren(uiContainer *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct bin *b = (struct bin *) c;
	intmax_t marginLeft, marginTop;

	if (b->mainControl == NULL)
		return;
	marginLeft = uiDlgUnitsToX(b->marginLeft, d->sys->baseX);
	marginTop = uiDlgUnitsToY(b->marginTop, d->sys->baseY);
	x += marginLeft;
	y += marginTop;
	width -= marginLeft + uiDlgUnitsToX(b->marginRight, d->sys->baseX);
	height -= marginTop + uiDlgUnitsToY(b->marginBottom, d->sys->baseY);
	uiControlResize(b->mainControl, x, y, width, height, d);
}

uiContainer *newBin(void)
{
	struct bin *b;

	b = uiNew(struct bin);

	uiMakeContainer(uiContainer(b));

	b->baseDestroy = uiControl(b)->Destroy;
	uiControl(b)->Destroy = binDestroy;
	uiControl(b)->PreferredSize = binPreferredSize;

	uiContainer(b)->ResizeChildren = binResizeChildren;

	return uiContainer(b);
}

void binSetMainControl(uiContainer *c, uiControl *mainControl)
{
	struct bin *b = (struct bin *) c;

	if (b->mainControl != NULL)
		uiControlSetParent(b->mainControl, NULL);
	b->mainControl = mainControl;
	if (b->mainControl != NULL)
		uiControlSetParent(b->mainControl, uiContainer(b));
	uiContainerUpdate(uiContainer(b));
}

void binSetMargins(uiContainer *c, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	struct bin *b = (struct bin *) c;

	b->marginLeft = left;
	b->marginRight = right;
	b->marginTop = top;
	b->marginBottom = bottom;
	uiContainerUpdate(uiContainer(b));
}

void binSetParent(uiContainer *c, uintptr_t osParent)
{
	struct bin *b = (struct bin *) c;
	HWND hwnd;
	HWND newParent = (HWND) osParent;

	hwnd = (HWND) uiControlHandle(uiControl(b));
	if (SetParent(hwnd, newParent) == 0)
		logLastError("error changing bin's parent in binSetParent()");
}
