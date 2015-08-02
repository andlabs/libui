// 31 july 2015
#import "osxaltest.h"

@implementation tButton {
	NSButton *b;
	id<tControl> parent;
}

- (id)tInitWithText:(NSString *)text
{
	self = [super init];
	if (self) {
		self->b = [[NSButton alloc] initWithFrame:NSZeroRect];
		[self->b setTitle:text];
		[self->b setButtonType:NSMomentaryPushInButton];
		[self->b setBordered:YES];
		[self->b setBezelStyle:NSRoundedBezelStyle];
		[self->b setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->b setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->parent = nil;
	}
	return self;
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v
{
	self->parent = p;
	[v addSubview:self->b];
	[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSString *key;
	NSString *horzpred, *vertpred;

	key = tAutoLayoutKey(p->n);
	p->n++;
	horzpred = @"";
	vertpred = @"";
	if (p->stretchy) {
		NSString *predicate;

		if (p->firstStretchy)
			// TODO is this unnecessary? it seems like I need to do other things instead of this to ensure stretchiness...
			predicate = @"(>=0)";
		else
			predicate = [NSString stringWithFormat:@"(==%@)", tAutoLayoutKey(p->stretchyTo)];
		if (p->stretchyVert)
			vertpred = predicate;
		else
			horzpred = predicate;
	}
	[p->horz addObject:[NSString stringWithFormat:@"[%@%@]", key, horzpred]];
	[p->horzAttachLeft addObject:@YES];
	[p->horzAttachRight addObject:@YES];
	[p->vert addObject:[NSString stringWithFormat:@"[%@%@]", key, vertpred]];
	[p->vertAttachTop addObject:@YES];
	[p->vertAttachBottom addObject:@YES];
	[p->views setObject:self->b forKey:key];
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
