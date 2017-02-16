// 17 january 2017
#include "drawtext.h"

// okay everything is definitely bugged in the OS X code
// - occasional segfaults on startup
// - very rare size attributes in the attributed string example don't terminate for a while, making everything big
// - very very rare trace/bpt faults on startup
/*
objc[14827]: autorelease pool page 0x7feeab88b000 corrupted
  magic     0xe000007f 0xeea9f2df 0x0000007f 0xeea9f2e0
  should be 0xa1a1a1a1 0x4f545541 0x454c4552 0x21455341
  pthread   0x0
  should be 0x7fff727a1000
*/

static uiWindow *mainwin;
static uiBox *box;
static uiCombobox *exampleList;
static uiArea *area;
static uiAreaHandler handler;

#define nExamples 20
static struct example *examples[nExamples];
static int curExample = 0;

static void onExampleChanged(uiCombobox *c, void *data)
{
	uiControlHide(examples[curExample]->panel);
	curExample = uiComboboxSelected(exampleList);
	uiControlShow(examples[curExample]->panel);
	redraw();
}

void redraw(void)
{
	uiAreaQueueRedrawAll(area);
}

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	examples[curExample]->draw(p);
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
	if (examples[curExample]->mouse != NULL)
		examples[curExample]->mouse(e);
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
	if (examples[curExample]->key != NULL)
		return examples[curExample]->key(e);
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
	int n;

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

	mainwin = uiNewWindow("libui Text-Drawing Example", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	box = uiNewVerticalBox();
	uiWindowSetChild(mainwin, uiControl(box));

	exampleList = uiNewCombobox();
	uiBoxAppend(box, uiControl(exampleList), 0);

	area = uiNewArea(&handler);
	uiBoxAppend(box, uiControl(area), 1);

	n = 0;
	examples[n] = mkBasicExample();
	uiComboboxAppend(exampleList, examples[n]->name);
	uiControlHide(examples[n]->panel);
	uiBoxAppend(box, examples[n]->panel, 0);
	n++;
	examples[n] = mkHitTestExample();
	uiComboboxAppend(exampleList, examples[n]->name);
	uiControlHide(examples[n]->panel);
	uiBoxAppend(box, examples[n]->panel, 0);
	n++;
	examples[n] = mkAttributesExample();
	uiComboboxAppend(exampleList, examples[n]->name);
	uiControlHide(examples[n]->panel);
	uiBoxAppend(box, examples[n]->panel, 0);
	n++;
	// and set things up for the initial state
	uiComboboxSetSelected(exampleList, 0);
	uiComboboxOnSelected(exampleList, onExampleChanged, NULL);
	// and set up the first one
	onExampleChanged(NULL, NULL);

	uiControlShow(uiControl(mainwin));
	uiMain();

	// TODO free examples

	uiUninit();
	return 0;
}
