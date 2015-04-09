// 7 april 2015
#import "uipriv_darwin.h"

@interface uiCheckboxNSButton : NSButton
@property uiControl *uiC;
@property void (*uiOnToggled)(uiControl *, void *);
@property void *uiOnToggledData;
@end

@implementation uiCheckboxNSButton

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(self.uiC, [self superview])) {
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

static void defaultOnToggled(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewCheckbox(const char *text)
{
	uiControl *c;
	uiCheckboxNSButton *cc;

	c = uiDarwinNewControl([uiCheckboxNSButton class], NO, NO);
	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	cc.uiC = c;

	[cc setTitle:toNSString(text)];
	[cc setButtonType:NSSwitchButton];
	[cc setBordered:NO];
	setStandardControlFont((NSControl *) cc);

	[cc setTarget:cc];
	[cc setAction:@selector(uiCheckboxToggled:)];

	cc.uiOnToggled = defaultOnToggled;

	return cc.uiC;
}

char *uiCheckboxText(uiControl *c)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	return strdup(fromNSString([cc title]));
}

void uiCheckboxSetText(uiControl *c, const char *text)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	[cc setTitle:toNSString(text)];
}

void uiCheckboxOnToggled(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	cc.uiOnToggled = f;
	cc.uiOnToggledData = data;
}

int uiCheckboxChecked(uiControl *c)
{
	uiCheckboxNSButton *cc;

	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	return [cc state] == NSOnState;
}

void uiCheckboxSetChecked(uiControl *c, int checked)
{
	uiCheckboxNSButton *cc;
	NSInteger state;

	cc = (uiCheckboxNSButton *) uiControlHandle(c);
	state = NSOnState;
	if (!checked)
		state = NSOffState;
	[cc setState:state];
}
