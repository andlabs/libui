// 1 august 2015
#include "osxaltest.h"

@implementation tWindow {
	NSWindow *w;
	id<tControl> c;
	BOOL margined;
}

- (id)init
{
	self = [super init];
	if (self) {
		self->w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 320, 240)
			styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)
			backing:NSBackingStoreBuffered
			defer:YES];
		[self->w setTitle:@"Auto Layout Test"];
	}
	return self;
}

- (void)tSetControl:(id<tControl>)cc
{
	self->c = cc;
	[self->c tSetParent:self addToView:[self->w contentView] relayout:NO];
	[self tRelayout];
}

- (void)tSetMargined:(BOOL)m
{
	self->margined = m;
	[self tRelayout];
}

- (void)tShow
{
	[self->w cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[self->w makeKeyAndOrderFront:self];
}

- (void)tRelayout
{
	NSView *contentView;
	tAutoLayoutParams p;
	NSDictionary *views;
	NSString *margin;
	NSMutableString *constraint;
	NSArray *constraints;

	if (self->c == nil)
		return;

	contentView = [self->w contentView];
	[contentView removeConstraints:[contentView constraints]];

	[self->c tFillAutoLayout:&p];

	views = [NSDictionary dictionaryWithObject:p.view forKey:@"view"];

	margin = @"";
	if (self->margined)
		margin = @"-";

	// TODO always append margins even if not attached?
	// or if not attached, append ->=0- as well?
	constraint = [NSMutableString stringWithString:@"H:"];
	if (p.attachLeft) {
		[constraint appendString:@"|"];
		[constraint appendString:margin];
	}
	[constraint appendString:@"[view]"];
	if (p.attachRight) {
		[constraint appendString:margin];
		[constraint appendString:@"|"];
	}
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:views];
	[contentView addConstraints:constraints];
	// TODO do not release constraint; it's autoreleased?

	constraint = [NSMutableString stringWithString:@"V:"];
	if (p.attachTop) {
		[constraint appendString:@"|"];
		[constraint appendString:margin];
	}
	[constraint appendString:@"[view]"];
	if (p.attachBottom) {
		[constraint appendString:margin];
		[constraint appendString:@"|"];
	}
	constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:views];
	[contentView addConstraints:constraints];
	// TODO do not release constraint; it's autoreleased?

	// TODO do not release views; it's autoreleased?
	// (for all of these, look up whether the constructor autoreleases)
}

@end
