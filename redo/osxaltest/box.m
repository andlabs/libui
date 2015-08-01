// 31 july 2015
#import "osxaltest.h"

@interface tBox : NSObject<tControl> {
	NSMutableArray *children;
	NSView *sv;
}
@end

@implementation tBox

- (id)init
{
	self = [super init];
	if (self) {
		self->children = [NSMutableArray new];
		self->sv = nil;
	}
	return self;
}

- (void)addControl:(NSObject<tControl> *)c stretchy:(BOOL)s
{
	// TODO
}

- (void)tAddToView:(NSView *)v
{
	self->sv = v;
	// TODO
}

- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)n
{
	// TODO
}

// TODO build auto layout constraints

@end
