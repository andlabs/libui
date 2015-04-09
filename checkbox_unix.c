// 7 april 2015
#include "uipriv_unix.h"

struct checkbox {
	void (*onToggled)(uiControl *, void *);
	void *onToggledData;
};

static void onToggled(GtkToggleButton *b, gpointer data)
{
	uiControl *c = (uiControl *) data;
	struct checkbox *cc = (struct checkbox *) (c->data);

	(*(cc->onToggled))(c, cc->onToggledData);
}

static void defaultOnToggled(uiControl *c, void *data)
{
	// do nothing
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct checkbox *cc = (struct checkbox *) data;

	uiFree(cc);
}

uiControl *uiNewCheckbox(const char *text)
{
	uiControl *c;
	struct checkbox *cc;
	GtkWidget *widget;

	c = uiUnixNewControl(GTK_TYPE_CHECK_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c));
	g_signal_connect(widget, "toggled", G_CALLBACK(onToggled), c);

	cc = uiNew(struct checkbox);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), cc);
	cc->onToggled = defaultOnToggled;
	c->data = cc;

	return c;
}

char *uiCheckboxText(uiControl *c)
{
	return g_strdup(gtk_button_get_label(GTK_BUTTON(uiControlHandle(c))));
}

void uiCheckboxSetText(uiControl *c, const char *text)
{
	gtk_button_set_label(GTK_BUTTON(uiControlHandle(c)), text);
}

void uiCheckboxOnToggled(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct checkbox *cc = (struct checkbox *) (c->data);

	cc->onToggled = f;
	cc->onToggledData = data;
}

int uiCheckboxChecked(uiControl *c)
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(uiControlHandle(c))) != FALSE;
}

void uiCheckboxSetChecked(uiControl *c, int checked)
{
	gboolean active;

	active = FALSE;
	if (checked)
		active = TRUE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(uiControlHandle(c)), active);
}
