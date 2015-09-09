// 8 september 2015
#import <Cocoa/Cocoa.h>
#import <stdint.h>
#import "ui.h"

extern uiArea *newArea(uiAreaHandler *ah);

extern uiDrawContext *newContext(CGContextRef);

extern NSView *areaView(uiArea *);

extern void areaUpdateScroll(uiArea *);
