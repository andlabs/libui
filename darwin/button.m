// 7 april 2015
#import "uipriv_darwin.h"

@interface uiNSButton : NSButton
@property uiButton *uiB;
@property void (*uiOnClicked)(uiButton *, void *);
@property void *uiOnClickedData;
@end

@implementation uiNSButton

- (void)viewDidMoveToSuperview
{
	if (uiDarwinControlFreeWhenAppropriate(uiControl(self.uiB), [self superview])) {
		[self setTarget:nil];
		self.uiB = NULL;
	}
	[super viewDidMoveToSuperview];
}

- (IBAction)uiButtonClicked:(id)sender
{
	(*(self.uiOnClicked))(self.uiB, self.uiOnClickedData);
}

@end

static void defaultOnClicked(uiButton *c, void *data)
{
	// do nothing
}

static char *buttonText(uiButton *bb)
{
	uiNSButton *b;

	b = (uiNSButton *) uiControlHandle(uiControl(bb));
	return uiDarwinNSStringToText([b title]);
}

static void buttonSetText(uiButton *bb, const char *text)
{
	uiNSButton *b;

	b = (uiNSButton *) uiControlHandle(uiControl(bb));
	[b setTitle:toNSString(text)];
}

static void buttonOnClicked(uiButton *bb, void (*f)(uiButton *, void *), void *data)
{
	uiNSButton *b;

	b = (uiNSButton *) uiControlHandle(uiControl(bb));
	b.uiOnClicked = f;
	b.uiOnClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;
	uiNSButton *bb;

	b = uiNew(uiButton);

	uiDarwinNewControl(uiControl(b), [uiNSButton class], NO, NO);
	bb = (uiNSButton *) uiControlHandle(uiControl(b));

	[bb setTitle:toNSString(text)];
	[bb setButtonType:NSMomentaryPushInButton];
	[bb setBordered:YES];
	[bb setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont((NSControl *) bb);

	[bb setTarget:bb];
	[bb setAction:@selector(uiButtonClicked:)];

	bb.uiOnClicked = defaultOnClicked;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	bb.uiB = b;

	return bb.uiB;
}
