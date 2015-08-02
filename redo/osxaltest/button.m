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

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout
{
	self->parent = p;
	[v addSubview:self->b];
	if (relayout)
		[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSString *key;
	NSString *horzpred, *vertpred;

	key = tAutoLayoutKey(p->n);
	p->n++;
	horzpred = @"";
	if (p->horzStretchy)
		if (p->horzFirstStretchy)
			// TODO is this unnecessary? it seems like I need to do other things instead of this to ensure stretchiness...
			horzpred = @"(>=0)";
		else
			horzpred = [NSString stringWithFormat:@"(==%@)", tAutoLayoutKey(p->horzStretchyTo)];
	vertpred = @"";
	if (p->vertStretchy)
		if (p->vertFirstStretchy)
			// TODO is this unnecessary? it seems like I need to do other things instead of this to ensure stretchiness...
			vertpred = @"(>=0)";
		else
			vertpred = [NSString stringWithFormat:@"(==%@)", tAutoLayoutKey(p->vertStretchyTo)];
	[p->horz addObject:[NSString stringWithFormat:@"[%@%@]", key, horzpred]];
	[p->horzAttachLeft addObject:[NSNumber numberWithBool:p->horzFirst]];
	[p->horzAttachRight addObject:[NSNumber numberWithBool:p->horzLast]];
	[p->vert addObject:[NSString stringWithFormat:@"[%@%@]", key, vertpred]];
	[p->vertAttachTop addObject:[NSNumber numberWithBool:p->vertFirst]];
	[p->vertAttachBottom addObject:[NSNumber numberWithBool:p->vertLast]];
	[p->views setObject:self->b forKey:key];
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
