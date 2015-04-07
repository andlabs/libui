// 7 april 2015
#import "uipriv_darwin.h"

@interface button : NSObject
@property uiControl *c;
@property void (*onClicked)(uiControl *, void *);
@property void *onClickedData;
@end

@implementation button

- (IBAction)buttonClicked:(id)sender
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
	button *b;
	NSButton *bb;

	b = [button new];
	b.c = uiDarwinNewControl([NSButton class], NO, NO, b);

	bb = (NSButton *) uiControlHandle(b.c);
	[bb setTitle:toNSString(text)];
	[bb setButtonType:NSMomentaryPushInButton];
	[bb setBordered:YES];
	[bb setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont((NSControl *) bb);

	[bb setTarget:b];
	[bb setAction:@selector(buttonClicked:)];

	b.onClicked = defaultOnClicked;

	return b.c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	button *b;

	b = (button *) uiDarwinControlData(c);
	b.onClicked = f;
	b.onClickedData = data;
}
