// 31 july 2015
#import "osxaltest.h"

@implementation tBox {
	NSMutableArray *children;
	NSView *sv;
	BOOL vertical;
	id<tControl> parent;
}

- (id)tInitVertical:(BOOL)vert
{
	self = [super init];
	if (self) {
		self->children = [NSMutableArray new];
		self->sv = nil;
		self->vertical = vert;
		self->parent = nil;
	}
	return self;
}

- (void)tAddControl:(id<tControl>)c stretchy:(BOOL)s
{
	if (self->sv != nil)
		[c tSetParent:self->parent addToView:self->sv];
	[self->children addObject:c];
	// TODO mark as needing relayout
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v
{
	self->parent = p;
	self->sv = v;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;

		c = (id<tControl>) obj;
		[c tSetParent:self->parent addToView:self->sv];
	}];
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
	NSMutableString *out;

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
	// TODO vertical
	out = [NSMutableString new];
	[subhorz enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
//TODO		if (index != 0)
//TODO			[out appendString:@"-"];
		[out appendString:((NSString *) obj)];
	}];
	[horz addObject:out];
	[vert addObjectsFromArray:subvert];
	[subhorz release];
	[subvert release];
	free(first);
}

// TODOs:
// - lateral dimension: for each view of n+1, make other dimension next to first n
// 	this way, subelement views get positioned right

@end
