// 9 april 2015
#import "uipriv_darwin.h"

@interface uiLabelNSTextField : NSTextField
@property uiControl *uiC;
@end

@implementation uiLabelNSTextField

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(self.uiC, [self superview])) {
		[self setTarget:nil];
		self.uiC = NULL;
	}
	[super viewDidMoveToSuperview];
}

@end

uiControl *uiNewLabel(const char *text)
{
	uiControl *c;
	uiLabelNSTextField *l;

	c = uiDarwinNewControl([uiLabelNSTextField class], NO, NO);
	l = (uiLabelNSTextField *) uiControlHandle(c);
	l.uiC = c;

	[l setStringValue:toNSString(text)];
	[l setEditable:NO];
	[l setSelectable:NO];
	[l setDrawsBackground:NO];
	finishNewTextField((NSTextField *) l, NO);

	return l.uiC;
}

char *uiLabelText(uiControl *c)
{
	uiLabelNSTextField *t;

	t = (uiLabelNSTextField *) uiControlHandle(c);
	return uiDarwinNSStringToText([t stringValue]);
}

void uiLabelSetText(uiControl *c, const char *text)
{
	uiLabelNSTextField *t;

	t = (uiLabelNSTextField *) uiControlHandle(c);
	[t setStringValue:toNSString(text)];
}
