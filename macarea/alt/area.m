// 9 september 2015
#include "area.h"

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
@end

@interface areaView : NSView {
	uiArea *libui_a;
	areaDrawingView *drawingView;
	areaScroller *hscrollbar;
	areaScroller *vscrollbar;
}
- (id)initWithFrame:(NSRect)r area:(uiArea *)a;
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
//TODO		[self->hscrollbar setArrowsPosition:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx];
		[self->hscrollbar setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->vscrollbar = [[areaScroller alloc]
			initWithFrame:NSMakeRect(0, 0, swidth, swidth * 5)
			vertical:YES];
		[self->vscrollbar setScrollerStyle:style];
		[self->vscrollbar setKnobStyle:NSScrollerKnobStyleDefault];
		[self->vscrollbar setControlTint:NSDefaultControlTint];
		[self->vscrollbar setControlSize:NSRegularControlSize];
//TODO		[self->vscrollbar setArrowsPosition:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx];
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
	}
	return self;
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
