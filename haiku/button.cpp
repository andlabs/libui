// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiButton {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiButton,								// type name
	uiButtonType,							// type function
	dummy								// handle
)

char *uiButtonText(uiButton *b)
{
	// TODO
	return NULL;
}

void uiButtonSetText(uiButton *b, const char *text)
{
	// TODO
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *b, void *data), void *data)
{
	// TODO
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;

	b = (uiButton *) uiNewControl(uiButtonType());

	b->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiButton not implemented");

	uiHaikuFinishNewControl(b, uiButton);

	return b;
}
