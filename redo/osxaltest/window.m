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
	[self->c tSetParent:self addToView:[self->w contentView]];
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
	NSUInteger i;
	NSString *margin;

	if (self->c == nil)
		return;

	contentView = [self->w contentView];
	[contentView removeConstraints:[contentView constraints]];

	p.horz = [NSMutableArray new];
	p.vert = [NSMutableArray new];
	p.extra = [NSMutableArray new];
	p.extraVert = [NSMutableArray new];
	p.views = [NSMutableDictionary new];
	p.n = 0;
	[self->c tFillAutoLayout:&p];

	margin = @"";
	if (self->margined)
		margin = @"-";
	[p.horz enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		[p.extra addObject:[NSString stringWithFormat:@"|%@%@%@|", margin, obj, margin]];
		[p.extraVert addObject:@NO];
	}];
	[p.vert enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		[p.extra addObject:[NSString stringWithFormat:@"|%@%@%@|", margin, obj, margin]];
		[p.extraVert addObject:@YES];
	}];
	for (i = 0; i < [p.extra count]; i++) {
		NSString *constraint;
		NSNumber *vertical;
		NSArray *constraints;

		vertical = (NSNumber *) [p.extraVert objectAtIndex:i];
		if ([vertical boolValue])
			constraint = [NSString stringWithFormat:@"V:%@", [p.extra objectAtIndex:i]];
		else
			constraint = [NSString stringWithFormat:@"H:%@", [p.extra objectAtIndex:i]];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:p.views];
		[contentView addConstraints:constraints];
	}

	// TODO release everything
}

@end
