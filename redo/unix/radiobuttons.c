// 11 june 2015
#include "uipriv_unix.h"

// on GTK+ a uiRadioButtons is a GtkBox with each of the GtkRadioButtons as children

struct radiobuttons {
	uiRadioButtons r;
	GtkWidget *boxWidget;
	GtkBox *box;
};

uiDefineControlType(uiRadioButtons, uiTypeRadioButtons, struct radiobuttons)

static uintptr_t radiobuttonsHandle(uiControl *c)
{
	return 0;
}

static void radiobuttonsAppend(uiRadioButtons *rr, const char *text)
{
	struct radiobuttons *r = (struct radiobuttons *) rr;

	PUT_CODE_HERE;
	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	struct radiobuttons *r;

	r = (struct radiobuttons *) uiNewControl(uiTypeRadioButtons());

	r->boxWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	r->box = GTK_BOX(r->boxWidget);
	uiUnixMakeSingleWidgetControl(uiControl(r), r->boxWidget);

	uiControl(r)->Handle = radiobuttonsHandle;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
