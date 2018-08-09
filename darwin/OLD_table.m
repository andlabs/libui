// 21 june 2016
#import "uipriv_darwin.h"

// TODOs
// - header cell seems off
// - background color shows up for a line or two below selection
// - editable NSTextFields have no intrinsic width
// - is the Y position of checkbox cells correct?

@implementation tablePart

- (NSView *)mkView:(uiTableModel *)m row:(int)row
{
	// if stretchy, don't hug, otherwise hug forcibly
	if (self.expand)
		[view setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
	else
		[view setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
}

@end

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn *c;

	c = uiprivNew(uiTableColumn);
	c->c = [[tableColumn alloc] initWithIdentifier:@""];
	c->c.libui_col = c;
	// via Interface Builder
	[c->c setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	// 10.10 adds -[NSTableColumn setTitle:]; before then we have to do this
	[[c->c headerCell] setStringValue:uiprivToNSString(name)];
	// TODO is this sufficient?
	[[c->c headerCell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	c->parts = [NSMutableArray new];
	[t->tv addTableColumn:c->c];
	return c;
}
