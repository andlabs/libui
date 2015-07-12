// 10 june 2015
#include "uipriv_darwin.h"

@interface checkboxDelegate : NSObject {
	uiCheckbox *c;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
}
- (IBAction)checkboxToggled:(id)sender;
- (void)setCheckbox:(uiCheckbox *)newc;
- (void)setOnToggled:(void (*)(uiCheckbox *, void *))f data:(void *)data;
@end

@implementation checkboxDelegate

- (IBAction)checkboxToggled:(id)sender
{
	(*(self->onToggled))(self->c, self->onToggledData);
}

- (void)setCheckbox:(uiCheckbox *)newc
{
	self->c = newc;
}

- (void)setOnToggled:(void (*)(uiCheckbox *, void *))f data:(void *)data
{
	self->onToggled = f;
	self->onToggledData = data;
}

@end

struct checkbox {
	uiCheckbox c;
	NSButton *checkbox;
	checkboxDelegate *delegate;
};

uiDefineControlType(uiCheckbox, uiTypeCheckbox, struct checkbox)

static uintptr_t checkboxHandle(uiControl *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return (uintptr_t) (c->checkbox);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static char *checkboxText(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return uiDarwinNSStringToText([c->checkbox title]);
}

static void checkboxSetText(uiCheckbox *cc, const char *text)
{
	struct checkbox *c = (struct checkbox *) cc;

	[c->checkbox setTitle:toNSString(text)];
}

static void checkboxOnToggled(uiCheckbox *cc, void (*f)(uiCheckbox *, void *), void *data)
{
	struct checkbox *c = (struct checkbox *) cc;

	[c->delegate setOnToggled:f data:data];
}

static int checkboxChecked(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return [c->checkbox state] == NSOnState;
}

static void checkboxSetChecked(uiCheckbox *cc, int checked)
{
	struct checkbox *c = (struct checkbox *) cc;
	NSInteger state;

	state = NSOnState;
	if (!checked)
		state = NSOffState;
	[c->checkbox setState:state];
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;

	c = (struct checkbox *) uiNewControl(uiTypeCheckbox());

	// TODO make a macro for the below
	c->checkbox = [[NSCheckbox alloc] initWithFrame:NSZeroRect];
	[c->checkbox setTitle:toNSString(text)];
	[c->checkbox setButtonType:NSSwitchButton];
	[c->checkbox setBordered:NO];
	uiDarwinMakeSingleViewControl(uiControl(c), c->checkbox, YES);

	c->delegate = [checkboxDelegate new];
	[c->checkbox setTarget:c->delegate];
	[c->checkbox setAction:@selector(checkboxToggled:)];
	[c->delegate setCheckbox:uiCheckbox(c)];
	[c->delegate setOnToggled:defaultOnToggled data:NULL];

	uiControl(c)->Handle = checkboxHandle;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
