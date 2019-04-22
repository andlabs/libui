// 20 april 2019
#import "uipriv_darwin.h"

@interface uiprivApplication : NSApplication
@end

@implementation uiprivApplication
@end

static NSApplication *uiprivApp;

#define errNSAppAlreadyInitialized "NSApp is not of type uiprivApplication; was likely already initialized beforehand"

static const char *initErrors[] = {
	errNSAppAlreadyInitialized,
	NULL,
};

int uiInit(void *options, uiInitError *err)
{
	if (!uiprivInitCheckParams(options, err, initErrors))
		return 0;

	uiprivApp = [uiprivApplication sharedApplication];
	if (![NSApp isKindOfClass:[uiprivApplication class]])
		return uiprivInitReturnError(err, errNSAppAlreadyInitialized);

	uiprivMarkInitialized();
	return 1;
}
