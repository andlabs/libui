// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiButton {
	uiHaikuControl c;
	BButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

uiHaikuDefineControl(
	uiButton,								// type name
	uiButtonType,							// type function
	button								// handle
)

#define mButtonClicked 0x4E754E75

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

char *uiButtonText(uiButton *b)
{
	return uiHaikuStrdupText(b->button->Label());
}

void uiButtonSetText(uiButton *b, const char *text)
{
	b->button->SetLabel(text);
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *b, void *data), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;

	b = (uiButton *) uiNewControl(uiButtonType());

	b->button = new BButton(text, new BMessage(mButtonClicked));

	// TODO hook up events
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiHaikuFinishNewControl(b, uiButton);

	return b;
}
