// 10 june 2015
#include "uipriv_unix.h"

struct checkbox {
	uiCheckbox c;
	GtkWidget *widget;
	GtkButton *button;
	GtkToggleButton *toggleButton;
	GtkCheckButton *checkButton;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

uiDefineControlType(uiCheckbox, uiTypeCheckbox, struct checkbox)

static uintptr_t checkboxHandle(uiControl *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return (uintptr_t) (c->widget);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static char *checkboxText(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return PUT_CODE_HERE;
}

static void checkboxSetText(uiCheckbox *cc, const char *text)
{
	struct checkbox *c = (struct checkbox *) cc;

	PUT_CODE_HERE;
	// changing the text might necessitate a change in the checkbox's size
	uiControlQueueResize(uiControl(c));
}

static void checkboxOnToggled(uiCheckbox *cc, void (*f)(uiCheckbox *, void *), void *data)
{
	struct checkbox *c = (struct checkbox *) cc;

	c->onToggled = f;
	c->onToggledData = data;
}

static int checkboxChecked(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return PUT_CODE_HERE;
}

static void checkboxSetChecked(uiCheckbox *cc, int checked)
{
	struct checkbox *c = (struct checkbox *) cc;

	PUT_CODE_HERE;
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;

	c = (struct checkbox *) uiNewControl(uiTypeCheckbox());

	c->widget = gtk_check_button_new_with_label(text);
	c->button = GTK_BUTTON(c->widget);
	c->toggleButton = GTK_TOGGLE_BUTTON(c->widget);
	c->checkButton = GTK_CHECK_BUTTON(c->widget);
	uiUnixMakeSingleWidgetControl(uiControl(c), c->widget);

	c->onToggled = defaultOnToggled;

	uiControl(c)->Handle = checkboxHandle;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
