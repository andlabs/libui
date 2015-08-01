// 31 july 2015
#import "osxaltest.h"

// #qo LDFLAGS: -framework Foundation -framework AppKit

BOOL margined = NO;

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	tWindow *mainwin;
	tButton *button;

	mainwin = [[tWindow alloc] init];
	[mainwin tSetMargined:margined];

	button = [[tButton alloc] tInitWithText:@"Button"];
	[mainwin tSetControl:button];

	[mainwin tShow];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)a
{
	return YES;
}

@end

int main(int argc, char *argv[])
{
	NSApplication *app;

	margined = argc > 1;

	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
