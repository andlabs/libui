// 14 april 2016
#import "uipriv_darwin.h"

@interface fontButton : NSButton {
	uiFontButton *libui_b;
	NSFont *libui_font;
}
- (id)initWithFrame:(NSRect)frame libuiFontButton:(uiFontButton *)b;
- (void)updateFontButtonLabel;
- (IBAction)fontButtonClicked:(id)sender;
- (void)activateFontButton;
- (void)deactivateFontButton:(BOOL)activatingAnother;
@end

// only one may be active at one time
static fontButton *activeFontButton = nil;

struct uiFontButton {
	uiDarwinControl c;
	fontButton *button;
};

uiDarwinDefineControl(
	uiFontButton,							// type name
	uiFontButtonType,						// type function
	button								// handle
)

@implementation fontButton

- (id)initWithFrame:(NSRect)frame libuiFontButton:(uiFontButton *)b
{
	self = [super initWithFrame:frame];
	if (self) {
		self->libui_b = b;

		// imitate a NSColorWell in appearance
		[self setButtonType:NSPushOnPushOffButton];
		[self setBordered:YES];
		[self setBezelStyle:NSShadowlessSquareBezelStyle];

		// default font values according to the CTFontDescriptor reference
		// this is autoreleased (thanks swillits in irc.freenode.net/#macdev)
		self->libui_font = [[NSFont fontWithName:@"Helvetica" size:12.0] retain];
		[self updateFontButtonLabel];

		// for when clicked
		[self setTarget:self];
		[self setAction:@selector(fontButtonClicked:)];
	}
	return self;
}

- (void)updateFontButtonLabel
{
	NSString *title;

	title = [NSString stringWithFormat:@"%@ %g",
		[self->libui_font displayName],
		[self->libui_font pointSize]];
	[self setTitle:title];
}

- (IBAction)fontButtonClicked:(id)sender
{
	if ([self state] == NSOnState)
		[self activateFontButton];
	else
		[self deactivateFontButton:NO];
}

- (void)activateFontButton
{
	NSFontManager *sfm;

	sfm = [NSFontManager sharedFontManager];
	if (activeFontButton != nil)
		[activeFontButton deactivateFontButton:YES];
	[sfm setTarget:self];
	[sfm setSelectedFont:self->libui_font isMultiple:NO];
	[sfm orderFrontFontPanel:self];
	activeFontButton = self;
	[self setState:NSOnState];
}

- (void)deactivateFontButton:(BOOL)activatingAnother
{
	NSFontManager *sfm;

	sfm = [NSFontManager sharedFontManager];
	[sfm setTarget:nil];
	if (!activatingAnother)
		[[NSFontPanel sharedFontPanel] orderOut:self];
	activeFontButton = nil;
	[self setState:NSOffState];
}

- (void)changeFont:(id)sender
{
	NSFontManager *fm;
	NSFont *old;

	fm = (NSFontManager *) sender;
	old = self->libui_font;
	self->libui_font = [sender convertFont:self->libui_font];
	// TODO do we get it back retained?
	// TODO is it even retained when we get it, regardless of value?
	if (self->libui_font != old)
		[old release];
	[self updateFontButtonLabel];
}

- (NSUInteger)validModesForFontPanel:(NSFontPanel *)panel
{
	return NSFontPanelFaceModeMask |
		NSFontPanelSizeModeMask |
		NSFontPanelCollectionModeMask;
}

@end

// we do not want font change events to be sent to any controls other than the font buttons
// see main.m for more details
BOOL fontButtonInhibitSendAction(SEL sel, id from, id to)
{
	if (sel != @selector(changeFont:))
		return NO;
	return ![to isKindOfClass:[fontButton class]];
}

// we do not want NSFontPanelValidation messages to be sent to any controls other than the font buttons when a font button is active
// see main.m for more details
BOOL fontButtonOverrideTargetForAction(SEL sel, id from, id to, id *override)
{
	if (activeFontButton == nil)
		return NO;
	if (sel != @selector(validModesForFontPanel:))
		return NO;
	*override = activeFontButton;
	return YES;
}

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	b = (uiFontButton *) uiNewControl(uiFontButtonType());

	b->button = [[fontButton alloc] initWithFrame:NSZeroRect libuiFontButton:b];
	uiDarwinSetControlFont(b->button, NSRegularControlSize);

	uiDarwinFinishNewControl(b, uiFontButton);

	return b;
}
