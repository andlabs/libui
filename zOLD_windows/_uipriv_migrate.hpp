
// menu.c
extern HMENU makeMenubar(void);
extern const uiMenuItem *menuIDToItem(UINT_PTR);
extern void runMenuEvent(WORD, uiWindow *);
extern void freeMenubar(HMENU);
extern void uninitMenus(void);

// draw.c
extern HRESULT initDraw(void);
extern void uninitDraw(void);
extern ID2D1HwndRenderTarget *makeHWNDRenderTarget(HWND hwnd);
extern uiDrawContext *newContext(ID2D1RenderTarget *);
extern void freeContext(uiDrawContext *);
