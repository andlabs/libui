
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

// dwrite.cpp
extern IDWriteFactory *dwfactory;
extern HRESULT initDrawText(void);
extern void uninitDrawText(void);
struct fontCollection {
	IDWriteFontCollection *fonts;
	WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
	int userLocaleSuccess;
};
extern fontCollection *loadFontCollection(void);
extern WCHAR *fontCollectionFamilyName(fontCollection *fc, IDWriteFontFamily *family);
extern void fontCollectionFree(fontCollection *fc);
extern WCHAR *fontCollectionCorrectString(fontCollection *fc, IDWriteLocalizedStrings *names);

// fontdialog.cpp
struct fontDialogParams {
	IDWriteFont *font;
	double size;
	WCHAR *familyName;
	WCHAR *styleName;
};
extern BOOL showFontDialog(HWND parent, struct fontDialogParams *params);
extern void loadInitialFontDialogParams(struct fontDialogParams *params);
extern void destroyFontDialogParams(struct fontDialogParams *params);
extern WCHAR *fontDialogParamsToString(struct fontDialogParams *params);
