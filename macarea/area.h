// 8 september 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>
#import "ui.h"
#import "uipriv_darwin.h"

extern uiArea *newArea(uiAreaHandler *ah);

extern uiDrawContext *newContext(CGContextRef);

extern NSView *areaGetView(uiArea *);

extern void areaUpdateScroll(uiArea *);
