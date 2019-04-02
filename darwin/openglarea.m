#import "uipriv_darwin.h"

#define NSEventModifierFlags NSUInteger

@interface openGLAreaView : uiprivAreaCommonView {
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
	NSOpenGLPixelFormat *pixFmt;
	NSOpenGLContext *ctx;
	BOOL initialized;
};

// This functionality is wrapped up here to guard against buffer overflows in the attribute list.
static void assignNextPixelFormatAttribute(NSOpenGLPixelFormatAttribute *as, unsigned int *ai, NSOpenGLPixelFormatAttribute a)
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

		NSOpenGLPixelFormatAttribute attrs[ATTRIBUTE_LIST_SIZE];
		unsigned int attrIndex = 0;
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAColorSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->RedBits + attribs->GreenBits + attribs->BlueBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAAlphaSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->AlphaBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFADepthSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->DepthBits);
		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAStencilSize);
		assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->StencilBits);
		if (attribs->Stereo)
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAStereo);
		if (attribs->Samples > 0) {
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAMultisample);
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFASamples);
			assignNextPixelFormatAttribute(attrs, &attrIndex, attribs->Samples);
		}
		if (attribs->DoubleBuffer)
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFADoubleBuffer);

		assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLPFAOpenGLProfile);
		if (attribs->MajorVersion < 3) {
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLProfileVersionLegacy);
		} else if (attribs->MajorVersion < 4){
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLProfileVersion3_2Core);
		} else {
			// TODO only 10.10+
			// check at runtime?
			// how to handle non availability?
			assignNextPixelFormatAttribute(attrs, &attrIndex, NSOpenGLProfileVersion4_1Core);
		}
		assignNextPixelFormatAttribute(attrs, &attrIndex, 0); // "a 0-terminated array"

		self->libui_a->pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
		if (self->libui_a->pixFmt == nil)
			uiprivUserBug("No available pixel format!");

		self->libui_a->ctx = [[NSOpenGLContext alloc] initWithFormat:self->libui_a->pixFmt shareContext:nil];
		if(self->libui_a->ctx == nil)
			uiprivUserBug("Couldn't create OpenGL context!");

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewBoundsDidChange:) name:NSViewFrameDidChangeNotification object:self];
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
	[a->pixFmt release];
	uiFreeControl(uiControl(a));
}

void uiOpenGLAreaBeginUserWindowMove(uiOpenGLArea *a)
{
	uiprivNSWindow *w;

	w = (uiprivNSWindow *) [a->view window];
	if (w == nil)
		return;		// TODO
	if (a->dragevent == nil)
		return;		// TODO
	[w uiprivDoMove:a->dragevent];
}

void uiOpenGLAreaBeginUserWindowResize(uiOpenGLArea *a, uiWindowResizeEdge edge)
{
	uiprivNSWindow *w;

	w = (uiprivNSWindow *) [a->view window];
	if (w == nil)
		return;		// TODO
	if (a->dragevent == nil)
		return;		// TODO
	[w uiprivDoResize:a->dragevent on:edge];
}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int v)
{
	[a->ctx setValues:&v forParameter:NSOpenGLContextParameterSwapInterval];
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	[a->view setNeedsDisplay:YES];
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	[a->ctx makeCurrentContext];
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	[a->ctx flushBuffer];
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
