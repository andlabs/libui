// 11 june 2015
#include "uipriv_unix.h"

struct uiProgressBar {
	uiUnixControl c;
	GtkWidget *widget;
	GtkProgressBar *pbar;
};

uiUnixControlAllDefaults(uiProgressBar)

int uiProgressBarValue(uiProgressBar *p)
{
	return (int) (gtk_progress_bar_get_fraction(p->pbar) * 100);
}

void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (value < 0 || value > 100)
		userbug("Value %d is out of range for a uiProgressBar.", value);
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
