// 7 april 2015
#import "uipriv_darwin.h"

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

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
// This one is 8 for most pairs of controls that I've tried; the only difference is between two pushbuttons, where it's 12...
#define macXPadding 8
// Likewise, this one appears to be 12 for pairs of push buttons...
#define macYPadding 8

uiSizing *uiDarwinNewSizing(void)
{
	uiSizing *d;

	d = uiNew(uiSizing);
	d->XPadding = macXPadding;
	d->YPadding = macYPadding;
	d->Sys = uiNew(uiSizingSys);
	return d;
}

void uiFreeSizing(uiSizing *d)
{
	uiFree(d->Sys);
	uiFree(d);
}
