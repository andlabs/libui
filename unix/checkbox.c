// 7 april 2015
#include "uipriv_unix.h"

struct checkbox {
	uiCheckbox c;
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

	uiFree(c);
}

#define CHECKBOX(c) GTK_CHECK_BUTTON(uiControlHandle(uiControl(c)))

static char *getText(uiCheckbox *c)
{
	return g_strdup(gtk_button_get_label(GTK_BUTTON(CHECKBOX(c))));
}

static void setText(uiCheckbox *c, const char *text)
{
	gtk_button_set_label(GTK_BUTTON(CHECKBOX(c)), text);
}

static void setOnToggled(uiCheckbox *cc, void (*f)(uiCheckbox *, void *), void *data)
{
	struct checkbox *c = (struct checkbox *) cc;

	c->onToggled = f;
	c->onToggledData = data;
}

static int getChecked(uiCheckbox *c)
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CHECKBOX(c))) != FALSE;
}

static void setChecked(uiCheckbox *cc, int checked)
{
	struct checkbox *c = (struct checkbox *) cc;
	GtkToggleButton *button;
	gboolean active;

	active = FALSE;
	if (checked)
		active = TRUE;
	// we need to inhibit sending of ::toggled because this WILL send a ::toggled otherwise
	button = GTK_TOGGLE_BUTTON(CHECKBOX(c));
	g_signal_handler_block(button, c->onToggledSignal);
	gtk_toggle_button_set_active(button, active);
	g_signal_handler_unblock(button, c->onToggledSignal);
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;
	GtkWidget *widget;

	c = uiNew(struct checkbox);

	uiUnixNewControl(uiControl(c), GTK_TYPE_CHECK_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = GTK_WIDGET(CHECKBOX(c));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), c);
	c->onToggledSignal = g_signal_connect(widget, "toggled", G_CALLBACK(onToggled), c);
	c->onToggled = defaultOnToggled;

	uiCheckbox(c)->Text = getText;
	uiCheckbox(c)->SetText = setText;
	uiCheckbox(c)->OnToggled = setOnToggled;
	uiCheckbox(c)->Checked = getChecked;
	uiCheckbox(c)->SetChecked = setChecked;

	return uiCheckbox(c);
}
