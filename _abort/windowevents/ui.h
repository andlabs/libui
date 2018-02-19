// uiWindowSetTitle
_UI_EXTERN void uiWindowPosition(uiWindow *w, int *x, int *y);
_UI_EXTERN void uiWindowSetPosition(uiWindow *w, int x, int y);
_UI_EXTERN void uiWindowCenter(uiWindow *w);
_UI_EXTERN void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data);
// uiWindowContentSize
