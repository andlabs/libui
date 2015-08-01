// 1 august 2015
#import "osxaltest.h"

@implementation tWindowDelegate {
	NSObject<tControl> *c;
}

- (id)init
{
	self = [super init];
	if (self)
		self->c = NULL;
	return self;
}

- (void)tSetControl:(NSObject<tControl> *)cc
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
