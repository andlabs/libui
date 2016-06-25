// 25 june 2016
#import "uipriv_darwin.h"

struct uiImage {
	NSImage *i;
	NSSize size;
};

uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiNew(uiImage);
	i->size = NSMakeSize(width, height);
	i->i = [[NSImage alloc] initWithSize:i->size];
	return i;
}

void uiFreeImage(uiImage *i)
{
	[i->i release];
	uiFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride)
{
	NSBitmapImageRep *repCalibrated, *repsRGB;
	unsigned char *pix[1];

	pix[0] = (unsigned char *) pixels;
	repCalibrated = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:pix
		pixelsWide:pixelWidth
		pixelsHigh:pixelHeight
		bitsPerSample:8
		samplesPerPixel:4
		hasAlpha:YES
		isPlanar:NO
		colorSpaceName:NSCalibratedRGBColorSpace
		bitmapFormat:0
		bytesPerRow:pixelStride
		bitsPerPixel:32];
	repsRGB = [repCalibrated bitmapImageRepByRetaggingWithColorSpace:[NSColorSpace sRGBColorSpace]];
	[repCalibrated release];

	[i->i addRepresentation:repsRGB];
	[repsRGB setSize:i->size];
	[repsRGB release];
}
