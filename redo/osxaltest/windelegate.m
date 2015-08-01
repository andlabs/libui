// 1 august 2015
#import "osxaltest.h"

@implementation tWindowDelegate {
	id<tControl> c;
}

- (id)init
{
	self = [super init];
	if (self)
		self->c = NULL;
	return self;
}

- (void)tSetControl:(id<tControl>)cc
{
	self->c = cc;
}

- (void)tRelayout
{
	if (self->c == NULL)
		return;
	// TODO
}

@end
