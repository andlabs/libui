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

- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n
{
	[views setObject:self->t forKey:tAutoLayoutKey(n)];
	[views setObject:self->s forKey:tAutoLayoutKey(n + 1)];
	n += 2;
	return n;
}

- (void)tFillAutoLayoutHorz:(NSMutableString *)horz
	vert:(NSMutableString *)vert
	extra:(NSMutableArray *)extra
	extraVert:(NSMutableArray *)extraVert
	views:(NSMutableDictionary *)views
{
	[horz setString:@"[view0]-[view1]"];
	[vert setString:@"[view0]"];
	[views setObject:self->t forKey:@"view0"];
	[views setObject:self->s forKey:@"view1"];
}

@end
