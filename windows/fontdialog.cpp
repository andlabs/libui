// 14 april 2016
#include "uipriv_windows.h"

struct fontDialog {
	HWND hwnd;
	HWND familyCombobox;
	HWND styleCombobox;
	HWND sizeCombobox;
	// TODO desc;
	fontCollection *fc;
	IDWriteFontFamily **families;
	UINT32 nFamilies;
	IDWriteGdiInterop *gdiInterop;
	RECT sampleRect;
};

static LRESULT cbAddString(HWND cb, WCHAR *str)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM) str);
	if (lr == (LRESULT) CB_ERR || lr == (LRESULT) CB_ERRSPACE)
		logLastError("error adding item to combobox in cbAddString()");
	return lr;
}

static LRESULT cbInsertStringAtTop(HWND cb, WCHAR *str)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_INSERTSTRING, 0, (LPARAM) str);
	if (lr == (LRESULT) CB_ERR || lr == (LRESULT) CB_ERRSPACE)
		logLastError("error inserting item to combobox in cbInsertStringAtTop()");
	return lr;
}

static void wipeStylesBox(struct fontDialog *f)
{
	IDWriteFont *font;
	LRESULT i, n;

	n = SendMessageW(f->styleCombobox, CB_GETCOUNT, 0, 0);
	if (n == (LRESULT) CB_ERR)
		logLastError("error getting combobox item count in wipeStylesBox()");
	for (i = 0; i < n; i++) {
		font = (IDWriteFont *) SendMessageW(f->styleCombobox, CB_GETITEMDATA, (WPARAM) i, 0);
		if (font == (IDWriteFont *) CB_ERR)
			logLastError("error getting font to release it in wipeStylesBox()");
		font->Release();
	}
	SendMessageW(f->styleCombobox, CB_RESETCONTENT, 0, 0);
}

static WCHAR *fontStyleName(struct fontDialog *f, IDWriteFont *font)
{
	IDWriteLocalizedStrings *str;
	BOOL exists;
	WCHAR *wstr;
	HRESULT hr;

	// first try this; if this is present, use it...
	hr = font->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_PREFERRED_SUBFAMILY_NAMES, &str, &exists);
	if (hr != S_OK)
		logHRESULT("error getting preferred subfamily string in fontStyleName()", hr);
	if (exists)
		goto good;

	// ...otherwise this font is good enough to be part of the main one on GDI as well, so try that name
	hr = font->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES, &str, &exists);
	if (hr != S_OK)
		logHRESULT("error getting Win32 subfamily string in fontStyleName()", hr);
	// TODO what if !exists?

good:
	wstr = fontCollectionCorrectString(f->fc, str);
	str->Release();
	return wstr;
}

static void familyChanged(struct fontDialog *f)
{
	LRESULT n;
	IDWriteFontList *specifics;
	IDWriteFont *specific;
	UINT32 i, ns;
	WCHAR *label;
	LRESULT pos;
	HRESULT hr;

	wipeStylesBox(f);

	n = SendMessageW(f->familyCombobox, CB_GETCURSEL, 0, 0);
	if (n == (LRESULT) CB_ERR)
		return;		// TODO restore previous selection

	// TODO figure out what the correct sort order is
	hr = f->families[n]->GetMatchingFonts(
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		&specifics);
	if (hr != S_OK)
		logHRESULT("error getting styles for font in familyChanged()", hr);

	// TODO test mutliple streteches; all the fonts I have have only one stretch value
	ns = specifics->GetFontCount();
	for (i = 0; i < ns; i++) {
		hr = specifics->GetFont(i, &specific);
		if (hr != S_OK)
			logHRESULT("error getting font for filling styles box in familyChanged()", hr);
		label = fontStyleName(f, specific);
		pos = cbAddString(f->styleCombobox, label);
		uiFree(label);
		if (SendMessageW(f->styleCombobox, CB_SETITEMDATA, (WPARAM) pos, (LPARAM) specific) == (LRESULT) CB_ERR)
			logLastError("error setting font data in styles box in familyChanged()");
	}

	specifics->Release();

	// TODO how do we preserve style selection? the real thing seems to have a very elaborate method of doing so
	// TODO check error
	SendMessageW(f->styleCombobox, CB_SETCURSEL, 0, 0);
	// TODO refine this a bit
	InvalidateRect(f->hwnd, NULL, TRUE/*TODO*/);
}

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;
	UINT32 i;
	WCHAR *wname;
	LRESULT ten;
	HWND samplePlacement;
	HRESULT hr;

	f = uiNew(struct fontDialog);
	f->hwnd = hwnd;

	f->familyCombobox = GetDlgItem(f->hwnd, rcFontFamilyCombobox);
	if (f->familyCombobox == NULL)
		logLastError("error getting font family combobox handle in beginFontDialog()");
	f->styleCombobox = GetDlgItem(f->hwnd, rcFontStyleCombobox);
	if (f->styleCombobox == NULL)
		logLastError("error getting font style combobox handle in beginFontDialog()");
	f->sizeCombobox = GetDlgItem(f->hwnd, rcFontSizeCombobox);
	if (f->sizeCombobox == NULL)
		logLastError("error getting font size combobox handle in beginFontDialog()");

	f->fc = loadFontCollection();
	f->nFamilies = f->fc->fonts->GetFontFamilyCount();
	f->families = new IDWriteFontFamily *[f->nFamilies];
	for (i = 0; i < f->nFamilies; i++) {
		hr = f->fc->fonts->GetFontFamily(i, &(f->families[i]));
		if (hr != S_OK)
			logHRESULT("error getting font family in beginFontDialog()", hr);
		wname = fontCollectionFamilyName(f->fc, f->families[i]);
		cbAddString(f->familyCombobox, wname);
		uiFree(wname);
	}

	// TODO all comboboxes should select on type; these already scroll on type but not select

	// TODO behavior for the real thing:
	// - if prior size is in list, select and scroll to it
	// - if not, select nothing and don't scroll list at all (keep at top)
	// we do 8 and 9 later
	ten = cbAddString(f->sizeCombobox, L"10");
	cbAddString(f->sizeCombobox, L"11");
	cbAddString(f->sizeCombobox, L"12");
	cbAddString(f->sizeCombobox, L"14");
	cbAddString(f->sizeCombobox, L"16");
	cbAddString(f->sizeCombobox, L"18");
	cbAddString(f->sizeCombobox, L"20");
	cbAddString(f->sizeCombobox, L"22");
	cbAddString(f->sizeCombobox, L"24");
	cbAddString(f->sizeCombobox, L"26");
	cbAddString(f->sizeCombobox, L"28");
	cbAddString(f->sizeCombobox, L"36");
	cbAddString(f->sizeCombobox, L"48");
	cbAddString(f->sizeCombobox, L"72");
	if (SendMessageW(f->sizeCombobox, CB_SETCURSEL, (WPARAM) ten, 0) != ten)
		logLastError("error selecting 10 in the size combobox in beginFontDialog()");
	// if we just use CB_ADDSTRING 8 and 9 will appear at the bottom of the list due to lexicographical sorting
	// if we use CB_INSERTSTRING instead it won't
	cbInsertStringAtTop(f->sizeCombobox, L"9");
	cbInsertStringAtTop(f->sizeCombobox, L"8");
	// 10 moved because of the above; figure out where it is now
	// we selected it earlier; getting the selection is easiest
	ten = SendMessageW(f->sizeCombobox, CB_GETCURSEL, 0, 0);
	// and finally put 10 at the top to imitate ChooseFont()
	if (SendMessageW(f->sizeCombobox, CB_SETTOPINDEX, (WPARAM) ten, 0) != 0)
		logLastError("error making 10 visible in the size combobox in beginFontDialog()");

	// note: we can't add ES_NUMBER to the combobox entry (it seems to disable the entry instead?!), so we must do validation when the box is dmissed; TODO

	// TODO actually select Arial
	if (SendMessageW(f->familyCombobox, CB_SETCURSEL, (WPARAM) 0, 0) != 0)
		logLastError("error selecting Arial in the family combobox in beginFontDialog()");
	familyChanged(f);

	hr = dwfactory->GetGdiInterop(&(f->gdiInterop));
	if (hr != S_OK)
		logHRESULT("error getting GDI interop for font dialog in beginFontDialog()", hr);

	samplePlacement = GetDlgItem(f->hwnd, rcFontSamplePlacement);
	if (samplePlacement == NULL)
		logLastError("error getting sample placement static control handle in beginFontDialog()");
	if (GetWindowRect(samplePlacement, &(f->sampleRect)) == 0)
		logLastError("error getting sample placement in beginFontDialog()");
	mapWindowRect(NULL, f->hwnd, &(f->sampleRect));
	if (DestroyWindow(samplePlacement) == 0)
		logLastError("error getting rid of the sample placement static control in beginFontDialog()");

	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
	UINT32 i;

	f->gdiInterop->Release();
	wipeStylesBox(f);
	for (i = 0; i < f->nFamilies; i++)
		f->families[i]->Release();
	delete[] f->families;
	fontCollectionFree(f->fc);
	if (EndDialog(f->hwnd, code) == 0)
		logLastError("error ending font dialog in endFontDialog()");
	uiFree(f);
}

static INT_PTR tryFinishDialog(struct fontDialog *f, WPARAM wParam)
{
	// cancelling
	if (LOWORD(wParam) != IDOK) {
		endFontDialog(f, 1);
		return TRUE;
	}

	// TODO

	endFontDialog(f, 2);
	return TRUE;
}

class gdiRenderer : public IDWriteTextRenderer {
public:
	ULONG refcount;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IDWritePixelSnapping
	STDMETHODIMP GetCurrentTransform(void *clientDrawingContext, DWRITE_MATRIX *transform);
	STDMETHODIMP GetPixelsPerDip(void *clientDrawingContext, FLOAT *pixelsPerDip);
	STDMETHODIMP IsPixelSnappingDisabled(void *clientDrawingContext, BOOL *isDisabled);

	// IDWriteTextRenderer
	STDMETHODIMP DrawGlyphRun(
		void *clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_MEASURING_MODE measuringMode,
		const DWRITE_GLYPH_RUN *glyphRun,
		const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription,
		IUnknown *clientDrawingEffect);
	STDMETHODIMP DrawInlineObject(void *clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject *inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown *clientDrawingEffect);
	STDMETHODIMP DrawStrikethrough(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_STRIKETHROUGH *strikethrough, IUnknown *clientDrawingEffect);
	STDMETHODIMP DrawUnderline(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_UNDERLINE *underline, IUnknown *clientDrawingEffect);
};

STDMETHODIMP gdiRenderer::QueryInterface(REFIID riid, void **ppv)
{
	if (ppv == NULL)
		return E_POINTER;
	if (riid == IID_IUnknown ||
		riid == __uuidof (IDWritePixelSnapping) ||
		riid == __uuidof (IDWriteTextRenderer)) {
		*ppv = static_cast<IDWriteTextRenderer *>(this);
		this->AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) gdiRenderer::AddRef()
{
	this->refcount++;
	return this->refcount;
}

STDMETHODIMP_(ULONG) gdiRenderer::Release()
{
	this->refcount--;
	if (this->refcount == 0) {
		delete this;
		return 0;
	}
	return this->refcount;
}

STDMETHODIMP gdiRenderer::GetCurrentTransform(void *clientDrawingContext, DWRITE_MATRIX *transform)
{
	IDWriteBitmapRenderTarget *target = (IDWriteBitmapRenderTarget *) clientDrawingContext;

	return target->GetCurrentTransform(transform);
}

STDMETHODIMP gdiRenderer::GetPixelsPerDip(void *clientDrawingContext, FLOAT *pixelsPerDip)
{
	IDWriteBitmapRenderTarget *target = (IDWriteBitmapRenderTarget *) clientDrawingContext;

	if (pixelsPerDip == NULL)
		return E_POINTER;
	*pixelsPerDip = target->GetPixelsPerDip();
	return S_OK;
}

STDMETHODIMP gdiRenderer::IsPixelSnappingDisabled(void *clientDrawingContext, BOOL *isDisabled)
{
	// TODO this is the MSDN recommendation
	if (isDisabled == NULL)
		return E_POINTER;
	*isDisabled = FALSE;
	return S_OK;
}

STDMETHODIMP gdiRenderer::DrawGlyphRun(
	void *clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	const DWRITE_GLYPH_RUN *glyphRun,
	const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription,
	IUnknown *clientDrawingEffect)
{
	IDWriteBitmapRenderTarget *target = (IDWriteBitmapRenderTarget *) clientDrawingContext;
	RECT dirtyRect;
	IDWriteRenderingParams *rp;
	HRESULT hr;

	// TODO I cannot believe this is required; we really do need to switch to Direct2D
	hr = dwfactory->CreateRenderingParams(&rp);
	if (hr != S_OK)
		return hr;
	hr = target->DrawGlyphRun(
		baselineOriginX,
		baselineOriginY,
		measuringMode,
		glyphRun,
		rp,
		RGB(0, 0, 0),
		&dirtyRect);
	rp->Release();
	if (hr != S_OK)
		return hr;
	if (SetBoundsRect(target->GetMemoryDC(), &dirtyRect, DCB_ACCUMULATE) == 0)
		// TODO
		return E_FAIL;
	return S_OK;
}

STDMETHODIMP gdiRenderer::DrawInlineObject(void *clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject *inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown *clientDrawingEffect)
{
	return E_NOTIMPL;
}

STDMETHODIMP gdiRenderer::DrawStrikethrough(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_STRIKETHROUGH *strikethrough, IUnknown *clientDrawingEffect)
{
	return E_NOTIMPL;
}

STDMETHODIMP gdiRenderer::DrawUnderline(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_UNDERLINE *underline, IUnknown *clientDrawingEffect)
{
	return E_NOTIMPL;
}

// TODO rename this function
// TODO consider using Direct2D instead
static void doPaint(struct fontDialog *f)
{
	PAINTSTRUCT ps;
	HDC dc;
	IDWriteBitmapRenderTarget *target;
	gdiRenderer *renderer;
	LRESULT i;
	IDWriteFont *font;
	IDWriteLocalizedStrings *sampleStrings;
	BOOL exists;
	WCHAR *sample;
	WCHAR *family;
	WCHAR *wsize;
	double size;
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	HDC memoryDC;
	RECT memoryRect;
	HRESULT hr;

	dc = BeginPaint(f->hwnd, &ps);
	if (dc == NULL)
		logLastError("error beginning font dialog redraw in doPaint()");

	hr = f->gdiInterop->CreateBitmapRenderTarget(dc,
		f->sampleRect.right - f->sampleRect.left, f->sampleRect.bottom - f->sampleRect.top,
		&target);
	if (hr != S_OK)
		logHRESULT("error creating bitmap render target for font dialog in doPaint()", hr);

	// TODO why is this needed?
	// TODO error check
	{
		RECT rdraw;

		rdraw.left = 0;
		rdraw.top = 0;
		rdraw.right = f->sampleRect.right - f->sampleRect.left;
		rdraw.bottom = f->sampleRect.bottom - f->sampleRect.top;
		FillRect(target->GetMemoryDC(), &rdraw, GetSysColorBrush(COLOR_BTNFACE));
	}

	renderer = new gdiRenderer;
	renderer->refcount = 1;

	i = SendMessageW(f->styleCombobox, CB_GETCURSEL, 0, 0);
	if (i == (LRESULT) CB_ERR)
		{EndPaint(f->hwnd,&ps);return;}		// TODO something more appropriate
	font = (IDWriteFont *) SendMessageW(f->styleCombobox, CB_GETITEMDATA, (WPARAM) i, 0);
	if (font == (IDWriteFont *) CB_ERR)
		logLastError("error getting font to draw font dialog sample in doPaint()");
	// TOOD allow for a fallback
	hr = font->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_SAMPLE_TEXT, &sampleStrings, &exists);
	if (hr != S_OK)
		exists = FALSE;
	if (exists) {
		sample = fontCollectionCorrectString(f->fc, sampleStrings);
		sampleStrings->Release();
	} else
		sample = L"TODO get this from GTK+ instead of AaBbYyZz";
	// TODO get this from the currently selected item
	family = windowText(f->familyCombobox);
	// TODO but NOT this
	wsize = windowText(f->sizeCombobox);
	// TODO error check?
	size = _wtof(wsize);
	uiFree(wsize);

	hr = dwfactory->CreateTextFormat(family,
		NULL,
		font->GetWeight(),
		font->GetStyle(),
		font->GetStretch(),
		// typographic points are 1/72 inch; this parameter is 1/96 inch
		// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
		size * (96.0 / 72.0),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale again?
		L"",
		&format);
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextFormat for font dialog sample in doPaint()", hr);
	uiFree(family);

	hr = dwfactory->CreateTextLayout(sample, wcslen(sample),
		format,
		// FLOAT is float, not double, so this should work... TODO
		// TODO we don't want wrapping here
		FLT_MAX, FLT_MAX,
		&layout);
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextLayout for font dialog sample in doPaint()", hr);

	hr = layout->Draw(target,
		renderer,
		0, 0);
	if (hr != S_OK)
		logHRESULT("error drawing font dialog sample text in doPaint()", hr);

	memoryDC = target->GetMemoryDC();
	if (GetBoundsRect(memoryDC, &memoryRect, 0) == 0)
		logLastError("error getting size of memory DC for font dialog in doPaint()");
	if (BitBlt(dc,
		f->sampleRect.left, f->sampleRect.top,
		memoryRect.right - memoryRect.left, memoryRect.bottom - memoryRect.top,
		memoryDC,
		0, 0,
		SRCCOPY | NOMIRRORBITMAP) == 0)
		logLastError("error blitting sample text to font dialog in doPaint()");

	layout->Release();
	format->Release();
	if (exists)
		uiFree(sample);
	renderer->Release();
	target->Release();
	EndPaint(f->hwnd, &ps);
}

static INT_PTR CALLBACK fontDialogDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct fontDialog *f;

	f = (struct fontDialog *) GetWindowLongPtrW(hwnd, DWLP_USER);
	if (f == NULL) {
		if (uMsg == WM_INITDIALOG) {
			f = beginFontDialog(hwnd, lParam);
			SetWindowLongPtrW(hwnd, DWLP_USER, (LONG_PTR) f);
			return TRUE;
		}
		return FALSE;
	}

	switch (uMsg) {
	case WM_COMMAND:
		SetWindowLongPtrW(f->hwnd, DWLP_MSGRESULT, 0);		// just in case
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			if (HIWORD(wParam) != BN_CLICKED)
				return FALSE;
			return tryFinishDialog(f, wParam);
		case rcFontFamilyCombobox:
			if (HIWORD(wParam) != CBN_SELCHANGE)
				return FALSE;
			familyChanged(f);
			return TRUE;
		// TODO
		case rcFontStyleCombobox:
		case rcFontSizeCombobox:
			if (HIWORD(wParam) != CBN_SELCHANGE)
				return FALSE;
			// TODO error check; refine
			InvalidateRect(f->hwnd, NULL, TRUE);
			return TRUE;
		}
		return FALSE;
	case WM_PAINT:
		doPaint(f);
		SetWindowLongPtrW(f->hwnd, DWLP_MSGRESULT, 0);
		return TRUE;
	}
	return FALSE;
}

void showFontDialog(HWND parent)
{
	switch (DialogBoxParamW(hInstance, MAKEINTRESOURCE(rcFontDialog), parent, fontDialogDlgProc, (LPARAM) NULL)) {
	case 1:
		// TODO cancel
		break;
	case 2:
		// TODO OK
		break;
	default:
		logLastError("error running font dialog in showFontDialog()");
	}
}
