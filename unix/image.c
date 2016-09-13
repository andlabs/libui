// 13 september 2016
#include "uipriv_unix.h"

struct uiImage {
	uiUnixControl c;
	GtkWidget *widget;
};

uiUnixControlAllDefaults(uiImage)

uiImage *uiNewImage(const char *filename)
{
	uiImage *img;

	uiUnixNewControl(uiImage, img);

	img->widget = gtk_image_new_from_file(filename);

	return img;
}
