// 13 october 2015
#include "test.h"

static uiArea *area;
static uiEntry *startAngle;
static uiEntry *sweep;
static uiCheckbox *negative;
static uiCheckbox *radians;

struct handler {
	uiAreaHandler ah;
};

static struct handler handler;

// based on the cairo arc sample
static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	double xc = 128.0;
	double yc = 128.0;
	double radius = 100.0;
	uiDrawBrush source;
	uiDrawStrokeParams sp;
	uiDrawPath *path;
	char *startText;
	char *sweepText;
	double factor;

	source.Type = uiDrawBrushTypeSolid;
	source.R = 0;
	source.G = 0;
	source.B = 0;
	source.A = 1;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	sp.Dashes = NULL;
	sp.NumDashes = 0;
	sp.DashPhase = 0;

	startText = uiEntryText(startAngle);
	sweepText = uiEntryText(sweep);

	factor = uiPi / 180;
	if (uiCheckboxChecked(radians))
		factor = 1;

	sp.Thickness = 10.0;
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path, xc, yc);
	uiDrawPathArcTo(path,
		xc, yc,
		radius,
		atof(startText) * factor,
		atof(sweepText) * factor,
		uiCheckboxChecked(negative));
	uiDrawPathEnd(path);
	uiDrawStroke(p->Context, path, &source, &sp);
	uiDrawFreePath(path);

	uiFreeText(startText);
	uiFreeText(sweepText);
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	// do nothing
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
	// do nothing
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	// do nothing
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	return 0;
}

static void entryChanged(uiEntry *e, void *data)
{
	uiAreaQueueRedrawAll(area);
}

static void checkboxToggled(uiCheckbox *c, void *data)
{
	uiAreaQueueRedrawAll(area);
}

uiGroup *makePage7a(void)
{
	uiGroup *group;
	uiBox *box, *box2;

	handler.ah.Draw = handlerDraw;
	handler.ah.MouseEvent = handlerMouseEvent;
	handler.ah.MouseCrossed = handlerMouseCrossed;
	handler.ah.DragBroken = handlerDragBroken;
	handler.ah.KeyEvent = handlerKeyEvent;

	group = newGroup("Arc Test");

	box = newVerticalBox();
	uiGroupSetChild(group, uiControl(box));

	area = uiNewArea((uiAreaHandler *) (&handler));
	uiBoxAppend(box, uiControl(area), 1);

	box2 = newHorizontalBox();
	uiBoxAppend(box, uiControl(box2), 0);

	uiBoxAppend(box2, uiControl(uiNewLabel("Start Angle")), 0);
	startAngle = uiNewEntry();
	uiEntryOnChanged(startAngle, entryChanged, NULL);
	uiBoxAppend(box2, uiControl(startAngle), 1);

	box2 = newHorizontalBox();
	uiBoxAppend(box, uiControl(box2), 0);

	uiBoxAppend(box2, uiControl(uiNewLabel("Sweep")), 0);
	sweep = uiNewEntry();
	uiEntryOnChanged(sweep, entryChanged, NULL);
	uiBoxAppend(box2, uiControl(sweep), 1);

	negative = uiNewCheckbox("Negative");
	uiCheckboxOnToggled(negative, checkboxToggled, NULL);
	uiBoxAppend(box, uiControl(negative), 0);

	radians = uiNewCheckbox("Radians");
	uiCheckboxOnToggled(radians, checkboxToggled, NULL);
	uiBoxAppend(box, uiControl(radians), 0);

	return group;
}
