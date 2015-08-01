// 31 july 2015
#import "osxaltest.h"

@implementation tButton {
	NSButton *b;
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
	}
	return self;
}

- (void)tAddToView:(NSView *)v
{
	[v addSubview:self->b];
}

- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n
{
	[views setObject:self->b forKey:tAutoLayoutKey(n)];
	n++;
	return n;
}

@end
