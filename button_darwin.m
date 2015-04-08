// 7 april 2015
#import "uipriv_darwin.h"

@interface uiNSButton : NSButton <uiFreeOnDealloc>
@property uiControl *uiC;
@property void (*uiOnClicked)(uiControl *, void *);
@property void *uiOnClickedData;
@property NSMutableArray *uiFreeList;
@end

@implementation uiNSButton

uiLogObjCClassAllocations(uiDoFreeOnDealloc(self.uiFreeList);)
uiFreeOnDeallocImpl

- (IBAction)uiButtonClicked:(id)sender
{
	(*(self.onClicked))(self.c, self.onClickedData);
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
	b.c = c;

	[b setTitle:toNSString(text)];
	[b setButtonType:NSMomentaryPushInButton];
	[b setBordered:YES];
	[b setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont((NSControl *) bb);

	[b setTarget:b];
	[b setAction:@selector(uiButtonClicked:)];

	b.onClicked = defaultOnClicked;

	return b.c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	button *b;

	b = (uiNSButton *) uiControlHandle(c);
	b.onClicked = f;
	b.onClickedData = data;
}
