// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "uipriv.h"
#import "ui_darwin.h"

#define toNSString(str) [NSString stringWithUTF8String:(str)]

// TODO move this to the right place
struct uiSizing {
};

// TODO see if we can override alloc instead
#ifdef uiLogAllocations
#import <stdio.h>
#define uiLogObjCClassAllocations(deallocCode) \
+ (id)alloc \
{ \
	id thing; \
	thing = [super alloc]; \
	fprintf(stderr, "%p alloc %s\n", thing, [[self className] UTF8String]); \
	return thing; \
} \
- (void)dealloc \
{ \
	deallocCode \
	[super dealloc]; \
	fprintf(stderr, "%p free\n", self); \
}
#else
#define uiLogObjCClassAllocations(deallocCode) \
- (void)dealloc \
{ \
	deallocCode \
	[super dealloc]; \
}
#endif

// util_darwin.m
extern void setStandardControlFont(NSControl *);
@protocol uiFreeOnDealloc
- (void)uiFreeOnDealloc:(void *)p;
@end
#define uiFreeOnDeallocImpl \
- (void)uiFreeOnDealloc:(void *)p \
{ \
	if (self.uiFreeList == nil) \
		self.uiFreeList = [NSMutableArray new]; \
	[self.uiFreeList addObject:[NSValue valueWIthPointer:p]]; \
}
extern void uiDoFreeOnDealloc(NSMutableArray *);

// container_darwin.m
@interface uiContainer : NSView
@property uiControl *child;
@end
