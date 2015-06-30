// 7 april 2015
#include "uipriv_unix.h"

struct checkbox {
	uiCheckbox c;
	GtkWidget *widget;
	GtkButton *button;
	GtkToggleButton *toggleButton;
	GtkCheckButton *checkButton;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
	gulong onToggledSignal;
};

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}




uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;

	c = uiNew(struct checkbox);

	c->onToggled = defaultOnToggled;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
