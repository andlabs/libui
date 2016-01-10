// 13 august 2015
#import "uipriv_darwin.h"

struct uiButton {
	uiDarwinControl c;
	NSButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

@interface buttonDelegateClass : NSObject {
	struct mapTable *buttons;
}
- (IBAction)onClicked:(id)sender;
- (void)registerButton:(uiButton *)b;
- (void)unregisterButton:(uiButton *)b;
@end

@implementation buttonDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->buttons = newMap();
	return self;
}

- (void)dealloc
{
	mapDestroy(self->buttons);
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	uiButton *b;

	b = (uiButton *) mapGet(self->buttons, sender);
	(*(b->onClicked))(b, b->onClickedData);
}

- (void)registerButton:(uiButton *)b
{
	mapSet(self->buttons, b->button, b);
	[b->button setTarget:self];
	[b->button setAction:@selector(onClicked:)];
}

- (void)unregisterButton:(uiButton *)b
{
	[b->button setTarget:nil];
	mapDelete(self->buttons, b->button);
}

@end

static buttonDelegateClass *buttonDelegate = nil;

uiDarwinDefineControlWithOnDestroy(
	uiButton,								// type name
	uiButtonType,							// type function
	button,								// handle
	[buttonDelegate unregisterButton:this];		// on destroy
)

char *uiButtonText(uiButton *b)
{
	return uiDarwinNSStringToText([b->button title]);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	[b->button setTitle:toNSString(text)];
	// this may result in the size of the button changing
	uiDarwinControlTriggerRelayout(uiDarwinControl(b));
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;

	b = (uiButton *) uiNewControl(uiButtonType());

	b->button = [[NSButton alloc] initWithFrame:NSZeroRect];
	[b->button setTitle:toNSString(text)];
	[b->button setButtonType:NSMomentaryPushInButton];
	[b->button setBordered:YES];
	[b->button setBezelStyle:NSRoundedBezelStyle];
	uiDarwinSetControlFont(b->button, NSRegularControlSize);

	if (buttonDelegate == nil) {
		buttonDelegate = [buttonDelegateClass new];
		[delegates addObject:buttonDelegate];
	}
	[buttonDelegate registerButton:b];
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiDarwinFinishNewControl(b, uiButton);

	return b;
}
