// 14 august 2015
#import "uipriv_darwin.h"

struct uiCheckbox {
	uiDarwinControl c;
	NSButton *button;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

@interface checkboxDelegateClass : NSObject {
	NSMapTable *buttons;
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
		self->buttons = newMap();
	return self;
}

- (void)dealloc
{
	if ([self->buttons count] != 0)
		complain("attempt to destroy shared checkbox delegate but checkboxes are still registered to it");
	[self->buttons release];
	[super dealloc];
}

- (IBAction)onToggled:(id)sender
{
	uiCheckbox *c;

	c = (uiCheckbox *) mapGet(self->buttons, sender);
	(*(c->onToggled))(c, c->onToggledData);
}

- (void)registerCheckbox:(uiCheckbox *)c
{
	mapSet(self->buttons, c->button, c);
	[c->button setTarget:self];
	[c->button setAction:@selector(onToggled:)];
}

- (void)unregisterCheckbox:(uiCheckbox *)c
{
	[c->button setTarget:nil];
	[self->buttons removeObjectForKey:c->button];
}

@end

static checkboxDelegateClass *checkboxDelegate = nil;

uiDarwinDefineControlWithOnDestroy(
	uiCheckbox,								// type name
	uiCheckboxType,							// type function
	button,									// handle
	[checkboxDelegate unregisterCheckbox:this];		// on destroy
)

char *uiCheckboxText(uiCheckbox *c)
{
	return uiDarwinNSStringToText([c->button title]);
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	[c->button setTitle:toNSString(text)];
	// this may result in the size of the checkbox changing
	// TODO something somewhere is causing this to corrupt some memory so that, for instance, page7b's mouseExited: never triggers on 10.11; figure out what
	// TODO is this related to map-related crashes?
	uiDarwinControlTriggerRelayout(uiDarwinControl(c));
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

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	c->button = [[NSButton alloc] initWithFrame:NSZeroRect];
	[c->button setTitle:toNSString(text)];
	[c->button setButtonType:NSSwitchButton];
	[c->button setBordered:NO];
	uiDarwinSetControlFont(c->button, NSRegularControlSize);

	if (checkboxDelegate == nil) {
		checkboxDelegate = [checkboxDelegateClass new];
		[delegates addObject:checkboxDelegate];
	}
	[checkboxDelegate registerCheckbox:c];
	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	uiDarwinFinishNewControl(c, uiCheckbox);

	return c;
}
