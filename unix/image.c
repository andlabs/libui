// 27 june 2016
#include "uipriv_unix.h"

struct uiImage {
	double width;
	double height;
	GPtrArray *images;
};

static void freeImageRep(gpointer item)
{
	cairo_surface_t *cs = (cairo_surface_t *) item;
	unsigned char *buf;

	buf = cairo_image_surface_get_data(cs);
	cairo_surface_destroy(cs);
	uiFree(buf);
}

uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiNew(uiImage);
	i->width = width;
	i->height = height;
	i->images = g_ptr_array_new_with_free_func(freeImageRep);
	return i;
}

void uiFreeImage(uiImage *i)
{
	g_ptr_array_free(i->images, TRUE);
	uiFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride)
{
	cairo_surface_t *cs;
	unsigned char *buf, *p;
	uint8_t *src = (uint8_t *) pixels;
	int cstride;
	int y;

	cstride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, pixelWidth);
	buf = (unsigned char *) uiAlloc((cstride * pixelHeight * 4) * sizeof (unsigned char), "unsigned char[]");
	p = buf;
	for (y = 0; y < pixelWidth * pixelHeight; y += pixelStride) {
		memmove(p, src + y, cstride);
		p += cstride;
	}
	cs = cairo_image_surface_create_for_data(buf, CAIRO_FORMAT_ARGB32,
		pixelWidth, pixelHeight,
		cstride);
	if (cairo_surface_status(cs) != CAIRO_STATUS_SUCCESS)
		/* TODO */;
	cairo_surface_flush(cs);
	g_ptr_array_add(i->images, cs);
}
