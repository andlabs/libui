// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "../ui.h"
#import "../ui_darwin.h"
#import "../uipriv.h"

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

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
#define macXMargin 20
#define macYMargin 20

// menu.m
@interface menuManager : NSObject {
	NSMutableDictionary *items;
}
@property (strong) NSMenuItem *quitItem;
@property (strong) NSMenuItem *preferencesItem;
@property (strong) NSMenuItem *aboutItem;
- (IBAction)onMenuItemClicked:(id)sender;
- (NSMenu *)makeMenubar;
@end

// init.m
@interface appDelegate : NSObject <NSApplicationDelegate>
@property (strong) menuManager *menuManager;
@end
#define appDelegate() ((appDelegate *) [NSApp delegate])

// util.m
extern void setStandardControlFont(NSControl *);
extern void disableAutocorrect(NSTextView *);

// entry.m
extern void finishNewTextField(NSTextField *, BOOL);
