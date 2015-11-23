// 17 november 2015
// TODO versioning macros?
#include <AppKit.h>
#include <InterfaceKit.h>
#include <GroupLayout.h>
#include "../ui.h"
#include "../ui_haiku.hpp"
#include "../common/uipriv.h"

// alloc.cpp
extern void initAlloc(void);
extern void uninitAlloc(void);

// singlechild.cpp
extern struct singleChild *newSingleChild(uiControl *c, uiControl *parent, void (*attach)(void *, BLayoutItem *), void *attachTo);
extern void singleChildRemove(struct singleChild *s);
extern void singleChildDestroy(struct singleChild *s);
extern BLayoutItem *singleChildLayoutItem(struct singleChild *s);
extern void singleChildUpdateState(struct singleChild *s);
extern void singleChildSetMargined(struct singleChild *s, float inset);

// TODO make this public?
#define mControlField "libui_uiControl"
// TODO write helper functions?

// draw.cpp
extern uiDrawContext *newContext(BView *view);
extern void freeContext(uiDrawContext *c);
