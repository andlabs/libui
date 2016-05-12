// 6 december 2015
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../ui.h"
using namespace std;

uiMultilineEntry *e;
condition_variable cv;
mutex m;
unique_lock<mutex> ourlock(m);
thread *timeThread;

void sayTime(void *data)
{
	char *s = (char *) data;

	uiMultilineEntryAppend(e, s);
	delete s;
}

void threadproc(void)
{
	ourlock.lock();
	while (cv.wait_for(ourlock, chrono::seconds(1)) == cv_status::timeout) {
		time_t t;
		char *base;
		char *s;

		t = time(NULL);
		base = ctime(&t);
		s = new char[strlen(base) + 1];
		strcpy(s, base);
		uiQueueMain(sayTime, s);
	}
}

int onClosing(uiWindow *w, void *data)
{
	cv.notify_all();
	// C++ throws a hissy fit if you don't do this
	// we might as well, to ensure no uiQueueMain() gets in after uiQuit()
	timeThread->join();
	uiQuit();
	return 1;
}

void saySomething(uiButton *b, void *data)
{
	uiMultilineEntryAppend(e, "Saying something\n");
}

int main(void)
{
	uiInitOptions o;
	uiWindow *w;
	uiBox *b;
	uiButton *btn;

	memset(&o, 0, sizeof (uiInitOptions));
	if (uiInit(&o) != NULL)
		abort();

	w = uiNewWindow("Hello", 320, 240, 0);
	uiWindowSetMargined(w, 1);

	b = uiNewVerticalBox();
	uiBoxSetPadded(b, 1);
	uiWindowSetChild(w, uiControl(b));

	e = uiNewMultilineEntry();
	uiMultilineEntrySetReadOnly(e, 1);

	btn = uiNewButton("Say Something");
	uiButtonOnClicked(btn, saySomething, NULL);
	uiBoxAppend(b, uiControl(btn), 0);

	uiBoxAppend(b, uiControl(e), 1);

	// timeThread needs to lock ourlock itself - see http://stackoverflow.com/a/34121629/3408572
	ourlock.unlock();
	timeThread = new thread(threadproc);

	uiWindowOnClosing(w, onClosing, NULL);
	uiControlShow(uiControl(w));
	uiMain();
	return 0;
}
