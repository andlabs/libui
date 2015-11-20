// 18 november 2015
#include "uipriv_haiku.hpp"

// TODO forcibly aligned to the bottom of the allocation

struct uiLabel {
	uiHaikuControl c;
	BStringView *label;
};

uiHaikuDefineControl(
	uiLabel,								// type name
	uiLabelType,							// type function
	label									// handle
)

char *uiLabelText(uiLabel *l)
{
	return uiHaikuStrdupText(l->label->Text());
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	l->label->SetText(text);
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	l = (uiLabel *) uiNewControl(uiLabelType());

	l->label = new BStringView(NULL, text);

	uiHaikuFinishNewControl(l, uiLabel);

	return l;
}
