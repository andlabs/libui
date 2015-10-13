// 13 october 2015
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

uiWindow *mainwin;
uiArea *histogram;
uiAreaHandler handler;
uiSpinbox *datapoints[10];

// some metrics
#define xoffLeft 20			/* histogram margins */
#define yoffTop 20
#define xoffRight 20
#define yoffBottom 20

// helper to quickly set a brush color
static void setSolidBrush(uiDrawBrush *brush, uint32_t color, double alpha)
{
	uint8_t component;

	brush->Type = uiDrawBrushTypeSolid;
	component = (uint8_t) ((color >> 16) & 0xFF);
	brush->R = ((double) component) / 255;
	component = (uint8_t) ((color >> 8) & 0xFF);
	brush->G = ((double) component) / 255;
	component = (uint8_t) (color & 0xFF);
	brush->B = ((double) component) / 255;
	brush->A = alpha;
}

// and some colors
// names and values from https://msdn.microsoft.com/en-us/library/windows/desktop/dd370907%28v=vs.85%29.aspx
#define colorWhite 0xFFFFFF
#define colorBlack 0x000000
#define colorDodgerBlue 0x1E90FF

static uiDrawPath *constructGraph(double width, double height, int extend)
{
	uiDrawPath *path;
	double xincr, yincr;
	int i, n;

	path = uiDrawNewPath(uiDrawFillModeWinding);

	xincr = width / 9;		// 10 - 1 to make the last point be at the end
	yincr = height / 100;
	for (i = 0; i < 10; i++) {
		// get the value of the point
		n = uiSpinboxValue(datapoints[i]);
		// because y=0 is the top but n=0 is the bottom, we need to flip
		n = 100 - n;
		if (i == 0)
			uiDrawPathNewFigure(path,
				xincr * i,
				yincr * n);
		else
			uiDrawPathLineTo(path,
				xincr * i,
				yincr * n);
	}

	if (extend) {
		uiDrawPathLineTo(path, width, height);
		uiDrawPathLineTo(path, 0, height);
		uiDrawPathCloseFigure(path);
	}

	uiDrawPathEnd(path);
	return path;
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush brush;
	uiDrawStrokeParams sp;
	uiDrawMatrix m;
	double graphWidth, graphHeight;

	// fill the area with white
	setSolidBrush(&brush, colorWhite, 1.0);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 0, 0, p->ClientWidth, p->ClientHeight);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);

	// figure out dimensions
	graphWidth = p->ClientWidth - xoffLeft - xoffRight;
	graphHeight = p->ClientHeight - yoffTop - yoffBottom;

	// draw the axes
	setSolidBrush(&brush, colorBlack, 1.0);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathNewFigure(path,
		xoffLeft, yoffTop);
	uiDrawPathLineTo(path,
		xoffLeft, yoffTop + graphHeight);
	uiDrawPathLineTo(path,
		xoffLeft + graphWidth, yoffTop + graphHeight);
	uiDrawPathEnd(path);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 2;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	// now transform the coordinate space so (0, 0) is the top-left corner of the graph
	uiDrawMatrixSetIdentity(&m);
	uiDrawMatrixTranslate(&m, xoffLeft, yoffTop);
	uiDrawTransform(p->Context, &m);

	// now create the fill for the graph below the graph line
	path = constructGraph(graphWidth, graphHeight, 1);
	setSolidBrush(&brush, colorDodgerBlue, 0.5);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{
	return 0;
}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{
	return 0;
}

static int handlerRedrawOnResize(uiAreaHandler *a, uiArea *area)
{
	return 1;
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
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

static int onClosing(uiWindow *w, void *data)
{
	uiControlDestroy(uiControl(mainwin));
	uiQuit();
	return 0;
}

static int shouldQuit(void *data)
{
	uiControlDestroy(uiControl(mainwin));
	return 1;
}

int main(void)
{
	uiInitOptions o;
	const char *err;
	uiBox *hbox, *vbox;
	int i;

	handler.Draw = handlerDraw;
	handler.HScrollMax = handlerHScrollMax;
	handler.VScrollMax = handlerVScrollMax;
	handler.RedrawOnResize = handlerRedrawOnResize;
	handler.MouseEvent = handlerMouseEvent;
	handler.DragBroken = handlerDragBroken;
	handler.KeyEvent = handlerKeyEvent;

	memset(&o, 0, sizeof (uiInitOptions));
	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	uiOnShouldQuit(shouldQuit, NULL);

	mainwin = uiNewWindow("libui Histogram Example", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiWindowSetChild(mainwin, uiControl(hbox));

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
	uiBoxAppend(hbox, uiControl(vbox), 0);

	for (i = 0; i < 10; i++) {
		datapoints[i] = uiNewSpinbox(0, 100);
		uiBoxAppend(vbox, uiControl(datapoints[i]), 0);
	}

	histogram = uiNewArea(&handler);
	uiBoxAppend(hbox, uiControl(histogram), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}
