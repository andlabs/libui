// 9 april 2015
#import "uipriv_darwin.h"

@interface uiLabelNSTextField : NSTextField
@property uiLabel *uiL;
@end

@implementation uiLabelNSTextField

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(uiControl(self.uiL), [self superview])) {
		[self setTarget:nil];
		self.uiL = NULL;
	}
	[super viewDidMoveToSuperview];
}

@end

static char *labelText(uiLabel *l)
{
	uiLabelNSTextField *t;

	t = (uiLabelNSTextField *) uiControlHandle(uiControl(l));
	return uiDarwinNSStringToText([t stringValue]);
}

static void labelSetText(uiLabel *l, const char *text)
{
	uiLabelNSTextField *t;

	t = (uiLabelNSTextField *) uiControlHandle(uiControl(l));
	[t setStringValue:toNSString(text)];
}

uiControl *uiNewLabel(const char *text)
{
	uiLabel *l;
	uiLabelNSTextField *t;

	l = uiNew(uiLabel);

	uiDarwinNewControl(uiControl(l), [uiLabelNSTextField class], NO, NO);
	t = (uiLabelNSTextField *) uiControlHandle(c);

	[t setStringValue:toNSString(text)];
	[t setEditable:NO];
	[t setSelectable:NO];
	[t setDrawsBackground:NO];
	finishNewTextField((NSTextField *) t, NO);

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	t.uiC = l;

	return t.uiC;
}
