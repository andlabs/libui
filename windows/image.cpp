#include "uipriv_windows.hpp"
// stubbed out windows image list implementation.
// Required for uiTable control, but windows implemenation
// doesn't currently have image support.

struct uiImage {
	double width;
	double height;
	// HIMAGELIST images;
};

uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiprivNew(uiImage);
	i->width = width;
	i->height = height;
	return i;
}

void uiFreeImage(uiImage *i)
{
	uiprivFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride)
{
	// not implemented
}

