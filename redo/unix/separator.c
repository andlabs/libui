// 11 june 2015
#include "uipriv_unix.h"

struct separator {
	uiSeparator s;
	GtkWidget *widget;
};

uiDefineControlType(uiSeparator, uiTypeSeparator, struct separator)

static uintptr_t separatorHandle(uiControl *c)
{
	struct separator *s = (struct separator *) c;

	return (uintptr_t) (s->widget);
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	struct separator *s;

	s = (struct separator *) uiNewControl(uiTypeSeparator());

	PUT_CODE_HERE;

	uiControl(s)->Handle = separatorHandle;

	return uiSeparator(s);
}
