// 6 january 2015

#ifndef __UI_UI_DARWIN_H__
#define __UI_UI_DARWIN_H__

#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_7
#import <Cocoa/Cocoa.h>
#import "ui.h"

// alloc_darwin.m
extern void *uiAlloc(size_t);
extern void *uiRealloc(void *, size_t);
extern void uiFree(void *);

#endif
