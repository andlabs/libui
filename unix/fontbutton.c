// 14 april 2016
#include "uipriv_unix.h"

// TODO display style? or is it already displayed?

struct uiFontButton {
	uiUnixControl c;
	GtkWidget *widget;
	GtkButton *button;
	GtkFontButton *fb;
	GtkFontChooser *fc;
};

uiUnixDefineControl(
	uiFontButton,							// type name
	uiFontButtonType						// type function
)

// TODO NOTE no need to inhibit the signal; font-set is documented as only being sent when the user changes the font
static void onFontSet(GtkFontButton *button, gpointer data)
{
	uiFontButton *b = uiFontButton(data);

//TODO	(*(b->onClicked))(b, b->onClickedData);
}

#if 0
TODO
static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}
#endif

#if 0
TODO
void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}
#endif

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	b = (uiFontButton *) uiNewControl(uiFontButtonType());

	b->widget = gtk_font_button_new();
	b->button = GTK_BUTTON(b->widget);
	b->fb = GTK_FONT_BUTTON(b->widget);
	b->fc = GTK_FONT_CHOOSER(b->widget);

	// match behavior on other platforms
	gtk_font_button_set_show_style(b->fb, TRUE);
	gtk_font_button_set_show_size(b->fb, TRUE);
	gtk_font_button_set_use_font(b->fb, FALSE);
	gtk_font_button_set_use_size(b->fb, FALSE);
	// other customizations
	gtk_font_chooser_set_show_preview_entry(b->fc, TRUE);

	g_signal_connect(b->widget, "font-set", G_CALLBACK(onFontSet), b);
//TODO	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiUnixFinishNewControl(b, uiFontButton);

	return b;
}
