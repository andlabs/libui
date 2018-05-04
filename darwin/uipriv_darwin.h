// 6 january 2015
// note: as of OS X Sierra, the -mmacosx-version-min compiler options governs deprecation warnings; keep these around anyway just in case
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_8
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_8
#import <Cocoa/Cocoa.h>
#import <dlfcn.h>		// see future.m
#import "../ui.h"
#import "../ui_darwin.h"
#import "../common/uipriv.h"

#if __has_feature(objc_arc)
#error Sorry, libui cannot be compiled with ARC.
#endif

#define uiprivToNSString(str) [NSString stringWithUTF8String:(str)]
#define uiprivFromNSString(str) [(str) UTF8String]

// TODO find a better place for this
#ifndef NSAppKitVersionNumber10_9
#define NSAppKitVersionNumber10_9 1265
#endif

/*TODO remove this*/typedef struct uiImage uiImage;

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

#import "OLD_uipriv_darwin.h"
