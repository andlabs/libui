#include "uipriv_unix.h"

int uiClipboardSetText(const char *text)
{
	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, text, -1);
	return TRUE;
}

char *uiClipboardGetText()
{
	GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	return uiUnixStrdupText(gtk_clipboard_wait_for_text(clipboard));
}