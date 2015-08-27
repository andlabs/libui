// 11 june 2015
#include "uipriv_unix.h"

struct spinbox {
	uiSpinbox s;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkSpinButton *spinButton;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	gulong onChangedSignal;
};

uiDefineControlType(uiSpinbox, uiTypeSpinbox, struct spinbox)

static void onChanged(GtkSpinButton *sb, gpointer data)
{
	struct spinbox *s = (struct spinbox *) data;

	(*(s->onChanged))(uiSpinbox(s), s->onChangedData);
}

static uintptr_t spinboxHandle(uiControl *c)
{
	struct spinbox *s = (struct spinbox *) c;

	return (uintptr_t) (s->widget);
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

static intmax_t spinboxValue(uiSpinbox *ss)
{
	struct spinbox *s = (struct spinbox *) ss;

	return (intmax_t) gtk_spin_button_get_value(s->spinButton);
}

static void spinboxSetValue(uiSpinbox *ss, intmax_t value)
{
	struct spinbox *s = (struct spinbox *) ss;

	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->spinButton, s->onChangedSignal);
	// TODO does this clamp?
	gtk_spin_button_set_value(s->spinButton, (gdouble) value);
	g_signal_handler_unblock(s->spinButton, s->onChangedSignal);
}

static void spinboxOnChanged(uiSpinbox *ss, void (*f)(uiSpinbox *, void *), void *data)
{
	struct spinbox *s = (struct spinbox *) ss;

	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	struct spinbox *s;

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (struct spinbox *) uiNewControl(uiTypeSpinbox());

	s->widget = gtk_spin_button_new_with_range(min, max, 1);
	s->entry = GTK_ENTRY(s->widget);
	s->spinButton = GTK_SPIN_BUTTON(s->widget);
	uiUnixMakeSingleWidgetControl(uiControl(s), s->widget);

	// TODO needed?
	gtk_spin_button_set_digits(s->spinButton, 0);

	s->onChangedSignal = g_signal_connect(s->spinButton, "value-changed", G_CALLBACK(onChanged), s);
	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = spinboxHandle;

	uiSpinbox(s)->Value = spinboxValue;
	uiSpinbox(s)->SetValue = spinboxSetValue;
	uiSpinbox(s)->OnChanged = spinboxOnChanged;

	return uiSpinbox(s);
}
