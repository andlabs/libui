// 7 april 2015
#import "uipriv_darwin.h"

@interface uiCheckboxNSButton : NSButton
@property uiCheckbox *uiC;
@property void (*uiOnToggled)(uiCheckbox *, void *);
@property void *uiOnToggledData;
@end

@implementation uiCheckboxNSButton

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(uiControl(self.uiC), [self superview])) {
		[self setTarget:nil];
		self.uiC = NULL;
	}
	[super viewDidMoveToSuperview];
}

- (IBAction)uiCheckboxToggled:(id)sender
{
	(*(self.uiOnToggled))(self.uiC, self.uiOnToggledData);
}

@end

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static char *checkboxText(uiCheckbox *c)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(uiControl(c));
	return uiDarwinNSStringToText([cc title]);
}

static void checkboxSetText(uiCheckbox *c, const char *text)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(uiControl(c));
	[cc setTitle:toNSString(text)];
}

static void checkboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(uiCheckbox(c));
	cc.uiOnToggled = f;
	cc.uiOnToggledData = data;
}

static int checkboxChecked(uiCheckbox *c)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(uiControl(c));
	return [cc state] == NSOnState;
}

static void checkboxSetChecked(uiCheckbox *c, int checked)
{
	uiCheckboxNSButton *cc;
	NSInteger state;

	cc = (uiCheckboxNSButton *) uiControlHandle(uiControl(c));
	state = NSOnState;
	if (!checked)
		state = NSOffState;
	[cc setState:state];
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;
	uiCheckboxNSButton *cc;

	c = uiNew(uiCheckbox);

	uiDarwinNewControl(uiControl(c), [uiCheckboxNSButton class], NO, NO);
	cc = (uiCheckboxNSButton *) uiControlHandle(c);

	[cc setTitle:toNSString(text)];
	[cc setButtonType:NSSwitchButton];
	[cc setBordered:NO];
	setStandardControlFont((NSControl *) cc);

	[cc setTarget:cc];
	[cc setAction:@selector(uiCheckboxToggled:)];

	cc.uiOnToggled = defaultOnToggled;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	cc.uiC = c;

	return cc.uiC;
}
