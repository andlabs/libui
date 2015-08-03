// 31 july 2015
#import "osxaltest.h"

// #qo LDFLAGS: -framework Foundation -framework AppKit

BOOL spaced = NO;
BOOL firstvert = YES;

@interface appDelegate : NSObject<NSApplicationDelegate>
@end

@implementation appDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	tWindow *mainwin;
	tBox *box, *hbox;
	tButton *button;
	tSpinbox *spinbox;
	tEntry *entry;

	mainwin = [[tWindow alloc] init];
	[mainwin tSetMargined:spaced];

	box = [[tBox alloc] tInitVertical:firstvert];

	spinbox = [[tSpinbox alloc] init];
	[box tAddControl:spinbox stretchy:NO];

	[mainwin tSetControl:box];

	hbox = [[tBox alloc] tInitVertical:!firstvert];
	button = [[tButton alloc] tInitWithText:@"Button"];
	[hbox tAddControl:button stretchy:YES];
	button = [[tButton alloc] tInitWithText:@"Button"];
	[hbox tAddControl:button stretchy:YES];
	[box tAddControl:hbox stretchy:NO];

	hbox = [[tBox alloc] tInitVertical:!firstvert];
	button = [[tButton alloc] tInitWithText:@"Button"];
	[hbox tAddControl:button stretchy:YES];
	button = [[tButton alloc] tInitWithText:@"Button"];
	[hbox tAddControl:button stretchy:YES];
	[box tAddControl:hbox stretchy:NO];

	hbox = [[tBox alloc] tInitVertical:!firstvert];
	button = [[tButton alloc] tInitWithText:@"Button"];
	[hbox tAddControl:button stretchy:YES];
	button = [[tButton alloc] tInitWithText:@"A"];
	[hbox tAddControl:button stretchy:NO];
	button = [[tButton alloc] tInitWithText:@"BB"];
	[hbox tAddControl:button stretchy:NO];
	button = [[tButton alloc] tInitWithText:@"CCC"];
	[hbox tAddControl:button stretchy:NO];
	[box tAddControl:hbox stretchy:NO];

	// TODO this isn't stretchy in the proper order
	hbox = [[tBox alloc] tInitVertical:!firstvert];
	spinbox = [[tSpinbox alloc] init];
	[hbox tAddControl:spinbox stretchy:NO];
	spinbox = [[tSpinbox alloc] init];
	[hbox tAddControl:spinbox stretchy:YES];
	[box tAddControl:hbox stretchy:NO];

	hbox = [[tBox alloc] tInitVertical:!firstvert];
	entry = [[tEntry alloc] init];
	[hbox tAddControl:entry stretchy:NO];
	entry = [[tEntry alloc] init];
	[hbox tAddControl:entry stretchy:YES];
	[box tAddControl:hbox stretchy:NO];

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

	spaced = argc > 1;

	app = [NSApplication sharedApplication];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app setDelegate:[appDelegate new]];
	[app run];
	return 0;
}
