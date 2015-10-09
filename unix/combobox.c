// 11 june 2015
#include "uipriv_unix.h"

struct uiCombobox {
	uiUnixControl c;
	GtkWidget *widget;
	GtkComboBox *combobox;
	GtkComboBoxText *comboboxText;
	void (*onSelected)(uiCombobox *, void *);
	void *onSelectedData;
	gulong onSelectedSignal;
};

uiUnixDefineControl(
	uiCombobox,							// type name
	uiComboboxType						// type function
)

// TODO this is triggered when editing an editable combobox's text
static void onChanged(GtkComboBox *cbox, gpointer data)
{
	uiCombobox *c = uiCombobox(data);

	(*(c->onSelected))(c, c->onSelectedData);
}

static void defaultOnSelected(uiCombobox *c, void *data)
{
	// do nothing
}

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	gtk_combo_box_text_append(c->comboboxText, NULL, text);
}

intmax_t uiComboboxSelected(uiCombobox *c)
{
	return gtk_combo_box_get_active(c->combobox);
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	// we need to inhibit sending of ::changed because this WILL send a ::changed otherwise
	g_signal_handler_block(c->combobox, c->onSelectedSignal);
	gtk_combo_box_set_active(c->combobox, n);
	g_signal_handler_unblock(c->combobox, c->onSelectedSignal);
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	c->onSelected = f;
	c->onSelectedData = data;
}

static uiCombobox *finishNewCombobox(GtkWidget *(*newfunc)(void))
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->widget = (*newfunc)();
	c->combobox = GTK_COMBO_BOX(c->widget);
	c->comboboxText = GTK_COMBO_BOX_TEXT(c->widget);

	c->onSelectedSignal = g_signal_connect(c->widget, "changed", G_CALLBACK(onChanged), c);
	uiComboboxOnSelected(c, defaultOnSelected, NULL);

	uiUnixFinishNewControl(c, uiCombobox);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(gtk_combo_box_text_new);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(gtk_combo_box_text_new_with_entry);
}
