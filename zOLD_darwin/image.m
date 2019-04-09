// 25 june 2016
#import "uipriv_darwin.h"

struct uiImage {
	NSImage *i;
	NSSize size;
};

uiImage *uiNewImage(double width, double height)
{
	uiImage *i;

	i = uiprivNew(uiImage);
	i->size = NSMakeSize(width, height);
	i->i = [[NSImage alloc] initWithSize:i->size];
	return i;
}

void uiFreeImage(uiImage *i)
{
	[i->i release];
	uiprivFree(i);
}

void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int byteStride)
{
	NSBitmapImageRep *repCalibrated, *repsRGB;
	int x, y;
	uint8_t *pix, *data;
	NSInteger realStride;

	repCalibrated = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
		pixelsWide:pixelWidth
		pixelsHigh:pixelHeight
		bitsPerSample:8
		samplesPerPixel:4
		hasAlpha:YES
		isPlanar:NO
		colorSpaceName:NSCalibratedRGBColorSpace
		bitmapFormat:0
		bytesPerRow:0
		bitsPerPixel:32];

	// Apple doesn't explicitly document this, but we apparently need to use native system endian for the data :|
	// TODO split this into a utility routine?
	// TODO find proper documentation
	// TODO test this on a big-endian system somehow; I have a feeling the above comment is wrong about the diagnosis since the order we are specifying is now 0xAABBGGRR
	pix = (uint8_t *) pixels;
	data = (uint8_t *) [repCalibrated bitmapData];
	realStride = [repCalibrated bytesPerRow];
	for (y = 0; y < pixelHeight; y++) {
		for (x = 0; x < pixelWidth * 4; x += 4) {
			union {
				uint32_t v32;
				uint8_t v8[4];
			} v;

			v.v32 = ((uint32_t) (pix[x + 3])) << 24;
			v.v32 |= ((uint32_t) (pix[x + 2])) << 16;
			v.v32 |= ((uint32_t) (pix[x + 1])) << 8;
			v.v32 |= ((uint32_t) (pix[x]));
			data[x] = v.v8[0];
			data[x + 1] = v.v8[1];
			data[x + 2] = v.v8[2];
			data[x + 3] = v.v8[3];
		}
		pix += byteStride;
		data += realStride;
	}

	// we can't call the constructor with this, but we can retag (NOT convert)
	repsRGB = [repCalibrated bitmapImageRepByRetaggingWithColorSpace:[NSColorSpace sRGBColorSpace]];

	[i->i addRepresentation:repsRGB];
	[repsRGB setSize:i->size];
	// don't release repsRGB; it may be equivalent to repCalibrated
	// do release repCalibrated though; NSImage has a ref to either it or to repsRGB
	[repCalibrated release];
}

NSImage *uiprivImageNSImage(uiImage *i)
{
	return i->i;
}
