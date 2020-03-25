// 13 august 2015
#import "uipriv_darwin.h"
#import "../common/general.h"

@interface libui_NSButton : NSButton
@property int minWidth;
@property int minHeight;
- (id)initWithFrame:(NSRect)frameRect;
@end

@implementation libui_NSButton

- (id)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame: frameRect];
	if (self) {
		self.minHeight = -1;
		self.minWidth = -1;
	}
	return self;
}

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = max(self.minWidth, s.width);
	s.height = max(self.minHeight, s.height);
	return s;
}

@end

struct uiButton {
	uiDarwinControl c;
	NSButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

@interface buttonDelegateClass : NSObject {
	uiprivMap *buttons;
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
		self->buttons = uiprivNewMap();
	return self;
}

- (void)dealloc
{
	uiprivMapDestroy(self->buttons);
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	uiButton *b;

	b = (uiButton *) uiprivMapGet(self->buttons, sender);
	(*(b->onClicked))(b, b->onClickedData);
}

- (void)registerButton:(uiButton *)b
{
	uiprivMapSet(self->buttons, b->button, b);
	[b->button setTarget:self];
	[b->button setAction:@selector(onClicked:)];
}

- (void)unregisterButton:(uiButton *)b
{
	[b->button setTarget:nil];
	uiprivMapDelete(self->buttons, b->button);
}

@end

static buttonDelegateClass *buttonDelegate = nil;

uiDarwinControlAllDefaultsExceptDestroy(uiButton, button)

static void uiButtonDestroy(uiControl *c)
{
	uiButton *b = uiButton(c);

	[buttonDelegate unregisterButton:b];
	[b->button release];
	uiFreeControl(uiControl(b));
}

char *uiButtonText(uiButton *b)
{
	return uiDarwinNSStringToText([b->button title]);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	[b->button setTitle:uiprivToNSString(text)];
}

void uiButtonSetMinSize(uiButton *b, int width, int height)
{
	libui_NSButton *libui_btn = (libui_NSButton *)b->button;
	libui_btn.minWidth = width;
	libui_btn.minHeight = height;
}

void uiButtonPreferredSize(uiButton *b, int *width, int *height)
{
	NSSize s = b->button.intrinsicContentSize;
	*width = s.width;
	*height = s.height;
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

	uiDarwinNewControl(uiButton, b);

	b->button = [[libui_NSButton alloc] initWithFrame:NSZeroRect];
	[b->button setTitle:uiprivToNSString(text)];
	[b->button setButtonType:NSMomentaryPushInButton];
	[b->button setBordered:YES];
	[b->button setBezelStyle:NSRoundedBezelStyle];
	uiDarwinSetControlFont(b->button, NSRegularControlSize);

	if (buttonDelegate == nil) {
		buttonDelegate = [[buttonDelegateClass new] autorelease];
		[uiprivDelegates addObject:buttonDelegate];
	}
	[buttonDelegate registerButton:b];
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	return b;
}
