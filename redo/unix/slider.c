// 11 june 2015
#include "uipriv_unix.h"

struct slider {
	uiSlider s;
	GtkWidget *widget;
	GtkRange *range;
	GtkScale *scale;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

uiDefineControlType(uiSlider, uiTypeSlider, struct slider)

static uintptr_t sliderHandle(uiControl *c)
{
	struct slider *s = (struct slider *) c;

	return (uintptr_t) (s->widget);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

static intmax_t sliderValue(uiSlider *ss)
{
	struct slider *s = (struct slider *) ss;

	return PUT_CODE_HERE;
}

static void sliderSetValue(uiSlider *ss, intmax_t value)
{
	struct slider *s = (struct slider *) ss;

	PUT_CODE_HERE;
}

static void sliderOnChanged(uiSlider *ss, void (*f)(uiSlider *, void *), void *data)
{
	struct slider *s = (struct slider *) ss;

	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	struct slider *s;

	s = (struct slider *) uiNewControl(uiTypeSlider());

	s->widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, 1);
	s->range = GTK_RANGE(s->widget);
	s->scale = GTK_SCALE(s->widget);
	uiUnixMakeSingleWidgetControl(uiControl(s), s->widget);

	// TODO needed?
	gtk_scale_set_digits(s->scale, 0);

	s->onChanged = defaultOnChanged;

	uiControl(s)->Handle = sliderHandle;

	uiSlider(s)->Value = sliderValue;
	uiSlider(s)->SetValue = sliderSetValue;
	uiSlider(s)->OnChanged = sliderOnChanged;

	return uiSlider(s);
}
