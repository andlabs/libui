// draw.m
extern uiDrawContext *newContext(CGContextRef, CGFloat);
extern void freeContext(uiDrawContext *);

// fontbutton.m
extern BOOL uiprivFontButtonInhibitSendAction(SEL sel, id from, id to);
extern BOOL uiprivFontButtonOverrideTargetForAction(SEL sel, id from, id to, id *override);
extern void uiprivSetupFontPanel(void);

// colorbutton.m
extern BOOL colorButtonInhibitSendAction(SEL sel, id from, id to);

// scrollview.m
struct scrollViewCreateParams {
	NSView *DocumentView;
	NSColor *BackgroundColor;
	BOOL DrawsBackground;
	BOOL Bordered;
	BOOL HScroll;
	BOOL VScroll;
};
struct scrollViewData;
extern NSScrollView *mkScrollView(struct scrollViewCreateParams *p, struct scrollViewData **dout);
extern void scrollViewSetScrolling(NSScrollView *sv, struct scrollViewData *d, BOOL hscroll, BOOL vscroll);
extern void scrollViewFreeData(NSScrollView *sv, struct scrollViewData *d);

// label.m
extern NSTextField *newLabel(NSString *str);

// image.m
extern NSImage *imageImage(uiImage *);

// winmoveresize.m
extern void doManualMove(NSWindow *w, NSEvent *initialEvent);
extern void doManualResize(NSWindow *w, NSEvent *initialEvent, uiWindowResizeEdge edge);

// future.m
extern CFStringRef *FUTURE_kCTFontOpenTypeFeatureTag;
extern CFStringRef *FUTURE_kCTFontOpenTypeFeatureValue;
extern CFStringRef *FUTURE_kCTBackgroundColorAttributeName;
extern void loadFutures(void);
extern void FUTURE_NSLayoutConstraint_setIdentifier(NSLayoutConstraint *constraint, NSString *identifier);
extern BOOL FUTURE_NSWindow_performWindowDragWithEvent(NSWindow *w, NSEvent *initialEvent);

// undocumented.m
extern CFStringRef UNDOC_kCTFontPreferredSubFamilyNameKey;
extern CFStringRef UNDOC_kCTFontPreferredFamilyNameKey;
extern void loadUndocumented(void);
