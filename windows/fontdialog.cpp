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

	// TODO do we preserve style selection?
}

static struct fontDialog *beginFontDialog(HWND hwnd, LPARAM lParam)
{
	struct fontDialog *f;
	UINT32 i;
	WCHAR *wname;
	LRESULT ten;
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

	return f;
}

static void endFontDialog(struct fontDialog *f, INT_PTR code)
{
	UINT32 i;

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
