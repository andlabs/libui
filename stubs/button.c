// 10 june 2015
#include "uipriv_OSHERE.h"

struct button {
	uiButton b;
	OSTYPE OSHANDLE;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

uiDefineControlType(uiButton, uiTypeButton, struct button)

static uintptr_t buttonHandle(uiControl *c)
{
	struct button *b = (struct button *) c;

	return (uintptr_t) (b->OSHANDLE);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static char *buttonText(uiButton *bb)
{
	struct button *b = (struct button *) bb;

	return PUT_CODE_HERE;
}

static void buttonSetText(uiButton *bb, const char *text)
{
	struct button *b = (struct button *) bb;

	PUT_CODE_HERE;
	// changing the text might necessitate a change in the button's size
	uiControlQueueResize(uiControl(b));
}

static void buttonOnClicked(uiButton *bb, void (*f)(uiButton *, void *), void *data)
{
	struct button *b = (struct button *) bb;

	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	struct button *b;

	b = (struct button *) MAKE_CONTROL_INSTANCE(uiTypeButton());

	PUT_CODE_HERE;

	b->onClicked = defaultOnClicked;

	uiControl(b)->Handle = buttonHandle;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
