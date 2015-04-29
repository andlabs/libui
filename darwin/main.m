// 6 april 2015
#import "uipriv_darwin.h"

void uiMain(void)
{
	[NSApp run];
}

void uiQuit(void)
{
	NSEvent *e;

	[NSApp stop:NSApp];
	// stop: won't register until another event has passed; let's synthesize one
	e = [NSEvent otherEventWithType:NSApplicationDefined
		location:NSZeroPoint
		modifierFlags:0
		timestamp:[[NSProcessInfo processInfo] systemUptime]
		windowNumber:0
		context:[NSGraphicsContext currentContext]
		subtype:0
		data1:0
		data2:0];
	[NSApp postEvent:e atStart:NO];		// let pending events take priority (this is what PostQuitMessage() on Windows does so we have to do it here too for parity; thanks to mikeash in irc.freenode.net/#macdev for confirming that this parameter should indeed be NO)
}
