// 8 december 2015
#import "uipriv_darwin.h"

struct uiMultilineEntry {
	uiDarwinControl c;
	NSScrollView *sv;
	NSTextView *tv;
	void (*onChanged)(uiMultilineEntry *, void *);
	void *onChangedData;
};

// TODO events

uiDarwinDefineControl(
	uiMultilineEntry,						// type name
	uiMultilineEntryType,					// type function
	sv									// handle
)

static void defaultOnChanged(uiMultilineEntry *e, void *data)
{
	// do nothing
}

char *uiMultilineEntryText(uiMultilineEntry *e)
{
	return uiDarwinNSStringToText([e->tv string]);
}

void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text)
{
	// TODO does this send a changed signal?
	[e->tv setString:toNSString(text)];
}

// TODO scroll to end?
void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text)
{
	// TODO better way?
	NSString *str;

	// TODO does this send a changed signal?
	str = [e->tv string];
	str = [str stringByAppendingString:toNSString(text)];
	[e->tv setString:str];
}

void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *e, void *data), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiMultilineEntryReadOnly(uiMultilineEntry *e)
{
	return [e->tv isEditable] == NO;
}

void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly)
{
	BOOL editable;

	editable = YES;
	if (readonly)
		editable = NO;
	[e->tv setEditable:editable];
}

uiMultilineEntry *uiNewMultilineEntry(void)
{
	uiMultilineEntry *e;

	e = (uiMultilineEntry *) uiNewControl(uiMultilineEntryType());

	e->sv = [[NSScrollView alloc] initWithFrame:NSZeroRect];
	// TODO verify against Interface Builder
	[e->sv setHasHorizontalScroller:NO];
	[e->sv setHasVerticalScroller:YES];
	[e->sv setAutohidesScrollers:YES];
	[e->sv setBorderType:NSBezelBorder];

	e->tv = [[NSTextView alloc] initWithFrame:NSZeroRect];
	// verified against Interface Builder, except for rich text options
	[e->tv setAllowsDocumentBackgroundColorChange:NO];
	[e->tv setBackgroundColor:[NSColor textBackgroundColor]];
	[e->tv setTextColor:[NSColor textColor]];
	[e->tv setAllowsUndo:YES];
	[e->tv setEditable:YES];
	[e->tv setSelectable:YES];
	[e->tv setRichText:NO];
	[e->tv setImportsGraphics:NO];
	[e->tv setBaseWritingDirection:NSWritingDirectionNatural];
	// TODO default paragraph format
	[e->tv setAllowsImageEditing:NO];
	[e->tv setAutomaticQuoteSubstitutionEnabled:NO];
	[e->tv setAutomaticLinkDetectionEnabled:NO];
	[e->tv setUsesRuler:NO];
	[e->tv setRulerVisible:NO];
	[e->tv setUsesInspectorBar:NO];
	[e->tv setSelectionGranularity:NSSelectByCharacter];
//TODO	[e->tv setInsertionPointColor:[NSColor insertionColor]];
	[e->tv setContinuousSpellCheckingEnabled:NO];
	[e->tv setGrammarCheckingEnabled:NO];
	[e->tv setUsesFontPanel:NO];
	[e->tv setEnabledTextCheckingTypes:0];
	[e->tv setAutomaticDashSubstitutionEnabled:NO];
	[e->tv setAutomaticSpellingCorrectionEnabled:NO];
	[e->tv setAutomaticTextReplacementEnabled:NO];
	[e->tv setSmartInsertDeleteEnabled:NO];
	[e->tv setLayoutOrientation:NSTextLayoutOrientationHorizontal];
	// TODO default find panel behavior
	// now just to be safe; this will do some of the above but whatever
	disableAutocorrect(e->tv);
	// this option is complex; just set it to the Interface Builder default
	[[e->tv layoutManager] setAllowsNonContiguousLayout:YES];
	// this will work because it's the same selector
	uiDarwinSetControlFont((NSControl *) (e->tv), NSRegularControlSize);
	[e->sv setDocumentView:e->tv];

	uiMultilineEntryOnChanged(e, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(e, uiMultilineEntry);

	return e;
}
