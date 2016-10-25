// 15 june 2016
#include "test.h"

static uiAreaHandler borderAH;
static int borderAHInit = 0;
static double lastx = -1, lasty = -1;

struct trect {
	double left;
	double top;
	double right;
	double bottom;
	int in;
};

#define tsetrect(re, l, t, r, b) re.left = l; re.top = t; re.right = r; re.bottom = b; re.in = lastx >= re.left && lastx < re.right && lasty >= re.top && lasty < re.bottom

struct tareas {
	struct trect move;
	struct trect alsomove;
	struct trect leftresize;
	struct trect topresize;
	struct trect rightresize;
	struct trect bottomresize; 
	struct trect topleftresize;
	struct trect toprightresize;
	struct trect bottomleftresize;
	struct trect bottomrightresize;
	struct trect close;
};

static void filltareas(double awid, double aht, struct tareas *ta)
{
	tsetrect(ta->move, 20, 20, awid - 20, 20 + 30);
	tsetrect(ta->alsomove, 30, 200, 100, 270);
	tsetrect(ta->leftresize, 5, 20, 15, aht - 20);
	tsetrect(ta->topresize, 20, 5, awid - 20, 15);
	tsetrect(ta->rightresize, awid - 15, 20, awid - 5, aht - 20);
	tsetrect(ta->bottomresize, 20, aht - 15, awid - 20, aht - 5);
	tsetrect(ta->topleftresize, 5, 5, 15, 15);
	tsetrect(ta->toprightresize, awid - 15, 5, awid - 5, 15);
	tsetrect(ta->bottomleftresize, 5, aht - 15, 15, aht - 5);
	tsetrect(ta->bottomrightresize, awid - 15, aht - 15, awid - 5, aht - 5);
	tsetrect(ta->close, 130, 200, 200, 270);
}

static void drawtrect(uiDrawContext *c, struct trect tr, double r, double g, double bl)
{
	uiDrawPath *p;
	uiDrawBrush b;

	memset(&b, 0, sizeof (uiDrawBrush));
	b.Type = uiDrawBrushTypeSolid;
	b.R = r;
	b.G = g;
	b.B = bl;
	b.A = 1.0;
	if (tr.in) {
		b.R += b.R * 0.75;
		b.G += b.G * 0.75;
		b.B += b.B * 0.75;
	}
	p = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(p,
		tr.left,
		tr.top,
		tr.right - tr.left,
		tr.bottom - tr.top);
	uiDrawPathEnd(p);
	uiDrawFill(c, p, &b);
	uiDrawFreePath(p);
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	struct tareas ta;

	filltareas(p->AreaWidth, p->AreaHeight, &ta);
	drawtrect(p->Context, ta.move, 0, 0.5, 0);
	drawtrect(p->Context, ta.alsomove, 0, 0.5, 0);
	drawtrect(p->Context, ta.leftresize, 0, 0, 0.5);
	drawtrect(p->Context, ta.topresize, 0, 0, 0.5);
	drawtrect(p->Context, ta.rightresize, 0, 0, 0.5);
	drawtrect(p->Context, ta.bottomresize, 0, 0, 0.5);
	drawtrect(p->Context, ta.topleftresize, 0, 0.5, 0.5);
	drawtrect(p->Context, ta.toprightresize, 0, 0.5, 0.5);
	drawtrect(p->Context, ta.bottomleftresize, 0, 0.5, 0.5);
	drawtrect(p->Context, ta.bottomrightresize, 0, 0.5, 0.5);
	drawtrect(p->Context, ta.close, 0.5, 0, 0);

	// TODO add current position prints here
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	struct tareas ta;

	lastx = e->X;
	lasty = e->Y;
	filltareas(e->AreaWidth, e->AreaHeight, &ta);
	// redraw our highlighted rect
	uiAreaQueueRedrawAll(area);
	if (e->Down != 1)
		return;
	if (ta.move.in || ta.alsomove.in) {
		uiAreaBeginUserWindowMove(area);
		return;
	}
#define resize(cond, edge) if (cond) { uiAreaBeginUserWindowResize(area, edge); return; }
	resize(ta.leftresize.in, uiWindowResizeEdgeLeft)
	resize(ta.topresize.in, uiWindowResizeEdgeTop)
	resize(ta.rightresize.in, uiWindowResizeEdgeRight)
	resize(ta.bottomresize.in, uiWindowResizeEdgeBottom)
	resize(ta.topleftresize.in, uiWindowResizeEdgeTopLeft)
	resize(ta.toprightresize.in, uiWindowResizeEdgeTopRight)
	resize(ta.bottomleftresize.in, uiWindowResizeEdgeBottomLeft)
	resize(ta.bottomrightresize.in, uiWindowResizeEdgeBottomRight)
	if (ta.close.in) {
		// TODO
		return;
	}
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	return 0;
}

static void borderWindowOpen(uiButton *b, void *data)
{
	uiWindow *w;
	uiArea *a;

	if (!borderAHInit) {
		borderAH.Draw = handlerDraw;
		borderAH.MouseEvent = handlerMouseEvent;
		borderAH.MouseCrossed = handlerMouseCrossed;
		borderAH.DragBroken = handlerDragBroken;
		borderAH.KeyEvent = handlerKeyEvent;
		borderAHInit = 1;
	}

	w = uiNewWindow("Border Resize Test", 300, 500, 0);
	uiWindowSetBorderless(w, 1);

	a = uiNewArea(&borderAH);
//	uiWindowSetChild(w, uiControl(a));
{uiBox *b;
b=uiNewHorizontalBox();
uiBoxAppend(b,uiControl(a),1);
uiWindowSetChild(w,uiControl(b));}
//TODO why is this hack needed? GTK+ issue

	uiControlShow(uiControl(w));
}

static uiSpinbox *width, *height;
static uiCheckbox *fullscreen;

static void sizeWidth(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	xp = uiSpinboxValue(width);
	uiWindowSetContentSize(w, xp, yp);
}

static void sizeHeight(uiSpinbox *s, void *data)
{
	uiWindow *w = uiWindow(data);
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	yp = uiSpinboxValue(height);
	uiWindowSetContentSize(w, xp, yp);
}

static void updatesize(uiWindow *w)
{
	int xp, yp;

	uiWindowContentSize(w, &xp, &yp);
	uiSpinboxSetValue(width, xp);
	uiSpinboxSetValue(height, yp);
	// TODO on OS X this is updated AFTER sending the size change, not before
	uiCheckboxSetChecked(fullscreen, uiWindowFullscreen(w));
}

void onSize(uiWindow *w, void *data)
{
	printf("size\n");
	updatesize(w);
}

void setFullscreen(uiCheckbox *cb, void *data)
{
	uiWindow *w = uiWindow(data);

	uiWindowSetFullscreen(w, uiCheckboxChecked(fullscreen));
	updatesize(w);
}

static void borderless(uiCheckbox *c, void *data)
{
	uiWindow *w = uiWindow(data);

	uiWindowSetBorderless(w, uiCheckboxChecked(c));
}

uiBox *makePage15(uiWindow *w)
{
	uiBox *page15;
	uiBox *hbox;
	uiButton *button;
	uiCheckbox *checkbox;

	page15 = newVerticalBox();

	hbox = newHorizontalBox();
	uiBoxAppend(page15, uiControl(hbox), 0);

	uiBoxAppend(hbox, uiControl(uiNewLabel("Size")), 0);
	width = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(width), 1);
	height = uiNewSpinbox(INT_MIN, INT_MAX);
	uiBoxAppend(hbox, uiControl(height), 1);
	fullscreen = uiNewCheckbox("Fullscreen");
	uiBoxAppend(hbox, uiControl(fullscreen), 0);

	uiSpinboxOnChanged(width, sizeWidth, w);
	uiSpinboxOnChanged(height, sizeHeight, w);
	uiCheckboxOnToggled(fullscreen, setFullscreen, w);
	uiWindowOnContentSizeChanged(w, onSize, NULL);
	updatesize(w);

	checkbox = uiNewCheckbox("Borderless");
	uiCheckboxOnToggled(checkbox, borderless, w);
	uiBoxAppend(page15, uiControl(checkbox), 0);

	button = uiNewButton("Borderless Resizes");
	uiButtonOnClicked(button, borderWindowOpen, NULL);
	uiBoxAppend(page15, uiControl(button), 0);

	hbox = newHorizontalBox();
	uiBoxAppend(page15, uiControl(hbox), 1);

	uiBoxAppend(hbox, uiControl(uiNewVerticalSeparator()), 0);

	return page15;
}
