// 11 june 2015
#include "uipriv_unix.h"

struct uiProgressBar {
	uiUnixControl c;
	GtkWidget *widget;
	GtkProgressBar *pbar;
	int indeterminate;
};

uiUnixControlAllDefaults(uiProgressBar)

int uiProgressBarValue(uiProgressBar *p)
{
	if (p->indeterminate)
		return -1;

	return (int) (gtk_progress_bar_get_fraction(p->pbar) * 100);
}

gboolean uiProgressBarPulse(void* data)
{
	uiProgressBar *p = (uiProgressBar*) data;

	if (!GTK_IS_WIDGET(p->pbar) || !p->indeterminate)
		return 0;

	gtk_progress_bar_pulse(p->pbar);
	return 1;
}

void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (value == -1) {
		if (!p->indeterminate) {
			p->indeterminate = 1;
			g_timeout_add(100, uiProgressBarPulse, p);
		}
		return;
	}

	if (value < 0 || value > 100)
		userbug("Value %d is out of range for a uiProgressBar.", value);

	p->indeterminate = 0;
	gtk_progress_bar_set_fraction(p->pbar, ((gdouble) value) / 100);
}

uiProgressBar *uiNewProgressBar(void)
{
	uiProgressBar *p;

	uiUnixNewControl(uiProgressBar, p);

	p->widget = gtk_progress_bar_new();
	p->pbar = GTK_PROGRESS_BAR(p->widget);

	return p;
}
