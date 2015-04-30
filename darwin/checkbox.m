// 7 april 2015
#import "uipriv_darwin.h"

@interface uipCheckboxDelegate : NSObject {
	uiCheckbox *c;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
}
- (IBAction)checkboxToggled:(id)sender;
- (void)setCheckbox:(uiCheckbox *)c;
- (void)setOnToggled:(void (*)(uiCheckbox *, void *))f data:(void *)data;
@end

@implementation uipCheckboxDelegate

- (IBAction)checkboxToggled:(id)sender
{
	(*(self->onToggled))(self->c, self->onToggledData);
}

- (void)setCheckbox:(uiCheckbox *)c
{
	self->c = c;
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
	uipCheckboxDelegate *delegate;
};

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static void destroy(void *data)
{
	struct checkbox *c = (struct checkbox *) data;

	[c->checkbox setTarget:nil];
	[c->delegate release];
	uiFree(c);
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

	c = uiNew(struct checkbox);

	uiDarwinNewControl(uiControl(c), [NSButton class], NO, NO, destroy, c);

	c->checkbox = (NSButton *) uiControlHandle(uiControl(c));

	[c->checkbox setTitle:toNSString(text)];
	[c->checkbox setButtonType:NSSwitchButton];
	[c->checkbox setBordered:NO];
	setStandardControlFont(c->checkbox);

	c->delegate = [uipCheckboxDelegate new];
	[c->checkbox setTarget:c->delegate];
	[c->checkbox setAction:@selector(checkboxToggled:)];
	[c->delegate setCheckbox:uiCheckbox(c)];
	[c->delegate setOnToggled:defaultOnToggled data:NULL];

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
