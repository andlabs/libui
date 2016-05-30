#import "uipriv_darwin.h"

#define NSEventModifierFlags NSUInteger

@interface openGLAreaView : NSView {
	uiOpenGLArea *libui_a;
	NSTrackingArea *libui_ta;
	BOOL libui_enabled;
}

- (id)initWithFrame:(NSRect)r area:(uiOpenGLArea *)a attributes:(uiOpenGLAttributes *)attribs;
- (uiModifiers)parseModifiers:(NSEvent *)e;
- (void)doMouseEvent:(NSEvent *)e;
- (int)sendKeyEvent:(uiAreaKeyEvent *)ke;
- (int)doKeyDownUp:(NSEvent *)e up:(int)up;
- (int)doKeyDown:(NSEvent *)e;
- (int)doKeyUp:(NSEvent *)e;
- (int)doFlagsChanged:(NSEvent *)e;
- (void)setupNewTrackingArea;
- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)e;
@end

#define ATTRIBUTE_LIST_SIZE	256

struct uiOpenGLArea {
	uiDarwinControl c;
	openGLAreaView *view;
	uiOpenGLAreaHandler *ah;
	CGLPixelFormatObj pix;
	GLint npix;
	NSOpenGLContext *ctx;
	BOOL initialized;
	NSEvent *dragevent;
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

	if (!a->initialized) {
		(*(a->ah->InitGL))(a->ah, a);
		a->initialized = YES;
	}
	(*(a->ah->DrawGL))(a->ah, a);
}

- (BOOL)isFlipped
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (uiModifiers)parseModifiers:(NSEvent *)e
{
	NSEventModifierFlags mods;
	uiModifiers m;

	m = 0;
	mods = [e modifierFlags];
	if ((mods & NSControlKeyMask) != 0)
		m |= uiModifierCtrl;
	if ((mods & NSAlternateKeyMask) != 0)
		m |= uiModifierAlt;
	if ((mods & NSShiftKeyMask) != 0)
		m |= uiModifierShift;
	if ((mods & NSCommandKeyMask) != 0)
		m |= uiModifierSuper;
	return m;
}

- (void)setupNewTrackingArea
{
	self->libui_ta = [[NSTrackingArea alloc] initWithRect:[self bounds]
		options:(NSTrackingMouseEnteredAndExited |
			NSTrackingMouseMoved |
			NSTrackingActiveAlways |
			NSTrackingInVisibleRect |
			NSTrackingEnabledDuringMouseDrag)
		owner:self
		userInfo:nil];
	[self addTrackingArea:self->libui_ta];
}

- (void)updateTrackingAreas
{
	[self removeTrackingArea:self->libui_ta];
	[self->libui_ta release];
	[self setupNewTrackingArea];
}

// capture on drag is done automatically on OS X
- (void)doMouseEvent:(NSEvent *)e
{
	uiOpenGLArea *a = self->libui_a;
	uiAreaMouseEvent me;
	NSPoint point;
	int buttonNumber;
	NSUInteger pmb;
	unsigned int i, max;

	// this will convert point to drawing space
	// thanks swillits in irc.freenode.net/#macdev
	point = [self convertPoint:[e locationInWindow] fromView:nil];
	me.X = point.x;
	me.Y = point.y;

	me.AreaWidth = 0;
	me.AreaHeight = 0;

	buttonNumber = [e buttonNumber] + 1;
	// swap button numbers 2 and 3 (right and middle)
	if (buttonNumber == 2)
		buttonNumber = 3;
	else if (buttonNumber == 3)
		buttonNumber = 2;

	me.Down = 0;
	me.Up = 0;
	me.Count = 0;
	switch ([e type]) {
	case NSLeftMouseDown:
	case NSRightMouseDown:
	case NSOtherMouseDown:
		me.Down = buttonNumber;
		me.Count = [e clickCount];
		break;
	case NSLeftMouseUp:
	case NSRightMouseUp:
	case NSOtherMouseUp:
		me.Up = buttonNumber;
		break;
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSOtherMouseDragged:
		// we include the button that triggered the dragged event in the Held fields
		buttonNumber = 0;
		break;
	}

	me.Modifiers = [self parseModifiers:e];

	pmb = [NSEvent pressedMouseButtons];
	me.Held1To64 = 0;
	if (buttonNumber != 1 && (pmb & 1) != 0)
		me.Held1To64 |= 1;
	if (buttonNumber != 2 && (pmb & 4) != 0)
		me.Held1To64 |= 2;
	if (buttonNumber != 3 && (pmb & 2) != 0)
		me.Held1To64 |= 4;
	// buttons 4..32
	// https://developer.apple.com/library/mac/documentation/Carbon/Reference/QuartzEventServicesRef/index.html#//apple_ref/c/tdef/CGMouseButton says Quartz only supports up to 32 buttons
	max = 32;
	for (i = 4; i <= max; i++) {
		uint64_t j;

		if (buttonNumber == i)
			continue;
		j = 1 << (i - 1);
		if ((pmb & j) != 0)
			me.Held1To64 |= j;
	}

	if (self->libui_enabled) {
		// and allow dragging here
		a->dragevent = e;
		(*(a->ah->MouseEvent))(a->ah, a, &me);
		a->dragevent = nil;
	}
}

#define mouseEvent(name) \
	- (void)name:(NSEvent *)e \
	{ \
		[self doMouseEvent:e]; \
	}
mouseEvent(mouseMoved)
mouseEvent(mouseDragged)
mouseEvent(rightMouseDragged)
mouseEvent(otherMouseDragged)
mouseEvent(mouseDown)
mouseEvent(rightMouseDown)
mouseEvent(otherMouseDown)
mouseEvent(mouseUp)
mouseEvent(rightMouseUp)
mouseEvent(otherMouseUp)

- (void)mouseEntered:(NSEvent *)e
{
	uiOpenGLArea *a = self->libui_a;

	if (self->libui_enabled)
		(*(a->ah->MouseCrossed))(a->ah, a, 0);
}

- (void)mouseExited:(NSEvent *)e
{
	uiOpenGLArea *a = self->libui_a;

	if (self->libui_enabled)
		(*(a->ah->MouseCrossed))(a->ah, a, 1);
}

// note: there is no equivalent to WM_CAPTURECHANGED on Mac OS X; there literally is no way to break a grab like that
// even if I invoke the task switcher and switch processes, the mouse grab will still be held until I let go of all buttons
// therefore, no DragBroken()

- (int)sendKeyEvent:(uiAreaKeyEvent *)ke
{
	uiOpenGLArea *a = self->libui_a;

	return (*(a->ah->KeyEvent))(a->ah, a, ke);
}

- (int)doKeyDownUp:(NSEvent *)e up:(int)up
{
	uiAreaKeyEvent ke;

	ke.Key = 0;
	ke.ExtKey = 0;
	ke.Modifier = 0;

	ke.Modifiers = [self parseModifiers:e];

	ke.Up = up;

	if (!uiprivFromKeycode([e keyCode], &ke))
		return 0;
	return [self sendKeyEvent:&ke];
}

- (int)doKeyDown:(NSEvent *)e
{
	return [self doKeyDownUp:e up:0];
}

- (int)doKeyUp:(NSEvent *)e
{
	return [self doKeyDownUp:e up:1];
}

- (int)doFlagsChanged:(NSEvent *)e
{
	uiAreaKeyEvent ke;
	uiModifiers whichmod;

	ke.Key = 0;
	ke.ExtKey = 0;

	// Mac OS X sends this event on both key up and key down.
	// Fortunately -[e keyCode] IS valid here, so we can simply map from key code to Modifiers, get the value of [e modifierFlags], and check if the respective bit is set or not — that will give us the up/down state
	if (!uiprivKeycodeModifier([e keyCode], &whichmod))
		return 0;
	ke.Modifier = whichmod;
	ke.Modifiers = [self parseModifiers:e];
	ke.Up = (ke.Modifiers & ke.Modifier) == 0;
	// and then drop the current modifier from Modifiers
	ke.Modifiers &= ~ke.Modifier;
	return [self sendKeyEvent:&ke];
}

- (void)setFrameSize:(NSSize)size
{
	// uiOpenGLArea *a = self->libui_a;

	[super setFrameSize:size];
	// if (!a->scrolling)
		// we must redraw everything on resize because Windows requires it
		[self setNeedsDisplay:YES];
}

// // TODO does this update the frame?
// - (void)setScrollingSize:(NSSize)s
// {
// 	self->libui_ss = s;
// 	[self invalidateIntrinsicContentSize];
// }

// - (NSSize)intrinsicContentSize
// {
// 	if (!self->libui_a->scrolling)
// 		return [super intrinsicContentSize];
// 	return self->libui_ss;
// }

- (BOOL)becomeFirstResponder
{
	return [self isEnabled];
}

- (BOOL)isEnabled
{
	return self->libui_enabled;
}

- (void)setEnabled:(BOOL)e
{
	self->libui_enabled = e;
	if (!self->libui_enabled && [self window] != nil)
		if ([[self window] firstResponder] == self)
			[[self window] makeFirstResponder:nil];
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

void uiOpenGLAreaGetSize(uiOpenGLArea *a, int *width, int *height)
{
	NSRect rect = [a->view frame];
	*width = rect.size.width;
	*height = rect.size.height;
}

void uiOpenGLAreaSetSwapInterval(uiOpenGLArea *a, int si)
{
	if (!CGLSetParameter([a->ctx CGLContextObj], kCGLCPSwapInterval, &si) != kCGLNoError)
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
	a->ah = ah;
	a->view = [[openGLAreaView alloc] initWithFrame:NSZeroRect area:a attributes:attribs];
	return a;
}
