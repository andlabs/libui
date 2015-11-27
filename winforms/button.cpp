// 25 november 2015
#include "uipriv_winforms.hpp"

struct uiButton {
	uiWindowsControl c;
	gcroot<Button ^> *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

uiWindowsDefineControl(
	uiButton,								// type name
	uiButtonType,							// type function
	button								// handle
)

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

char *uiButtonText(uiButton *b)
{
	String ^text;

	// TOOD bad cast?
	text = (String ^) ((*(b->button))->Content);
	return uiWindowsCLRStringToText(text);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	(*(b->button))->Content = fromUTF8(text);
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

	b->button = new gcroot<Button ^>();
	*(b->button) = gcnew Button();
	(*(b->button))->Content = fromUTF8(text);

	// TODO hook up events
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiWindowsFinishNewControl(b, uiButton, button);

	return b;
}
