// 14 april 2016
#include "uipriv_windows.h"

// TODOs
// - quote the Choose Font sample here for reference
// - the Choose Font sample defaults to Regular/Italic/Bold/Bold Italic in some case (no styles?); do we? find out what the case is
// - do we set initial family and style topmost as well?

struct fontDialog {
	HWND hwnd;
	HWND familyCombobox;
	HWND styleCombobox;
	HWND sizeCombobox;

	uiDrawTextFontDescriptor *desc;

	fontCollection *fc;

	RECT sampleRect;
	HWND sampleBox;

	// we store the current selections in case an invalid string is typed in (partial or nonexistent or invalid number)
	// on OK, these are what are read
	LRESULT curFamily;
	LRESULT curStyle;
	double curSize;

	// these are finding the style that's closest to the previous one (these fields) when changing a font
	DWRITE_FONT_WEIGHT weight;
	DWRITE_FONT_STYLE style;
	DWRITE_FONT_STRETCH stretch;
};

static LRESULT cbAddString(HWND cb, const WCHAR *str)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM) str);
	if (lr == (LRESULT) CB_ERR || lr == (LRESULT) CB_ERRSPACE)
		logLastError("error adding item to combobox in cbAddString()");
	return lr;
}

static LRESULT cbInsertString(HWND cb, const WCHAR *str, WPARAM pos)
{
	LRESULT lr;

	lr = SendMessageW(cb, CB_INSERTSTRING, pos, (LPARAM) str);
	if (lr != (LRESULT) pos)
		logLastError("error inserting item to combobox in cbInsertString()");
	return lr;
}

static LRESULT cbGetItemData(HWND cb, WPARAM item)
{
	LRESULT data;

	data = SendMessageW(cb, CB_GETITEMDATA, item, 0);
	if (data == (LRESULT) CB_ERR)
		logLastError("error getting combobox item data for font dialog in cbGetItemData()");
	return data;
}

static void cbSetItemData(HWND cb, WPARAM item, LPARAM data)
{
	if (SendMessageW(cb, CB_SETITEMDATA, item, data) == (LRESULT) CB_ERR)
		logLastError("error setting combobox item data in cbSetItemData()");
}

static BOOL cbGetCurSel(HWND cb, LRESULT *sel)
{
	LRESULT n;

	n = SendMessageW(cb, CB_GETCURSEL, 0, 0);
	if (n == (LRESULT) CB_ERR)
		return FALSE;
	if (sel != NULL)
		*sel = n;
	return TRUE;
}

static void cbSetCurSel(HWND cb, WPARAM item)
{
	if (SendMessageW(cb, CB_SETCURSEL, item, 0) != (LRESULT) item)
		logLastError("error selecting combobox item in cbSetCurSel()");
}

static LRESULT cbGetCount(HWND cb)
{
	LRESULT n;

	n = SendMessageW(cb, CB_GETCOUNT, 0, 0);
	if (n == (LRESULT) CB_ERR)
		logLastError("error getting combobox item count in cbGetCount()");
	return n;
}

static void cbWipeAndReleaseData(HWND cb)
{
	IUnknown *obj;
	LRESULT i, n;

	n = cbGetCount(cb);
	for (i = 0; i < n; i++) {
		obj = (IUnknown *) cbGetItemData(cb, (WPARAM) i);
		obj->Release();
	}
	SendMessageW(cb, CB_RESETCONTENT, 0, 0);
}

static WCHAR *cbGetItemText(HWND cb, WPARAM item)
{
	LRESULT len;
	WCHAR *text;

	// note: neither message includes the terminating L'\0'
	len = SendMessageW(cb, CB_GETLBTEXTLEN, item, 0);
	if (len == (LRESULT) CB_ERR)
		logLastError("error getting item text length from combobox in cbGetItemText()");
	text = (WCHAR *) uiAlloc((len + 1) * sizeof (WCHAR), "WCHAR[]");
	if (SendMessageW(cb, CB_GETLBTEXT, item, (LPARAM) text) != len)
		logLastError("error getting item text from combobox in cbGetItemText()");
	return text;
}

static BOOL cbTypeToSelect(HWND cb, LRESULT *posOut, BOOL restoreAfter)
{
	WCHAR *text;
	LRESULT pos;
	DWORD selStart, selEnd;

	// start by saving the current selection as setting the item will change the selection
	SendMessageW(cb, CB_GETEDITSEL, (WPARAM) (&selStart), (LPARAM) (&selEnd));
	text = windowText(cb);
	pos = SendMessageW(cb, CB_FINDSTRINGEXACT, (WPARAM) (-1), (LPARAM) text);
	if (pos == (LRESULT) CB_ERR) {
		uiFree(text);
		return FALSE;
	}
	cbSetCurSel(cb, (WPARAM) pos);
	if (posOut != NULL)
		*posOut = pos;
	if (restoreAfter)
		if (SendMessageW(cb, WM_SETTEXT, 0, (LPARAM) text) != (LRESULT) TRUE)
			logLastError("error restoring old combobox text in cbTypeToSelect()");
	uiFree(text);
	// and restore the selection like above
	// TODO isn't there a 32-bit version of this
	if (SendMessageW(cb, CB_SETEDITSEL, 0, MAKELPARAM(selStart, selEnd)) != (LRESULT) TRUE)
		logLastError("error restoring combobox edit selection in cbTypeToSelect()");
	return TRUE;
}

static void wipeStylesBox(struct fontDialog *f)
{
	cbWipeAndReleaseData(f->styleCombobox);
}

static WCHAR *fontStyleName(struct fontDialog *f, IDWriteFont *font)
{
	IDWriteLocalizedStrings *str;
	WCHAR *wstr;
	HRESULT hr;

	hr = font->GetFaceNames(&str);
	if (hr != S_OK)
		logHRESULT("error getting font style name for font dialog in fontStyleName()", hr);
	wstr = fontCollectionCorrectString(f->fc, str);
	str->Release();
	return wstr;
}

static void queueRedrawSampleText(struct fontDialog *f)
{
	// TODO TRUE?
	if (InvalidateRect(f->sampleBox, NULL, TRUE) == 0)
		logLastError("error queueing a redraw of the font dialog's sample text in queueRedrawSampleText()");
}

static void styleChanged(struct fontDialog *f)
{
	LRESULT pos;
	BOOL selected;
	IDWriteFont *font;

	selected = cbGetCurSel(f->styleCombobox, &pos);
	if (!selected)		// on deselect, do nothing
		return;
	f->curStyle = pos;

	font = (IDWriteFont *) cbGetItemData(f->styleCombobox, (WPARAM) (f->curStyle));
	// these are for the nearest match when changing the family; see below
	f->weight = font->GetWeight();
	f->style = font->GetStyle();
	f->stretch = font->GetStretch();

	queueRedrawSampleText(f);
}

static void styleEdited(struct fontDialog *f)
{
	if (cbTypeToSelect(f->styleCombobox, &(f->curStyle), FALSE))
		styleChanged(f);
}

static void familyChanged(struct fontDialog *f)
{
	LRESULT pos;
	BOOL selected;
	IDWriteFontFamily *family;
	IDWriteFont *font, *matchFont;
	DWRITE_FONT_WEIGHT weight;
	DWRITE_FONT_STYLE style;
	DWRITE_FONT_STRETCH stretch;
	UINT32 i, n;
	UINT32 matching;
	WCHAR *label;
	HRESULT hr;

	selected = cbGetCurSel(f->familyCombobox, &pos);
	if (!selected)		// on deselect, do nothing
		return;
	f->curFamily = pos;

	family = (IDWriteFontFamily *) cbGetItemData(f->familyCombobox, (WPARAM) (f->curFamily));

	// for the nearest style match
	// when we select a new family, we want the nearest style to the previously selected one to be chosen
	// this is how the Choose Font sample does it
	hr = family->GetFirstMatchingFont(
		f->weight,
		f->stretch,
		f->style,
		&matchFont);
	if (hr != S_OK)
		logHRESULT("error finding first matching font to previous style in font dialog in familyChanged()", hr);
	// we can't just compare pointers; a "newly created" object comes out
	// the Choose Font sample appears to do this instead
	weight = matchFont->GetWeight();
	style = matchFont->GetStyle();
	stretch = matchFont->GetStretch();
	matchFont->Release();

	// TODO test mutliple streteches; all the fonts I have have only one stretch value?
	wipeStylesBox(f);
	n = family->GetFontCount();
	matching = 0;			// a safe/suitable default just in case
	for (i = 0; i < n; i++) {
		hr = family->GetFont(i, &font);
		if (hr != S_OK)
			logHRESULT("error getting font for filling styles box in familyChanged()", hr);
		label = fontStyleName(f, font);
		pos = cbAddString(f->styleCombobox, label);
		uiFree(label);
		cbSetItemData(f->styleCombobox, (WPARAM) pos, (LPARAM) font);
		if (font->GetWeight() == weight &&
			font->GetStyle() == style &&
			font->GetStretch() == stretch)
			matching = i;
	}

	// and now, load the match
	cbSetCurSel(f->styleCombobox, (WPARAM) matching);
	styleChanged(f);
}

// TODO search language variants like the sample does
static void familyEdited(struct fontDialog *f)
{
	if (cbTypeToSelect(f->familyCombobox, &(f->curFamily), FALSE))
		familyChanged(f);
}

static const struct {
	const WCHAR *text;
	double value;
} defaultSizes[] = {
	{ L"8", 8 },
	{ L"9", 9 },
	{ L"10", 10 },
	{ L"11", 11 },
	{ L"12", 12 },
	{ L"14", 14 },
	{ L"16", 16 },
	{ L"18", 18 },
	{ L"20", 20 },
	{ L"22", 22 },
	{ L"24", 24 },
	{ L"26", 26 },
	{ L"28", 28 },
	{ L"36", 36 },
	{ L"48", 48 },
	{ L"72", 72 },
	{ NULL, 0 },
};

static void sizeChanged(struct fontDialog *f)
{
	LRESULT pos;
	BOOL selected;

	selected = cbGetCurSel(f->sizeCombobox, &pos);
	if (!selected)		// on deselect, do nothing
		return;
	f->curSize = defaultSizes[pos].value;
	queueRedrawSampleText(f);
}

static void sizeEdited(struct fontDialog *f)
{
	WCHAR *wsize;
	double size;

	// handle type-to-selection
	if (cbTypeToSelect(f->sizeCombobox, NULL, FALSE)) {
		sizeChanged(f);
		return;
	}
	// selection not chosen, try to parse the typing
	wsize = windowText(f->sizeCombobox);
	// this is what the Choose Font dialog does; it swallows errors while the real ChooseFont() is not lenient (and only checks on OK)
	size = wcstod(wsize, NULL);
	if (size <= 0)		// don't change on invalid size
		return;
	f->curSize = size;
	queueRedrawSampleText(f);
}

static void fontDialogDrawSampleText(struct fontDialog *f, ID2D1RenderTarget *rt)
{
	D2D1_COLOR_F color;
	D2D1_BRUSH_PROPERTIES props;
	ID2D1SolidColorBrush *black;
	IDWriteFont *font;
	IDWriteLocalizedStrings *sampleStrings;
	BOOL exists;
	WCHAR *sample;
	WCHAR *family;
	IDWriteTextFormat *format;
	D2D1_RECT_F rect;
	HRESULT hr;

	color.r = 0.0;
	color.g = 0.0;
	color.b = 0.0;
	color.a = 1.0;
	ZeroMemory(&props, sizeof (D2D1_BRUSH_PROPERTIES));
	props.opacity = 1.0;
	// identity matrix
	props.transform._11 = 1;
	props.transform._22 = 1;
	hr = rt->CreateSolidColorBrush(
		&color,
		&props,
		&black);
	if (hr != S_OK)
		logHRESULT("error creating solid brush in fontDialogDrawSampleText()", hr);

	font = (IDWriteFont *) cbGetItemData(f->styleCombobox, (WPARAM) f->curStyle);
	hr = font->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_SAMPLE_TEXT, &sampleStrings, &exists);
	if (hr != S_OK)
		exists = FALSE;
	if (exists) {
		sample = fontCollectionCorrectString(f->fc, sampleStrings);
		sampleStrings->Release();
	} else
		sample = L"The quick brown fox jumps over the lazy dog.";

	// DirectWrite doesn't allow creating a text format from a font; we need to get this ourselves
	family = cbGetItemText(f->familyCombobox, f->curFamily);
	hr = dwfactory->CreateTextFormat(family,
		NULL,
		font->GetWeight(),
		font->GetStyle(),
		font->GetStretch(),
		// typographic points are 1/72 inch; this parameter is 1/96 inch
		// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
		f->curSize * (96.0 / 72.0),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale again?
		L"",
		&format);
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextFormat in fontDialogDrawSampleText()", hr);
	uiFree(family);

	rect.left = 0;
	rect.top = 0;
	rect.right = rt->GetSize().width;
	rect.bottom = rt->GetSize().height;
	rt->DrawText(sample, wcslen(sample),
		format,
		&rect,
		black,
		// TODO really?
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL);

	format->Release();
	if (exists)
		uiFree(sample);
	black->Release();
}

static LRESULT CALLBACK fontDialogSampleSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ID2D1RenderTarget *rt;
	struct fontDialog *f;

	switch (uMsg) {
	case msgD2DScratchPaint:
		rt = (ID2D1RenderTarget *) lParam;
		f = (struct fontDialog *) dwRefData;
		fontDialogDrawSampleText(f, rt);
		return 0;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, fontDialogSampleSubProc, uIdSubclass) == FALSE)
			logLastError("error removing font dialog sample text subclass in fontDialogSampleSubProc()");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

static void setupInitialFontDialogState(struct fontDialog *f)
{
	WCHAR *wfamily;
	struct dwriteAttr attr;
	WCHAR wsize[512];		// this should be way more than enough
	LRESULT pos;

	// first convert f->desc into a usable form
	wfamily = toUTF16(f->desc->Family);
	// see below for why we do this specifically
	// TODO is 512 the correct number to pass to _snwprintf()?
	// TODO will this revert to scientific notation?
	_snwprintf(wsize, 512, L"%g", f->desc->Size);
	attr.weight = f->desc->Weight;
	attr.italic = f->desc->Italic;
	attr.stretch = f->desc->Stretch;
	attrToDWriteAttr(&attr);

	// first let's load the size
	// the real font dialog:
	// - if the chosen font size is in the list, it selects that item AND makes it topmost
	// - if the chosen font size is not in the list, don't bother
	// we'll simulate it by setting the text to a %f representation, then pretending as if it was entered
	// TODO make this a setWindowText()
	if (SendMessageW(f->sizeCombobox, WM_SETTEXT, 0, (LPARAM) wsize) != (LRESULT) TRUE)
		logLastError("error setting size combobox to initial font size in setupInitialFontDialogState()");
	sizeEdited(f);
	if (cbGetCurSel(f->sizeCombobox, &pos))
		if (SendMessageW(f->sizeCombobox, CB_SETTOPINDEX, (WPARAM) pos, 0) != 0)
			logLastError("error making chosen size topmost in the size combobox in setupInitialFontDialogState()");

	// now we set the family and style
	// we do this by first setting the previous style attributes, then simulating a font entered
	f->weight = attr.dweight;
	f->style = attr.ditalic;
	f->stretch = attr.dstretch;
	if (SendMessageW(f->familyCombobox, WM_SETTEXT, 0, (LPARAM) wfamily) != (LRESULT) TRUE)
		logLastError("error setting family combobox to initial font family in setupInitialFontDialogState()");
	familyEdited(f);
	uiFree(wfamily);
}

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;
	UINT32 i, nFamilies;
	IDWriteFontFamily *family;
	WCHAR *wname;
	LRESULT pos;
	HWND samplePlacement;
	HRESULT hr;

	f = uiNew(struct fontDialog);
	f->hwnd = hwnd;
	f->desc = (uiDrawTextFontDescriptor *) lParam;

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
	nFamilies = f->fc->fonts->GetFontFamilyCount();
	for (i = 0; i < nFamilies; i++) {
		hr = f->fc->fonts->GetFontFamily(i, &family);
		if (hr != S_OK)
			logHRESULT("error getting font family in beginFontDialog()", hr);
		wname = fontCollectionFamilyName(f->fc, family);
		pos = cbAddString(f->familyCombobox, wname);
		uiFree(wname);
		cbSetItemData(f->familyCombobox, (WPARAM) pos, (LPARAM) family);
	}

	for (i = 0; defaultSizes[i].text != NULL; i++)
		cbInsertString(f->sizeCombobox, defaultSizes[i].text, (WPARAM) i);

	samplePlacement = GetDlgItem(f->hwnd, rcFontSamplePlacement);
	if (samplePlacement == NULL)
		logLastError("error getting sample placement static control handle in beginFontDialog()");
	if (GetWindowRect(samplePlacement, &(f->sampleRect)) == 0)
		logLastError("error getting sample placement in beginFontDialog()");
	mapWindowRect(NULL, f->hwnd, &(f->sampleRect));
	if (DestroyWindow(samplePlacement) == 0)
		logLastError("error getting rid of the sample placement static control in beginFontDialog()");
	f->sampleBox = newD2DScratch(f->hwnd, &(f->sampleRect), (HMENU) rcFontSamplePlacement, fontDialogSampleSubProc, (DWORD_PTR) f);

	setupInitialFontDialogState(f);
	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
	wipeStylesBox(f);
	cbWipeAndReleaseData(f->familyCombobox);
	fontCollectionFree(f->fc);
	if (EndDialog(f->hwnd, code) == 0)
		logLastError("error ending font dialog in endFontDialog()");
	uiFree(f);
}

static INT_PTR tryFinishDialog(struct fontDialog *f, WPARAM wParam)
{
	WCHAR *wfamily;
	IDWriteFont *font;
	struct dwriteAttr attr;

	// cancelling
	if (LOWORD(wParam) != IDOK) {
		endFontDialog(f, 1);
		return TRUE;
	}

	// OK
	wfamily = cbGetItemText(f->familyCombobox, f->curFamily);
	f->desc->Family = toUTF8(wfamily);
	uiFree(wfamily);
	f->desc->Size = f->curSize;
	font = (IDWriteFont *) cbGetItemData(f->styleCombobox, f->curStyle);
	attr.dweight = font->GetWeight();
	attr.ditalic = font->GetStyle();
	attr.dstretch = font->GetStretch();
	dwriteAttrToAttr(&attr);
	f->desc->Weight = attr.weight;
	f->desc->Italic = attr.italic;
	f->desc->Stretch = attr.stretch;
	endFontDialog(f, 2);
	return TRUE;
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
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				familyChanged(f);
				return TRUE;
			}
			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				familyEdited(f);
				return TRUE;
			}
			return FALSE;
		case rcFontStyleCombobox:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				styleChanged(f);
				return TRUE;
			}
			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				styleEdited(f);
				return TRUE;
			}
			return FALSE;
		case rcFontSizeCombobox:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				sizeChanged(f);
				return TRUE;
			}
			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				sizeEdited(f);
				return TRUE;
			}
			return FALSE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL showFontDialog(HWND parent, uiDrawTextFontDescriptor *desc)
{
	switch (DialogBoxParamW(hInstance, MAKEINTRESOURCE(rcFontDialog), parent, fontDialogDlgProc, (LPARAM) desc)) {
	case 1:			// cancel
		return FALSE;
	case 2:			// ok
		// make the compiler happy by putting the return after the switch
		break;
	default:
		logLastError("error running font dialog in showFontDialog()");
	}
	return TRUE;
}
