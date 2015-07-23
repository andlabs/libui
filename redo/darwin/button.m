// 10 june 2015
#include "uipriv_darwin.h"

// TODO reimplement CommitDestroy() on all of these

@interface buttonDelegate : NSObject {
	uiButton *b;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
}
- (IBAction)buttonClicked:(id)sender;
- (void)setButton:(uiButton *)newb;
- (void)setOnClicked:(void (*)(uiButton *, void *))f data:(void *)data;
@end

@implementation buttonDelegate

- (IBAction)buttonClicked:(id)sender
{
	(*(self->onClicked))(self->b, self->onClickedData);
}

- (void)setButton:(uiButton *)newb
{
	self->b = newb;
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
	buttonDelegate *delegate;
};

uiDefineControlType(uiButton, uiTypeButton, struct button)

static uintptr_t buttonHandle(uiControl *c)
{
	struct button *b = (struct button *) c;

	return (uintptr_t) (b->button);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
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

	b = (struct button *) uiNewControl(uiTypeButton());

	b->button = [[NSButton alloc] initWithFrame:NSZeroFrame];
	[b->button setTitle:toNSString(text)];
	[b->button setButtonType:NSMomentaryPushInButton];
	[b->button setBordered:YES];
	[b->button setBezelStyle:NSRoundedBezelStyle];
	uiDarwinMakeSingleViewControl(uiControl(b), b->button, YES);

	b->delegate = [buttonDelegate new];
	[b->button setTarget:b->delegate];
	[b->button setAction:@selector(buttonClicked:)];
	[b->delegate setButton:uiButton(b)];
	[b->delegate setOnClicked:defaultOnClicked data:NULL];

	uiControl(b)->Handle = buttonHandle;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
