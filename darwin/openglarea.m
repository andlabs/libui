#import "uipriv_darwin.h"

#define NSEventModifierFlags NSUInteger

@interface openGLAreaView : areaCommonView {
	uiOpenGLArea *libui_a;
}

- (id)initWithFrame:(NSRect)r area:(uiOpenGLArea *)a attributes:(uiOpenGLAttributes *)attribs;
@end

#define ATTRIBUTE_LIST_SIZE	256

struct uiOpenGLArea {
	uiDarwinControl c;
	openGLAreaView *view;
	uiOpenGLAreaHandler *ah;
	NSEvent *dragevent;
	BOOL scrolling;
	CGLPixelFormatObj pix;
	GLint npix;
	NSOpenGLContext *ctx;
	BOOL initialized;
};

// This functionality is wrapped up here to guard against buffer overflows in the attribute list.
static void assignNextPixelFormatAttribute(CGLPixelFormatAttribute *as, unsigned *ai, CGLPixelFormatAttribute a)
{
	if (*ai >= ATTRIBUTE_LIST_SIZE)
		uiprivImplBug("Too many pixel format attributes; increase ATTRIBUTE_LIST_SIZE!");
	as[*ai] = a;
	(*ai)++;
}

@implementation openGLAreaView

- (id)initWithFrame:(NSRect)r area:(uiOpenGLArea *)a attributes:(uiOpenGLAttributes *)attribs
{
	self = [super initWithFrame:r];
	if (self) {
		self->libui_a = a;
		[self setArea:(uiArea *)a];
		[self setupNewTrackingArea];
		self->libui_enabled = YES;

		CGLPixelFormatAttribute pfAttributes[ATTRIBUTE_LIST_SIZE];
		unsigned pfAttributeIndex = 0;
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAColorSize);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, attribs->RedBits + attribs->GreenBits + attribs->BlueBits);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAAlphaSize);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, attribs->AlphaBits);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFADepthSize);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, attribs->DepthBits);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAStencilSize);
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, attribs->StencilBits);
		if (attribs->Stereo)
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAStereo);
		if (attribs->Samples > 0) {
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAMultisample);
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFASamples);
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, attribs->Samples);
		}
		if (attribs->DoubleBuffer)
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFADoubleBuffer);
		if (attribs->MajorVersion < 3) {
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAOpenGLProfile);
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy);
		} else {
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, kCGLPFAOpenGLProfile);
			assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core);
		}
		assignNextPixelFormatAttribute(pfAttributes, &pfAttributeIndex, 0);

		if (CGLChoosePixelFormat(pfAttributes, &self->libui_a->pix, &self->libui_a->npix) != kCGLNoError)
			uiprivUserBug("No available pixel format!");

		CGLContextObj ctx;
		if (CGLCreateContext(self->libui_a->pix, NULL, &ctx) != kCGLNoError)
			uiprivUserBug("Couldn't create OpenGL context!");
		self->libui_a->ctx = [[NSOpenGLContext alloc] initWithCGLContextObj:ctx];
		[[NSNotificationCenter defaultCenter] addObserver:self selector: @selector(viewBoundsDidChange:) name:NSViewFrameDidChangeNotification object:self];

	}
	return self;
}

- (void)viewBoundsDidChange:(NSNotification *)notification
{
	[self->libui_a->ctx setView:self];
	[self->libui_a->ctx update];
}

- (void)drawRect:(NSRect)r
{
	uiOpenGLArea *a = self->libui_a;
	uiOpenGLAreaMakeCurrent(a);

	double width = [self frame].size.width;
	double height = [self frame].size.height;

	if (!a->initialized) {
		(*(a->ah->InitGL))(a->ah, a);
		a->initialized = YES;
	}
	(*(a->ah->DrawGL))(a->ah, a, width, height);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiOpenGLArea, view)

static void uiOpenGLAreaDestroy(uiControl *c)
{
	uiOpenGLArea *a = uiOpenGLArea(c);

	[a->view release];
	[a->ctx release];
	CGLReleasePixelFormat(a->pix);
	uiFreeControl(uiControl(a));
}

void uiOpenGLAreaGetSize(uiOpenGLArea *a, double *width, double *height)
{
	NSRect rect = [a->view frame];
	if(width != NULL)
		*width = rect.size.width;
	if(height != NULL)
		*height = rect.size.height;
}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int si)
{
	if ((!CGLSetParameter([a->ctx CGLContextObj], kCGLCPSwapInterval, &si)) != kCGLNoError)
		uiprivUserBug("Couldn't set the swap interval!");
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	[a->view setNeedsDisplay:YES];
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	CGLSetCurrentContext([a->ctx CGLContextObj]);
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	CGLFlushDrawable([a->ctx CGLContextObj]);
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;
	uiDarwinNewControl(uiOpenGLArea, a);
	a->initialized = NO;
	a->scrolling = NO;
	a->ah = ah;
	a->view = [[openGLAreaView alloc] initWithFrame:NSZeroRect area:a attributes:attribs];
	return a;
}
