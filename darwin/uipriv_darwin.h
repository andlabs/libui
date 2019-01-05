// 6 january 2015
// note: as of OS X Sierra, the -mmacosx-version-min compiler options governs deprecation warnings; keep these around anyway just in case
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_8
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_8
#import <Cocoa/Cocoa.h>
#import <dlfcn.h>		// see future.m
#import "../ui.h"
#import "../ui_darwin.h"
#import "../common/uipriv.h"

// TODO should we rename the uiprivMk things or not
// TODO what about renaming class wrapper functions that return the underlying class (like uiprivNewLabel())

#if __has_feature(objc_arc)
#error Sorry, libui cannot be compiled with ARC.
#endif

#define uiprivToNSString(str) [NSString stringWithUTF8String:(str)]
#define uiprivFromNSString(str) [(str) UTF8String]

// TODO find a better place for this
#ifndef NSAppKitVersionNumber10_9
#define NSAppKitVersionNumber10_9 1265
#endif

// map.m
typedef struct uiprivMap uiprivMap;
extern uiprivMap *uiprivNewMap(void);
extern void uiprivMapDestroy(uiprivMap *m);
extern void *uiprivMapGet(uiprivMap *m, void *key);
extern void uiprivMapSet(uiprivMap *m, void *key, void *value);
extern void uiprivMapDelete(uiprivMap *m, void *key);
extern void uiprivMapWalk(uiprivMap *m, void (*f)(void *key, void *value));
extern void uiprivMapReset(uiprivMap *m);

// menu.m
@interface uiprivMenuManager : NSObject {
	uiprivMap *items;
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
extern void uiprivFinalizeMenus(void);
extern void uiprivUninitMenus(void);

// main.m
@interface uiprivApplicationClass : NSApplication
@end
// this is needed because NSApp is of type id, confusing clang
#define uiprivNSApp() ((uiprivApplicationClass *) NSApp)
@interface uiprivAppDelegate : NSObject<NSApplicationDelegate>
@property (strong) uiprivMenuManager *menuManager;
@end
#define uiprivAppDelegate() ((uiprivAppDelegate *) [uiprivNSApp() delegate])
typedef struct uiprivNextEventArgs uiprivNextEventArgs;
struct uiprivNextEventArgs {
	NSEventMask mask;
	NSDate *duration;
	// LONGTERM no NSRunLoopMode?
	NSString *mode;
	BOOL dequeue;
};
extern int uiprivMainStep(uiprivNextEventArgs *nea, BOOL (^interceptEvent)(NSEvent *));

// util.m
extern void uiprivDisableAutocorrect(NSTextView *);

// entry.m
extern void uiprivFinishNewTextField(NSTextField *, BOOL);
extern NSTextField *uiprivNewEditableTextField(void);

// window.m
@interface uiprivNSWindow : NSWindow
- (void)uiprivDoMove:(NSEvent *)initialEvent;
- (void)uiprivDoResize:(NSEvent *)initialEvent on:(uiWindowResizeEdge)edge;
@end
extern uiWindow *uiprivWindowFromNSWindow(NSWindow *);

// alloc.m
extern NSMutableArray *uiprivDelegates;
extern void uiprivInitAlloc(void);
extern void uiprivUninitAlloc(void);

// autolayout.m
extern NSLayoutConstraint *uiprivMkConstraint(id view1, NSLayoutAttribute attr1, NSLayoutRelation relation, id view2, NSLayoutAttribute attr2, CGFloat multiplier, CGFloat c, NSString *desc);
extern void uiprivJiggleViewLayout(NSView *view);
typedef struct uiprivSingleChildConstraints uiprivSingleChildConstraints;
struct uiprivSingleChildConstraints {
	NSLayoutConstraint *leadingConstraint;
	NSLayoutConstraint *topConstraint;
	NSLayoutConstraint *trailingConstraintGreater;
	NSLayoutConstraint *trailingConstraintEqual;
	NSLayoutConstraint *bottomConstraintGreater;
	NSLayoutConstraint *bottomConstraintEqual;
};
extern void uiprivSingleChildConstraintsEstablish(uiprivSingleChildConstraints *c, NSView *contentView, NSView *childView, BOOL hugsTrailing, BOOL hugsBottom, int margined, NSString *desc);
extern void uiprivSingleChildConstraintsRemove(uiprivSingleChildConstraints *c, NSView *cv);
extern void uiprivSingleChildConstraintsSetMargined(uiprivSingleChildConstraints *c, int margined);

// area.m
extern int uiprivSendAreaEvents(NSEvent *);

// areaevents.m
extern BOOL uiprivFromKeycode(unsigned short keycode, uiAreaKeyEvent *ke);
extern BOOL uiprivKeycodeModifier(unsigned short keycode, uiModifiers *mod);

// draw.m
extern uiDrawContext *uiprivDrawNewContext(CGContextRef, CGFloat);
extern void uiprivDrawFreeContext(uiDrawContext *);

// fontbutton.m
extern BOOL uiprivFontButtonInhibitSendAction(SEL sel, id from, id to);
extern BOOL uiprivFontButtonOverrideTargetForAction(SEL sel, id from, id to, id *override);
extern void uiprivSetupFontPanel(void);

// colorbutton.m
extern BOOL uiprivColorButtonInhibitSendAction(SEL sel, id from, id to);

// scrollview.m
typedef struct uiprivScrollViewCreateParams uiprivScrollViewCreateParams;
struct uiprivScrollViewCreateParams {
	// TODO MAYBE fix these identifiers
	NSView *DocumentView;
	NSColor *BackgroundColor;
	BOOL DrawsBackground;
	BOOL Bordered;
	BOOL HScroll;
	BOOL VScroll;
};
typedef struct uiprivScrollViewData uiprivScrollViewData;
extern NSScrollView *uiprivMkScrollView(uiprivScrollViewCreateParams *p, uiprivScrollViewData **dout);
extern void uiprivScrollViewSetScrolling(NSScrollView *sv, uiprivScrollViewData *d, BOOL hscroll, BOOL vscroll);
extern void uiprivScrollViewFreeData(NSScrollView *sv, uiprivScrollViewData *d);

// label.m
extern NSTextField *uiprivNewLabel(NSString *str);

// image.m
extern NSImage *uiprivImageNSImage(uiImage *);

// winmoveresize.m
extern void uiprivDoManualMove(NSWindow *w, NSEvent *initialEvent);
extern void uiprivDoManualResize(NSWindow *w, NSEvent *initialEvent, uiWindowResizeEdge edge);

// future.m
extern CFStringRef *uiprivFUTURE_kCTFontOpenTypeFeatureTag;
extern CFStringRef *uiprivFUTURE_kCTFontOpenTypeFeatureValue;
extern CFStringRef *uiprivFUTURE_kCTBackgroundColorAttributeName;
extern void uiprivLoadFutures(void);
extern void uiprivFUTURE_NSLayoutConstraint_setIdentifier(NSLayoutConstraint *constraint, NSString *identifier);
extern BOOL uiprivFUTURE_NSWindow_performWindowDragWithEvent(NSWindow *w, NSEvent *initialEvent);

// undocumented.m
extern CFStringRef uiprivUNDOC_kCTFontPreferredSubFamilyNameKey;
extern CFStringRef uiprivUNDOC_kCTFontPreferredFamilyNameKey;
extern void uiprivLoadUndocumented(void);
