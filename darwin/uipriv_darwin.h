// 6 january 2015
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "../ui.h"
#import "../ui_darwin.h"
#import "../common/uipriv.h"

#if __has_feature(objc_arc)
#error Sorry, libui cannot be compiled with ARC.
#endif

#define toNSString(str) [NSString stringWithUTF8String:(str)]
#define fromNSString(str) [(str) UTF8String]

// menu.m
@interface menuManager : NSObject {
	struct mapTable *items;
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
extern void finishNewTextField(NSTextField *, BOOL);
extern NSTextField *newEditableTextField(void);

// window.m
extern uiWindow *windowFromNSWindow(NSWindow *);

// alloc.m
extern NSMutableArray *delegates;
extern void initAlloc(void);
extern void uninitAlloc(void);

// autolayout.m
extern NSLayoutConstraint *mkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c, NSString *desc);

// map.m
extern struct mapTable *newMap(void);
extern void mapDestroy(struct mapTable *m);
extern void *mapGet(struct mapTable *m, void *key);
extern void mapSet(struct mapTable *m, void *key, void *value);
extern void mapDelete(struct mapTable *m, void *key);

// area.m
extern int sendAreaEvents(NSEvent *);

// areaevents.m
extern BOOL fromKeycode(unsigned short keycode, uiAreaKeyEvent *ke);
extern BOOL keycodeModifier(unsigned short keycode, uiModifiers *mod);

// draw.m
extern uiDrawContext *newContext(CGContextRef, CGFloat);
extern void freeContext(uiDrawContext *);

// drawtext.m
extern uiDrawTextFont *mkTextFont(CTFontRef f, BOOL retain);
extern uiDrawTextFont *mkTextFontFromNSFont(NSFont *f);
extern void doDrawText(CGContextRef c, CGFloat cheight, double x, double y, uiDrawTextLayout *layout);

// fontbutton.m
extern BOOL fontButtonInhibitSendAction(SEL sel, id from, id to);
extern BOOL fontButtonOverrideTargetForAction(SEL sel, id from, id to, id *override);

// MASSIVE TODO
#define uiDarwinControlTriggerRelayout(...)
