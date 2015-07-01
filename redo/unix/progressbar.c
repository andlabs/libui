// 11 june 2015
#include "uipriv_unix.h"

struct progressbar {
	uiProgressBar p;
	GtkWidget *widget;
	GtkProgressBar *pbar;
};

uiDefineControlType(uiProgressBar, uiTypeProgressBar, struct progressbar)

static uintptr_t progressbarHandle(uiControl *c)
{
	struct progressbar *p = (struct progressbar *) c;

	return (uintptr_t) (p->widget);
}

static void progressbarSetValue(uiProgressBar *pp, int value)
{
	struct progressbar *p = (struct progressbar *) pp;

	if (value < 0 || value > 100)
		complain("value %d out of range in progressbarSetValue()", value);
	gtk_progress_bar_set_fraction(p->pbar, ((gdouble) value) / 100);
}

uiProgressBar *uiNewProgressBar(void)
{
	struct progressbar *p;

	p = (struct progressbar *) uiNewControl(uiTypeProgressBar());

	p->widget = gtk_progress_bar_new();
	p->pbar = GTK_PROGRESS_BAR(p->widget);
	uiUnixMakeSingleWidgetControl(uiControl(p), p->widget);

	uiControl(p)->Handle = progressbarHandle;

	uiProgressBar(p)->SetValue = progressbarSetValue;

	return uiProgressBar(p);
}
