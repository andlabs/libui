// 27 april 2015
#include "ui.h"
#include "uipriv.h"

struct bin {
	uiBin b;
	void (*baseDestroy)(uiControl *);
	uiControl *mainControl;
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
};

static void binDestroy(uiControl *c)
{
	struct bin *b = (struct bin *) c;

	// ensure clean removal by making sure the bin has no OS parent
	if (uiBinHasOSParent(uiBin(b)))
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

static void binPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct bin *b = (struct bin *) c;
	intmax_t left, top, right, bottom;
	intmax_t cwid, cht;

	if (b->mainControl == NULL) {
		*width = 0;
		*height = 0;
		return;
	}

	// first do margins
	left = b->marginLeft;
	top = b->marginTop;
	right = b->marginRight;
	bottom = b->marginBottom;
	uiBinTranslateMargins(uiBin(b), &left, &top, &right, &bottom, d);
	*width = left + right;
	*height = top + bottom;

	// then do the control
	uiControlPreferredSize(b->mainControl, d, &cwid, &cht);
	*width += cwid;
	*height += cht;
}

static void binSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	struct bin *b = (struct bin *) c;

	if (b->mainControl != NULL)
		uiControlSysFunc(b->mainControl, p);
}

static void binResizeChildren(uiContainer *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct bin *b = (struct bin *) c;
	intmax_t left, top, right, bottom;

	if (b->mainControl == NULL)
		return;
	left = b->marginLeft;
	top = b->marginTop;
	right = b->marginRight;
	bottom = b->marginBottom;
	uiBinTranslateMargins(uiBin(b), &left, &top, &right, &bottom, d);
	x += left;
	y += top;
	width -= left + right;
	height -= top + bottom;
	uiControlResize(b->mainControl, x, y, width, height, d);
}

static void binSetMainControl(uiBin *bb, uiControl *mainControl)
{
	struct bin *b = (struct bin *) bb;

	if (b->mainControl != NULL)
		uiControlSetParent(b->mainControl, NULL);
	b->mainControl = mainControl;
	if (b->mainControl != NULL)
		uiControlSetParent(b->mainControl, uiContainer(b));
}

static void binSetMargins(uiBin *bb, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	struct bin *b = (struct bin *) bb;

	b->marginLeft = left;
	b->marginRight = right;
	b->marginTop = top;
	b->marginBottom = bottom;
}

uiBin *newBin(void)
{
	struct bin *b;

	b = uiNew(struct bin);

	uiMakeContainer(uiContainer(b));

	b->baseDestroy = uiControl(b)->Destroy;
	uiControl(b)->Destroy = binDestroy;
	uiControl(b)->PreferredSize = binPreferredSize;
	uiControl(b)->SysFunc = binSysFunc;

	uiContainer(b)->ResizeChildren = binResizeChildren;

	uiBin(b)->SetMainControl = binSetMainControl;
	uiBin(b)->SetMargins = binSetMargins;
	// these are defined by each OS's bin.c
	uiBin(b)->HasOSParent = binHasOSParent;
	uiBin(b)->SetOSParent = binSetOSParent;
	uiBin(b)->RemoveOSParent = binRemoveOSParent;
	uiBin(b)->ResizeRootAndUpdate = binResizeRootAndUpdate;
	uiBin(b)->TranslateMargins = binTranslateMargins;

	return uiBin(b);
}
