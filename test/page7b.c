// 13 october 2015
#include "test.h"

static uiArea *area;
static uiCheckbox *label;

struct handler {
	uiAreaHandler ah;
};

static struct handler handler;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	// do nothing
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	char pos[128];

	snprintf(pos, 127, "X %g Y %g", e->X, e->Y);
	uiCheckboxSetText(label, pos);
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
	uiCheckboxSetChecked(label, !left);
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	// do nothing
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	if (e->Key == 'h' && !e->Up) {
		// TODO hide the widget momentarily on the h key
		return 1;
	}
	return 0;
}

uiGroup *makePage7b(void)
{
	uiGroup *group;
	uiBox *box;

	handler.ah.Draw = handlerDraw;
	handler.ah.MouseEvent = handlerMouseEvent;
	handler.ah.MouseCrossed = handlerMouseCrossed;
	handler.ah.DragBroken = handlerDragBroken;
	handler.ah.KeyEvent = handlerKeyEvent;

	group = newGroup("Scrolling Mouse Test");

	box = newVerticalBox();
	uiGroupSetChild(group, uiControl(box));

	area = uiNewScrollingArea((uiAreaHandler *) (&handler), 5000, 5000);
	uiBoxAppend(box, uiControl(area), 1);

	label = uiNewCheckbox("");
	uiBoxAppend(box, uiControl(label), 0);

	return group;
}
