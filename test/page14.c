// 9 june 2016
#include "test.h"

enum {
	red,
	green,
	blue,
	yellow,
};

static const struct {
	double r;
	double g;
	double b;
} colors[] = {
	{ 1, 0, 0 },
	{ 0, 0.5, 0 },
	{ 0, 0, 1 },
	{ 1, 1, 0 },
};

static uiControl *testControl(const char *label, int color)
{
	uiColorButton *b;

	b = uiNewColorButton();
	uiColorButtonSetColor(b, colors[color].r, colors[color].g, colors[color].b, 1.0);
	return uiControl(b);
}

static uiControl *simpleGrid(void)
{
	uiGrid *g;
	uiControl *t4;

	g = newGrid();

	uiGridAppend(g, testControl("1", red),
		0, 0, 1, 1,
		0, uiAlignFill, 0, uiAlignFill);
	uiGridAppend(g, testControl("2", green),
		1, 0, 1, 1,
		0, uiAlignFill, 0, uiAlignFill);
	uiGridAppend(g, testControl("3", blue),
		2, 0, 1, 1,
		0, uiAlignFill, 0, uiAlignFill);
	t4 = testControl("4", green);
	uiGridAppend(g, t4,
		0, 1, 1, 1,
		0, uiAreaFill, 1, uiAreaFill);
	uiGridInsertAt(g, testControl("5", blue),
		t4, uiAtTrailing, 2, 1,
		0, uiAreaFill, 0, uiAreaFill);
	uiGridAppend(g, testControl("6", yellow),
		-1, 0, 1, 2,
		1, uiAreaFill, 0, uiAreaFill);

	return uiControl(g);
}

static const struct {
	const char *name;
	uiControl *(*f)(void);
} pages[] = {
	{ "Simple Grid", simpleGrid },		// from GTK+ test/testgrid.c
	{ NULL, NULL },
}

uiTab *makePage14(void)
{
	uiTab *page14;
	int i;

	page14 = newTab();

	for (i = 0; pages[i].name != NULL; i++)
		uiTabAppend(page14,
			pages[i].name,
			(*(pages[i].f))());

	return page14;
}
