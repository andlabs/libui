// 5 may 2016

// TODO resolve overlap between this and the other hpp files (some functions leaked into uipriv_windows.hpp)

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
// TODO these should not be exported
extern std::map<uiDrawTextItalic, DWRITE_FONT_STYLE> dwriteItalics;
extern std::map<uiDrawTextStretch, DWRITE_FONT_STRETCH> dwriteStretches;
