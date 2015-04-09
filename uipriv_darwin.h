// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "uipriv.h"
#import "ui_darwin.h"

#define toNSString(str) [NSString stringWithUTF8String:(str)]
#define fromNSString(str) [(str) UTF8String]

// TODO see if we can override alloc instead
#ifdef uiLogAllocations
#import <stdio.h>
#define uiLogObjCClassAllocations \
+ (id)alloc \
{ \
	id thing; \
	thing = [super alloc]; \
	fprintf(stderr, "%p alloc %s\n", thing, [[self className] UTF8String]); \
	return thing; \
} \
- (void)dealloc \
{ \
	[super dealloc]; \
	fprintf(stderr, "%p free\n", self); \
}
#else
#define uiLogObjCClassAllocations
#endif

// util_darwin.m
extern void setStandardControlFont(NSControl *);

// container_darwin.m
struct uiSizing {
	uiSizingCommon
};
@interface uiContainer : NSView
// TODO rename to uiChild
@property uiControl *child;
- (void)uiUpdateNow;
- (void)uiSetMargined:(BOOL)margined;
@end
