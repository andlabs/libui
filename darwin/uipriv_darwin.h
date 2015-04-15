// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "../uipriv.h"
#import "../ui_darwin.h"

#define toNSString(str) [NSString stringWithUTF8String:(str)]
#define fromNSString(str) [(str) UTF8String]

#define uiLogObjCClassAllocations \
+ (id)alloc \
{ \
	id thing; \
	thing = [super alloc]; \
	if (options.debugLogAllocations) \
		fprintf(stderr, "%p alloc %s\n", thing, [[self className] UTF8String]); \
	return thing; \
} \
- (void)dealloc \
{ \
	[super dealloc]; \
	if (options.debugLogAllocations) \
		fprintf(stderr, "%p free\n", self); \
}

// init_darwin.m
extern NSView *destroyedControlsView;

// util_darwin.m
extern void setStandardControlFont(NSControl *);
extern void disableAutocorrect(NSTextView *);

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
#define macXMargin 20
#define macYMargin 20

// entry_darwin.m
extern void finishNewTextField(NSTextField *, BOOL);
