// 25 november 2015
#include "uipriv_wpf.hpp"

// TODO save alignment of children?

ref class libuiWindow : public Window {
public:
	uiWindow *w;
	void onClosing(Object ^sender, CancelEventArgs ^e);
};

struct uiWindow {
	uiWindowsControl c;
	gcroot<libuiWindow ^> *window;

	gcroot<Border ^> *border;
	int margined;

	uiControl *child;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	gcroot<CancelEventHandler ^> *onClosingDelegate;
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
		uiControlDestroy(uiControl(this->w));
	e->Cancel = true;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void onDestroy(uiWindow *w)
{
	// first hide the window
	(*(w->window))->Hide();
	// take off the closing event; otherwise it will be recursed
	(*(w->window))->Closing -= *(w->onClosingDelegate);
	delete w->onClosingDelegate;
	// then destroy the child
	if (w->child != NULL) {
		(*(w->border))->Child = nullptr;
		uiControlSetParent(w->child, NULL);
		uiControlDestroy(w->child);
	}
	// clean up remaining .net objects
	delete w->border;
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
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		controlUpdateState(w->child);
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
	if (w->child != NULL) {
		uiControlSetParent(w->child, NULL);
		(*(w->border))->Child = nullptr;
	}
	w->child = child;
	if (w->child != NULL) {
		(*(w->border))->Child = genericHandle(w->child);
		uiControlSetParent(w->child, uiControl(w));
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

// TODO according to http://stackoverflow.com/questions/18299107/how-to-handle-control-spacing-in-wpf-window-designer - same for rest of library
void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	// TODO Margin or Padding?
	if (w->margined)
		(*(w->border))->Margin = Thickness(10, 10, 10, 10);
	else
		(*(w->border))->Margin = Thickness(0, 0, 0, 0);
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

	// reference source indicates that this does indeed map to COLOR_BTNFACE
	(*(w->window))->Background = SystemColors::ControlBrush;

	w->border = new gcroot<Border ^>();
	*(w->border) = gcnew Border();
	(*(w->window))->Content = *(w->border);

	w->onClosingDelegate = new gcroot<CancelEventHandler ^>();
	*(w->onClosingDelegate) = gcnew CancelEventHandler(*(w->window),
		&libuiWindow::onClosing);
	(*(w->window))->Closing += *(w->onClosingDelegate);
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiWindowsFinishNewControl(w, uiWindow, window);
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->CommitHide = windowCommitHide;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	return w;
}
