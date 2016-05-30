// 31 may 2016
#import "uipriv_darwin.h"

@implementation areaEventHandler

- (uiModifiers)parseModifiers:(NSEvent *)e
{
	NSEventModifierFlags mods;
	uiModifiers m;

	m = 0;
	mods = [e modifierFlags];
	if ((mods & NSControlKeyMask) != 0)
		m |= uiModifierCtrl;
	if ((mods & NSAlternateKeyMask) != 0)
		m |= uiModifierAlt;
	if ((mods & NSShiftKeyMask) != 0)
		m |= uiModifierShift;
	if ((mods & NSCommandKeyMask) != 0)
		m |= uiModifierSuper;
	return m;
}

// capture on drag is done automatically on OS X
- (uiAreaMouseEvent)doMouseEvent:(NSEvent *)e inView:(NSView *)view scrolling:(BOOL)scrolling
{
	uiAreaMouseEvent me;
	NSPoint point;
	uintmax_t buttonNumber;
	NSUInteger pmb;
	unsigned int i, max;

	// this will convert point to drawing space
	// thanks swillits in irc.freenode.net/#macdev
	point = [view convertPoint:[e locationInWindow] fromView:nil];
	me.X = point.x;
	me.Y = point.y;

	me.AreaWidth = 0;
	me.AreaHeight = 0;
	if (!scrolling) {
		me.AreaWidth = [view frame].size.width;
		me.AreaHeight = [view frame].size.height;
	}

	buttonNumber = [e buttonNumber] + 1;
	// swap button numbers 2 and 3 (right and middle)
	if (buttonNumber == 2)
		buttonNumber = 3;
	else if (buttonNumber == 3)
		buttonNumber = 2;

	me.Down = 0;
	me.Up = 0;
	me.Count = 0;
	switch ([e type]) {
	case NSLeftMouseDown:
	case NSRightMouseDown:
	case NSOtherMouseDown:
		me.Down = buttonNumber;
		me.Count = [e clickCount];
		break;
	case NSLeftMouseUp:
	case NSRightMouseUp:
	case NSOtherMouseUp:
		me.Up = buttonNumber;
		break;
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSOtherMouseDragged:
		// we include the button that triggered the dragged event in the Held fields
		buttonNumber = 0;
		break;
	}

	me.Modifiers = [self parseModifiers:e];

	pmb = [NSEvent pressedMouseButtons];
	me.Held1To64 = 0;
	if (buttonNumber != 1 && (pmb & 1) != 0)
		me.Held1To64 |= 1;
	if (buttonNumber != 2 && (pmb & 4) != 0)
		me.Held1To64 |= 2;
	if (buttonNumber != 3 && (pmb & 2) != 0)
		me.Held1To64 |= 4;
	// buttons 4..32
	// https://developer.apple.com/library/mac/documentation/Carbon/Reference/QuartzEventServicesRef/index.html#//apple_ref/c/tdef/CGMouseButton says Quartz only supports up to 32 buttons
	max = 32;
	for (i = 4; i <= max; i++) {
		uint64_t j;

		if (buttonNumber == i)
			continue;
		j = 1 << (i - 1);
		if ((pmb & j) != 0)
			me.Held1To64 |= j;
	}

	return me;
}

- (BOOL)doKeyDownUp:(NSEvent *)e up:(BOOL)up keyEvent:(uiAreaKeyEvent *)ke
{
	ke->Key = ke->ExtKey = ke->Modifier = 0;
	ke->Modifiers = [self parseModifiers:e];
	return fromKeycode([e keyCode], ke);
}

- (BOOL)doFlagsChanged:(NSEvent *)e keyEvent:(uiAreaKeyEvent *)ke
{
	uiModifiers whichmod;

	ke->Key = ke->ExtKey = 0;

	// Mac OS X sends this event on both key up and key down.
	// Fortunately -[e keyCode] IS valid here, so we can simply map from key code to Modifiers, get the value of [e modifierFlags], and check if the respective bit is set or not — that will give us the up/down state
	if (!keycodeModifier([e keyCode], &whichmod))
		return NO;

	ke->Modifier = whichmod;
	ke->Modifiers = [self parseModifiers:e];
	ke->Up = (ke->Modifiers & ke->Modifier) == 0;
	// and then drop the current modifier from Modifiers
	ke->Modifiers &= ~ke->Modifier;
	return YES;
}

- (void)updateTrackingAreaForView:(NSView *)view
{
	NSEnumerator *e = [[view trackingAreas] objectEnumerator];
	NSTrackingArea *ta;
	while ((ta = [e nextObject]) != nil)
		[view removeTrackingArea:ta];

	ta = [[NSTrackingArea alloc] initWithRect:[view bounds]
									  options:NSTrackingMouseEnteredAndExited |
											  NSTrackingMouseMoved |
											  NSTrackingActiveAlways |
											  NSTrackingInVisibleRect |
											  NSTrackingEnabledDuringMouseDrag
										owner:view
									 userInfo:nil];
	[view addTrackingArea:ta];
}

@end

// called by subclasses of -[NSApplication sendEvent:]
// by default, NSApplication eats some key events
// this prevents that from happening with uiArea
// see http://stackoverflow.com/questions/24099063/how-do-i-detect-keyup-in-my-nsview-with-the-command-key-held and http://lists.apple.com/archives/cocoa-dev/2003/Oct/msg00442.html
int sendAreaEvents(NSEvent *e)
{
	NSEventType type;
	id focused;
	id<areaKeyHandler> view;

	type = [e type];
	if (type != NSKeyDown && type != NSKeyUp && type != NSFlagsChanged)
		return 0;
	focused = [[e window] firstResponder];
	if (focused == nil)
		return 0;
	if (![focused conformsToProtocol:@protocol(areaKeyHandler)])
		return 0;
	view = focused;
	switch (type) {
	case NSKeyDown:
		return [view doKeyDownUp:e up:NO];
	case NSKeyUp:
		return [view doKeyDownUp:e up:YES];
	case NSFlagsChanged:
		return [view doFlagsChanged:e];
	}
	return 0;
}


