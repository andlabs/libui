// 11 june 2015
#include "uipriv_unix.h"

struct label {
	uiLabel l;
	GtkWidget *widget;
};

uiDefineControlType(uiLabel, uiTypeLabel, struct label)

static uintptr_t labelHandle(uiControl *c)
{
	struct label *l = (struct label *) c;

	return (uintptr_t) (l->widget);
}

static char *labelText(uiLabel *ll)
{
	struct label *l = (struct label *) ll;

	return PUT_CODE_HERE;
}

static void labelSetText(uiLabel *ll, const char *text)
{
	struct label *l = (struct label *) ll;

	PUT_CODE_HERE;
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;

	l = (struct label *) uiNewControl(uiTypeLabel());

	PUT_CODE_HERE;

	uiControl(l)->Handle = labelHandle;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
