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
	tBox *box;
	tButton *button;
	tSpinbox *spinbox;

	mainwin = [[tWindow alloc] init];
	[mainwin tSetMargined:margined];

	box = [[tBox alloc] tInitVertical:NO];

	button = [[tButton alloc] tInitWithText:@"Button"];
	[box tAddControl:button stretchy:YES];

	[mainwin tSetControl:box];

	spinbox = [[tSpinbox alloc] init];
	[box tAddControl:spinbox stretchy:NO];

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
