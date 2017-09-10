#include "uipriv_windows.hpp"

struct uiImage {
	double width;
	double height;
	//GPtrArray *images;
};


uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiNew(uiImage);
	i->width = width;
	i->height = height;
//	i->images = g_ptr_array_new_with_free_func(freeImageRep);
	return i;
}

void uiFreeImage(uiImage *i)
{
//	g_ptr_array_free(i->images, TRUE);
	uiFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride)
{
    return;
    // TODO
}

