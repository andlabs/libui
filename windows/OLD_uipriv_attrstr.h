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
textDrawingEffect:textDrawingEffect(void)
{
	this->refcount = 1;
	this->hasColor = false;
	this->hasUnderline = false;
	this->hasUnderlineColor = false;
}

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
};
