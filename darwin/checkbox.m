// 14 august 2015
#import "uipriv_darwin.h"

struct uiCheckbox {
	uiDarwinControl c;
	NSButton *button;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

@interface checkboxDelegateClass : NSObject {
	uiprivMap *buttons;
}
- (IBAction)onToggled:(id)sender;
- (void)registerCheckbox:(uiCheckbox *)c;
- (void)unregisterCheckbox:(uiCheckbox *)c;
@end

@implementation checkboxDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->buttons = uiprivNewMap();
	return self;
}

- (void)dealloc
{
	uiprivMapDestroy(self->buttons);
	[super dealloc];
}

- (IBAction)onToggled:(id)sender
{
	uiCheckbox *c;

	c = (uiCheckbox *) uiprivMapGet(self->buttons, sender);
	(*(c->onToggled))(c, c->onToggledData);
}

- (void)registerCheckbox:(uiCheckbox *)c
{
	uiprivMapSet(self->buttons, c->button, c);
	[c->button setTarget:self];
	[c->button setAction:@selector(onToggled:)];
}

- (void)unregisterCheckbox:(uiCheckbox *)c
{
	[c->button setTarget:nil];
	uiprivMapDelete(self->buttons, c->button);
}

@end

static checkboxDelegateClass *checkboxDelegate = nil;

uiDarwinControlAllDefaultsExceptDestroy(uiCheckbox, button)

static void uiCheckboxDestroy(uiControl *cc)
{
	uiCheckbox *c = uiCheckbox(cc);

	[checkboxDelegate unregisterCheckbox:c];
	[c->button release];
	uiFreeControl(uiControl(c));
}

char *uiCheckboxText(uiCheckbox *c)
{
	return uiDarwinNSStringToText([c->button title]);
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	[c->button setTitle:uiprivToNSString(text)];
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

int uiCheckboxChecked(uiCheckbox *c)
{
	return [c->button state] == NSOnState;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	NSInteger state;

	state = NSOnState;
	if (!checked)
		state = NSOffState;
	[c->button setState:state];
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	uiDarwinNewControl(uiCheckbox, c);

	c->button = [[NSButton alloc] initWithFrame:NSZeroRect];
	[c->button setTitle:uiprivToNSString(text)];
	[c->button setButtonType:NSSwitchButton];
	// doesn't seem to have an associated bezel style
	[c->button setBordered:NO];
	[c->button setTransparent:NO];
	uiDarwinSetControlFont(c->button, NSRegularControlSize);

	if (checkboxDelegate == nil) {
		checkboxDelegate = [[checkboxDelegateClass new] autorelease];
		[uiprivDelegates addObject:checkboxDelegate];
	}
	[checkboxDelegate registerCheckbox:c];
	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	return c;
}
