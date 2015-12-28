// 9 september 2015
#import "uipriv_darwin.h"

@interface areaView : NSView {
	uiArea *libui_a;
	NSTrackingArea *libui_ta;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a;
- (uiModifiers)parseModifiers:(NSEvent *)e;
- (void)doMouseEvent:(NSEvent *)e;
- (int)sendKeyEvent:(uiAreaKeyEvent *)ke;
- (int)doKeyDownUp:(NSEvent *)e up:(int)up;
- (int)doKeyDown:(NSEvent *)e;
- (int)doKeyUp:(NSEvent *)e;
- (int)doFlagsChanged:(NSEvent *)e;
- (void)setupNewTrackingArea;
@end

struct uiArea {
	uiDarwinControl c;
	NSView *view;			// either sv or area depending on whether it is scrolling
	NSScrollView *sv;
	areaView *area;
	uiAreaHandler *ah;
	BOOL scrolling;
};

uiDarwinDefineControl(
	uiArea,								// type name
	uiAreaType,							// type function
	view									// handle
)

@implementation areaView

- (id)initWithFrame:(NSRect)r area:(uiArea *)a
{
	self = [super initWithFrame:r];
	if (self) {
		self->libui_a = a;
		[self setupNewTrackingArea];
	}
	return self;
}

- (void)drawRect:(NSRect)r
{
	uiArea *a = self->libui_a;
	CGContextRef c;
	uiAreaDrawParams dp;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	dp.Context = newContext(c);

	dp.AreaWidth = 0;
	dp.AreaHeight = 0;
	if (!a->scrolling) {
		// TODO frame or bounds?
		dp.AreaWidth = [self frame].size.width;
		dp.AreaHeight = [self frame].size.height;
	}

	dp.ClipX = r.origin.x;
	dp.ClipY = r.origin.y;
	dp.ClipWidth = r.size.width;
	dp.ClipHeight = r.size.height;

	// no need to save or restore the graphics state to reset transformations; Cocoa creates a brand-new context each time
	(*(a->ah->Draw))(a->ah, a, &dp);

	freeContext(dp.Context);
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
	// TODO NSTrackingAssumeInside?
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

// TODO when do we call super here?
- (void)updateTrackingAreas
{
	[self removeTrackingArea:self->libui_ta];
	[self->libui_ta release];
	[self setupNewTrackingArea];
}

// capture on drag is done automatically on OS X
- (void)doMouseEvent:(NSEvent *)e
{
	uiArea *a = self->libui_a;
	uiAreaMouseEvent me;
	NSPoint point;
	uintmax_t buttonNumber;
	NSUInteger pmb;
	unsigned int i, max;

	// this will convert point to drawing space
	// thanks swillits in irc.freenode.net/#macdev
	point = [self convertPoint:[e locationInWindow] fromView:nil];
	me.X = point.x;
	me.Y = point.y;

	me.AreaWidth = 0;
	me.AreaHeight = 0;
	if (!a->scrolling) {
		// TODO frame or bounds?
		me.AreaWidth = [self frame].size.width;
		me.AreaHeight = [self frame].size.height;
	}

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
	// buttons 4..64
	max = 32;
	// TODO are the upper 32 bits just mirrored?
//	if (sizeof (NSUInteger) == 8)
//		max = 64;
	for (i = 4; i <= max; i++) {
		uint64_t j;

		if (buttonNumber == i)
			continue;
		j = 1 << (i - 1);
		if ((pmb & j) != 0)
			me.Held1To64 |= j;
	}

	(*(a->ah->MouseEvent))(a->ah, a, &me);
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
	uiArea *a = self->libui_a;

	(*(a->ah->MouseCrossed))(a->ah, a, 0);
}

- (void)mouseExited:(NSEvent *)e
{
	uiArea *a = self->libui_a;

	(*(a->ah->MouseCrossed))(a->ah, a, 1);
}

// note: there is no equivalent to WM_CAPTURECHANGED on Mac OS X; there literally is no way to break a grab like that
// even if I invoke the task switcher and switch processes, the mouse grab will still be held until I let go of all buttons
// therefore, no DragBroken()

- (int)sendKeyEvent:(uiAreaKeyEvent *)ke
{
	uiArea *a = self->libui_a;

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

	if (!fromKeycode([e keyCode], &ke))
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
	if (!keycodeModifier([e keyCode], &whichmod))
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
	uiArea *a = self->libui_a;

	[super setFrameSize:size];
	if (!a->scrolling)
		// we must redraw everything on resize because Windows requires it
		[self setNeedsDisplay:YES];
}

@end

// called by subclasses of -[NSApplication sendEvent:]
// by default, NSApplication eats some key events
// this prevents that from happening with uiArea
// see http://stackoverflow.com/questions/24099063/how-do-i-detect-keyup-in-my-nsview-with-the-command-key-held and http://lists.apple.com/archives/cocoa-dev/2003/Oct/msg00442.html
int sendAreaEvents(NSEvent *e)
{
	NSEventType type;
	id focused;
	areaView *view;

	type = [e type];
	if (type != NSKeyDown && type != NSKeyUp && type != NSFlagsChanged)
		return 0;
	focused = [[e window] firstResponder];
	if (focused == nil)
		return 0;
	if (![focused isKindOfClass:[areaView class]])
		return 0;
	view = (areaView *) focused;
	switch (type) {
	case NSKeyDown:
		return [view doKeyDown:e];
	case NSKeyUp:
		return [view doKeyUp:e];
	case NSFlagsChanged:
		return [view doFlagsChanged:e];
	}
	return 0;
}

void uiAreaSetSize(uiArea *a, intmax_t width, intmax_t height)
{
	if (!a->scrolling)
		complain("attempt to call uiAreaSetSize() on a non-scrolling uiArea");
	[a->area setFrameSize:NSMakeSize(width, height)];
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	[a->area setNeedsDisplay:YES];
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

	a->ah = ah;
	a->scrolling = NO;

	a->area = [[areaView alloc] initWithFrame:NSZeroRect area:a];

	a->view = a->area;

	uiDarwinFinishNewControl(a, uiArea);

	return a;
}

uiArea *uiNewScrollingArea(uiAreaHandler *ah, intmax_t width, intmax_t height)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

	a->ah = ah;
	a->scrolling = YES;

	a->sv = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	// TODO configure a->sv for real
	[a->sv setHasHorizontalScroller:YES];
	[a->sv setHasVerticalScroller:YES];

	a->area = [[areaView alloc] initWithFrame:NSMakeRect(0, 0, width, height)
		area:a];

	a->view = a->sv;

	[a->sv setDocumentView:a->area];

	uiDarwinFinishNewControl(a, uiArea);

	return a;
}
