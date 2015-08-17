// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "../ui.h"
#import "../ui_darwin.h"
#import "../uipriv.h"

#define toNSString(str) [NSString stringWithUTF8String:(str)]
#define fromNSString(str) [(str) UTF8String]

// menu.m
@interface menuManager : NSObject {
	// unfortunately NSMutableDictionary copies its keys, meaning we can't use it for pointers
	NSMapTable *items;
	BOOL hasQuit;
	BOOL hasPreferences;
	BOOL hasAbout;
}
@property (strong) NSMenuItem *quitItem;
@property (strong) NSMenuItem *preferencesItem;
@property (strong) NSMenuItem *aboutItem;
// NSMenuValidation is only informal
- (BOOL)validateMenuItem:(NSMenuItem *)item;
- (NSMenu *)makeMenubar;
@end
extern void finalizeMenus(void);
extern void uninitMenus(void);

// init.m
@interface applicationClass : NSApplication
@end
// this is needed because NSApp is of type id, confusing clang
#define realNSApp() ((applicationClass *) NSApp)
@interface appDelegate : NSObject <NSApplicationDelegate>
@property (strong) menuManager *menuManager;
@end
#define appDelegate() ((appDelegate *) [realNSApp() delegate])

// util.m
extern void disableAutocorrect(NSTextView *);

// entry.m
extern void finishNewTextField(uiControl *, NSTextField *, BOOL);

// window.m
extern uiWindow *windowFromNSWindow(NSWindow *);

// alloc.m
extern void initAlloc(void);
extern void uninitAlloc(void);

// autolayout.m
extern void addConstraint(NSView *, NSString *, NSDictionary *, NSDictionary *);
extern NSLayoutPriority horzHuggingPri(NSView *);
extern NSLayoutPriority vertHuggingPri(NSView *);
extern void setHuggingPri(NSView *, NSLayoutPriority, NSLayoutConstraintOrientation);
extern void layoutSingleView(NSView *, NSView *, int);
extern NSSize fittingAlignmentSize(NSView *);

// TODO
#define PUT_CODE_HERE 0
