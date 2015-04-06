// 6 april 2015
#include "ui_darwin.h"

// TODO merge with init_darwin.m?

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
	[NSApp postEvent:e atStart:NO];		// let pending events take priority
	// TODO really wait?
}
