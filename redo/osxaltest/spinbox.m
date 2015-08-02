// 31 july 2015
#import "osxaltest.h"

// leave a whole lot of space around the alignment rect, just to be safe
@interface tSpinboxContainer : NSView
@end

@implementation tSpinboxContainer

- (NSEdgeInsets)alignmentRectInsets
{
	return NSEdgeInsetsMake(50, 50, 50, 50);
}

@end

@implementation tSpinbox {
	tSpinboxContainer *c;
	NSTextField *t;
	NSStepper *s;
	id<tControl> parent;
}

- (id)init
{
	self = [super init];
	if (self) {
		NSMutableDictionary *views;
		NSArray *constraints;

		self->c = [[tSpinboxContainer alloc] initWithFrame:NSZeroRect];
		[self->c setTranslatesAutoresizingMaskIntoConstraints:NO];

		self->t = [[NSTextField alloc] initWithFrame:NSZeroRect];
		[self->t setSelectable:YES];
		[self->t setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
		[self->t setBordered:NO];
		[self->t setBezelStyle:NSTextFieldSquareBezel];
		[self->t setBezeled:YES];
		[[self->t cell] setLineBreakMode:NSLineBreakByClipping];
		[[self->t cell] setScrollable:YES];
		[self->t setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self->c addSubview:self->t];

		self->s = [[NSStepper alloc] initWithFrame:NSZeroRect];
		[self->s setIncrement:1];
		[self->s setValueWraps:NO];
		[self->s setAutorepeat:YES];
		[self->s setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self->c addSubview:self->s];

		views = [NSMutableDictionary new];
		[views setObject:self->t forKey:@"t"];
		[views setObject:self->s forKey:@"s"];

		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[t]-[s]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[t]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];
		constraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[s]|" options:0 metrics:nil views:views];
		[self->c addConstraints:constraints];

		[views release];

		self->parent = nil;
	}
	return self;
}

- (void)tSetParent:(id<tControl>)p addToView:(NSView *)v
{
	self->parent = p;
	[v addSubview:self->c];
}

- (void)tFillAutoLayout:(tAutoLayoutParams *)p
{
	NSString *key;
	NSString *horzpred, *vertpred;

	key = tAutoLayoutKey(p->n);
	p->n++;
	horzpred = @"(==96)";		// TODO only the entry
	vertpred = @"";
	if (p->stretchy) {
		NSString *predicate;

		if (p->firstStretchy)
			predicate = @"(>=0)";
		else
			predicate = [NSString stringWithFormat:@"(==%@)", tAutoLayoutKey(p->stretchyTo)];
		if (p->stretchyVert)
			vertpred = predicate;
		else
			horzpred = predicate;
	}
	[p->horz addObject:[NSString stringWithFormat:@"[%@%@]", key, horzpred]];
	[p->horzAttachLeft addObject:@YES];
	[p->horzAttachRight addObject:@YES];
	[p->vert addObject:[NSString stringWithFormat:@"[%@%@]", key, vertpred]];
	[p->vertAttachTop addObject:@YES];
	[p->vertAttachBottom addObject:@YES];
	[p->views setObject:self->c forKey:key];
}

- (void)tRelayout
{
	if (self->parent != nil)
		[self->parent tRelayout];
}

@end
