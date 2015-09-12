// 9 september 2015
#include "area.h"

// We are basically cloning NSScrollView here, managing scrolling ourselves.
// TODOs
// - is the page increment set up right?
// - do we need to draw anything in the empty corner?

// TODO remove this
void addConstraint(NSView *view, NSString *constraint, NSDictionary *metrics, NSDictionary *views)
{
	NSArray *constraints;

	constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint
		options:0
		metrics:metrics
		views:views];
	[view addConstraints:constraints];
}

// NSScrollers have no intrinsic size; here we give it one
@interface areaScroller : NSScroller {
	BOOL libui_vertical;
}
- (id)initWithFrame:(NSRect)r vertical:(BOOL)v;
@end

@implementation areaScroller

- (id)initWithFrame:(NSRect)r vertical:(BOOL)v
{
	self = [super initWithFrame:r];
	if (self)
		self->libui_vertical = v;
	return self;
}

- (NSSize)intrinsicContentSize
{
	NSSize s;
	CGFloat scrollerWidth;

	s = [super intrinsicContentSize];
	scrollerWidth = [NSScroller scrollerWidthForControlSize:[self controlSize]
		scrollerStyle:[self scrollerStyle]];
	if (self->libui_vertical)
		s.width = scrollerWidth;
	else
		s.height = scrollerWidth;
	return s;
}

- (void)setControlSize:(NSControlSize)size
{
	[super setControlSize:size];
	[self invalidateIntrinsicContentSize];
}

- (void)setScrollerStyle:(NSScrollerStyle)style
{
	[super setScrollerStyle:style];
	[self invalidateIntrinsicContentSize];
}

@end

@interface areaDrawingView : NSView {
	uiArea *libui_a;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a;
- (void)doMouseEvent:(NSEvent *)e;
@end

@interface areaView : NSView {
	uiArea *libui_a;
	areaDrawingView *drawingView;
	areaScroller *hscrollbar;
	areaScroller *vscrollbar;
	intmax_t hscrollpos;
	intmax_t vscrollpos;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a;
- (void)dvFrameSizeChanged:(NSNotification *)note;
- (IBAction)hscrollEvent:(id)sender;
- (IBAction)vscrollEvent:(id)sender;
- (intmax_t)hscrollPos;
- (intmax_t)vscrollPos;
// scroll utilities
- (intmax_t)hpagesize;
- (intmax_t)vpagesize;
- (intmax_t)hscrollmax;
- (intmax_t)vscrollmax;
- (intmax_t)hscrollbarPosition;
- (intmax_t)vscrollbarPosition;
- (void)hscrollTo:(intmax_t)pos;
- (void)vscrollTo:(intmax_t)pos;
@end

struct uiArea {
//	uiDarwinControl c;
	areaView *view;
	uiAreaHandler *ah;
};

@implementation areaDrawingView

- (id)initWithFrame:(NSRect)r area:(uiArea *)a
{
	self = [super initWithFrame:r];
	if (self)
		self->libui_a = a;
	return self;
}

- (void)drawRect:(NSRect)r
{
	CGContextRef c;
	uiAreaDrawParams dp;

	c = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	dp.Context = newContext(c);

	dp.ClientWidth = [self frame].size.width;
	dp.ClientHeight = [self frame].size.height;

	dp.ClipX = r.origin.x;
	dp.ClipY = r.origin.y;
	dp.ClipWidth = r.size.width;
	dp.ClipHeight = r.size.height;

	// TODO DPI

	// TODO scroll position

	(*(self->libui_a->ah->Draw))(self->libui_a->ah, self->libui_a, &dp);
}

- (BOOL)isFlipped
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)doMouseEvent:(NSEvent *)e
{
	uiAreaMouseEvent me;
	NSPoint point;
	areaView *av;
	uintmax_t buttonNumber;
	NSUInteger pmb;
	unsigned int i, max;

	av = (areaView *) [self superview];

	point = [self convertPoint:[e locationInWindow] fromView:nil];
	me.X = point.x;
	me.Y = point.y;
	me.HScrollPos = [av hscrollPos];
	me.VScrollPos = [av vscrollPos];
	// don't clip to outside the view in the case of captures
	// TODO cocoa captures automatically on a drag?

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

	me.Modifiers = 0;		// TODO

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

	(*(self->libui_a->ah->MouseEvent))(self->libui_a->ah, self->libui_a, &me);
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

@end

@implementation areaView

- (id)initWithFrame:(NSRect)r area:(uiArea *)a
{
	NSScrollerStyle style;
	CGFloat swidth;
	NSMutableDictionary *views;
	NSLayoutConstraint *constraint;

	self = [super initWithFrame:r];
	if (self) {
		self->libui_a = a;

		self->drawingView = [[areaDrawingView alloc] initWithFrame:NSZeroRect area:self->libui_a];
		[self->drawingView setTranslatesAutoresizingMaskIntoConstraints:NO];

		style = [NSScroller preferredScrollerStyle];
		swidth = [NSScroller scrollerWidthForControlSize:NSRegularControlSize
			scrollerStyle:style];

		self->hscrollbar = [[areaScroller alloc]
			initWithFrame:NSMakeRect(0, 0, swidth * 5, swidth)
			vertical:NO];
		[self->hscrollbar setScrollerStyle:style];
		[self->hscrollbar setKnobStyle:NSScrollerKnobStyleDefault];
		[self->hscrollbar setControlTint:NSDefaultControlTint];
		[self->hscrollbar setControlSize:NSRegularControlSize];
		[self->hscrollbar setArrowsPosition:NSScrollerArrowsDefaultSetting];
		[self->hscrollbar setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->vscrollbar = [[areaScroller alloc]
			initWithFrame:NSMakeRect(0, 0, swidth, swidth * 5)
			vertical:YES];
		[self->vscrollbar setScrollerStyle:style];
		[self->vscrollbar setKnobStyle:NSScrollerKnobStyleDefault];
		[self->vscrollbar setControlTint:NSDefaultControlTint];
		[self->vscrollbar setControlSize:NSRegularControlSize];
		[self->vscrollbar setArrowsPosition:NSScrollerArrowsDefaultSetting];
		[self->vscrollbar setTranslatesAutoresizingMaskIntoConstraints:NO];

		[self addSubview:self->drawingView];
		[self addSubview:self->hscrollbar];
		[self addSubview:self->vscrollbar];

		// use visual constraints to arrange:
		// - the drawing view and vertical scrollbar horizontally
		// - the drawing view and horizontal scrollbar vertically
		// - the horizontal scrollbar flush left
		// - the vertical scrollbar flush top
		views = [NSMutableDictionary new];
		[views setObject:self->drawingView forKey:@"drawingView"];
		[views setObject:self->hscrollbar forKey:@"hscrollbar"];
		[views setObject:self->vscrollbar forKey:@"vscrollbar"];
		addConstraint(self, @"H:|[drawingView][vscrollbar]|", nil, views);
		addConstraint(self, @"V:|[drawingView][hscrollbar]|", nil, views);
		addConstraint(self, @"H:|[hscrollbar]", nil, views);
		addConstraint(self, @"V:|[vscrollbar]", nil, views);
		[views release];

		// use explicit layout constraints to line up
		// - the bottom edge of the drawing view with the bottom edge of the vertical scrollbar
		// - the right edge of the drawing view with the right edge of the horizontal scrollbar
		constraint = [NSLayoutConstraint constraintWithItem:self->drawingView
			attribute:NSLayoutAttributeBottom
			relatedBy:NSLayoutRelationEqual
			toItem:self->vscrollbar
			attribute:NSLayoutAttributeBottom
			multiplier:1
			constant:0];
		[self addConstraint:constraint];
		[constraint release];
		constraint = [NSLayoutConstraint constraintWithItem:self->drawingView
			attribute:NSLayoutAttributeRight
			relatedBy:NSLayoutRelationEqual
			toItem:self->hscrollbar
			attribute:NSLayoutAttributeRight
			multiplier:1
			constant:0];
		[self addConstraint:constraint];
		[constraint release];

		self->hscrollpos = 0;
		self->vscrollpos = 0;

		// now set up events
		// first we need to monitor when the drawing view frame size has changed, as we need to recalculate all the scrollbar parameters in that case
		[[NSNotificationCenter defaultCenter]
			addObserver:self
			selector:@selector(dvFrameSizeChanged:)
			name:NSViewFrameDidChangeNotification
			object:self->drawingView];
		// and this will trigger a frame changed event to kick us off
		[self->drawingView setPostsFrameChangedNotifications:YES];

		// and the scrollbar events
		[self->hscrollbar setTarget:self];
		[self->hscrollbar setAction:@selector(hscrollEvent:)];
		[self->vscrollbar setTarget:self];
		[self->vscrollbar setAction:@selector(vscrollEvent:)];

		// TODO notification on preferred style change
	}
	return self;
}

- (void)dealloc
{
	[self->vscrollbar setTarget:nil];
	[self->hscrollbar setTarget:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

// TODO reduce code duplication

// TODO if the proportion becomes 1 we should disable the scrollbar
- (void)dvFrameSizeChanged:(NSNotification *)note
{
	intmax_t max;
	double proportion;

	max = [self hscrollmax];
	if (max == 0) {
		[self->hscrollbar setKnobProportion:0];
		// this hides the knob
		[self->hscrollbar setEnabled:NO];
	} else {
		proportion = [self hpagesize];
		proportion /= max;
		[self->hscrollbar setKnobProportion:proportion];
		[self->hscrollbar setEnabled:YES];
	}

	max = [self vscrollmax];
	if (max == 0) {
		[self->vscrollbar setKnobProportion:0];
		// this hides the knob
		[self->vscrollbar setEnabled:NO];
	} else {
		proportion = [self vpagesize];
		proportion /= max;
		[self->vscrollbar setKnobProportion:proportion];
		[self->vscrollbar setEnabled:YES];
	}

	// and update the scrolling position
	[self hscrollTo:self->hscrollpos];
	[self vscrollTo:self->vscrollpos];

	if ((*(self->libui_a->ah->RedrawOnResize))(self->libui_a->ah, self->libui_a))
		[self->drawingView setNeedsDisplay:YES];
}

- (IBAction)hscrollEvent:(id)sender
{
	uintmax_t pos;

	pos = self->hscrollpos;
	switch ([self->hscrollbar hitPart]) {
	case NSScrollerNoPart:
		// do nothing
		break;
	case NSScrollerDecrementPage:
		pos -= [self hpagesize];
		break;
	case NSScrollerKnob:
	case NSScrollerKnobSlot:
		pos = [self hscrollbarPosition];
		break;
	case NSScrollerIncrementPage:
		pos += [self hpagesize];
		break;
	case NSScrollerDecrementLine:
		pos--;
		break;
	case NSScrollerIncrementLine:
		pos++;
		break;
	}
	[self hscrollTo:pos];
}

- (IBAction)vscrollEvent:(id)sender
{
	uintmax_t pos;

	pos = self->vscrollpos;
	switch ([self->vscrollbar hitPart]) {
	case NSScrollerNoPart:
		// do nothing
		break;
	case NSScrollerDecrementPage:
		pos -= [self vpagesize];
		break;
	case NSScrollerKnob:
	case NSScrollerKnobSlot:
		pos = [self vscrollbarPosition];
		break;
	case NSScrollerIncrementPage:
		pos += [self vpagesize];
		break;
	case NSScrollerDecrementLine:
		pos--;
		break;
	case NSScrollerIncrementLine:
		pos++;
		break;
	}
	[self vscrollTo:pos];
}

- (intmax_t)hscrollPos
{
	return self->hscrollpos;
}

- (intmax_t)vscrollPos
{
	return self->vscrollpos;
}

// scroll utilities

- (intmax_t)hpagesize
{
	return [self->drawingView frame].size.width;
}

- (intmax_t)vpagesize
{
	return [self->drawingView frame].size.height;
}

- (intmax_t)hscrollmax
{
	intmax_t n;

	n = (*(self->libui_a->ah->HScrollMax))(self->libui_a->ah, self->libui_a);
	n -= [self hpagesize];
	if (n < 0)
		n = 0;
	return n;
}

- (intmax_t)vscrollmax
{
	intmax_t n;

	n = (*(self->libui_a->ah->VScrollMax))(self->libui_a->ah, self->libui_a);
	n -= [self vpagesize];
	if (n < 0)
		n = 0;
	return n;
}

- (intmax_t)hscrollbarPosition
{
	return [self->hscrollbar doubleValue] * [self hscrollmax];
}

- (intmax_t)vscrollbarPosition
{
	return [self->vscrollbar doubleValue] * [self vscrollmax];
}

- (void)hscrollTo:(intmax_t)pos
{
	double doubleVal;
	CGFloat by;
	NSRect update;

	if (pos > [self hscrollmax])
		pos = [self hscrollmax];
	if (pos < 0)
		pos = 0;

	by = -(pos - self->hscrollpos);
	[self->drawingView scrollRect:[self->drawingView bounds]
		by:NSMakeSize(by, 0)];
	update = [self->drawingView bounds];
	if (by < 0) {		// right of bounds needs updating
		// + by since by is negative and we need to subtract its absolute value from the width
		update.origin.x += update.size.width + by;
		update.size.width = -by;
	} else			// left of bounds needs updating
		update.size.width = by;
	[self->drawingView setNeedsDisplayInRect:update];

	self->hscrollpos = pos;
	doubleVal = ((double) (self->hscrollpos)) / [self hscrollmax];
	[self->hscrollbar setDoubleValue:doubleVal];
}

- (void)vscrollTo:(intmax_t)pos
{
	double doubleVal;
	CGFloat by;
	NSRect update;

	if (pos > [self vscrollmax])
		pos = [self vscrollmax];
	if (pos < 0)
		pos = 0;

	by = -(pos - self->vscrollpos);
	[self->drawingView scrollRect:[self->drawingView bounds]
		by:NSMakeSize(0, by)];
	update = [self->drawingView bounds];
	if (by < 0) {		// bottom of bounds needs updating
		// + by since by is negative and we need to subtract its absolute value from the height
		update.origin.y += update.size.height + by;
		update.size.height = -by;
	} else			// top of bounds needs updating
		update.size.height = by;
	[self->drawingView setNeedsDisplayInRect:update];

	self->vscrollpos = pos;
	doubleVal = ((double) (self->vscrollpos)) / [self vscrollmax];
	[self->vscrollbar setDoubleValue:doubleVal];
}

@end

uiArea *newArea(uiAreaHandler *ah)
{
	uiArea *a;

	// TODO
	a = (uiArea *) malloc(sizeof (uiArea));

	a->ah = ah;

	a->view = [[areaView alloc] initWithFrame:NSZeroRect area:a];

	// set initial state
	// TODO do this on other platforms?
	areaUpdateScroll(a);

	return a;
}

NSView *areaGetView(uiArea *a)
{
	return a->view;
}

void areaUpdateScroll(uiArea *a)
{
/* TODO
	NSRect frame;

	frame.origin = NSMakePoint(0, 0);
	frame.size.width = (*(a->ah->HScrollMax))(a->ah, a);
	frame.size.height = (*(a->ah->VScrollMax))(a->ah, a);
	[a->documentView setFrame:frame];
*/
}
