// 14 september 2016
#import "uipriv_darwin.h"

struct uiImage {
	uiDarwinControl c;
	NSImage *image;
	NSImageView *imageView;
};

uiDarwinControlAllDefaults(uiImage, imageView)

void uiImageSetSize(uiImage *i, unsigned int width, unsigned int height)
{
	i->image.size = NSMakeSize(width, height);
	[i->imageView setNeedsDisplay:YES];
}

void uiImageGetSize(uiImage *i, unsigned int *width, unsigned int *height)
{
	NSSize size = i->image.size;
	*width = size.width;
	*height = size.height;
}

uiImage *uiNewImage(const char *filename)
{
	uiImage *i;

	uiDarwinNewControl(uiImage, i);

	i->image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:filename]];

	i->imageView = [[NSImageView alloc] init];
	[i->imageView setImage:i->image];

	return i;
}
