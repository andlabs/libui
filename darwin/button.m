// 7 april 2015
#import "uipriv_darwin.h"


struct button {
	uiButton b;
	NSButton *button;
	buttonDelegate *delegate;
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


uiButton *uiNewButton(const char *text)
{
	struct button *b;

	b = uiNew(struct button);

	uiDarwinMakeControl(uiControl(b), [NSButton class], NO, NO, destroy, b);

	b->button = (NSButton *) uiControlHandle(uiControl(b));

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
