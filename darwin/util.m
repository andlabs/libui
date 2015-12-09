// 7 april 2015
#import "uipriv_darwin.h"

// TODO do we really want to do this? make it an option?
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

void complain(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[libui] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	abort();
}
