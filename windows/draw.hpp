// 5 may 2016

// draw.cpp
extern ID2D1Factory *d2dfactory;
struct uiDrawContext {
	ID2D1RenderTarget *rt;
	// TODO find out how this works
	std::vector<struct drawState> *states;
	ID2D1PathGeometry *currentClip;
};

// drawpath.cpp
extern ID2D1PathGeometry *pathGeometry(uiDrawPath *p);

// drawmatrix.cpp
extern void m2d(uiDrawMatrix *m, D2D1_MATRIX_3X2_F *d);

// attrstr.cpp
extern void attrstrToIDWriteTextLayoutAttrs(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout/*TODO, GPtrArray **backgroundClosures*/);

// drawtext.cpp
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

};
// TODO these should not be exported
extern std::map<uiDrawTextItalic, DWRITE_FONT_STYLE> dwriteItalics;
extern std::map<uiDrawTextStretch, DWRITE_FONT_STRETCH> dwriteStretches;
