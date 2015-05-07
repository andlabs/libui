// 7 april 2015
#import "uipriv_darwin.h"

@interface uipButtonDelegate : NSObject {
	uiButton *b;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
}
- (IBAction)buttonClicked:(id)sender;
- (void)setButton:(uiButton *)b;
- (void)setOnClicked:(void (*)(uiButton *, void *))f data:(void *)data;
@end

@implementation uipButtonDelegate

- (IBAction)buttonClicked:(id)sender
{
	(*(self->onClicked))(self->b, self->onClickedData);
}

- (void)setButton:(uiButton *)b
{
	self->b = b;
}

- (void)setOnClicked:(void (*)(uiButton *, void *))f data:(void *)data
{
	self->onClicked = f;
	self->onClickedData = data;
}

@end

struct button {
	uiButton b;
	NSButton *button;
	uipButtonDelegate *delegate;
};

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static void destroy(void *data)
{
	struct button *b = (struct button *) data;

	[b->button setTarget:nil];
	[b->delegate release];
	uiFree(b);
}

static char *buttonText(uiButton *bb)
{
	struct button *b = (struct button *) bb;

	return uiDarwinNSStringToText([b->button title]);
}

static void buttonSetText(uiButton *bb, const char *text)
{
	struct button *b = (struct button *) bb;

	[b->button setTitle:toNSString(text)];
}

static void buttonOnClicked(uiButton *bb, void (*f)(uiButton *, void *), void *data)
{
	struct button *b = (struct button *) bb;

	[b->delegate setOnClicked:f data:data];
}

uiButton *uiNewButton(const char *text)
{
	struct button *b;

	b = uiNew(struct button);

	uiDarwinMakeControl(uiControl(b), [NSButton class], NO, NO, destroy, b);

	b->button = (NSButton *) uiControlHandle(uiControl(b));

	[b->button setTitle:toNSString(text)];
	[b->button setButtonType:NSMomentaryPushInButton];
	[b->button setBordered:YES];
	[b->button setBezelStyle:NSRoundedBezelStyle];
	setStandardControlFont(b->button);

	b->delegate = [uipButtonDelegate new];
	[b->button setTarget:b->delegate];
	[b->button setAction:@selector(buttonClicked:)];
	[b->delegate setButton:uiButton(b)];
	[b->delegate setOnClicked:defaultOnClicked data:NULL];

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
