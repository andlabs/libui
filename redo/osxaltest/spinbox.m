// 31 july 2015
#import "osxaltest.h"

@implementation tSpinbox {
	NSTextField *t;
	NSStepper *s;
}

- (id)init
{
	self = [super init];
	if (self) {
		self->t = [[NSTextField alloc] initWithFrame:NSZeroRect];
		[self->t setSelectable:YES];
		[self->t setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->t setBordered:NO];
		[self->t setBezelStyle:NSTextFieldSquareBezel];
		[self->t setBezeled:YES];
		[[self->t cell] setLineBreakMode:NSLineBreakByClipping];
		[[self->t cell] setScrollable:YES];
		[self->t setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->s = [[NSStepper alloc] initWithFrame:NSZeroRect];
		[self->s setIncrement:1];
		[self->s setValueWraps:NO];
		[self->s setAutorepeat:YES];
		[self->s setTranslatesAutoresizingMaskIntoConstraints:NO];
	}
	return self;
}

- (void)tAddToView:(NSView *)v
{
	[v addSubview:self->t];
	[v addSubview:self->s];
}

- (void)tFillAutoLayoutHorz:(NSMutableArray *)horz
	vert:(NSMutableArray *)vert
	extra:(NSMutableArray *)extra
	extraVert:(NSMutableArray *)extraVert
	views:(NSMutableDictionary *)views
	first:(uintmax_t *)n
{
	NSString *keyt;
	NSString *keys;

	keyt = tAutoLayoutKey(*n);
	keys = tAutoLayoutKey(*n + 1);
	*n += 2;
	[horz addObject:[NSString stringWithFormat:@"[%@]-[%@]", keyt, keys]];
	[vert addObject:[NSString stringWithFormat:@"[%@]", keyt]];
	[vert addObject:[NSString stringWithFormat:@"[%@]", keys]];
	[views setObject:self->t forKey:keyt];
	[views setObject:self->s forKey:keys];
}

@end
