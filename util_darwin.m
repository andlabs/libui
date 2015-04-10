// 7 april 2015
#import "uipriv_darwin.h"

// also fine for NSCells and NSTexts (NSTextViews)
void setStandardControlFont(NSControl *control)
{
	[control setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
}

void disableAutocorrect(NSTextView *tv)
{
	[tv setEnabledTextCheckingTypes:0];
	[tv setAutomaticDashSubstitutionEnabled:NO];
	// don't worry about automatic data detection; it won't change stringValue (thanks pretty_function in irc.freenode.net/#macdev)
	[tv setAutomaticSpellingCorrectionEnabled:NO];
	[tv setAutomaticTextReplacementEnabled:NO];
	[tv setAutomaticQuoteSubstitutionEnabled:NO];
	[tv setAutomaticLinkDetectionEnabled:NO];
	[tv setSmartInsertDeleteEnabled:NO];
}
