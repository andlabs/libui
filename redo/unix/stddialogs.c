// 26 june 2015
#include "uipriv_unix.h"

static char *filedialog(GtkFileChooserAction mode, const gchar *stock)
{
	GtkWidget *fcd;
	GtkFileChooser *fc;
	gint response;
	char *filename;

	fcd = gtk_file_chooser_dialog_new(NULL, NULL, mode,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		stock, GTK_RESPONSE_ACCEPT,
		NULL);
	fc = GTK_FILE_CHOOSER(fcd);
	gtk_file_chooser_set_local_only(fc, FALSE);
	gtk_file_chooser_set_select_multiple(fc, FALSE);
	gtk_file_chooser_set_show_hidden(fc, TRUE);
	gtk_file_chooser_set_do_overwrite_confirmation(fc, TRUE);
	gtk_file_chooser_set_create_folders(fc, TRUE);
	response = gtk_dialog_run(GTK_DIALOG(fcd));
	if (response != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(fcd);
		return NULL;
	}
	filename = uiUnixStrdupText(gtk_file_chooser_get_filename(fc));
	gtk_widget_destroy(fcd);
	return filename;
}

char *uiOpenFile(void)
{
	return filedialog(GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN);
}

char *uiSaveFile(void)
{
	return filedialog(GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE);
}

void uiMsgBox(const char *title, const char *description)
{
}

void uiMsgBoxError(const char *title, const char *description)
{
}
