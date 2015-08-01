// 1 august 2015
#include "osxaltest.h"

@implementation tWindow {
	NSWindow *w;
	id<tControl> c;
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
	[self->c tAddToView:[self->w contentView]];
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
	NSMutableString *horz, *vert;
	NSMutableArray *extra, *extraVert;
	NSMutableDictionary *views;
	NSInteger i;

	contentView = [self->w contentView];
	[contentView removeConstraints:[contentView constraints]];
	horz = [NSMutableString new];
	vert = [NSMutableString new];
	extra = [NSMutableArray new];
	extraVert = [NSMutableArray new];
	views = [NSMutableDictionary new];
	[self->c tFillAutoLayoutHorz:horz vert:vert extra:extra extraVert:extraVert views:views];
	[extra addObject:[NSString stringWithFormat:@"|%@|", horz]];
	[extraVert addObject:@NO];
	[extra addObject:[NSString stringWithFormat:@"|%@|", vert]];
	[extraVert addObject:@YES];
	for (i = 0; i < [extra count]; i++) {
		NSString *constraint;
		NSNumber *vertical;
		NSArray *constraints;

		vertical = (NSNumber *) [extraVert objectAtIndex:i];
		if ([vertical boolValue])
			constraint = [NSString stringWithFormat:@"V:%@", [extra objectAtIndex:i]];
		else
			constraint = [NSString stringWithFormat:@"H:%@", [extra objectAtIndex:i]];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:views];
		[contentView addConstraints:constraints];
	}
	// TODO release everything
}

@end
