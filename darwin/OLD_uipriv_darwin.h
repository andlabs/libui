
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
