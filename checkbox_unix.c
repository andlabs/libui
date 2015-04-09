// 7 april 2015
#include "uipriv_unix.h"

struct checkbox {
	uiControl *c;
	void (*onToggled)(uiControl *, void *);
	void *onToggledData;
};

#define C(x) ((struct checkbox *) (x))

static void onToggled(GtkToggleButton *b, gpointer data)
{
	(*(C(data)->onToggled))(C(data)->c, C(data)->onToggledData);
}

static void defaultOnToggled(uiControl *c, void *data)
{
	// do nothing
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct checkbox *c = (struct checkbox *) data;

	uiFree(c);
}

uiControl *uiNewCheckbox(const char *text)
{
	struct checkbox *c;
	GtkWidget *widget;

	c = uiNew(struct checkbox);

	c->c = uiUnixNewControl(GTK_TYPE_CHECK_BUTTON,
		FALSE, FALSE, c,
		"label", text,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c->c));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), c);
	g_signal_connect(widget, "toggled", G_CALLBACK(onToggled), c);

	c->onToggled = defaultOnToggled;

	return c->c;
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
	struct checkbox *cc;

	cc = (struct checkbox *) uiUnixControlData(c);
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
