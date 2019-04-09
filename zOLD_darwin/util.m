// 7 april 2015
#import "uipriv_darwin.h"

// LONGTERM do we really want to do this? make it an option?
// TODO figure out why we removed this from window.m
void uiprivDisableAutocorrect(NSTextView *tv)
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
