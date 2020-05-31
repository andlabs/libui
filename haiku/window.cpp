// 31 may 2020
#include "uipriv_haiku.hpp"

struct windowImplData {
	BWindow *window;
	char *title;
};

static bool windowInit(uiControl *c, void *implData, void *initData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	// TODO figure out what the correct BRect is
	wi->window = new BWindow(BRect(0, 0, 0, 0), "",
		B_TITLED_WINDOW,
		// TODO B_AUTO_UPDATE_SIZE_LIMITS?
		B_ASYNCHRONOUS_CONTROLS);

	return true;
}

static void windowFree(uiControl *c, void *implData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	if (wi->title != NULL) {
		uiprivFreeUTF8(wi->title);
		wi->title = NULL;
	}
	delete wi->window;
}

static void windowParentChanging(uiControl *c, void *implData, uiControl *oldParent)
{
	uiprivProgrammerErrorCannotHaveWindowsAsChildren();
}

static void windowParentChanged(uiControl *c, void *implData, uiControl *newParent)
{
	uiprivProgrammerErrorCannotHaveWindowsAsChildren();
}

static void *windowHandle(uiControl *c, void *implData)
{
	struct windowImplData *wi = (struct windowImplData *) implData;

	return wi->window;
}

// gotta do this because of lack of C99-style initializers in C++11
static void windowVtable(uiControlVtable *vt)
{
	vt->Size = sizeof (uiControlVtable);
	vt->Init = windowInit;
	vt->Free = windowFree;
	vt->ParentChanging = windowParentChanging;
	vt->ParentChanged = windowParentChanged;
}

static void windowOSVtable(uiControlOSVtable *osvt)
{
	osvt->Size = sizeof (uiControlOSVtable);
	osvt->Handle = windowHandle;
}

static uint32_t windowType = 0;

uint32_t uiprivSysWindowType(void)
{
	if (windowType == 0) {
		uiControlVtable vt;
		uiControlOSVtable osvt;

		windowVtable(&vt);
		windowOSVtable(&osvt);
		windowType = uiRegisterControlType("uiWindow", &vt, &osvt, sizeof (struct windowImplData));
	}
	return windowType;
}

uiWindow *uiprivSysNewWindow(void)
{
	return (uiWindow *) uiNewControl(uiWindowType(), NULL);
}

const char *uiprivSysWindowTitle(uiWindow *w)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	if (wi->title == NULL)
		// TODO replace this with a dedicated UTF-8 empty string object
		return "";
	return wi->title;
}

void uiprivSysWindowSetTitle(uiWindow *w, const char *title)
{
	struct windowImplData *wi = (struct windowImplData *) uiControlImplData(uiControl(w));

	if (wi->title != NULL)
		uiprivFreeUTF8(wi->title);
	wi->title = uiprivSanitizeUTF8(title);
	wi->window->SetTitle(wi->title);
}
