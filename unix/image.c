// 13 september 2016
#include "uipriv_unix.h"

struct uiImage {
	uiUnixControl c;
	GtkWidget *widget;
};

uiUnixControlAllDefaults(uiImage)

void uiImageSetSize(uiImage *i, unsigned int width, unsigned int height)
{
	GdkPixbuf *pixbuf;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(i->widget));
	pixbuf = gdk_pixbuf_scale_simple(pixbuf,
		width,
		height,
		GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(GTK_IMAGE(i->widget), pixbuf);
	g_object_unref(pixbuf);
}

void uiImageGetSize(uiImage *i, unsigned int *width, unsigned int *height)
{
	GdkPixbuf *pixbuf;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(i->widget));
	*width = gdk_pixbuf_get_width(pixbuf);
	*height = gdk_pixbuf_get_height(pixbuf);
}

uiImage *uiNewImage(const char *filename)
{
	uiImage *img;

	uiUnixNewControl(uiImage, img);

	img->widget = gtk_image_new_from_file(filename);

	return img;
}
