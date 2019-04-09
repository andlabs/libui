// 2 june 2017

// The OS X 10.12 SDK introduces a number of new names for
// existing constants to align the naming conventions of
// Objective-C and Swift (particularly in AppKit).
// 
// Unfortunately, in a baffling move, instead of using the existing
// AvailabilityMacros.h method of marking things deprecated, they
// rewrote the relevant constants in ways that make
// source-compatible building much more annoying:
// 
// - The replacement names are now the only names in the enum
// 	or define sets they used to be in.
// - The old names are provided as static const variables, which
// 	means any code that used the old names in a switch case now
// 	spit out a compiler warning in strict C99 mode (TODO and in C++ mode?).
// - The old names are marked with new deprecated-symbol
// 	macros that are *not* compatible with the AvailabilityMacros.h
// 	macros, meaning their deprecation warnings still come
// 	through. (It should be noted that AvailabilityMacros.h was still
// 	updated for 10.12 regardless, hence our #ifdef below.)
// 
// As far as I can gather, these facts are not documented *at all*, so
// in the meantime, other open-source projects just use their own
// #defines to maintain source compatibility, either by making the
// new names available everywhere or the old ones un-deprecated.
// We choose the latter.
// TODO file a radar on the issue (after determining C++ compatibility) so this can be pinned down once and for all
// TODO after that, link my stackoverflow question here too
// TODO make sure this #ifdef does actually work on older systems

#ifdef MAC_OS_X_VERSION_10_12

#define NSControlKeyMask NSEventModifierFlagControl
#define NSAlternateKeyMask NSEventModifierFlagOption
#define NSShiftKeyMask NSEventModifierFlagShift
#define NSCommandKeyMask NSEventModifierFlagCommand

#define NSLeftMouseDown NSEventTypeLeftMouseDown
#define NSRightMouseDown NSEventTypeRightMouseDown
#define NSOtherMouseDown NSEventTypeOtherMouseDown
#define NSLeftMouseUp NSEventTypeLeftMouseUp
#define NSRightMouseUp NSEventTypeRightMouseUp
#define NSOtherMouseUp NSEventTypeOtherMouseUp
#define NSLeftMouseDragged NSEventTypeLeftMouseDragged
#define NSRightMouseDragged NSEventTypeRightMouseDragged
#define NSOtherMouseDragged NSEventTypeOtherMouseDragged
#define NSKeyDown NSEventTypeKeyDown
#define NSKeyUp NSEventTypeKeyUp
#define NSFlagsChanged NSEventTypeFlagsChanged
#define NSApplicationDefined NSEventTypeApplicationDefined
#define NSPeriodic NSEventTypePeriodic
#define NSMouseMoved NSEventTypeMouseMoved

#define NSRegularControlSize NSControlSizeRegular
#define NSSmallControlSize NSControlSizeSmall

#define NSAnyEventMask NSEventMaskAny
#define NSLeftMouseDraggedMask NSEventMaskLeftMouseDragged
#define NSLeftMouseUpMask NSEventMaskLeftMouseUp

#define NSTickMarkAbove NSTickMarkPositionAbove

#define NSLinearSlider NSSliderTypeLinear

#define NSInformationalAlertStyle NSAlertStyleInformational
#define NSCriticalAlertStyle NSAlertStyleCritical

#define NSBorderlessWindowMask NSWindowStyleMaskBorderless
#define NSTitledWindowMask NSWindowStyleMaskTitled
#define NSClosableWindowMask NSWindowStyleMaskClosable
#define NSMiniaturizableWindowMask NSWindowStyleMaskMiniaturizable
#define NSResizableWindowMask NSWindowStyleMaskResizable

#endif

// TODO /Users/pietro/src/github.com/andlabs/libui/darwin/stddialogs.m:83:15: warning: 'beginSheetModalForWindow:modalDelegate:didEndSelector:contextInfo:' is deprecated: first deprecated in macOS 10.10 - Use -beginSheetModalForWindow:completionHandler: instead [-Wdeprecated-declarations]

// TODO https://developer.apple.com/library/content/releasenotes/Miscellaneous/RN-Foundation-OSX10.12/
