// 11 june 2015
#include "uipriv_unix.h"

struct uiCombobox {
	uiUnixControl c;
	GtkWidget *widget;
	GtkComboBox *combobox;
	GtkComboBoxText *comboboxText;
};

uiUnixDefineControl(
	uiCombobox,							// type name
	uiComboboxType						// type function
)

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	gtk_combo_box_text_append(c->comboboxText, NULL, text);
}

static uiCombobox *finishNewCombobox(GtkWidget *(*newfunc)(void))
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->widget = (*newfunc)();
	c->combobox = GTK_COMBO_BOX(c->widget);
	c->comboboxText = GTK_COMBO_BOX_TEXT(c->widget);

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
