// 4 august 2014
#import "uipriv_darwin.h"

// calling -[className] on the content views of NSWindow, NSTabItem, and NSBox all return NSView, so I'm assuming I just need to override these
// fornunately:
// - NSWindow resizing calls -[setFrameSize:] (but not -[setFrame:])
// - NSTab resizing calls both -[setFrame:] and -[setFrameSIze:] on the current tab
// - NSTab switching tabs calls both -[setFrame:] and -[setFrameSize:] on the new tab
// so we just override setFrameSize:
// thanks to mikeash and JtRip in irc.freenode.net/#macdev
@implementation uiContainer

uiLogObjCClassAllocations

- (void)viewDidMoveToSuperview
{
	if ([self superview] == nil)
		if (self.child != NULL) {
			uiControlDestroy(self.child);
			self.child = NULL;
		}
	[super viewDidMoveToSuperview];
}

- (void)setFrameSize:(NSSize)s
{
	[super setFrameSize:s];
	[self uiUpdateNow];
}

- (void)uiUpdateNow
{
	uiSizing d;

	if (self.child != NULL)
		(*(self.child->resize))(self.child, [self bounds].origin.y, [self bounds].origin.y, [self bounds].size.width, [self bounds].size.height, &d);
}

@end

void updateParent(uintptr_t parent)
{
	uiContainer *c;

	if (parent == 0)
		return;
	c = (uiContainer *) parent;
	[c uiUpdateNow];
}
