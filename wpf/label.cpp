// 26 november 2015
#include "uipriv_wpf.hpp"

// TODO alignment
// TODO lots of padding
// TODO Label also has mnemonic value; use TextField instead

struct uiLabel {
	uiWindowsControl c;
	gcroot<Label ^> *label;
};

uiWindowsDefineControl(
	uiLabel,								// type name
	uiLabelType,							// type function
	label									// handle
)

char *uiLabelText(uiLabel *l)
{
	String ^text;

	// TOOD bad cast?
	text = (String ^) ((*(l->label))->Content);
	return uiWindowsCLRStringToText(text);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	(*(l->label))->Content = fromUTF8(text);
	// TODO does this adjust the layout?
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	l = (uiLabel *) uiNewControl(uiLabelType());

	l->label = new gcroot<Label ^>();
	*(l->label) = gcnew Label();
	(*(l->label))->Content = fromUTF8(text);

	uiWindowsFinishNewControl(l, uiLabel, label);

	return l;
}
