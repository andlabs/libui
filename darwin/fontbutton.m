// 14 april 2016
#import "uipriv_darwin.h"
#import "attrstr.h"

@interface uiprivFontButton : NSButton {
	uiFontButton *libui_b;
	NSFont *libui_font;
}
- (id)initWithFrame:(NSRect)frame libuiFontButton:(uiFontButton *)b;
- (void)updateFontButtonLabel;
- (IBAction)fontButtonClicked:(id)sender;
- (void)activateFontButton;
- (void)deactivateFontButton:(BOOL)activatingAnother;
- (void)deactivateOnClose:(NSNotification *)note;
- (void)getfontdesc:(uiFontDescriptor *)uidesc;
@end

// only one may be active at one time
static uiprivFontButton *activeFontButton = nil;

struct uiFontButton {
	uiDarwinControl c;
	uiprivFontButton *button;
	void (*onChanged)(uiFontButton *, void *);
	void *onChangedData;
};

@implementation uiprivFontButton

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

- (void)dealloc
{
	// clean up notifications
	if (activeFontButton == self)
		[self deactivateFontButton:NO];
	[self->libui_font release];
	[super dealloc];
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
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(deactivateOnClose:)
		name:NSWindowWillCloseNotification
		object:[NSFontPanel sharedFontPanel]];
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
	[[NSNotificationCenter defaultCenter] removeObserver:self
		name:NSWindowWillCloseNotification
		object:[NSFontPanel sharedFontPanel]];
	[self setState:NSOffState];
}

- (void)deactivateOnClose:(NSNotification *)note
{
	[self deactivateFontButton:NO];
}

- (void)changeFont:(id)sender
{
	NSFontManager *fm;
	NSFont *old;
	uiFontButton *b = self->libui_b;

	fm = (NSFontManager *) sender;
	old = self->libui_font;
	self->libui_font = [sender convertFont:self->libui_font];
	// do this even if it returns the same; we don't own anything that isn't from a new or alloc/init
	[self->libui_font retain];
	// do this second just in case
	[old release];
	[self updateFontButtonLabel];
	(*(b->onChanged))(b, b->onChangedData);
}

- (NSUInteger)validModesForFontPanel:(NSFontPanel *)panel
{
	return NSFontPanelFaceModeMask |
		NSFontPanelSizeModeMask |
		NSFontPanelCollectionModeMask;
}

- (void)getfontdesc:(uiFontDescriptor *)uidesc
{
	CTFontRef ctfont;
	CTFontDescriptorRef ctdesc;

	ctfont = (CTFontRef) (self->libui_font);
	ctdesc = CTFontCopyFontDescriptor(ctfont);
	uiprivFontDescriptorFromCTFontDescriptor(ctdesc, uidesc);
	CFRelease(ctdesc);
	uidesc->Size = CTFontGetSize(ctfont);
}

@end

uiDarwinControlAllDefaults(uiFontButton, button)

// we do not want font change events to be sent to any controls other than the font buttons
// see main.m for more details
BOOL uiprivFontButtonInhibitSendAction(SEL sel, id from, id to)
{
	if (sel != @selector(changeFont:))
		return NO;
	return ![to isKindOfClass:[uiprivFontButton class]];
}

// we do not want NSFontPanelValidation messages to be sent to any controls other than the font buttons when a font button is active
// see main.m for more details
BOOL uiprivFontButtonOverrideTargetForAction(SEL sel, id from, id to, id *override)
{
	if (activeFontButton == nil)
		return NO;
	if (sel != @selector(validModesForFontPanel:))
		return NO;
	*override = activeFontButton;
	return YES;
}

// we also don't want the panel to be usable when there's a dialog running; see stddialogs.m for more details on that
// unfortunately the panel seems to ignore -setWorksWhenModal: so we'll have to do things ourselves
@interface uiprivNonModalFontPanel : NSFontPanel
@end

@implementation uiprivNonModalFontPanel

- (BOOL)worksWhenModal
{
	return NO;
}

@end

void uiprivSetupFontPanel(void)
{
	[NSFontManager setFontPanelFactory:[uiprivNonModalFontPanel class]];
}

static void defaultOnChanged(uiFontButton *b, void *data)
{
	// do nothing
}

void uiFontButtonFont(uiFontButton *b, uiFontDescriptor *desc)
{
	[b->button getfontdesc:desc];
}

void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data)
{
	b->onChanged = f;
	b->onChangedData = data;
}

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	uiDarwinNewControl(uiFontButton, b);

	b->button = [[uiprivFontButton alloc] initWithFrame:NSZeroRect libuiFontButton:b];
	uiDarwinSetControlFont(b->button, NSRegularControlSize);

	uiFontButtonOnChanged(b, defaultOnChanged, NULL);

	return b;
}

void uiFreeFontButtonFont(uiFontDescriptor *desc)
{
	// TODO ensure this is synchronized with fontmatch.m
	uiFreeText((char *) (desc->Family));
}
