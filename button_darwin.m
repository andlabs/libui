// 7 april 2015
#import "uipriv_darwin.h"

@interface uiNSButton : NSButton
@property uiControl *uiC;
@property void (*uiOnClicked)(uiControl *, void *);
@property void *uiOnClickedData;
@end

@implementation uiNSButton

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(self.uiC, [self superview])) {
		[self setTarget:nil];
		self.uiC = NULL;
	}
	[super viewDidMoveToSuperview];
}

- (IBAction)uiButtonClicked:(id)sender
{
	(*(self.uiOnClicked))(self.uiC, self.uiOnClickedData);
}

@end

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewButton(const char *text)
{
	uiControl *c;
	uiNSButton *b;

	c = uiDarwinNewControl([uiNSButton class], NO, NO);
	b = (uiNSButton *) uiControlHandle(c);
	b.uiC = c;

	[b setTitle:toNSString(text)];
	[b setButtonType:NSMomentaryPushInButton];
	[b setBordered:YES];
	[b setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont((NSControl *) b);

	[b setTarget:b];
	[b setAction:@selector(uiButtonClicked:)];

	b.uiOnClicked = defaultOnClicked;

	return b.uiC;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	uiNSButton *b;

	b = (uiNSButton *) uiControlHandle(c);
	b.uiOnClicked = f;
	b.uiOnClickedData = data;
}
