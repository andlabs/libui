// 8 october 2015
#include "test.h"

static uiArea *area;
static uiCombobox *which;
static uiSpinbox *hamount;
static uiSpinbox *vamount;
static uiCheckbox *swallowKeys;

struct handler {
	uiAreaHandler ah;
};

static struct handler handler;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	runDrawTest(uiComboboxSelected(which), p);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{
	return uiSpinboxValue(hamount);
}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{
	return uiSpinboxValue(vamount);
}

static int handlerRedrawOnResize(uiAreaHandler *a, uiArea *area)
{
	// TODO make a checkbox
	return uiSpinboxValue(hamount) == 0 && uiSpinboxValue(vamount) == 0;
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	printf("mouse (%d,%d):(%d,%d) down:%d up:%d count:%d mods:%x held:%x\n",
		(int) e->X,
		(int) e->Y,
		(int) e->HScrollPos,
		(int) e->VScrollPos,
		(int) e->Down,
		(int) e->Up,
		(int) e->Count,
		(uint32_t) e->Modifiers,
		e->Held1To64);
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	printf("drag broken\n");
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	char k[4];

	k[0] = '\'';
	k[1] = e->Key;
	k[2] = '\'';
	k[3] = '\0';
	if (e->Key == 0) {
		k[0] = '0';
		k[1] = '\0';
	}
	printf("key key:%s extkey:%d mod:%d mods:%d up:%d\n",
		k,
		(int) e->ExtKey,
		(int) e->Modifier,
		(int) e->Modifiers,
		e->Up);
	return uiCheckboxChecked(swallowKeys);
}

static void onAmountChanged(uiSpinbox *s, void *data)
{
	uiAreaUpdateScroll(area);
}

static void shouldntHappen(uiCombobox *c, void *data)
{
	fprintf(stderr, "YOU SHOULD NOT SEE THIS. If you do, uiComboboxSetSelected() is triggering uiComboboxOnSelected(), which it should not.\n");
}

static void redraw(uiCombobox *c, void *data)
{
	uiAreaQueueRedrawAll(area);
}

uiBox *makePage6(void)
{
	uiBox *page6;
	uiBox *hbox;

	handler.ah.Draw = handlerDraw;
	handler.ah.HScrollMax = handlerHScrollMax;
	handler.ah.VScrollMax = handlerVScrollMax;
	handler.ah.RedrawOnResize = handlerRedrawOnResize;
	handler.ah.MouseEvent = handlerMouseEvent;
	handler.ah.DragBroken = handlerDragBroken;
	handler.ah.KeyEvent = handlerKeyEvent;

	page6 = newVerticalBox();

	hbox = newHorizontalBox();
	uiBoxAppend(page6, uiControl(hbox), 0);

	which = uiNewCombobox();
	populateComboboxWithTests(which);
	// this is to make sure that uiComboboxOnSelected() doesn't trigger with uiComboboxSetSelected()
	uiComboboxOnSelected(which, shouldntHappen, NULL);
	uiComboboxSetSelected(which, 0);
	uiComboboxOnSelected(which, redraw, NULL);
	uiBoxAppend(hbox, uiControl(which), 0);

	// make these first in case the area handler calls the information as part of the constructor
	hamount = uiNewSpinbox(0, 100000);
	uiSpinboxOnChanged(hamount, onAmountChanged, NULL);
	vamount = uiNewSpinbox(0, 100000);
	uiSpinboxOnChanged(vamount, onAmountChanged, NULL);

	area = uiNewArea((uiAreaHandler *) (&handler));
	uiBoxAppend(page6, uiControl(area), 1);

	hbox = newHorizontalBox();
	uiBoxAppend(hbox, uiControl(uiNewLabel("H ")), 0);
	uiBoxAppend(hbox, uiControl(hamount), 0);
	uiBoxAppend(hbox, uiControl(uiNewLabel(" V ")), 0);
	uiBoxAppend(hbox, uiControl(vamount), 0);
	uiBoxAppend(page6, uiControl(hbox), 0);

	swallowKeys = uiNewCheckbox("Consider key events handled");
	uiBoxAppend(page6, uiControl(swallowKeys), 0);

	return page6;
}
