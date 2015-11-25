// 25 november 2015
#include "uipriv_wpf.hpp"

ref class libuiWindow : public Window {
public:
	uiWindow *w;
	void onClosing(Object ^sender, CancelEventArgs ^e);
};

struct uiWindow {
	uiWindowsControl c;
	gcroot<libuiWindow ^> *window;

	int margined;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

static void onDestroy(uiWindow *);

uiWindowsDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	window,								// handle
	onDestroy(hthis);						// on destroy
)

void libuiWindow::onClosing(Object ^sender, CancelEventArgs ^e)
{
	// TODO copy comments
	if ((*(this->w->onClosing))(this->w, this->w->onClosingData))
		// TODO this triggers another onClosing() when it's too late
		uiControlDestroy(uiControl(this->w));
	e->Cancel = true;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void onDestroy(uiWindow *w)
{
	// TODO
}

static void windowCommitShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// TODO does this behave properly in re presentation?
	(*(w->window))->Show();
}

static void windowCommitHide(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	(*(w->window))->Hide();
}

static void windowContainerUpdateState(uiControl *c)
{
/*TODO
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		childUpdateState(w->child);
*/
}

char *uiWindowTitle(uiWindow *w)
{
	return uiWindowsCLRStringToText((*(w->window))->Title);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	(*(w->window))->Title = fromUTF8(title);
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
/*TODO
	if (w->child != NULL)
		childRemove(w->child);
	w->child = newChild(child, uiControl(w), w->hwnd);
	if (w->child != NULL) {
		childSetSoleControlID(w->child);
		childQueueRelayout(w->child);
	}
*/
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
//TODO	uiWindowsControlQueueRelayout(uiWindowsControl(w));
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	w = (uiWindow *) uiNewControl(uiWindowType());

	w->window = new gcroot<libuiWindow ^>();
	*(w->window) = gcnew libuiWindow();
	(*(w->window))->w = w;

	(*(w->window))->Title = fromUTF8(title);
	// TODO is this the client size?
	(*(w->window))->Width = width;
	(*(w->window))->Height = height;

	// TODO background color

	(*(w->window))->Closing += gcnew CancelEventHandler(*(w->window),
		&libuiWindow::onClosing);
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiWindowsFinishNewControl(w, uiWindow, window);
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->CommitHide = windowCommitHide;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	return w;
}
