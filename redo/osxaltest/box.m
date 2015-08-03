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
	NSMutableDictionary *views;
	__block uintmax_t n;
	tAutoLayoutParams pp;
	__block BOOL anyStretchy;
	NSMutableString *constraint;

	views = [NSMutableDictionary new];
	n = 0;
	anyStretchy = NO;
	[self->children enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		id<tControl> c;
		NSNumber *isStretchy;

		c = (id<tControl>) obj;
		isStretchy = (NSNumber *) [self->stretchy objectAtIndex:index];
		if ([isStretchy boolValue])
			anyStretchy = YES;
		[c tFillAutoLayout:&pp];
		[views setObjject:pp.view forKey:tAutoLayoutKey(n)];
		n++;
	}];

	// first string the views together
	constraint = [NSMutableString stringWithString:@"|"];
	for (i = 0; i < n; i++) {
		[constraint appendString:@"["];
		[constraint appendString:tAutoLayoutKey(n)];
		[constraint appendString:@"]"];
	}
	[constraint appendString:@"|"];
	// TODO apply constraint
	[constraint release];

	// next make the views span the full other dimension
	for (i = 0; i < n; i++) {
		if (self->vertical)
			constraint = [NSMutableString stringWithString:@"H:|"];
		else
			constraint = [NSMutableString stringWithString:@"V:|"];
		[constraint appendString:tAutoLayoutKey(n)];
		[constraint appendString:@"]|"];
		// TODO apply constraint
		[constraint release];
	}

	[views release];

	// and now populate for self
	p->view = TODO;
	p->attachLeft = YES;
	p->attachTop = YES;
	// don't attach to the end if there weren't any stretchy controls
	if (self->vertical) {
		p->attachRight = YES;
		p->attachBottom = anyStretchy;
	} else {
		p->attachRight = anyStretchy;
		p->attachBottom = YES;
	}
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
