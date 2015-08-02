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

// TODO MASSIVE CLEANUP and comments everywhere too
- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSMutableArray *subhorz, *subvert;
	NSMutableArray *subhorzleft, *subhorzright;
	NSMutableArray *subverttop, *subvertbottom;
	uintmax_t *first;
	NSUInteger i;
	NSMutableString *out;
	tAutoLayoutParams pp;
	NSMutableArray *primaryin, *primaryout;
	BOOL primaryinstart, primaryinend;
	NSMutableArray *primaryoutstart, *primaryoutend;
	NSMutableArray *secondaryin, *secondaryout;
	NSMutableArray *secondaryinstart, *secondaryinend;
	NSMutableArray *secondaryoutstart, *secondaryoutend;

	first = (uintmax_t *) malloc([self->children count] * sizeof (uintmax_t));
	if (first == NULL)
		abort();
	subhorz = [NSMutableArray new];
	subhorzleft = [NSMutableArray new];
	subhorzright = [NSMutableArray new];
	subvert = [NSMutableArray new];
	subverttop = [NSMutableArray new];
	subvertbottom = [NSMutableArray new];

	pp.horz = subhorz;
	pp.horzAttachLeft = subhorzleft;
	pp.horzAttachRight = subhorzright;
	pp.vert = subvert;
	pp.vertAttachTop = subverttop;
	pp.vertAttachBottom = subvertbottom;
	pp.views = p->views;
	pp.n = p->n;
	pp.stretchyVert = self->vertical;
	pp.firstStretchy = TRUE;
	for (i = 0; i < [self->children count]; i++) {
		id<tControl> cur;
		NSNumber *isStretchy;

		first[i] = pp.n;
		cur = (id<tControl>) [self->children objectAtIndex:i];
		isStretchy = (NSNumber *) [self->stretchy objectAtIndex:i];
		pp.stretchy = [isStretchy boolValue];
		if (self->vertical) {
			pp.vertFirst = p->vertFirst && i == 0;
			pp.vertLast = p->vertLast && i == ([self->children count] - 1);
			pp.horzFirst = p->horzFirst;
			pp.horzLast = p->horzLast;
		} else {
			pp.horzFirst = p->horzFirst && i == 0;
			pp.horzLast = p->horzLast && i == ([self->children count] - 1);
			pp.vertFirst = p->vertFirst;
			pp.vertLast = p->vertLast;
		}
		[cur tFillAutoLayout:&pp];
		if (pp.stretchy && pp.firstStretchy) {
			pp.firstStretchy = FALSE;
			pp.stretchyTo = first[i];
		}
	}
	p->n = pp.n;

	out = [NSMutableString new];
	primaryin = subhorz;
	primaryinstart = p->horzFirst;
	primaryinend = p->horzLast;
	primaryout = p->horz;
	primaryoutstart = p->horzAttachLeft;
	primaryoutend = p->horzAttachRight;
	secondaryin = subvert;
	secondaryinstart = subverttop;
	secondaryinend = subvertbottom;
	secondaryout = p->vert;
	secondaryoutstart = p->vertAttachTop;
	secondaryoutend = p->vertAttachBottom;
	if (self->vertical) {
		primaryin = subvert;
		primaryinstart = p->vertFirst;
		primaryinend = p->vertLast;
		primaryout = p->vert;
		primaryoutstart = p->vertAttachTop;
		primaryoutend = p->vertAttachBottom;
		secondaryin = subhorz;
		secondaryinstart = subhorzleft;
		secondaryinend = subhorzright;
		secondaryout = p->horz;
		secondaryoutstart = p->horzAttachLeft;
		secondaryoutend = p->horzAttachRight;
	}
	[primaryin enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
//TODO		if (index != 0)
//TODO			[out appendString:@"-"];
		[out appendString:((NSString *) obj)];
	}];
	[primaryout addObject:out];
	[primaryoutstart addObject:[NSNumber numberWithBool:primaryinstart]];
	[primaryoutend addObject:[NSNumber numberWithBool:primaryinend]];
	[secondaryout addObjectsFromArray:secondaryin];
	[secondaryoutstart addObjectsFromArray:secondaryinstart];
	[secondaryoutend addObjectsFromArray:secondaryinend];

	[subhorz release];
	[subhorzleft release];
	[subhorzright release];
	[subvert release];
	[subverttop release];
	[subvertbottom release];
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
