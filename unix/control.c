// 16 august 2015
#include "uipriv_unix.h"

static void defaultCommitShow(uiControl *c)
{
	gtk_widget_show(GTK_WIDGET(uiControlHandle(c)));
}

static void defaultCommitHide(uiControl *c)
{
	gtk_widget_hide(GTK_WIDGET(uiControlHandle(c)));
}

void osCommitEnable(uiControl *c)
{
	gtk_widget_set_sensitive(GTK_WIDGET(uiControlHandle(c)), TRUE);
}

void osCommitDisable(uiControl *c)
{
	gtk_widget_set_sensitive(GTK_WIDGET(uiControlHandle(c)), FALSE);
}

void uiUnixFinishControl(uiControl *c)
{
	g_object_ref_sink(GTK_WIDGET(uiControlHandle(c)));
	if (!isToplevel(c))
		gtk_widget_show(GTK_WIDGET(uiControlHandle(c)));
	c->CommitShow = defaultCommitShow;
	c->CommitHide = defaultCommitHide;
}

#define uiUnixControlSignature 0x556E6978

uiUnixControl *uiUnixNewControl(size_t n, uint32_t typesig, const char *typenamestr)
{
	return uiUnixControl(newControl(n, uiUnixControlSignature, typesig, typenamestr));
}
