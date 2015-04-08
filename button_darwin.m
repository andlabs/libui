// 7 april 2015
#import "uipriv_darwin.h"

@interface uiNSButton : NSButton
@property uiControl *uiC;
@property void (*uiOnClicked)(uiControl *, void *);
@property void *uiOnClickedData;
@property NSMutableArray *uiFreeList;
@end

@implementation uiNSButton

- (void)dealloc
{
	uiDarwinControlFree(self.uiC);
	[super dealloc];
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

// TODO destruction
uiControl *uiNewButton(const char *text)
{
	uiControl *c;
	uiNSButton *b;

	c = uiDarwinNewControl([uiNSButton class], NO, NO, NULL);
	b = (uiNSButton *) uiControlHandle(c);
	b.uiC = c;

	[b setTitle:toNSString(text)];
	[b setButtonType:NSMomentaryPushInButton];
	[b setBordered:YES];
	[b setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont((NSControl *) bb);

	[b setTarget:b];
	[b setAction:@selector(uiButtonClicked:)];

	b.uiOnClicked = defaultOnClicked;

	return b.c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	button *b;

	b = (uiNSButton *) uiControlHandle(c);
	b.uiOnClicked = f;
	b.uiOnClickedData = data;
}
