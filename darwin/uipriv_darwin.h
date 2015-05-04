// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "../ui.h"
#import "../ui_darwin.h"
#import "../uipriv.h"

#define toNSString(str) [NSString stringWithUTF8String:(str)]
#define fromNSString(str) [(str) UTF8String]

// These are based on measurements from Interface Builder.
// These seem to be based on Auto Layout constants, but I don't see an API that exposes these...
#define macXMargin 20
#define macYMargin 20

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

// window.m
extern uiWindow *windowFromNSWindow(NSWindow *);
