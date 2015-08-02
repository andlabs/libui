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
	NSString *margin;
	void (^run)(NSArray *, NSArray *, NSArray *, NSString *);

	if (self->c == nil)
		return;

	contentView = [self->w contentView];
	[contentView removeConstraints:[contentView constraints]];

	p.horz = [NSMutableArray new];
	p.horzAttachLeft = [NSMutableArray new];
	p.horzAttachRight = [NSMutableArray new];
	p.horzFirst = YES;		// only control here
	p.horzLast = YES;
	p.vert = [NSMutableArray new];
	p.vertAttachTop = [NSMutableArray new];
	p.vertAttachBottom = [NSMutableArray new];
	p.vertFirst = YES;
	p.vertLast = YES;
	p.views = [NSMutableDictionary new];
	p.n = 0;
	[self->c tFillAutoLayout:&p];

	margin = @"";
	if (self->margined)
		margin = @"-";

	run = ^(NSArray *side, NSArray *attachStart, NSArray *attachEnd, NSString *prefix) {
		NSUInteger i;

		for (i = 0; i < [side count]; i++) {
			NSMutableString *constraint;
			NSNumber *attach;
			NSArray *constraints;

			constraint = [NSMutableString stringWithString:prefix];
			attach = (NSNumber *) [attachStart objectAtIndex:i];
			if ([attach boolValue]) {
				[constraint appendString:@"|"];
				[constraint appendString:margin];
			}
			[constraint appendString:[side objectAtIndex:i]];
			attach = (NSNumber *) [attachEnd objectAtIndex:i];
			if ([attach boolValue]) {
				[constraint appendString:margin];
				[constraint appendString:@"|"];
			}
			constraints = [NSLayoutConstraint constraintsWithVisualFormat:constraint options:0 metrics:nil views:p.views];
			[contentView addConstraints:constraints];
			[constraint release];
		}
	};

	run(p.horz, p.horzAttachLeft, p.horzAttachRight, @"H:");
	run(p.vert, p.vertAttachTop, p.vertAttachBottom, @"V:");

	// TODO release everything
}

@end
