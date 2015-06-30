// 7 april 2015
#include "uipriv_unix.h"

struct button {
	uiButton b;
	GtkWidget *widget;
	GtkButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

static void onClicked(GtkButton *button, gpointer data)
{
	struct button *b = (struct button *) data;

	(*(b->onClicked))(uiButton(b), b->onClickedData);
}





