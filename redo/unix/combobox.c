// 11 june 2015
#include "uipriv_unix.h"

struct combobox {
	uiCombobox c;
	GtkWidget *widget;
	GtkComboBox *combobox;
	GtkComboBoxText *comboboxText;
};

uiDefineControlType(uiCombobox, uiTypeCombobox, struct combobox)

static uintptr_t comboboxHandle(uiControl *cc)
{
	struct combobox *c = (struct combobox *) cc;

	return (uintptr_t) (c->widget);
}

static void comboboxAppend(uiCombobox *cc, const char *text)
{
	struct combobox *c = (struct combobox *) cc;

	gtk_combo_box_text_append(c->comboboxText, NULL, text);
}

static uiCombobox *finishNewCombobox(GtkWidget *(*newfunc)(void))
{
	struct combobox *c;

	c = (struct combobox *) uiNewControl(uiTypeCombobox());

	c->widget = (*newfunc)();
	c->combobox = GTK_COMBO_BOX(c->widget);
	c->comboboxText = GTK_COMBO_BOX_TEXT(c->widget);
	uiUnixMakeSingleWidgetControl(uiControl(c), c->widget);

	uiControl(c)->Handle = comboboxHandle;

	uiCombobox(c)->Append = comboboxAppend;

	return uiCombobox(c);
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(gtk_combo_box_text_new);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(gtk_combo_box_text_new_with_entry);
}
