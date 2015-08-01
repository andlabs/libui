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
	if (self->sv != nil)
		[c tAddToView:self->sv];
	[self->children addObject:c];
	// TODO mark as needing relayout
}

- (void)tAddToView:(NSView *)v
{
	self->sv = v;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;

		c = (id<tControl>) obj;
		[c tAddToView:self->sv];
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

- (void)tFillAutoLayoutHorz:(NSMutableArray *)horz
	vert:(NSMutableArray *)vert
	extra:(NSMutableArray *)extra
	extraVert:(NSMutableArray *)extraVert
	views:(NSMutableDictionary *)views
	first:(uintmax_t *)n
{
	NSMutableArray *subhorz, *subvert;
	uintmax_t *first;
	NSUInteger i;

	first = (uintmax_t *) malloc([self->children count] * sizeof (uintmax_t));
	if (first == NULL)
		abort();
	subhorz = [NSMutableArray new];
	subvert = [NSMutableArray new];
	for (i = 0; i < [self->children count]; i++) {
		id<tControl> cur;

		first[i] = *n;
		cur = (id<tControl>) [self->children objectAtIndex:i];
		[cur tFillAutoLayoutHorz:subhorz vert:subvert
			extra:extra extraVert:extraVert
			views:views first:n];
	}
	// TODO combine subhorz/subvert
	[subhorz release];
	[subvert release];
	free(first);
}

// TODOs:
// - lateral dimension: for each view of n+1, make other dimension next to first n
// 	this way, subelement views get positioned right

@end
