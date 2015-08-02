// 31 july 2015
#import "osxaltest.h"

@implementation tBox {
	NSMutableArray *children;
	NSMutableArray *stretchy;
	NSView *sv;
	BOOL vertical;
	id<tControl> parent;
}

- (id)tInitVertical:(BOOL)vert
{
	self = [super init];
	if (self) {
		self->children = [NSMutableArray new];
		self->stretchy = [NSMutableArray new];
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
	[self->stretchy addObject:[NSNumber numberWithBool:s]];
	// TODO mark as needing relayout
	[self tRelayout];
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
	[self tRelayout];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSMutableArray *subhorz, *subvert;
	uintmax_t *first;
	NSUInteger i;
	NSMutableString *out;
	tAutoLayoutParams pp;
	NSMutableArray *primaryin, *primaryout;
	NSMutableArray *secondaryin, *secondaryout;

	first = (uintmax_t *) malloc([self->children count] * sizeof (uintmax_t));
	if (first == NULL)
		abort();
	subhorz = [NSMutableArray new];
	subvert = [NSMutableArray new];

	pp.horz = subhorz;
	pp.vert = subvert;
	pp.extra = p->extra;
	pp.extraVert = p->extraVert;
	pp.views = p->views;
	pp.n = p->n;
	pp.stretchyVert = self->vertical;
	pp.firstStretchy = TRUE;
	for (i = 0; i < [self->children count]; i++) {
		id<tControl> cur;
		NSNumber *stretchy;

		first[i] = pp.n;
		cur = (id<tControl>) [self->children objectAtIndex:i];
		stretchy = (NSNumber *) [self->stretchy objectAtIndex:i];
		pp.stretchy = [stretchy boolValue];
		[cur tFillAutoLayout:&pp];
		if (pp.stretchy && pp.firstStretchy) {
			pp.firstStretchy = FALSE;
			pp.stretchyTo = first[i];
		}
	}
	p->n = pp.n;

	out = [NSMutableString new];
	primaryin = subhorz;
	primaryout = p->horz;
	secondaryin = subvert;
	secondaryout = p->vert;
	if (self->vertical) {
		primaryin = subvert;
		primaryout = p->vert;
		secondaryin = subhorz;
		secondaryout = p->horz;
	}
	[primaryin enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
//TODO		if (index != 0)
//TODO			[out appendString:@"-"];
		[out appendString:((NSString *) obj)];
	}];
	[primaryout addObject:out];
	[secondaryout addObjectsFromArray:secondaryin];

	[subhorz release];
	[subvert release];
	free(first);
}

// TODOs:
// - lateral dimension: for each view of n+1, make other dimension next to first n
// 	this way, subelement views get positioned right

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
