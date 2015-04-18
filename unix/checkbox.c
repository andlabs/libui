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

static void onToggled(GtkToggleButton *b, gpointer data)
{
	struct checkbox *c = (struct checkbox *) data;

	(*(c->onToggled))(uiCheckbox(c), c->onToggledData);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct checkbox *c = (struct checkbox *) data;

	if (options.debugLogLifetimes)
		fprintf(stderr, "GtkWidget::destroy checkbox %p\n", c);
	uiFree(c);
}

static char *checkboxText(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return g_strdup(gtk_button_get_label(c->button));
}

static void checkboxSetText(uiCheckbox *cc, const char *text)
{
	struct checkbox *c = (struct checkbox *) cc;

	gtk_button_set_label(GTK_BUTTON(c->button), text);
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

	return gtk_toggle_button_get_active(c->toggleButton) != FALSE;
}

static void checkboxSetChecked(uiCheckbox *cc, int checked)
{
	struct checkbox *c = (struct checkbox *) cc;
	gboolean active;

	active = FALSE;
	if (checked)
		active = TRUE;
	// we need to inhibit sending of ::toggled because this WILL send a ::toggled otherwise
	g_signal_handler_block(c->toggleButton, c->onToggledSignal);
	gtk_toggle_button_set_active(c->toggleButton, active);
	g_signal_handler_unblock(c->toggleButton, c->onToggledSignal);
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;

	c = uiNew(struct checkbox);
	if (options.debugLogLifetimes)
		fprintf(stderr, "uiNewCheckbox() %p %s\n", c, text);

	uiUnixNewControl(uiControl(c), GTK_TYPE_CHECK_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	c->widget = WIDGET(c);
	c->button = GTK_BUTTON(c->widget);
	c->toggleButton = GTK_TOGGLE_BUTTON(c->widget);
	c->checkButton = GTK_CHECK_BUTTON(c->widget);

	g_signal_connect(c->widget, "destroy", G_CALLBACK(onDestroy), c);

	c->onToggledSignal = g_signal_connect(c->widget, "toggled", G_CALLBACK(onToggled), c);
	c->onToggled = defaultOnToggled;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
