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
	NSFont *font;

	e = (uiMultilineEntry *) uiNewControl(uiMultilineEntry);

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
	// don't use uiDarwinSetControlFont() directly; we have to do a little extra work to set the font
	font = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]];
	[e->tv setTypingAttributes:[NSDictionary
		dictionaryWithObject:font
		forKey:NSFontAttributeName]];
	// e->tv font from Interface Builder is nil, but setFont:nil throws an exception
	// let's just set it to the standard control font anyway, just to be safe
	[e->tv setFont:font];

	// TODO this (via https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/TextUILayer/Tasks/TextInScrollView.html) is the magic incantation needed to get things to show up; figure out why
	// it especially seems weird we're mixing this with auto layout...
	[e->tv setAutoresizingMask:NSViewWidthSizable];

//TODO	[e->tv setTranslatesAutoresizingMaskIntoConstraints:NO];
	[e->sv setDocumentView:e->tv];

//TODO:void printinfo(NSScrollView *sv, NSTextView *tv);
//printinfo(e->sv, e->tv);

	uiMultilineEntryOnChanged(e, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(e, uiMultilineEntry);

	return e;
}

NSMutableString *s;
static void add(const char *fmt, ...)
{
	va_list ap;
	NSString *fmts;
	NSString *a;
	
	va_start(ap, fmt);
	fmts = [NSString stringWithUTF8String:fmt];
	a = [[NSString alloc] initWithFormat:fmts arguments:ap];
	[s appendString:a];
	[s appendString:@"\n"];
	va_end(ap);
}

static NSString *edgeInsetsStr(NSEdgeInsets i)
{
	return [NSString
		stringWithFormat:@"left:%g top:%g right:%g bottom:%g",
			i.left, i.top, i.right, i.bottom];
}

void printinfo(NSScrollView *sv, NSTextView *tv)
{
	s = [NSMutableString new];
	
#define self _s
struct {
NSScrollView *sv;
NSTextView *tv;
} _s;
_s.sv = sv;
_s.tv = tv;
	
	add("NSScrollView");
	add(" backgroundColor %@", [self.sv backgroundColor]);
	add(" drawsBackground %d", [self.sv drawsBackground]);
	add(" borderType %d", [self.sv borderType]);
	add(" documentCursor %@", [self.sv documentCursor]);
	add(" hasHorizontalScroller %d", [self.sv hasHorizontalScroller]);
	add(" hasVerticalScroller %d", [self.sv hasVerticalScroller]);
	add(" autohidesScrollers %d", [self.sv autohidesScrollers]);
	add(" hasHorizontalRuler %d", [self.sv hasHorizontalRuler]);
	add(" hasVerticalRuler %d", [self.sv hasVerticalRuler]);
	add(" rulersVisible %d", [self.sv rulersVisible]);
	add(" automaticallyAdjustsContentInsets %d",
		[self.sv automaticallyAdjustsContentInsets]);
	add(" contentInsets %@",
		edgeInsetsStr([self.sv contentInsets]));
	add(" scrollerInsets %@",
		edgeInsetsStr([self.sv scrollerInsets]));
	add(" scrollerKnobStyle %d", [self.sv scrollerKnobStyle]);
	add(" scrollerStyle %d", [self.sv scrollerStyle]);
	add(" lineScroll %g", [self.sv lineScroll]);
	add(" horizontalLineScroll %g", [self.sv horizontalLineScroll]);
	add(" verticalLineScroll %g", [self.sv verticalLineScroll]);
	add(" pageScroll %g", [self.sv pageScroll]);
	add(" horizontalPageScroll %g", [self.sv horizontalPageScroll]);
	add(" verticalPageScroll %g", [self.sv verticalPageScroll]);
	add(" scrollsDynamically %d", [self.sv scrollsDynamically]);
	add(" findBarPosition %d", [self.sv findBarPosition]);
	add(" usesPredominantAxisScrolling %d",
		[self.sv usesPredominantAxisScrolling]);
	add(" horizontalScrollElasticity %d",
		[self.sv horizontalScrollElasticity]);
	add(" verticalScrollElasticity %d",
		[self.sv verticalScrollElasticity]);
	add(" allowsMagnification %d", [self.sv allowsMagnification]);
	add(" magnification %g", [self.sv magnification]);
	add(" maxMagnification %g", [self.sv maxMagnification]);
	add(" minMagnification %g", [self.sv minMagnification]);

	add("");
	
	add("NSTextView");
	add(" textContainerInset %@",
		NSStringFromSize([self.tv textContainerInset]));
	add(" textContainerOrigin %@",
		NSStringFromPoint([self.tv textContainerOrigin]));
	add(" backgroundColor %@", [self.tv backgroundColor]);
	add(" drawsBackground %d", [self.tv drawsBackground]);
	add(" allowsDocumentBackgroundColorChange %d",
		[self.tv allowsDocumentBackgroundColorChange]);
	add(" allowedInputSourceLocales %@",
		[self.tv allowedInputSourceLocales]);
	add(" allowsUndo %d", [self.tv allowsUndo]);
	add(" isEditable %d", [self.tv isEditable]);
	add(" isSelectable %d", [self.tv isSelectable]);
	add(" isFieldEditor %d", [self.tv isFieldEditor]);
	add(" isRichText %d", [self.tv isRichText]);
	add(" importsGraphics %d", [self.tv importsGraphics]);
	add(" defaultParagraphStyle %@",
		[self.tv defaultParagraphStyle]);
	add(" allowsImageEditing %d", [self.tv allowsImageEditing]);
	add(" isAutomaticQuoteSubstitutionEnabled %d",
		[self.tv isAutomaticQuoteSubstitutionEnabled]);
	add(" isAutomaticLinkDetectionEnabled %d",
		[self.tv isAutomaticLinkDetectionEnabled]);
	add(" displaysLinkToolTips %d", [self.tv displaysLinkToolTips]);
	add(" usesRuler %d", [self.tv usesRuler]);
	add(" isRulerVisible %d", [self.tv isRulerVisible]);
	add(" usesInspectorBar %d", [self.tv usesInspectorBar]);
	add(" selectionAffinity %d", [self.tv selectionAffinity]);
	add(" selectionGranularity %d", [self.tv selectionGranularity]);
	add(" insertionPointColor %@", [self.tv insertionPointColor]);
	add(" selectedTextAttributes %@",
		[self.tv selectedTextAttributes]);
	add(" markedTextAttributes %@", [self.tv markedTextAttributes]);
	add(" linkTextAttributes %@", [self.tv linkTextAttributes]);
	add(" typingAttributes %@", [self.tv typingAttributes]);
	add(" smartInsertDeleteEnabled %d",
		[self.tv smartInsertDeleteEnabled]);
	add(" isContinuousSpellCheckingEnabled %d",
		[self.tv isContinuousSpellCheckingEnabled]);
	add(" isGrammarCheckingEnabled %d",
		[self.tv isGrammarCheckingEnabled]);
	add(" acceptsGlyphInfo %d", [self.tv acceptsGlyphInfo]);
	add(" usesFontPanel %d", [self.tv usesFontPanel]);
	add(" usesFindPanel %d", [self.tv usesFindPanel]);
	add(" enabledTextCheckingTypes %d",
		[self.tv enabledTextCheckingTypes]);
	add(" isAutomaticDashSubstitutionEnabled %d",
		[self.tv isAutomaticDashSubstitutionEnabled]);
	add(" isAutomaticDataDetectionEnabled %d",
		[self.tv isAutomaticDataDetectionEnabled]);
	add(" isAutomaticSpellingCorrectionEnabled %d",
		[self.tv isAutomaticSpellingCorrectionEnabled]);
	add(" isAutomaticTextReplacementEnabled %d",
		[self.tv isAutomaticTextReplacementEnabled]);
	add(" usesFindBar %d", [self.tv usesFindBar]);
	add(" isIncrementalSearchingEnabled %d",
		[self.tv isIncrementalSearchingEnabled]);
	add(" NSText:");
	add("  font %@", [self.tv font]);
	add("  textColor %@", [self.tv textColor]);
	add("  baseWritingDirection %d", [self.tv baseWritingDirection]);
	add("  maxSize %@",
		NSStringFromSize([self.tv maxSize]));
	add("  minSize %@",
		NSStringFromSize([self.tv minSize]));
	add("  isVerticallyResizable %d",
		[self.tv isVerticallyResizable]);
	add("  isHorizontallyResizable %d",
		[self.tv isHorizontallyResizable]);

#undef self
	
	fprintf(stdout, "%s", [s UTF8String]);
}
