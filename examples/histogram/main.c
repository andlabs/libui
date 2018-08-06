// 13 october 2015
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../ui.h"

uiWindow *mainwin;
uiArea *histogram;
uiAreaHandler handler;
uiSpinbox *datapoints[10];
uiColorButton *colorButton;
int currentPoint = -1;

// some metrics
#define xoffLeft 20			/* histogram margins */
#define yoffTop 20
#define xoffRight 20
#define yoffBottom 20
#define pointRadius 5

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

static void pointLocations(double width, double height, double *xs, double *ys)
{
	double xincr, yincr;
	int i, n;

	xincr = width / 9;		// 10 - 1 to make the last point be at the end
	yincr = height / 100;

	for (i = 0; i < 10; i++) {
		// get the value of the point
		n = uiSpinboxValue(datapoints[i]);
		// because y=0 is the top but n=0 is the bottom, we need to flip
		n = 100 - n;
		xs[i] = xincr * i;
		ys[i] = yincr * n;
	}
}

static uiDrawPath *constructGraph(double width, double height, int extend)
{
	uiDrawPath *path;
	double xs[10], ys[10];
	int i;

	pointLocations(width, height, xs, ys);

	path = uiDrawNewPath(uiDrawFillModeWinding);

	uiDrawPathNewFigure(path, xs[0], ys[0]);
	for (i = 1; i < 10; i++)
		uiDrawPathLineTo(path, xs[i], ys[i]);

	if (extend) {
		uiDrawPathLineTo(path, width, height);
		uiDrawPathLineTo(path, 0, height);
		uiDrawPathCloseFigure(path);
	}

	uiDrawPathEnd(path);
	return path;
}

static void graphSize(double clientWidth, double clientHeight, double *graphWidth, double *graphHeight)
{
	*graphWidth = clientWidth - xoffLeft - xoffRight;
	*graphHeight = clientHeight - yoffTop - yoffBottom;
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawBrush brush;
	uiDrawStrokeParams sp;
	uiDrawMatrix m;
	double graphWidth, graphHeight;
	double graphR, graphG, graphB, graphA;

	// fill the area with white
	setSolidBrush(&brush, colorWhite, 1.0);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 0, 0, p->AreaWidth, p->AreaHeight);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);

	// figure out dimensions
	graphSize(p->AreaWidth, p->AreaHeight, &graphWidth, &graphHeight);

	// clear sp to avoid passing garbage to uiDrawStroke()
	// for example, we don't use dashing
	memset(&sp, 0, sizeof (uiDrawStrokeParams));

	// make a stroke for both the axes and the histogram line
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 2;
	sp.MiterLimit = uiDrawDefaultMiterLimit;

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
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	// now transform the coordinate space so (0, 0) is the top-left corner of the graph
	uiDrawMatrixSetIdentity(&m);
	uiDrawMatrixTranslate(&m, xoffLeft, yoffTop);
	uiDrawTransform(p->Context, &m);

	// now get the color for the graph itself and set up the brush
	uiColorButtonColor(colorButton, &graphR, &graphG, &graphB, &graphA);
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = graphR;
	brush.G = graphG;
	brush.B = graphB;
	// we set brush->A below to different values for the fill and stroke

	// now create the fill for the graph below the graph line
	path = constructGraph(graphWidth, graphHeight, 1);
	brush.A = graphA / 2;
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);

	// now draw the histogram line
	path = constructGraph(graphWidth, graphHeight, 0);
	brush.A = graphA;
	uiDrawStroke(p->Context, path, &brush, &sp);
	uiDrawFreePath(path);

	// now draw the point being hovered over
	if (currentPoint != -1) {
		double xs[10], ys[10];

		pointLocations(graphWidth, graphHeight, xs, ys);
		path = uiDrawNewPath(uiDrawFillModeWinding);
		uiDrawPathNewFigureWithArc(path,
			xs[currentPoint], ys[currentPoint],
			pointRadius,
			0, 6.23,		// TODO pi
			0);
		uiDrawPathEnd(path);
		// use the same brush as for the histogram lines
		uiDrawFill(p->Context, path, &brush);
		uiDrawFreePath(path);
	}
}

static int inPoint(double x, double y, double xtest, double ytest)
{
	// TODO switch to using a matrix
	x -= xoffLeft;
	y -= yoffTop;
	return (x >= xtest - pointRadius) &&
		(x <= xtest + pointRadius) &&
		(y >= ytest - pointRadius) &&
		(y <= ytest + pointRadius);
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	double graphWidth, graphHeight;
	double xs[10], ys[10];
	int i;

	graphSize(e->AreaWidth, e->AreaHeight, &graphWidth, &graphHeight);
	pointLocations(graphWidth, graphHeight, xs, ys);

	for (i = 0; i < 10; i++)
		if (inPoint(e->X, e->Y, xs[i], ys[i]))
			break;
	if (i == 10)		// not in a point
		i = -1;

	currentPoint = i;
	// TODO only redraw the relevant area
	uiAreaQueueRedrawAll(histogram);
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
	// reject all keys
	return 0;
}

static void onDatapointChanged(uiSpinbox *s, void *data)
{
	uiAreaQueueRedrawAll(histogram);
}

static void onColorChanged(uiColorButton *b, void *data)
{
	uiAreaQueueRedrawAll(histogram);
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
	uiDrawBrush brush;

	handler.Draw = handlerDraw;
	handler.MouseEvent = handlerMouseEvent;
	handler.MouseCrossed = handlerMouseCrossed;
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

	srand(time(NULL));
	for (i = 0; i < 10; i++) {
		datapoints[i] = uiNewSpinbox(0, 100);
		uiSpinboxSetValue(datapoints[i], rand() % 101);
		uiSpinboxOnChanged(datapoints[i], onDatapointChanged, NULL);
		uiBoxAppend(vbox, uiControl(datapoints[i]), 0);
	}

	colorButton = uiNewColorButton();
	// TODO inline these
	setSolidBrush(&brush, colorDodgerBlue, 1.0);
	uiColorButtonSetColor(colorButton,
		brush.R,
		brush.G,
		brush.B,
		brush.A);
	uiColorButtonOnChanged(colorButton, onColorChanged, NULL);
	uiBoxAppend(vbox, uiControl(colorButton), 0);

	histogram = uiNewArea(&handler);
	uiBoxAppend(hbox, uiControl(histogram), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}
