// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "uipriv.h"
#import "ui_darwin.h"

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
extern NSView *deletedControlsView;

// util_darwin.m
extern void setStandardControlFont(NSControl *);
extern void disableAutocorrect(NSTextView *);

// container_darwin.m
@interface uiContainer : NSView
@property uiControl *uiChild;
- (void)uiUpdateNow;
- (BOOL)uiMargined;
- (void)uiSetMargined:(BOOL)margined;
@end
