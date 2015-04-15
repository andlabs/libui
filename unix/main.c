// 6 april 2015
#include "uipriv_unix.h"

// #qo pkg-config: gtk+-3.0

void uiMain(void)
{
	gtk_main();
}

// gtk_main_quit() may run immediately, or it may wait for other pending events; "it depends" (thanks mclasen in irc.gimp.net/#gtk+)
// PostQuitMessage() on Windows always waits, so we must do so too
// we'll do it by using an idle callback
static gboolean quit(gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

void uiQuit(void)
{
	gdk_threads_add_idle(quit, NULL);
}
