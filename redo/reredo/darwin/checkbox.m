// 14 august 2015
#import "uipriv_darwin.h"

struct uiCheckbox {
	uiDarwinControl c;
	NSButton *button;		// TODO rename to checkbox?
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

@interface checkboxDelegateClass : NSObject {
	NSMutableDictionary *buttons;			// TODO rename to checkboxes?
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
		self->buttons = [NSMutableDictionary new];
	return self;
}

// TODO have this called
- (void)dealloc
{
	if ([self->buttons count] != 0)
		complain("attempt to destroy shared checkbox delegate but checkboxes are still registered to it");
	[self->buttons release];
	[super dealloc];
}

- (IBAction)onToggled:(id)sender
{
	NSValue *v;
	uiCheckbox *c;

	v = (NSValue *) [self->buttons objectForKey:sender];
	c = (uiCheckbox *) [v pointerValue];
	(*(c->onToggled))(c, c->onToggledData);
}

- (void)registerCheckbox:(uiCheckbox *)c
{
	[self->buttons setObject:[NSValue valueWithPointer:c]
		forKey:c->button];
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
	uiControlQueueResize(uiControl(c));
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static int uiCheckboxChecked(uiCheckbox *c)
{
	return [c->button state] == NSOnState;
}

static void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	NSInteger state;

	state = NSOnState;
	if (!checked)
		state = NSOffState;
	[c->button setState:state];
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	c->button = [[NSButton alloc] initWithFrame:NSZeroRect];
	[c->button setTitle:toNSString(text)];
	[c->button setButtonType:NSSwitchButton];
	[c->button setBordered:NO];
	uiDarwinSetControlFont(c->button, NSControlSizeRegular);

	if (checkboxDelegate == nil)
		checkboxDelegate = [checkboxDelegateClass new];
	[checkboxDelegate registerCheckbox:c];
	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	uiDarwinFinishNewControl(c, uiCheckbox);

	return c;
}
