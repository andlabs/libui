// 11 june 2015
#include "uipriv_OSHERE.h"

struct separator {
	uiSeparator s;
	OSTYPE OSHANDLE;
};

uiDefineControlType(uiSeparator, uiTypeSeparator, struct separator)

static uintptr_t separatorHandle(uiControl *c)
{
	struct separator *s = (struct separator *) c;

	return (uintptr_t) (s->OSHANDLE);
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	struct separator *s;

	s = (struct separator *) MAKE_CONTROL_INSTANCE(uiTypeSeparator());

	PUT_CODE_HERE;

	uiControl(s)->Handle = separatorHandle;

	return uiSeparator(s);
}
