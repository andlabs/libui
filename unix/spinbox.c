// 11 june 2015
#include "uipriv_unix.h"

struct uiSpinbox {
	uiUnixControl c;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkSpinButton *spinButton;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
	gulong onChangedSignal;
};

uiUnixDefineControl(
	uiSpinbox							// type name
)

static void onChanged(GtkSpinButton *sb, gpointer data)
{
	uiSpinbox *s = uiSpinbox(data);

	(*(s->onChanged))(s, s->onChangedData);
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	return (intmax_t) gtk_spin_button_get_value(s->spinButton);
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->spinButton, s->onChangedSignal);
	// TODO does this clamp?
	gtk_spin_button_set_value(s->spinButton, (gdouble) value);
	g_signal_handler_unblock(s->spinButton, s->onChangedSignal);
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	uiSpinbox *s;

	if (min >= max)
		complain("error: min >= max in uiNewSpinbox()");

	s = (uiSpinbox *) uiNewControl(uiSpinboxType());

	s->widget = gtk_spin_button_new_with_range(min, max, 1);
	s->entry = GTK_ENTRY(s->widget);
	s->spinButton = GTK_SPIN_BUTTON(s->widget);

	// TODO needed?
	gtk_spin_button_set_digits(s->spinButton, 0);

	s->onChangedSignal = g_signal_connect(s->spinButton, "value-changed", G_CALLBACK(onChanged), s);
	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	uiUnixFinishNewControl(s, uiSpinbox);

	return s;
}
