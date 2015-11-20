// 18 november 2015
#include "uipriv_haiku.hpp"

class libuiBWindow : public BWindow {
public:
	// C++11! Inherit constructors.
	using BWindow::BWindow;

	uiWindow *w;
	virtual bool QuitRequested();
};

struct uiWindow {
	uiHaikuControl c;

	libuiBWindow *window;

	BGroupLayout *vbox;

	struct singleChild *child;
	int margined;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

uiHaikuDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	window,								// handle
	complain("attempt to use default CommitDestroy() code for uiWindow on Haiku");			// on destroy
)

bool libuiBWindow::QuitRequested()
{
	// manually destroy the window ourselves; don't let the default BWindow code do it directly
	if ((*(this->w->onClosing))(this->w, this->w->onClosingData))
		uiControlDestroy(uiControl(this->w));
	// don't continue to the default BWindow code; we destroyed the window by now
	return false;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void windowCommitDestroy(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// first hide ourselves
	w->window->Hide();
	// now destroy the child
	if (w->child != NULL)
		singleChildDestroy(w->child);
	// and finally destroy ourselves
	// this is why we don't use the libui-provided CommitDestroy() implementation
	// TODO check this for errors? or use B_QUIT_REQUESTED?
	w->window->Lock();
	w->window->Quit();
	// w->window is now destroyed for us
}

// The default implementations assume a BView, which a uiWindow is not.
static void windowCommitShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// This is documented as behaving how we want with regards to presentation.
	w->window->Show();
}

static void windowCommitHide(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	w->window->Hide();
}

static void windowContainerUpdateState(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		singleChildUpdateState(w->child);
}

char *uiWindowTitle(uiWindow *w)
{
	return uiHaikuStrdupText(w->window->Title());
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	w->window->SetTitle(title);
	// don't queue resize; the caption isn't part of what affects layout and sizing of the client area (it'll be ellipsized if too long)
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

// see singlechild.cpp
static void attach(void *attachTo, BLayoutItem *what)
{
	BGroupLayout *vbox = (BGroupLayout *) attachTo;

	vbox->AddItem(what);
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (w->child != NULL) {
		w->vbox->RemoveItem(singleChildLayoutItem(w->child));
		singleChildRemove(w->child);
	}
	w->child = newSingleChild(child, uiControl(w), attach, w->vbox);
	if (w->child != NULL)
		uiWindowSetMargined(w, w->margined);
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	if (w->child != NULL)
		if (w->margined)
			singleChildSetMargined(w->child, B_USE_WINDOW_SPACING);
		else
			singleChildSetMargined(w->child, 0);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	w = (uiWindow *) uiNewControl(uiWindowType());

	// TODO find out how to make it ignore position
	// The given rect is the size of the inside of the window, just as we want.
	w->window = new libuiBWindow(
		BRect(100, 100, width, height),
		title,
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS);
	w->window->w = w;

	w->vbox = new BGroupLayout(B_VERTICAL, 0);
	w->window->SetLayout(w->vbox);
	// Haiku itself does this, with a TODO
	w->vbox->Owner()->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiHaikuFinishNewControl(w, uiWindow);
	uiControl(w)->CommitDestroy = windowCommitDestroy;
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->CommitHide = windowCommitHide;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	return w;
}
