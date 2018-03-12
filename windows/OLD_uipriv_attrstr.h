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

// attrstr.cpp
typedef std::function<void(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y)> backgroundFunc;
extern void attrstrToIDWriteTextLayoutAttrs(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout, std::vector<backgroundFunc> **backgroundFuncs);

// drawtext.cpp
// TODO reconcile this with attrstr.cpp
class textDrawingEffect : public IUnknown {
	ULONG refcount;
public:
	bool hasColor;
	double r;
	double g;
	double b;
	double a;

	bool hasUnderline;
	uiDrawUnderlineStyle u;

	bool hasUnderlineColor;
	double ur;
	double ug;
	double ub;
	double ua;

	textDrawingEffect()
	{
		this->refcount = 1;
		this->hasColor = false;
		this->hasUnderline = false;
		this->hasUnderlineColor = false;
	}

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (ppvObject == NULL)
			return E_POINTER;
		if (riid == IID_IUnknown) {
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		this->refcount++;
		return this->refcount;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		this->refcount--;
		if (this->refcount == 0) {
			delete this;
			return 0;
		}
		return this->refcount;
	}

	// TODO deduplicate this with common/attrlist.c
	bool same(textDrawingEffect *b)
	{
		static auto boolsDiffer = [](bool a, bool b) -> bool {
			if (a && b)
				return false;
			if (!a && !b)
				return false;
			return true;
		};

		if (boolsDiffer(this->hasColor, b->hasColor))
			return false;
		if (this->hasColor) {
			// TODO use a closest match?
			if (this->r != b->r)
				return false;
			if (this->g != b->g)
				return false;
			if (this->b != b->b)
				return false;
			if (this->a != b->a)
				return false;
		}
		if (boolsDiffer(this->hasUnderline, b->hasUnderline))
			return false;
		if (this->hasUnderline)
			if (this->u != b->u)
				return false;
		if (boolsDiffer(this->hasUnderlineColor, b->hasUnderlineColor))
			return false;
		if (this->hasUnderlineColor) {
			// TODO use a closest match?
			if (this->ur != b->ur)
				return false;
			if (this->ug != b->ug)
				return false;
			if (this->ub != b->ub)
				return false;
			if (this->ua != b->ua)
				return false;
		}
		return true;
	}
};
