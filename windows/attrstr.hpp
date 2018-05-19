// 11 march 2018
#include "../common/attrstr.h"

// dwrite.cpp
extern IDWriteFactory *dwfactory;
extern HRESULT uiprivInitDrawText(void);
extern void uiprivUninitDrawText(void);
struct fontCollection {
	IDWriteFontCollection *fonts;
	WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
	int userLocaleSuccess;
};
extern fontCollection *uiprivLoadFontCollection(void);
extern WCHAR *uiprivFontCollectionFamilyName(fontCollection *fc, IDWriteFontFamily *family);
extern void uiprivFontCollectionFree(fontCollection *fc);
extern WCHAR *uiprivFontCollectionCorrectString(fontCollection *fc, IDWriteLocalizedStrings *names);

// opentype.cpp
extern IDWriteTypography *uiprivOpenTypeFeaturesToIDWriteTypography(const uiOpenTypeFeatures *otf);

// fontmatch.cpp
extern DWRITE_FONT_WEIGHT uiprivWeightToDWriteWeight(uiTextWeight w);
extern DWRITE_FONT_STYLE uiprivItalicToDWriteStyle(uiTextItalic i);
extern DWRITE_FONT_STRETCH uiprivStretchToDWriteStretch(uiTextStretch s);
extern void uiprivFontDescriptorFromIDWriteFont(IDWriteFont *font, uiFontDescriptor *uidesc);

// attrstr.cpp
// TODO
struct drawTextBackgroundParams;
extern void uiprivAttributedStringApplyAttributesToDWriteTextLayout(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout, std::vector<struct drawTextBackgroundParams *> **backgroundFuncs);

// drawtext.cpp
class drawingEffectsAttr : public IUnknown {
	ULONG refcount;

	bool hasColor;
	double r;
	double g;
	double b;
	double a;

	bool hasUnderline;
	uiUnderline u;

	bool hasUnderlineColor;
	double ur;
	double ug;
	double ub;
	double ua;
public:
	drawingEffectsAttr(void);

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	void setColor(double r, double g, double b, double a);
	void setUnderline(uiUnderline u);
	void setUnderlineColor(double r, double g, double b, double a);
	HRESULT mkColorBrush(ID2D1RenderTarget *rt, ID2D1SolidColorBrush **b);
	HRESULT underline(uiUnderline *u);
	HRESULT mkUnderlineBrush(ID2D1RenderTarget *rt, ID2D1SolidColorBrush **b);
};
// TODO figure out where this type should *really* go in all the headers...
struct drawTextBackgroundParams {
	size_t start;
	size_t end;
	double r;
	double g;
	double b;
	double a;
};

// fontdialog.cpp
struct fontDialogParams {
	IDWriteFont *font;
	double size;
	WCHAR *familyName;
	WCHAR *styleName;
};
extern BOOL uiprivShowFontDialog(HWND parent, struct fontDialogParams *params);
extern void uiprivLoadInitialFontDialogParams(struct fontDialogParams *params);
extern void uiprivDestroyFontDialogParams(struct fontDialogParams *params);
extern WCHAR *uiprivFontDialogParamsToString(struct fontDialogParams *params);
