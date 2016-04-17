// 14 april 2016
#include "uipriv_windows.h"

struct fontDialog {
	HWND hwnd;
	HWND familyCombobox;
	HWND styleCombobox;
	HWND sizeCombobox;

	// TODO desc;

	fontCollection *fc;

	IDWriteGdiInterop *gdiInterop;
	RECT sampleRect;
	HWND sampleBox;

	// we store the current selections in case an invalid string is typed in (partial or nonexistent or invalid number)
	// on OK, these are what are read
	LRESULT curFamily;
	LRESULT curStyle;
	LRESULT curSize;
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
	if (SendMessageW(cb, CB_SETCURSEL, item, 0) != 0)
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

static void wipeStylesBox(struct fontDialog *f)
{
	cbWipeAndReleaseData(f->styleCombobox);
}

static WCHAR *fontStyleName(struct fontDialog *f, IDWriteFont *font)
{
	IDWriteLocalizedStrings *str;
	BOOL exists;
	WCHAR *wstr;
	HRESULT hr;

	hr = font->GetFaceNames(&str);
	if (hr != S_OK)
		logHRESULT("error getting font style name for font dialog in fontStyleName()", hr);
	wstr = fontCollectionCorrectString(f->fc, str);
	str->Release();
	return wstr;
}

static void familyChanged(struct fontDialog *f)
{
	LRESULT pos;
	BOOL selected;
	IDWriteFontFamily *family;
	IDWriteFont *font;
	UINT32 i, n;
	WCHAR *label;
	HRESULT hr;

	selected = cbGetCurSel(f->familyCombobox, &pos);
	if (!selected)		// on deselect, do nothing
		return;
	f->curFamily = pos;

	family = (IDWriteFontFamily *) cbGetItemData(f->familyCombobox, (WPARAM) (f->curFamily));

	// TODO test mutliple streteches; all the fonts I have have only one stretch value?
	wipeStylesBox(f);
	n = family->GetFontCount();
	for (i = 0; i < n; i++) {
		hr = family->GetFont(i, &font);
		if (hr != S_OK)
			logHRESULT("error getting font for filling styles box in familyChanged()", hr);
		label = fontStyleName(f, font);
		pos = cbAddString(f->styleCombobox, label);
		uiFree(label);
		cbSetItemData(f->styleCombobox, (WPARAM) pos, (LPARAM) font);
	}

	// TODO how do we preserve style selection? the real thing seems to have a very elaborate method of doing so
	// TODO check error
	SendMessageW(f->styleCombobox, CB_SETCURSEL, 0, 0);
	f->curStyle = 0;
	// TODO refine this a bit
	InvalidateRect(f->sampleBox, NULL, TRUE/*TODO*/);
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
	WCHAR *family, *wsize;
	double size;
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

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;
	UINT32 i, nFamilies;
	IDWriteFontFamily *family;
	WCHAR *wname;
	LRESULT pos, ten;
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
	cbSetCurSel(f->familyCombobox, 0);
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
	f->sampleBox = newD2DScratch(f->hwnd, &(f->sampleRect), (HMENU) rcFontSamplePlacement, fontDialogSampleSubProc, (DWORD_PTR) f);

	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
	f->gdiInterop->Release();
	wipeStylesBox(f);
	cbWipeAndReleaseData(f->familyCombobox);
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
			// TODO really do the job
			cbGetCurSel(f->styleCombobox, &(f->curStyle));
			// TODO error check; refine
			InvalidateRect(f->sampleBox, NULL, TRUE);
			return TRUE;
		}
		return FALSE;
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
