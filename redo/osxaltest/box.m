// 31 july 2015
#import "osxaltest.h"

@implementation tBox {
	NSMutableArray *children;
	NSView *sv;
	BOOL vertical;
}

- (id)tInitVertical:(BOOL)vert
{
	self = [super init];
	if (self) {
		self->children = [NSMutableArray new];
		self->sv = nil;
		self->vertical = vert;
	}
	return self;
}

- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s
{
	// TODO
}

- (void)tAddToView:(NSView *)v
{
	self->sv = v;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSView *vv;

		vv = (NSView *) obj;
		[v addSubview:vv];
	}];
}

- (uintmax_t)tAddToAutoLayoutDictionary:(NSMutableDictionary *)views keyNumber:(uintmax_t)nn
{
	__block uintmax_t n = nn;

	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;

		c = (id<tControl>) obj;
		n = [c tAddToAutoLayoutDictionary:views keyNumber:n];
	}];
	return n;
}

- (NSString *)tBuildAutoLayoutConstraintsKeyNumber:(uintmax_t)nn
{
	NSMutableString *constraints;
	__block uintmax_t n = nn;

	if (self->vertical)
		constraints = [NSMutableString stringWithString:@"V:"];
	else
		constraints = [NSMutableString stringWithString:@"H:"];
	[constraints appendString:@"|"];
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		NSString *thisView;

		// TODO have every control do this
		[constraints appendString:tAutoLayoutKey(n)];
		n++;
	}];
	[constraints appendString:@"|"];
	return constraints;
	// TODOs:
	// - lateral dimension: for each view of n+1, make other dimension next to first n
	// 	this way, subelement views get positioned right
}

@end
