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
		[c tSetParent:self addToView:self->sv relayout:NO];
	[self->children addObject:c];
	[self->stretchy addObject:[NSNumber numberWithBool:s]];
	// TODO mark as needing relayout
	[self tRelayout];
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v relayout:(BOOL)relayout
{
	self->parent = p;
	self->sv = v;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;

		c = (id<tControl>) obj;
		[c tSetParent:self addToView:self->sv relayout:NO];
	}];
	if (relayout)
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
	tAutoLayoutParams pp;
	void (^buildPrimary)(NSMutableArray *in, BOOL first, BOOL last,
		NSMutableArray *out, NSMutableArray *outstart, NSMutableArray *outend);

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
	if (self->vertical) {
		pp.vertFirstStretchy = YES;
		pp.horzStretchy = YES;
		pp.horzFirstStretchy = YES;
	} else {
		pp.horzFirstStretchy = YES;
		pp.vertStretchy = YES;
		pp.vertFirstStretchy = YES;
	}
	for (i = 0; i < [self->children count]; i++) {
		id<tControl> cur;
		NSNumber *isStretchy;

		first[i] = pp.n;
		cur = (id<tControl>) [self->children objectAtIndex:i];
		isStretchy = (NSNumber *) [self->stretchy objectAtIndex:i];
		if (self->vertical) {
			pp.vertStretchy = [isStretchy boolValue];
			pp.vertFirst = p->vertFirst && i == 0;
			pp.vertLast = p->vertLast && i == ([self->children count] - 1);
			pp.horzFirst = p->horzFirst;
			pp.horzLast = p->horzLast;
		} else {
			pp.horzStretchy = [isStretchy boolValue];
			pp.horzFirst = p->horzFirst && i == 0;
			pp.horzLast = p->horzLast && i == ([self->children count] - 1);
			pp.vertFirst = p->vertFirst;
			pp.vertLast = p->vertLast;
		}
		[cur tFillAutoLayout:&pp];
		if (self->vertical) {
			if (pp.vertStretchy && pp.vertFirstStretchy) {
				pp.vertFirstStretchy = NO;
				pp.vertStretchyTo = first[i];
			}
		} else {
			if (pp.horzStretchy && pp.horzFirstStretchy) {
				pp.horzFirstStretchy = NO;
				pp.horzStretchyTo = first[i];
			}
		}
	}
	p->n = pp.n;

	buildPrimary = ^(NSMutableArray *in, BOOL first, BOOL last,
		NSMutableArray *out, NSMutableArray *outstart, NSMutableArray *outend) {
		NSMutableString *outstr;

		outstr = [NSMutableString new];
		[in enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
//TODO		if (index != 0)
//TODO			[outstr appendString:@"-"];
			[outstr appendString:((NSString *) obj)];
		}];
		[out addObject:outstr];
		[outstart addObject:[NSNumber numberWithBool:first]];
		[outend addObject:[NSNumber numberWithBool:last]];
	};

	if (self->vertical) {
		buildPrimary(subvert, p->vertFirst, p->vertLast,
			p->vert, p->vertAttachTop, p->vertAttachBottom);
		[p->horz addObjectsFromArray:subhorz];
		[p->horzAttachLeft addObjectsFromArray:subhorzleft];
		[p->horzAttachRight addObjectsFromArray:subhorzright];
	} else {
		buildPrimary(subhorz, p->horzFirst, p->horzLast,
			p->horz, p->horzAttachLeft, p->horzAttachRight);
		[p->vert addObjectsFromArray:subvert];
		[p->vertAttachTop addObjectsFromArray:subverttop];
		[p->vertAttachBottom addObjectsFromArray:subvertbottom];
	}

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
// - don't pin to end if no controls are stretchy

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
