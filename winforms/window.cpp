// 25 november 2015
#include "uipriv_winforms.hpp"

// TODO save alignment of children?

ref class libuiForm : public Form {
public:
	uiWindow *w;
//	void onClosing(Object ^sender, CancelEventArgs ^e);
};

struct uiWindow {
	uiWindowsControl c;
	gcroot<libuiForm ^> *form;

	uiControl *child;
	int margined;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
//	gcroot<CancelEventHandler ^> *onClosingDelegate;
};

static void onDestroy(uiWindow *);

uiWindowsDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	window,								// handle
	onDestroy(hthis);						// on destroy
)

/*
void libuiWindow::onClosing(Object ^sender, CancelEventArgs ^e)
{
	// TODO copy comments
	if ((*(this->w->onClosing))(this->w, this->w->onClosingData))
		uiControlDestroy(uiControl(this->w));
	e->Cancel = true;
}
*/

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void onDestroy(uiWindow *w)
{
/*
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
*/
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
	return uiWindowsCLRStringToText((*(w->window))->Text);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	(*(w->window))->Text = fromUTF8(title);
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
//		(*(w->border))->Child = nullptr;
	}
	w->child = child;
	if (w->child != NULL) {
//		(*(w->border))->Child = genericHandle(w->child);
		uiControlSetParent(w->child, uiControl(w));
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
/*	// TODO margin or padding?
	if (w->margined)
		(*(w->border))->Margin = Thickness(10, 10, 10, 10);
	else
		(*(w->border))->Margin = Thickness(0, 0, 0, 0);
*/}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	w = (uiWindow *) uiNewControl(uiWindowType());

	w->form = new gcroot<libuiForm ^>();
	*(w->form) = gcnew libuiForm();
	(*(w->form))->w = w;

	(*(w->window))->Text = fromUTF8(title);
//	(*(w->window))->ClientSize = xxxxx(width, height);

	// TODO background color?

/*	w->border = new gcroot<Border ^>();
	*(w->border) = gcnew Border();
	(*(w->window))->Content = *(w->border);

	w->onClosingDelegate = new gcroot<CancelEventHandler ^>();
	*(w->onClosingDelegate) = gcnew CancelEventHandler(*(w->window),
		&libuiWindow::onClosing);
	(*(w->window))->Closing += *(w->onClosingDelegate);
*/	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiWindowsFinishNewControl(w, uiWindow, window);
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->CommitHide = windowCommitHide;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	return w;
}
