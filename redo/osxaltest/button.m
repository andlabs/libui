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

- (void)tFillAutoLayoutHorz:(NSMutableArray *)horz
	vert:(NSMutableArray *)vert
	extra:(NSMutableArray *)extra
	extraVert:(NSMutableArray *)extraVert
	views:(NSMutableDictionary *)views
	first:(uintmax_t *)n
{
	NSString *key;

	key = tAutoLayoutKey(*n);
	(*n)++;
	[horz addObject:[NSString stringWithFormat:@"[%@]", key]];
	[vert addObject:[NSString stringWithFormat:@"[%@]", key]];
	[views setObject:self->b forKey:key];
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
